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
#include <iostream>
#include <string>

/**
 * Constructor
 * Initialize with ip_address and port
 */
SocketListener::SocketListener(std::string ip_address, int port)
    : m_ip_address(ip_address), m_port(port) {}

/**
 * Destructor
 * TODO: Determine if we should make buffer a class member
 */
SocketListener::~SocketListener() { cleanup(); }

/**
 * sendMessage
 * @method
 * Send a null-terminated array of characters, supplied as a const char pointer,
 * to a client socket
 */
void SocketListener::sendMessage(int clientSocket, std::string msg) {
  send(clientSocket, msg.c_str(), msg.size() + 1, 0);
}

/**
 * init
 * TODO: Initialize buffer memory, if buffer is to be a class member
 */
bool SocketListener::init() {
  std::cout << "Initializing socket listener" << std::endl;
  return true;
}

/**
 * run
 * @method
 * Main message loop
 * TODO: Implement multithreading
 */
void SocketListener::run() {
  // Declare, define and initialize a character buffer
  char buf[MAX_BUFFER_SIZE] = {};
  // Begin listening loop
  while (true) {
    // Call system to open a listening socket, and return its file descriptor
    int listening_socket_fd = createSocket();

    if (listening_socket_fd == SOCKET_ERROR) {
      std::cout << "Socket error: shutting down server" << std::endl;
      break;
    }
    // wait for a client connection and get its socket file descriptor
    int client_socket_fd = waitForConnection(listening_socket_fd);

    if (socket != SOCKET_ERROR) {
      // Destroy listening socket and deallocate its file descriptor. Only use
      // the client socket now.
      close(listening_socket_fd);
      std::string buffer_string{};  // Initialize a string buffer
      while (true) {
        memset(buf, 0, MAX_BUFFER_SIZE);  // Zero the character buffer
        int bytes_received = 0;
        // Receive and write incoming data to buffer and return the number of
        // bytes received
        bytes_received =
            recv(client_socket_fd, buf,
                 MAX_BUFFER_SIZE - 2,  // Leave room for null-termination
                 0);
        buf[MAX_BUFFER_SIZE - 1] = 0;  // Null-terminate the character buffer
        if (bytes_received > 0) {
          buffer_string += buf;
          std::cout << "Bytes received: " << bytes_received << "\nData: " << buf
                    << std::endl;
          // Handle incoming message
          onMessageReceived(socket, std::string(buf));
        } else {
          std::cout << "client disconnected" << std::endl;
          break;
        }
      }
      // Zero the buffer again before closing
      memset(buf, 0, MAX_BUFFER_SIZE);
      // TODO: Determine if we should free memory, or handle as class member
      close(client_socket_fd);  // Destroy client socket and deallocate its fd
    }
  }
}

/**
 * cleanUp
 * @method
 * TODO: Determine if we should be cleaning up buffer memory
 */
void SocketListener::cleanup() { std::cout << "Cleaning up" << std::endl; }
/**
 * createSocket
 * Open a listening socket and return its file descriptor
 */
int SocketListener::createSocket() {
  /* Call the system to open a socket passing arguments for
   ipv4 family, tcp type and no additional protocol info */
  int listening_socket_fd = socket(AF_INET, SOCK_STREAM, 0);

  if (listening_socket_fd != SOCKET_ERROR) {
    // Create socket structure to hold address and type
    sockaddr_in socket_struct;
    socket_struct.sin_family = AF_INET;  // ipv4
    socket_struct.sin_port =
        htons(m_port);  // convert byte order of port value from host to network
    inet_pton(AF_INET, m_ip_address.c_str(),  // convert address to binary
              &socket_struct.sin_addr);
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
/**
 * onMessageReceived
 * @method
 * @override
 * Handle messages successfully received from a client socket
 */
void SocketListener::onMessageReceived(int socket_id, std::string message) {
  sendMessage(socket_id, message);
}

