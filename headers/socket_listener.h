#ifndef __SOCKET_LISTENER_H__
#define __SOCKET_LISTENER_H__
// Project libraries
#include "listen_interface.h"
#include "send_interface.h"

// System libraries
#include <sys/socket.h>

// C++ Libraries
#include <string>

#define MAX_BUFFER_SIZE (49152)

class SocketListener : public ListenInterface, public SendInterface {
 public:
  // constructor
  SocketListener(std::string ipAddress, int port);

  // destructor
  ~SocketListener();

  /**
   * Send a message to a client socket described by its file descriptor
   * @param[in] {int} client_socket_fd The client socket file descriptor
   * @param[in] {std::string} The message to be sent
   */
  virtual void sendMessage(int client_socket_fd, std::string message) override;

  /**
   * Perform intialization work
   */
  bool init();

  /**
   * Main message loop
   */
  void run();

  /**
   * Perform any cleanup work
   */
  void cleanup();

  virtual static void onMessageReceived(int socket_id,
                                        std::string message) override;

 private:
  // private methods
  int createSocket();

  int waitForConnection(int listening);

  // private members
  std::string m_ip_address;
  int m_port;
};

#endif  // __SOCKET_LISTENER_H__
