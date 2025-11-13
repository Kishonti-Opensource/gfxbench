/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef EXECUTOR_H
#define EXECUTOR_H

#include <functional>
#include <memory>
#include <string>



class Executor
{
public:
    Executor();
    ~Executor();
    typedef std::function<void(void)> Task;
    void enqueueTask(const Task& task);
    void executeWaiting();
    void start();
    void stop();
private:
    class Private;
    std::unique_ptr<Private> d;

    void execute();
    Executor(const Executor&); // No copy
    Executor& operator=(const Executor&); // No copy
};



#endif // EXECUTOR_H
