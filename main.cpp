#include <iostream>
#include <string>
#include "headers/TcpListener.h"

void Listener_MessageReceived(CTcpListener* listener, int client, std::string msg);

int main () {
  CTcpListener server("0.0.0.0", 9009, Listener_MessageReceived);

  if (server.init()) {
    server.run();
  }
  return 0;
}

void Listener_MessageReceived(CTcpListener* listener, int client, std::string msg) {
  listener->sendMessage(client, msg);
}
