#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <iostream>
#include <string>

#include <sys/socket.h>
#include "headers/constants.h"
#include "headers/socket_listener.h"

int listening() { return socket(AF_INET, SOCK_STREAM, 0); }

SocketListener::SocketListener(std::string ip_address, int port)
    : m_ip_address(ip_address), m_port(port) {}

// destructor
SocketListener::~SocketListener() { cleanup(); }

// Send message to client
void SocketListener::sendMessage(int clientSocket, std::string msg) {
  send(clientSocket, msg.c_str(), msg.size() + 1, 0);
}

// Initialize
bool SocketListener::init() {
  std::cout << "Initializing socket listener" << std::endl;
  return true;
}

// Main process loop
void SocketListener::run() {
  char buf[MAX_BUFFER_SIZE];

  while (true) {
    int listening = createSocket();

    if (listening == SOCKET_ERROR) {
      std::cout << "Socket error: shutting down server" << std::endl;
      break;
    }
    int socket = waitForConnection(listening);

    if (socket != SOCKET_ERROR) {
      close(listening);
      std::string buffer_string;
      while (true) {
        memset(buf, 0, MAX_BUFFER_SIZE);
        int bytesReceived = 0;
        bytesReceived = recv(socket, buf, MAX_BUFFER_SIZE - 2, 0);
        buf[MAX_BUFFER_SIZE - 1] = 0;
        if (bytesReceived > 0) {
          // TODO: Verify that we aren't producig undefined behaviour
          buffer_string += buf;
          std::cout << "Received: " << buffer_string << std::endl;
          onMessageReceived(socket, buffer_string);
        } else {
          std::cout << "client disconnected" << std::endl;
          break;
        }
      }
      memset(buf, 0, MAX_BUFFER_SIZE);
      close(socket);
    }
  }
}

// Cleanup
void SocketListener::cleanup() { std::cout << "Cleaning up" << std::endl; }

int SocketListener::createSocket() {
  int listening = socket(AF_INET, SOCK_STREAM, 0);

  if (listening != SOCKET_ERROR) {
    // TODO: whatsup with the variable name "hint" ?
    sockaddr_in hint;
    hint.sin_family = AF_INET;
    hint.sin_port = htons(m_port);
    inet_pton(AF_INET, m_ip_address.c_str(), &hint.sin_addr);

    int bind_result = bind(listening, (sockaddr *)&hint, sizeof(hint));
    if (bind_result != SOCKET_ERROR) {
      int listen_result = listen(listening, SOMAXCONN);
      if (listen_result == SOCKET_ERROR) {
        return WAIT_SOCKET_FAILURE;
      }
    } else {
      return WAIT_SOCKET_FAILURE;
    }
  }
  return listening;
}

int SocketListener::waitForConnection(int listening) {
  int client = accept(listening, NULL, NULL);
  return client;
}

void SocketListener::onMessageReceived(int socket_id, std::string message) {
  sendMessage(socket_id, message);
}

