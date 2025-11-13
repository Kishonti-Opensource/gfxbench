/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef NETWORKGATEWAY_H
#define NETWORKGATEWAY_H

#include "benchmarkexception.h"

#include "properties.h"
#include "systeminfo.h"

#include <string>
#include <vector>



class NetworkGateway
{
public:
    virtual ~NetworkGateway() {}
    virtual std::string storedUsername() const = 0;
    virtual void cancel() = 0;
    virtual void pollSyncProgress(
            float &progress,
            long long &bytesNeeded,
            long long &bytesWritten) const = 0;
    virtual void initialize(
            sysinf::Properties &properties,
            const std::vector<std::string> &syncFlags,
            std::string &message,
            std::string &messageStamp,
            long long &bytesToSynchronize) = 0;
    virtual void synchronize() = 0;
    virtual void login(const std::string& username, const std::string& password) = 0;
    virtual void logout() = 0;
    virtual void signUp(
            const std::string &email,
            const std::string &username,
            const std::string &password) = 0;
    virtual void deleteUser() = 0;
};



class NullNetworkGateway : public NetworkGateway
{
public:
    virtual std::string storedUsername() const { return ""; }
    virtual void cancel() {}
    virtual void pollSyncProgress(float&, long long&, long long&) const {}
    virtual void initialize(
        sysinf::Properties&,
        const std::vector<std::string>&,
        std::string&,
        std::string&,
        long long &bytesToSynchronize)
    {
        bytesToSynchronize = 0;
    }
    virtual void synchronize() {}
    virtual void login(const std::string&, const std::string&) {}
    virtual void logout() {}
    virtual void signUp(const std::string&, const std::string&, const std::string&) {}
    virtual void deleteUser() {};
};



#endif
