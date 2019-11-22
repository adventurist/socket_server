#include <iostream>
#include <string>

#include "headers/socket_listener.h"

int main(int argc, char** argv) {
  SocketListener server("0.0.0.0", 9009);

  if (server.init()) {
    std::cout << "Running message loop" << std::endl;
    server.run();
  }
  return 0;
}

