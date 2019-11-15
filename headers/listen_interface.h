#ifndef __LISTEN_INTERFACE_H__
#define __LISTEN_INTERFACE_H__

#include <string>

class ListenInterface {
  virtual void onMessageReceived(int socket_id, std::string message) = 0;
};

#endif  // __LISTEN_INTERFACE_H__
