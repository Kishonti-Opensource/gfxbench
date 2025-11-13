/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef BENCHMARKEXCEPTION_H
#define BENCHMARKEXCEPTION_H

#include <stdexcept>
#include <string>



class BenchmarkException: public std::runtime_error
{
public:
    static const std::string INTERNAL_ERROR;
    static const std::string ALREADY_RUNNING;
    static const std::string DATABASE_ERROR;
    static const std::string NETWORK_ERROR;
    static const std::string OUTDATED;
    static const std::string USER_NAME_TAKEN;
    static const std::string EMAIL_TAKEN;
    static const std::string INVALID_EMAIL_ADDRESS;
    static const std::string INVALID_CREDENTIALS;
    static const std::string SYNCHRONIZATION_ERROR;
    static const std::string BENCHMARK_ERROR;
    static const std::string CANNOT_CREATE_TEST;
    static const std::string FOCUS_LOST;
    static const std::string BATTERY_CONNECTED;
    static const std::string PRIMARY_SCREEN;

    BenchmarkException(): std::runtime_error("unknown error") {}
    BenchmarkException(const std::string &errorId, const std::string &message):
        std::runtime_error(message),
        m_errorId(errorId)
    {}
    ~BenchmarkException() throw() {}
    virtual BenchmarkException* clone() const { return new BenchmarkException(*this); }
    const char* errorId() const { return m_errorId.c_str(); }
    virtual bool isFatal() const { return false; }
private:
    std::string m_errorId;
};



class FatalBenchmarkException: public BenchmarkException
{
public:
    FatalBenchmarkException() {}
    FatalBenchmarkException(const std::string &errorId, const std::string &message) :
        BenchmarkException(errorId, message)
    {}
    virtual FatalBenchmarkException* clone() const { return new FatalBenchmarkException(*this); }
    virtual bool isFatal() const { return true; }
};



#endif
