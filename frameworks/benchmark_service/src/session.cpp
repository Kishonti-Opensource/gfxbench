/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "session.h"

#include "cursor.h"

#include "Poco/DateTimeFormatter.h"
#include "Poco/LocalDateTime.h"



int Session::columnCount()
{
    return COLUMN_COUNT;
}



const char* Session::columnName(int column)
{
    switch (column) {
    case COLUMN_ROW_ID: return "_id";
    case COLUMN_TITLE: return "title";
    case COLUMN_CONFIGURATION: return "configuration";
    default: return "";
    }
}



Variant Session::get(int column) const
{
    switch (column) {
    case COLUMN_ROW_ID: return sessionId();
    case COLUMN_TITLE: return formatSessionId();
    case COLUMN_CONFIGURATION: return configurationName();
    default: return Variant();
    }
}




long long Session::sessionId() const
{
    return sessionId_;
}



void Session::setSessionId(long long sessionId)
{
    sessionId_ = sessionId;
}



std::string Session::configurationName() const
{
    return configurationName_;
}



void Session::setConfigurationName(const std::string& name)
{
    configurationName_ = name;
}



bool Session::isFinished() const
{
    return isFinished_;
}



void Session::setFinished(bool isFinished)
{
    isFinished_ = isFinished;
}



std::string Session::formatSessionId() const
{
    if (sessionId() == BEST_RESULTS_SESSION_ID) {
        return "BestResults";
    } else {
        return Poco::DateTimeFormatter::format(
                Poco::LocalDateTime(Poco::DateTime(Poco::Timestamp::fromEpochTime(sessionId() / 1000))),
                "%Y-%m-%d %H:%M:%S");
    }
}
