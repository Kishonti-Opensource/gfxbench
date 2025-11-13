/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "executor.h"

#include <Poco/Condition.h>
#include <Poco/Mutex.h>
#include <Poco/RunnableAdapter.h>
#include <Poco/ScopedLock.h>
#include <Poco/Thread.h>

#include <queue>



class Executor::Private
{
public:
    std::queue<Task> tasks;
    bool isRunning;
    Poco::Thread thread;
    std::unique_ptr<Poco::RunnableAdapter<Executor> > runnableAdapter;
    Poco::Condition condition;
    Poco::Mutex mutex;
};



Executor::Executor():
    d(new Private)
{
    d->isRunning = false;
    d->runnableAdapter.reset(
            new Poco::RunnableAdapter<Executor>(*this, &Executor::execute));
}



Executor::~Executor()
{}



void Executor::enqueueTask(const Task& task)
{
    /* synchronized */ {
        Poco::ScopedLock<Poco::Mutex> lock(d->mutex);
        d->tasks.push(task);
        d->condition.broadcast();
    }
}



void Executor::executeWaiting()
{
    for (;;) {
        Task task;
        /* synchronized */ {
            Poco::ScopedLock<Poco::Mutex> lock(d->mutex);
            if (d->tasks.empty()) {
                return;
            }
            task = d->tasks.front();
            d->tasks.pop();
        }
        task();
    }
}




void Executor::start()
{
    d->isRunning = true;
    d->thread.start(*d->runnableAdapter);
}



void Executor::stop()
{
    enqueueTask([this](){ d->isRunning = false; });
    d->thread.join();
}



void Executor::execute()
{
    while (d->isRunning) {
        Task task;
        /* synchronized */ {
            Poco::ScopedLock<Poco::Mutex> lock(d->mutex);
            if (d->tasks.empty()) {
                d->condition.wait(d->mutex);
            }
            task = d->tasks.front();
            d->tasks.pop();
        }
        task();
    }
}
