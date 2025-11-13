/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef SESSION_H
#define SESSION_H

#include <string>



namespace tfw
{

const long long BEST_RESULTS_SESSION_ID = ~0LL;



class Session
{
public:
    long long sessionId() const { return sessionId_; }
    void setSessionId(long long sessionId) { sessionId_ = sessionId; }

    std::string configurationName() const { return configurationName_; }
    void setConfigurationName(const std::string& name) { configurationName_ = name; }
    
    bool isFinished() const { return isFinished_; }
    void setFinished(bool isFinished) { isFinished_ = isFinished; }
private:
    long long sessionId_;
    std::string configurationName_;
    bool isFinished_;
};

}



#endif
