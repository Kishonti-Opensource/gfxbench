/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef DATAGATEWAY_H
#define DATAGATEWAY_H

#include "compareresult.h"
#include "resultitem.h"
#include "session.h"

#include "schemas/descriptors.h"
#include "schemas/result.h"

#include <stdexcept>
#include <memory>
#include <vector>



struct sqlite3;



class DatabaseException: public std::runtime_error
{
public:
    DatabaseException(const std::string& message, int errorCode):
        std::runtime_error(message),
        mErrorCode(errorCode)
    {}
    int errorCode() const { return mErrorCode; }
private:
    int mErrorCode;
};



class Transaction
{
public:
    Transaction(sqlite3* database);
    ~Transaction();
    void commit();
private:
    sqlite3* mDatabase;
    bool mIsCommited;

    Transaction(const Transaction&); // No copy
    Transaction& operator=(const Transaction&); // No copy
};



class DataGateway
{
public:
    static const int CURRENT_DATABASE_VERSION = 4;

    static int check(int resultCode);

    DataGateway();
    virtual ~DataGateway();
    virtual void openLocalDatabase(const std::string& path);

    virtual void saveSetting(const std::string& key, int value);
    virtual int loadSetting(const std::string& key, int defaultValue = -1);
    virtual void clear();
    virtual void addResults(const Session &session, const tfw::ResultGroup &result);
    virtual void addSession(const Session &session);
    virtual void addTestSelection(const std::string& testId, bool isSelected);
    virtual bool addServerMessage(
            const std::string &messageTimestamp,
            const std::string &message);
    virtual std::vector<std::pair<std::string, bool> > getTestSelections();
    virtual std::vector<Session> getSessions();
    virtual std::vector<tfw::ResultGroup> getResultsForSession(long long sessionId);
    virtual std::vector<tfw::ResultGroup> getBestResults();
	virtual std::vector<tfw::ResultGroup> getBestResults(std::string configurationName);
    virtual tfw::ResultGroup getResultForRowId(long long rowId);

    virtual void openCompareDatabase(const std::string& path);
    virtual std::vector<CompareResult> getResultsById(
            const ResultItem &resultItem,
            const std::string &deviceFilter,
            bool hideDesktop);
    virtual std::vector<CompareResult> getResultsByDevice(
            const std::string &api,
            const std::string &deviceId);

    virtual std::string getResource(const std::string& name);
    virtual tfw::Descriptor getDescriptorByTestId(const std::string& testId);
    sqlite3* database();
protected:
    virtual void create(const std::string& path);
    virtual void upgrade();
private:
    class Private;
    std::unique_ptr<Private> d;
};



#endif
