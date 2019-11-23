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
    //    int m_client_socket_fd;
    //    std::shared_ptr<char[]> m_char_ptr;
    std::function<void()> m_cb;
  };
  // constructor
  SocketListener(std::string ipAddress, int port);

  // destructor
  ~SocketListener();

  /**
   * Send a message to a client socket described by its file descriptor
   * @param[in] {int} client_socket_fd The client socket file descriptor
   * @param[in] {std::string} The message to be sent
   */
  virtual void sendMessage(int client_socket_fd,
                           std::shared_ptr<char[]> buffer) override;

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

  void loop_check();

  void done();

  void handle_loop();

  void detachThreads();

  void push_to_queue(std::function<void()> fn);

  void handle_client_socket(int client_socket_fd,
                            SocketListener::MessageHandler message_handler,
                            std::shared_ptr<char[]> buf);

  // private members
  std::string m_ip_address;
  int m_port;
  std::thread m_loop_thread;
  std::queue<std::function<void()>> task_queue;
  std::mutex m_mutex_lock;
  std::condition_variable pool_condition;
  std::atomic<bool> accepting_tasks;
  std::atomic<bool> shutdown_loop;
  std::atomic<bool> m_loop_switch;

  std::vector<std::thread> thread_pool;
};

#endif  // __SOCKET_LISTENER_H__
