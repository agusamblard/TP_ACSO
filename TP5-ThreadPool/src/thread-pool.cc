#include "thread-pool.h"
#include <condition_variable>
#include <deque>
#include <stdexcept>
#include <array>  // Necesario por uso indirecto en test_custom.cc

using namespace std;

ThreadPool::ThreadPool(size_t numThreads)
    : wts(numThreads), done(false), pendingTasks(0) {
    for (size_t i = 0; i < numThreads; ++i) {
        wts[i].available = true;
        wts[i].wake = make_shared<Semaphore>(0);
        wts[i].hasTask = false;
        wts[i].id = i;
        wts[i].ts = thread([this, i] { worker(i); });
    }
    dt = thread([this] { dispatcher(); });
}

void ThreadPool::schedule(const function<void(void)>& thunk) {
    if (!thunk) throw invalid_argument("Cannot schedule nullptr function");
    {
        lock_guard<mutex> lock(queueLock);
        if (done) throw runtime_error("Cannot schedule on destroyed ThreadPool");
        taskQueue.push_back(thunk);
        ++pendingTasks;
    }
    taskAvailable.notify_one();
}

void ThreadPool::dispatcher() {
    while (true) {
        function<void(void)> task;
        int target = -1;

        unique_lock<mutex> lock(queueLock);
        taskAvailable.wait(lock, [this] {
            return done || !taskQueue.empty();
        });

        if (done && taskQueue.empty()) break;

        for (size_t i = 0; i < wts.size(); ++i) {
            if (wts[i].available) {
                target = i;
                break;
            }
        }

        if (target == -1) {
            lock.unlock();
            this_thread::yield();
            continue;
        }

        if (!taskQueue.empty()) {
            task = taskQueue.front();
            taskQueue.pop_front();
            wts[target].thunk = task;
            wts[target].hasTask = true;
            wts[target].available = false;
            wts[target].wake->signal();
        }
    }
}

void ThreadPool::worker(int id) {
    while (true) {
        wts[id].wake->wait();
        if (done && !wts[id].hasTask) break;

        if (wts[id].hasTask) {
            wts[id].thunk();
            wts[id].hasTask = false;
            wts[id].available = true;
            {
                lock_guard<mutex> lock(queueLock);
                --pendingTasks;
                if (pendingTasks == 0) noMoreTasks.notify_all();
            }
        }
    }
}

void ThreadPool::wait() {
    unique_lock<mutex> lock(queueLock);
    noMoreTasks.wait(lock, [this] {
        return pendingTasks == 0;
    });
}

ThreadPool::~ThreadPool() {
    wait();
    {
        lock_guard<mutex> lock(queueLock);
        done = true;
    }
    taskAvailable.notify_all();
    for (auto& w : wts) {
        w.wake->signal();
        if (w.ts.joinable()) w.ts.join();
    }
    if (dt.joinable()) dt.join();
}
