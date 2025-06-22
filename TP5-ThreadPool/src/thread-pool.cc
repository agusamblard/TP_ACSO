// thread-pool.cc
#include "thread-pool.h"
#include <condition_variable>
#include <deque>
#include <stdexcept>
#include <array>

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

    bool destroyed = false;
    {
        lock_guard<mutex> lock(queueLock);
        destroyed = done;
        if (!destroyed) {
            taskQueue.push_back(thunk);
            ++pendingTasks;
        }
    }

    if (destroyed) throw runtime_error("Cannot schedule on destroyed ThreadPool");

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
            lock_guard<mutex> stateGuard(wts[i].stateLock);
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

            {
                lock_guard<mutex> stateGuard(wts[target].stateLock);
                wts[target].thunk = task;
                wts[target].hasTask = true;
                wts[target].available = false;
            }

            wts[target].wake->signal();
        }
    }
}

void ThreadPool::worker(int id) {
    while (true) {
        wts[id].wake->wait();

        bool exitNow = false;
        {
            lock_guard<mutex> lock(queueLock);
            lock_guard<mutex> stateGuard(wts[id].stateLock);
            if (done && !wts[id].hasTask) {
                exitNow = true;
            }
        }
        if (exitNow) break;

        bool execute = false;
        function<void(void)> task;

        {
            lock_guard<mutex> stateGuard(wts[id].stateLock);
            if (wts[id].hasTask) {
                task = wts[id].thunk;
                wts[id].hasTask = false;
                wts[id].available = true;
                execute = true;
            }
        }

        if (execute) {
            task();
            lock_guard<mutex> lock(queueLock);
            --pendingTasks;
            if (pendingTasks == 0) noMoreTasks.notify_all();
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