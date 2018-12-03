#include <string>
#include <sys/socket.h>

#define MAX_BUFFER_SIZE (49152)
class CTcpListener;

typedef void (*MessageReceivedHandler)(CTcpListener* listener, int socketId, std::string msg);

class CTcpListener {
  public:
    // constructor
    CTcpListener(std::string ipAddress, int port, MessageReceivedHandler handler);

    // destructor
    ~CTcpListener();

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
  std::string             m_ipAddress;
  int                     m_port;
  MessageReceivedHandler  MessageReceived;
};
