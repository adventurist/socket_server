#include <iostream>
#include <string>
#include "headers/socket_listener.h"

int main() {
  SocketListener server("0.0.0.0", 9009);

  if (server.init()) {
    server.run();
  }
  return 0;
}

