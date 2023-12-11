#ifndef _ExitTimerManager_H_
#define _ExitTimerManager_H_

#include <chrono>
#include <cstdlib>
#include <iostream>
#include <mutex>
#include <thread>

namespace sbndaq {
class ExitTimerManager {
public:
  ExitTimerManager() = default;
  ~ExitTimerManager() { stopTimer(); };
  void startExitTimer(unsigned int seconds = 10);

private:
  static void startTimer(unsigned int seconds);
  void stopTimer();
  std::mutex _mutex;
  std::thread _timerThread;
};
} // namespace sbndaq

// int mainTest() {
//   ExitTimerManager exitTimerManager;
//   exitTimerManager.startExitTimer(3);
//
//   return 0;
// }
#endif // _ExitTimerManager_H_
