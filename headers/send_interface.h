#ifndef __SEND_INTERFACE_H__
#define __SEND_INTERFACE_H__

#include <string>

class SendInterface {
 public:
  virtual void sendMessage(int client_socket_fd, std::string message) = 0;
};

#endif  // __SEND_INTERFACE_H__
