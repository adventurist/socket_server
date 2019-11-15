#include <iostream>
#include <string>
#include "headers/tcplistener.h"

void listener_message_received(TcpListener* listener, int client,
                               std::string msg);

int main() {
  TcpListener server("0.0.0.0", 9009, listener_message_received);

  if (server.init()) {
    server.run();
  }
  return 0;
}

void listener_message_received(TcpListener* listener, int client,
                               std::string msg) {
  listener->sendMessage(client, msg);
}
