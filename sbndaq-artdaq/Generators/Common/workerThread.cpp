#define TRACE_NAME "WorkerThread"
#include "workerThread.hh"
#include <chrono>
#include "artdaq/DAQdata/Globals.hh"

bool share::WorkerThreadFunctor::call() { return _functor(); }

share::WorkerThread::WorkerThread(WorkerThreadFunctorUPtr& functor, unsigned int usec_delay_if_failed,
                                  unsigned int max_stop_tries)
    : _functor(std::move(functor)),
      _stop_running(false),
      _usec_delay_iffailed(usec_delay_if_failed),
      _maxStopAttempts(max_stop_tries) {}

void share::WorkerThread::start() {
  TLOG(TLVL_DEBUG + 1) << "Starting " << _functor->name() << " thread.";
  _stop_running = false;
  _worker_thread_result = std::async(std::launch::async, &WorkerThread::run, this);
  TLOG(TLVL_INFO) << "Started " << _functor->name() << " thread.";
}

void share::WorkerThread::stop() {
  TLOG(TLVL_DEBUG + 2) << "Stopping " << _functor->name() << " thread.";
  if (_stop_running) {
    TLOG(TLVL_WARNING) << "Thread " << _functor->name() << " is not running.";
    return;
  }
  _stop_running = true;
  int availableRetryCount = _maxStopAttempts;
  bool isDone = _worker_thread_result.wait_for(std::chrono::seconds(1)) == std::future_status::ready;
  while (!isDone && availableRetryCount--)
    isDone = _worker_thread_result.wait_for(std::chrono::seconds(1)) == std::future_status::ready;
  if (!isDone) {
    TLOG(TLVL_ERROR) << "Cannot stop " << _functor->name() << " thread.";
  }
  _worker_thread_result.get();
  TLOG(TLVL_INFO) << "Stopped " << _functor->name() << " thread.";
}

void share::WorkerThread::run() {
  uint64_t iterationCount = 0;
  TLOG(TLVL_DEBUG + 4) << "Thread " << _functor->name() << " started running.";
  try {
    while (!_stop_running) {
      if (_functor->call())
        ++iterationCount;
      else
        usleep(_usec_delay_iffailed);
    }
  } catch (...) {
    TLOG(TLVL_ERROR) << "Caught exception while running " << _functor->name() << " thread.";
  }
  TLOG(TLVL_INFO) << "Stopped " << _functor->name() << " thread after " << iterationCount << " checks.";
  _stop_running = true;
}
