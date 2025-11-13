/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef SESSION_H
#define SESSION_H

#include "variant.h"

#include <string>



const long long BEST_RESULTS_SESSION_ID = ~0LL;



class Session
{
public:
    static int columnCount();
    static const char* columnName(int column);
    Variant get(int column) const;

    long long sessionId() const;
    void setSessionId(long long sessionId);

    std::string configurationName() const;
    void setConfigurationName(const std::string& name);
    
    bool isFinished() const;
    void setFinished(bool isFinished);

    std::string formatSessionId() const;
private:
    enum {
        COLUMN_ROW_ID,
        COLUMN_TITLE,
        COLUMN_CONFIGURATION,

        COLUMN_COUNT
    };

    long long sessionId_;
    std::string configurationName_;
    bool isFinished_;
};



#endif
