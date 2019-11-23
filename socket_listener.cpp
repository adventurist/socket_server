// Project headers
#include "headers/socket_listener.h"

#include "headers/constants.h"
// System libraries
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
// C++ Libraries
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <functional>
#include <iostream>
#include <memory>
#include <queue>
#include <string>
#include <thread>
#include <vector>

int num_threads = std::thread::hardware_concurrency();

/**
 * Constructor
 * Initialize with ip_address, port and message_handler
 */
SocketListener::SocketListener(std::string ip_address, int port)
    : m_ip_address(ip_address),
      m_port(port),
      accepting_tasks(true),
      shutdown_loop(false) {}

/**
 * Destructor
 * TODO: Determine if we should make buffer a class member
 */
SocketListener::~SocketListener() { cleanup(); }

SocketListener::MessageHandler SocketListener::createMessageHandler(
    std::function<void()> cb) {
  return MessageHandler(cb);
}

/**
 * sendMessage
 * @method
 * Send a null-terminated array of characters, supplied as a const char
 * pointer, to a client socket described by its file descriptor
 */
void SocketListener::sendMessage(int client_socket_fd,
                                 std::shared_ptr<char[]> s_ptr) {
  send(client_socket_fd, s_ptr.get(), static_cast<size_t>(MAX_BUFFER_SIZE) + 1,
       0);
}

/**
 * init
 * TODO: Initialize buffer memory, if buffer is to be a class member
 */
bool SocketListener::init() {
  std::cout << "Initializing socket listener" << std::endl;
  return true;
}

void SocketListener::push_to_queue(std::function<void()> fn) {
  std::unique_lock<std::mutex> lock(m_mutex_lock);
  task_queue.push(fn);
  lock.unlock();
  pool_condition.notify_one();
}

void SocketListener::handle_loop() {
  std::string accepting_str = accepting_tasks == 0
                                  ? std::string("Not accepting tasks")
                                  : std::string("Accepting tasks");
  std::cout << accepting_str << std::endl;
  std::function<void()> fn;
  for (;;) {
    {
      std::unique_lock<std::mutex> lock(m_mutex_lock);
      pool_condition.wait(
          lock, [this]() { return !accepting_tasks || !task_queue.empty(); });
      std::cout << "Wait condition met" << std::endl;
      if (!accepting_tasks && task_queue.empty()) {
        return;  // we are done here
      }
      std::cout << "Taking task" << std::endl;
      fn = task_queue.front();
      task_queue.pop();
    }
    fn();
  }
}

void SocketListener::loop_check() {
    for (int i = 0; i < task_queue.size() && i < (num_threads - 1); i++) {
      thread_pool.push_back(std::thread([this]() { handle_loop(); }));
    }
    done();
    std::this_thread::sleep_for(std::chrono::milliseconds(400));
    detachThreads();
    size_t task_num = task_queue.size();
    std::cout << "Task num: " << task_num << std::endl;
    accepting_tasks = true;
}

void SocketListener::done() {
  std::unique_lock<std::mutex> lock(m_mutex_lock);
  accepting_tasks = false;
  lock.unlock();
  // when we send the notification immediately, the consumer will try to get the
  // lock , so unlock asap
  pool_condition.notify_all();
}

void SocketListener::handle_client_socket(
    int client_socket_fd, SocketListener::MessageHandler message_handler,
    std::shared_ptr<char[]> buf) {
  while (true) {
    memset(buf.get(), 0, MAX_BUFFER_SIZE);  // Zero the character buffer
    int bytes_received = 0;
    // Receive and write incoming data to buffer and return the number of
    // bytes received
    bytes_received =
        recv(client_socket_fd, buf.get(),
             MAX_BUFFER_SIZE - 2,  // Leave room for null-termination
             0);
    buf.get()[MAX_BUFFER_SIZE - 1] = 0;  // Null-terminate the character buffer
    if (bytes_received > 0) {
      std::cout << "Client " << client_socket_fd
                << "\nBytes received: " << bytes_received
                << "\nData: " << buf.get() << std::endl;
      // Handle incoming message
      message_handler();
    } else {
      std::cout << "Client " << client_socket_fd << " disconnected" << std::endl;
      break;
    }
  }
  // Zero the buffer again before closing
  memset(buf.get(), 0, MAX_BUFFER_SIZE);
  // TODO: Determine if we should free memory, or handle as class member
  close(client_socket_fd);  // Destroy client socket and deallocate its fd
}

/**
 * run
 * @method
 * Main message loop
 * TODO: Implement multithreading
 */
void SocketListener::run() {
  // Begin listening loop
  while (true) {
    std::cout << "Begin" << std::endl;
    // Call system to open a listening socket, and return its file descriptor
    int listening_socket_fd = createSocket();

    if (listening_socket_fd == SOCKET_ERROR) {
      std::cout << "Socket error: shutting down server" << std::endl;
      break;
    }
    std::cout << "Attempting to wait for connection" << std::endl;
    // wait for a client connection and get its socket file descriptor
    int client_socket_fd = waitForConnection(listening_socket_fd);

    if (client_socket_fd != SOCKET_ERROR) {
      // Destroy listening socket and deallocate its file descriptor. Only use
      // the client socket now.
      close(listening_socket_fd);
      std::shared_ptr<char[]> s_ptr(new char[MAX_BUFFER_SIZE]);
      std::function<void()> message_send_fn = [this, client_socket_fd,
                                               s_ptr]() {
        this->sendMessage(client_socket_fd, s_ptr);
      };
      MessageHandler message_handler = createMessageHandler(message_send_fn);
      std::cout << "Pushing client to queue" << std::endl;
      push_to_queue(std::bind(&SocketListener::handle_client_socket, this,
                              client_socket_fd, message_handler, s_ptr));
      m_loop_thread = std::thread([this]() { loop_check(); });
      m_loop_thread.detach();
      accepting_tasks = false;
      std::cout << "At the end" << std::endl;
    }
  }
}

void SocketListener::detachThreads() {
  for (std::thread& t : thread_pool) {
      if (t.joinable()) {
        t.detach();
      }
    }
}

/**
 * cleanUp
 * @method
 * TODO: Determine if we should be cleaning up buffer memory
 */
void SocketListener::cleanup() {
  std::cout << "Cleaning up" << std::endl;
  if (m_loop_thread.joinable()) {
    m_loop_thread.join();
  }
}
/**
 * createSocket
 * Open a listening socket and return its file descriptor
 */
int SocketListener::createSocket() {
  /* Call the system to open a socket passing arguments for
   ipv4 family, tcp type and no additional protocol info */
  int listening_socket_fd = socket(AF_INET, SOCK_STREAM, 0);

  if (listening_socket_fd != SOCKET_ERROR) {
    std::cout << "Created listening socket" << std::endl;
    // Create socket structure to hold address and type
    sockaddr_in socket_struct;
    socket_struct.sin_family = AF_INET;  // ipv4
    socket_struct.sin_port =
        htons(m_port);  // convert byte order of port value from host to network
    inet_pton(AF_INET, m_ip_address.c_str(),  // convert address to binary
              &socket_struct.sin_addr);

    int socket_option = 1;
    // Free up the port to begin listening again
    setsockopt(listening_socket_fd, SOL_SOCKET, SO_REUSEADDR, &socket_option,
               sizeof(socket_option));

    // Bind local socket address to socket file descriptor
    int bind_result = bind(
        listening_socket_fd,        // TODO: Use C++ cast on next line?
        (sockaddr*)&socket_struct,  // cast socket_struct to more generic type
        sizeof(socket_struct));
    if (bind_result != SOCKET_ERROR) {
      // Listen for connections to socket and allow up to max number of
      // connections for queue
      int listen_result = listen(listening_socket_fd, SOMAXCONN);
      if (listen_result == SOCKET_ERROR) {
        return WAIT_SOCKET_FAILURE;
      }
    } else {
      return WAIT_SOCKET_FAILURE;
    }
  }
  return listening_socket_fd;  // Return socket file descriptor
}
/**
 * waitForConnection
 * @method
 * Takes first connection on queue of pending connections, creates a new socket
 * and returns its file descriptor
 */
int SocketListener::waitForConnection(int listening_socket) {
  int client_socket_fd = accept(listening_socket, NULL, NULL);
  return client_socket_fd;
}
