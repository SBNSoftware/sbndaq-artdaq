
#include "ExitTimerManager.hh"
#define TRACE_NAME "ExitTimerManager"

#include "artdaq/DAQdata/Globals.hh"
using sbndaq::ExitTimerManager;

void ExitTimerManager::startExitTimer(unsigned int seconds) {
  std::lock_guard<std::mutex> lock(_mutex);
  if (!_timerThread.joinable()) {
    _timerThread = std::thread(&ExitTimerManager::startTimer, seconds);
    TLOG(TLVL_INFO) << "Timer started.";
  }
  /*
  else {
    TLOG(TLVL_WARNING) << "Timer already started. Stopping and exiting...";
    stopTimer();
    std::exit(1);
  }
 */
}

void ExitTimerManager::startTimer(unsigned int seconds) {
  std::this_thread::sleep_for(std::chrono::seconds(seconds));
  TLOG(TLVL_ERROR) << "Timer expired! Exiting...";
  std::exit(1);
}

void ExitTimerManager::stopTimer() {
  if (_timerThread.joinable()) {
    _timerThread.join();
  }
  TLOG(TLVL_INFO) << "Timer stopped.";
}

// void test_ExitTimerManager() {
//   ExitTimerManager exitTimerManager;
//   exitTimerManager.startExitTimer(3);
// }
