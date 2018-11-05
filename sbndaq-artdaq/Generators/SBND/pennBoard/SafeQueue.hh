/*
 * SafeQueue.hh
 *
 *  Created on: May 22, 2014
 *      Author: tcn45
 */

#ifndef SAFEQUEUE_HH_
#define SAFEQUEUE_HH_

#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>

template <typename T>
class SafeQueue
{
 public:

  T pop()
  {
    std::unique_lock<std::mutex> mlock(mutex_);
    while (queue_.empty())
    {
      cond_.wait(mlock);
    }
    auto val = std::move(queue_.front());
    queue_.pop();
    return val;
  }

  void pop(T& item)
  {
    std::unique_lock<std::mutex> mlock(mutex_);
    while (queue_.empty())
    {
      cond_.wait(mlock);
    }
    item = std::move(queue_.front());
    queue_.pop();
  }

  bool try_pop(T& item, std::chrono::microseconds timeout)
  {
      std::unique_lock<std::mutex> mlock(mutex_);

      if(!cond_.wait_for(mlock, timeout, [this] { return !queue_.empty(); }))
          return false;

      item = std::move(queue_.front());
      queue_.pop();

      return true;
  }

  void push(const T& item)
  {
    std::unique_lock<std::mutex> mlock(mutex_);
    queue_.push(item);
    mlock.unlock();
    cond_.notify_one();
  }

  void push(T&& item)
  {
    std::unique_lock<std::mutex> mlock(mutex_);
    queue_.push(std::move(item));
    mlock.unlock();
    cond_.notify_one();
  }

  size_t size(void)
  {
	  std::unique_lock<std::mutex> mlock(mutex_);
	  size_t queue_size = queue_.size();
	  mlock.unlock();

	  return queue_size;
  }

  SafeQueue()=default;
  SafeQueue(const SafeQueue&) = delete;            // disable copying
  SafeQueue& operator=(const SafeQueue&) = delete; // disable assignment

 private:
  std::queue<T> queue_;
  std::mutex mutex_;
  std::condition_variable cond_;
};




#endif /* SAFEQUEUE_HH_ */
