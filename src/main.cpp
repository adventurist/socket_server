#include <iostream>
#include <string>

#include "../headers/socket_listener.hpp"

/** \mainpage
 * SocketListener constructor takes 2 parameters (std::string ip, int port).
 *
 * Calling the "run()" method will cause it to for and handle multiple
 * concurrent socket connections.
 */

int main(int argc, char** argv) {
  SocketListener server(argc, argv);
  server.init();
  server.run();

  return 0;
}

