#ifndef __SOCKET_LISTENER_H__
#define __SOCKET_LISTENER_H__

// Project libraries
#include "send_interface.h"
#include "types.h"

// System libraries
#include <sys/socket.h>

// C++ Libraries
#include <atomic>
#include <condition_variable>
#include <functional>
#include <iostream>
#include <memory>
#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <vector>

class SocketListener : public SendInterface {
 public:
  class MessageHandler {
   public:
    MessageHandler(std::function<void()> cb) : m_cb(cb) {}

    void operator()() { m_cb(); }

   private:
    std::function<void()> m_cb;
  };
  // constructor
  SocketListener(std::string ip_address, int port);

  // destructor
  ~SocketListener();

  /**
   * Send a message to a client socket described by its file descriptor
   * @param[in] {int} client_socket_fd The client socket file descriptor
   * @param[in] {std::string} The message to be sent
   */
  virtual void sendMessage(int client_socket_fd,
                           std::weak_ptr<char[]> w_buffer_ptr) override;

  MessageHandler createMessageHandler(std::function<void()> cb);
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

  // virtual void setMessageHandler(MessageHandler message_handler) override;

 private:
  // private methods
  int createSocket();

  int waitForConnection(int listening);

  void loopCheck();

  void done();

  void handleLoop();

  void detachThreads();

  void pushToQueue(std::function<void()> fn);

  void handleClientSocket(int client_socket_fd,
                          SocketListener::MessageHandler message_handler,
                          const std::shared_ptr<char[]>& s_buffer_ptr);

  /* private members */
  // Server arguments
  std::string m_ip_address;
  int m_port;

  std::thread m_loop_thread;
  std::mutex m_mutex_lock;
  std::condition_variable pool_condition;
  std::atomic<bool> accepting_tasks;

  std::queue<std::function<void()>> task_queue;
  std::vector<std::thread> thread_pool;
};

#endif  // __SOCKET_LISTENER_H__
