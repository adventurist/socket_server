#include <atomic>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>

class TaskQueue {
 public:
  TaskQueue();
  void initialize();
  void notifyPool();
  void pushToQueue(std::function<void()> fn);

 private:
  void handleLoop();
  void tasksReady();
  void done();
  void detachThreads();

  std::queue<std::function<void()>> m_task_queue;

  // ThreadPool m_thread_pool;
  std::vector<std::thread> m_thread_pool;
  std::thread m_loop_thread;

  std::mutex m_mutex_lock;
  std::condition_variable pool_condition;
  std::atomic<bool> accepting_tasks;
};
