#include <sys/socket.h>
#include <string>

#define MAX_BUFFER_SIZE (49152)
class TcpListener;

typedef void (*MessageReceivedHandler)(TcpListener* listener, int socketId,
                                       std::string msg);

class TcpListener {
 public:
  // constructor
  TcpListener(std::string ipAddress, int port, MessageReceivedHandler handler);

  // destructor
  ~TcpListener();

  // public methods

  // Send message to client
  void sendMessage(int clientSocket, std::string msg);

  // Initialize
  bool init();

  // Main process loop
  void run();

  // Cleanup
  void cleanup();

 private:
  // private methods
  int createSocket();

  int waitForConnection(int listening);

  // private members
  std::string m_ipAddress;
  int m_port;
  MessageReceivedHandler MessageReceived;
};
