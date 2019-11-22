// Project headers
#include "headers/socket_listener.h"

#include "headers/constants.h"
#include "headers/listen_interface.h"
// System libraries
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
// C++ Libraries
#include <atomic>
#include <condition_variable>
#include <functional>
#include <iostream>
#include <memory>
#include <queue>
#include <string>
#include <thread>
#include <vector>

/**
 * Constructor
 * Initialize with ip_address, port and message_handler
 */
SocketListener::SocketListener(std::string ip_address, int port)
    : m_ip_address(ip_address), m_port(port), accepting_tasks(true), shutdown_loop(false) {}

/**
 * Destructor
 * TODO: Determine if we should make buffer a class member
 */
SocketListener::~SocketListener() { cleanup(); }

SocketListener::MessageHandler SocketListener::createMessageHandler(
    std::function<void()> cb) {
  return MessageHandler(cb);
}

/*
  **setMessageHandler *@method *Set the function to handle received
   messages * /

  // SocketListener::setMessageHandler(MessageHandler message_handler) {
  //   m_message_handler = message_handler;
  // }

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
  std::function<void()> fn;
  for (;;) {
    {
      std::unique_lock<std::mutex> lock(m_mutex_lock);
      pool_condition.wait(
          lock, [this]() { return !accepting_tasks || !task_queue.empty(); });
      if (!accepting_tasks && task_queue.empty()) {
        return;  // we are done here
      }
      fn = task_queue.front();
      task_queue.pop();
    }
    fn();
  }
}

void SocketListener::loop_check() {
  if (!m_loop_switch) {
    m_loop_switch = true;
    m_loop_thread = std::thread(&SocketListener::handle_loop, this);
  }
}


void SocketListener::handle_client_socket(int client_socket_fd,
                          SocketListener::MessageHandler message_handler,
                          std::shared_ptr<char[]> buf) {
  // char buf[MAX_BUFFER_SIZE] =
  //     {};  // Declare, define and initialize a character buffer
  // std::string buffer_string{};  // Initialize a string buffer
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
      std::cout << "Bytes received: " << bytes_received
                << "\nData: " << buf.get() << std::endl;
      // Handle incoming message
      message_handler();
    } else {
      std::cout << "client disconnected" << std::endl;
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
      push_to_queue(std::bind(&SocketListener::handle_client_socket, this, client_socket_fd,
                              message_handler, s_ptr));
      loop_check();
      std::cout << "At the end" << std::endl;
    }
  }
}

/**
 * cleanUp
 * @method
 * TODO: Determine if we should be cleaning up buffer memory
 */
void SocketListener::cleanup() { std::cout << "Cleaning up" << std::endl;
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
    setsockopt(listening_socket_fd, SOL_SOCKET, SO_REUSEADDR, &socket_option, sizeof(socket_option));

    // Bind local socket address to socket file descriptor
    int bind_result = bind(
        listening_socket_fd,         // TODO: Use C++ cast on next line?
        (sockaddr *)&socket_struct,  // cast socket_struct to more generic type
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
