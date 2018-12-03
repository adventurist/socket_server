#include <iostream>
#include <sys/types.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <string>

#include "headers/TcpListener.h"
#include "headers/constants.h"
#include <sys/socket.h>
int listening () {
    return socket(AF_INET, SOCK_STREAM, 0);
}

CTcpListener::CTcpListener(std::string ipAddress, int port, MessageReceivedHandler handler)
  : m_ipAddress(ipAddress), m_port(port), MessageReceived(handler) {

}


    // destructor
CTcpListener::~CTcpListener() {
  cleanup();
}

// Send message to client
void CTcpListener::sendMessage(int clientSocket, std::string msg) {
  send(clientSocket, msg.c_str(), msg.size() +1, 0);
}

// Initialize
bool CTcpListener::init() {
  std::cout << "Initializing socket listener" << std::endl;
  return true;
}

// Main process loop
void CTcpListener::run() {

  char* buf[MAX_BUFFER_SIZE];

  while (true) {
    int listening = createSocket();

    if (listening == SOCKET_ERROR) {
      std::cout << "Socket error: shutting down server" << std::endl;
      break;
    }
    int socket = waitForConnection(listening);

    if (socket != SOCKET_ERROR) {
      close(listening);

      while (true) {
        memset(buf, 0, MAX_BUFFER_SIZE);
        int bytesReceived = 0;
        bytesReceived = recv(socket, buf, MAX_BUFFER_SIZE, 0);
        if (bytesReceived > 0) {
            const char* constString = (const char*) buf;
            std::cout << "Received: " << constString << std::endl;
            MessageReceived(this, socket, std::string(constString));
        } else {
            std::cout << "client disconnected" << std::endl;
            break;
        }
      }
      close(socket);
    }
  }
}

// Cleanup
void CTcpListener::cleanup() {
  std::cout << "Cleaning up" << std::endl;
}

int CTcpListener::createSocket() {
  int listening = socket(AF_INET, SOCK_STREAM, 0);

  if (listening != SOCKET_ERROR) {

    sockaddr_in hint;
    hint.sin_family = AF_INET;
    hint.sin_port = htons(m_port);
    inet_pton(AF_INET, m_ipAddress.c_str(), &hint.sin_addr);

    int bindOk = bind(listening, (sockaddr*)&hint, sizeof(hint));
    if (bindOk != SOCKET_ERROR) {
      int listenOk = listen(listening, SOMAXCONN);
      if (listenOk == SOCKET_ERROR) {
        return -1;
      }
    } else {
      return -1;
    }
  }

  return listening;
}

int CTcpListener::waitForConnection(int listening) {
  int client = accept(listening, NULL, NULL);
  return client;
}
