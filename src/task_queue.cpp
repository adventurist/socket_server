#include "../headers/task_queue.hpp"

#include <iostream>

int num_threads = std::thread::hardware_concurrency();

TaskQueue::TaskQueue() {}

void TaskQueue::pushToQueue(std::function<void()> fn) {
  std::unique_lock<std::mutex> lock(m_mutex_lock);
  m_task_queue.push(fn);
  lock.unlock();
  pool_condition.notify_one();
}

void TaskQueue::handleLoop() {
  std::function<void()> fn;
  for (;;) {
    {  // encapsulate atomic management of queue
      std::unique_lock<std::mutex> lock(m_mutex_lock);
      pool_condition.wait(
          lock, [this]() { return !accepting_tasks || !m_task_queue.empty(); });
      std::cout << "Wait condition met" << std::endl;
      if (!accepting_tasks && m_task_queue.empty()) {
        return;  // we are done here
      }
      std::cout << "Taking task" << std::endl;
      fn = m_task_queue.front();
      m_task_queue.pop();
    }      // queue management complete
    fn();  // work
  }
}

void TaskQueue::done() {
  std::unique_lock<std::mutex> lock(m_mutex_lock);
  accepting_tasks = false;
  lock.unlock();
  // when we send the notification immediately, the consumer will try to get the
  // lock , so unlock asap
  pool_condition.notify_all();
}

void TaskQueue::notifyPool() {
  for (int i = 0; i < m_task_queue.size() && i < (num_threads - 1); i++) {
    m_thread_pool.push_back(std::thread([this]() { handleLoop(); }));
  }
  done();
  std::this_thread::sleep_for(std::chrono::milliseconds(400));
  detachThreads();
  size_t task_num = m_task_queue.size();
  std::cout << "Task num: " << task_num << std::endl;
  accepting_tasks = true;
}

void TaskQueue::initialize() {
  /* m_loop_thread = std::thread([this]() { loopCheck(); }); */
  /* m_loop_thread.detach(); */
}

void TaskQueue::detachThreads() {
  for (std::thread& t : m_thread_pool) {
    if (t.joinable()) {
      t.detach();
    }
  }
}

