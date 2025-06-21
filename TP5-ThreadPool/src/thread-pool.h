#ifndef _thread_pool_
#define _thread_pool_

#include <cstddef>
#include <functional>
#include <thread>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <deque>
#include <memory>
#include "Semaphore.h"

using namespace std;

typedef struct worker {
    thread ts;
    function<void(void)> thunk;
    bool available = true;
    bool hasTask = false;
    shared_ptr<Semaphore> wake;
    int id;
} worker_t;

class ThreadPool {
  public:
    ThreadPool(size_t numThreads);
    void schedule(const function<void(void)>& thunk);
    void wait();
    ~ThreadPool();

  private:
    void worker(int id);
    void dispatcher();

    thread dt;
    vector<worker_t> wts;
    deque<function<void(void)>> taskQueue;
    mutex queueLock;
    condition_variable taskAvailable;
    condition_variable noMoreTasks;
    bool done;
    size_t pendingTasks;

    ThreadPool(const ThreadPool& original) = delete;
    ThreadPool& operator=(const ThreadPool& rhs) = delete;
};

#endif
