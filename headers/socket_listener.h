#ifndef __SOCKET_LISTENER_H__
#define __SOCKET_LISTENER_H__

#include <sys/socket.h>
#include <string>
#include "listen_interface.h"

#define MAX_BUFFER_SIZE (49152)

// typedef void (*MessageReceivedHandler)(SocketListener* listener, int
// socketId,
//                                       std::string msg);

class SocketListener : ListenInterface {
 public:
  // constructor
  SocketListener(std::string ipAddress, int port);

  // destructor
  ~SocketListener();

  // public methods

  // Send message to client
  void sendMessage(int socket, std::string msg);

  // Initialize
  bool init();

  // Main process loop
  void run();

  // Cleanup
  void cleanup();

  void onMessageReceived(int socket_id, std::string message);

 private:
  // private methods
  int createSocket();

  int waitForConnection(int listening);

  // private members
  std::string m_ip_address;
  int m_port;
};

#endif  // __SOCKET_LISTENER_H__
