/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "datagateway.h"

#include "getResource.h"
#include "sqlite3.h"

#include <sstream>



std::string trimUnderscores(const std::string& input) {
    std::string output = input;
    if (output.empty()) return output;
    if (output.front() == '_') {
        output.erase(output.begin());
    }
    if (output.empty()) return output;
    if (output.back() == '_') {
        output.pop_back();
    }
    return output;
}



void bindString(
        sqlite3_stmt* statement,
        int index,
        const std::string& text,
        void(*destructor)(void*))
{
    DataGateway::check(sqlite3_bind_text(statement, index, text.c_str(),
            static_cast<int>(text.size()), destructor));
}



std::string columnString(sqlite3_stmt* statement, int column)
{
    const unsigned char* pointer = sqlite3_column_text(statement, column);
    if (pointer != nullptr) {
        return std::string(reinterpret_cast<const char*>(pointer));
    } else {
        return std::string();
    }
}



Transaction::Transaction(sqlite3* database):
    mDatabase(database),
    mIsCommited(false)
{
#ifndef __ANDROID__
    DataGateway::check(sqlite3_exec(mDatabase, "SAVEPOINT sp", nullptr, nullptr, nullptr));
#endif
}



Transaction::~Transaction()
{
#ifndef __ANDROID__
    if (!mIsCommited) {
        DataGateway::check(sqlite3_exec(mDatabase, "ROLLBACK TO sp", nullptr, nullptr, nullptr));
        DataGateway::check(sqlite3_exec(mDatabase, "RELEASE sp", nullptr, nullptr, nullptr));
    }
#endif
}



void Transaction::commit()
{
#ifndef __ANDROID__
    DataGateway::check(sqlite3_exec(mDatabase, "RELEASE sp", nullptr, nullptr, nullptr));
    mIsCommited = true;
#endif
}



class DataGateway::Private
{
public:
    DataGateway* q;
    std::shared_ptr<sqlite3> database;

    Private(DataGateway* q) : q(q) {}
    std::shared_ptr<sqlite3_stmt> prepareStatement(const std::string& sql);
    void upgrade(int oldVersion, int newVersion);
    tfw::ResultGroup extractResult(sqlite3_stmt* statement);
    CompareResult extractCompareResult(sqlite3_stmt* statement, double max = -1);
};



std::shared_ptr<sqlite3_stmt> DataGateway::Private::prepareStatement(const std::string& sql)
{
    sqlite3_stmt* statementPointer;
    check(sqlite3_prepare_v2(database.get(), sql.c_str(), static_cast<int>(sql.size()),
            &statementPointer, nullptr));
    return std::shared_ptr<sqlite3_stmt>(statementPointer, &sqlite3_finalize);
}



int DataGateway::check(int resultCode)
{
    switch (resultCode) {
    case SQLITE_OK:
    case SQLITE_ROW:
    case SQLITE_DONE:
        return resultCode;
    default:
        std::string errorString = sqlite3_errstr(resultCode);
        throw DatabaseException(errorString, resultCode);
    }
}



DataGateway::DataGateway():
    d(new Private(this))
{}



DataGateway::~DataGateway()
{}



void DataGateway::openLocalDatabase(const std::string& path)
{
    std::string dbPath = path + "/results.sqlite";
    create(dbPath);
    upgrade();
}



void DataGateway::saveSetting(const std::string& key, int value)
{
    std::shared_ptr<sqlite3_stmt> statement = d->prepareStatement(
            "INSERT OR REPLACE INTO Metadata VALUES(?, ?, ?)");
    bindString(statement.get(), 1, key, SQLITE_STATIC);
    check(sqlite3_bind_int(statement.get(), 2, value));
    check(sqlite3_bind_null(statement.get(), 3));
    check(sqlite3_step(statement.get()));
}



int DataGateway::loadSetting(const std::string& key, int defaultValue)
{
    std::shared_ptr<sqlite3_stmt> statement = d->prepareStatement(
            "SELECT number FROM Metadata WHERE key = ?");
    bindString(statement.get(), 1, key, SQLITE_STATIC);
    if (sqlite3_step(statement.get()) == SQLITE_ROW) {
        return sqlite3_column_int(statement.get(), 0);
    } else {
        return defaultValue;
    }
}



void DataGateway::clear()
{
    Transaction transaction(d->database.get());
    check(sqlite3_exec(d->database.get(), "DELETE FROM Results", nullptr, nullptr, nullptr));
    check(sqlite3_exec(d->database.get(), "DELETE FROM Sessions", nullptr, nullptr, nullptr));
    transaction.commit();
}



void DataGateway::addResults(const Session &session, const tfw::ResultGroup &resultGroup)
{
    std::shared_ptr<sqlite3_stmt> statement = d->prepareStatement(
            "INSERT OR REPLACE INTO Results VALUES (?, ?, ?, ?)");
    for (size_t j = 0; j < resultGroup.results().size(); ++j) {
        const tfw::Result& result = resultGroup.results().at(j);
        const std::string& resultId = result.resultId();
        check(sqlite3_reset(statement.get()));
        bindString(statement.get(), 1, resultId, SQLITE_STATIC);
        check(sqlite3_bind_int64(statement.get(), 2, session.sessionId()));
        check(sqlite3_bind_double(statement.get(), 3, result.score()));
        const std::string& json = resultGroup.toJsonString();
        bindString(statement.get(), 4, json, SQLITE_STATIC);
        check(sqlite3_step(statement.get()));
    }
}



void DataGateway::addSession(const Session &session)
{
    std::shared_ptr<sqlite3_stmt> statement = d->prepareStatement(
            "INSERT OR REPLACE INTO Sessions VALUES (?, ?, ?)");
    check(sqlite3_bind_int64(statement.get(), 1, session.sessionId()));
    check(sqlite3_bind_int(statement.get(), 2, session.isFinished()));
    std::string configurationName = session.configurationName();
    if (configurationName.empty()) {
        check(sqlite3_bind_null(statement.get(), 3));
    } else {
        bindString(statement.get(), 3, configurationName, SQLITE_STATIC);
    }
    check(sqlite3_step(statement.get()));
}



void DataGateway::addTestSelection(const std::string& testId, bool isSelected)
{
    std::shared_ptr<sqlite3_stmt> statement = d->prepareStatement(
            "INSERT OR REPLACE INTO TestSelections VALUES(?, ?)");
    bindString(statement.get(), 1, testId, SQLITE_STATIC);
    check(sqlite3_bind_int(statement.get(), 2, isSelected));
    check(sqlite3_step(statement.get()));
}



bool DataGateway::addServerMessage(
    const std::string &timestamp,
    const std::string &message)
{
    Transaction transaction(d->database.get());

    std::shared_ptr<sqlite3_stmt> select = d->prepareStatement(
            "SELECT * FROM ServerMessages WHERE message_timestamp = ?");
    bindString(select.get(), 1, timestamp, SQLITE_STATIC);
    bool found = (check(sqlite3_step(select.get())) == SQLITE_ROW);

    std::shared_ptr<sqlite3_stmt> insert = d->prepareStatement(
            "INSERT OR REPLACE INTO ServerMessages VALUES(?, ?)");
    bindString(insert.get(), 1, timestamp, SQLITE_STATIC);
    bindString(insert.get(), 2, message, SQLITE_STATIC);
    check(sqlite3_step(insert.get()));

    transaction.commit();
    return found;
}



std::vector<std::pair<std::string, bool> > DataGateway::getTestSelections()
{
    std::shared_ptr<sqlite3_stmt> statement = d->prepareStatement(
            "SELECT test_id, is_selected FROM TestSelections");
    std::vector<std::pair<std::string, bool> > result;
    while (check(sqlite3_step(statement.get())) == SQLITE_ROW) {
        std::string testId = columnString(statement.get(), 0);
        bool isSelected = sqlite3_column_int(statement.get(), 1) != 0;
        result.push_back(std::make_pair(testId, isSelected));
    }
    return result;
}



std::vector<Session> DataGateway::getSessions()
{
    std::shared_ptr<sqlite3_stmt> statement = d->prepareStatement(
            "SELECT * FROM Sessions ORDER BY session_id DESC");
    std::vector<Session> sessions;
    while (check(sqlite3_step(statement.get())) == SQLITE_ROW) {
        Session session;
        session.setSessionId(sqlite3_column_int64(statement.get(), 0));
        session.setFinished(sqlite3_column_int(statement.get(), 1) != 0);
        session.setConfigurationName(columnString(statement.get(), 2));
        sessions.push_back(std::move(session));
    }
    return sessions;
}



std::vector<tfw::ResultGroup> DataGateway::getResultsForSession(long long sessionId)
{
    std::shared_ptr<sqlite3_stmt> statement = d->prepareStatement(
            "SELECT ROWID, raw FROM Results WHERE session_id = ?");
    check(sqlite3_bind_int64(statement.get(), 1, sessionId));
    std::vector<tfw::ResultGroup> results;
    while (check(sqlite3_step(statement.get())) == SQLITE_ROW) {
        results.push_back(d->extractResult(statement.get()));
    }
    return results;
}



std::vector<tfw::ResultGroup> DataGateway::getBestResults()
{
    std::shared_ptr<sqlite3_stmt> statement = d->prepareStatement(
            "SELECT ROWID, raw, MAX(score) FROM Results GROUP BY result_id");
    std::vector<tfw::ResultGroup> results;
    while (check(sqlite3_step(statement.get())) == SQLITE_ROW) {
        results.push_back(d->extractResult(statement.get()));
    }
    return results;
}



std::vector<tfw::ResultGroup> DataGateway::getBestResults(std::string configurationName)
{
	std::shared_ptr<sqlite3_stmt> statement = d->prepareStatement(
		std::string("SELECT ROWID, raw, MAX(score) FROM Results WHERE instr(raw, '\"configuration\": \"") + configurationName + std::string("\"') > 0 GROUP BY result_id"));
	std::vector<tfw::ResultGroup> results;
	while (check(sqlite3_step(statement.get())) == SQLITE_ROW) {
		results.push_back(d->extractResult(statement.get()));
	}
	return results;
}



tfw::ResultGroup DataGateway::getResultForRowId(long long rowId)
{
    std::shared_ptr<sqlite3_stmt> statement = d->prepareStatement(
            "SELECT ROWID, raw FROM Results WHERE ROWID = ?");
    check(sqlite3_bind_int64(statement.get(), 1, rowId));
    tfw::ResultGroup result;
    while (check(sqlite3_step(statement.get())) == SQLITE_ROW) {
        result = d->extractResult(statement.get());
    }
    return result;
}



void DataGateway::openCompareDatabase(const std::string& path)
{
    std::string sql = "ATTACH '" + path + "' AS CompareDatabase";
    check(sqlite3_exec(d->database.get(), sql.c_str(), nullptr, nullptr, nullptr));
}



std::vector<CompareResult> DataGateway::getResultsById(
    const ResultItem &resultItem,
    const std::string &deviceFilter,
    bool hideDesktop)
{
    std::string max_sql =
            "SELECT max(result.score) "
            "FROM result JOIN device ON result.device_id = device._id "
            "WHERE (result.test_base = ?) AND (result.variant = ?) AND (result.sub_type = ?) ";
    if (hideDesktop) {
        max_sql += "AND (device.form_factor != 'desktop') ";
    }
    
    std::shared_ptr<sqlite3_stmt> max_statement = d->prepareStatement(max_sql);
    check(sqlite3_bind_text(max_statement.get(), 1, trimUnderscores(resultItem.baseId()).c_str(), -1,
                            SQLITE_TRANSIENT));
    check(sqlite3_bind_text(max_statement.get(), 2,
                            (resultItem.variantPostfix() == "_off") ? "off" : "on", -1, SQLITE_STATIC));
    check(sqlite3_bind_text(max_statement.get(), 3,
                            trimUnderscores(resultItem.resultPostfix()).c_str(), -1, SQLITE_TRANSIENT));
    
    double maxScore = -1;
    if(check(sqlite3_step(max_statement.get())) == SQLITE_ROW)
        maxScore = sqlite3_column_double(max_statement.get(), 0);
    
    std::string sql =
            "SELECT device.name, device.image, result.score, result.fps, result.api, "
            "        result.test_base, result.variant, result.sub_type, result._id "
            "FROM result JOIN device ON result.device_id = device._id "
            "WHERE (result.test_base = ?) AND (result.variant = ?) AND (result.sub_type = ?) "
            "AND (device.name LIKE '%' || ? || '%') ";
    if (hideDesktop) {
        sql += "AND (device.form_factor != 'desktop') ";
    }
    sql += "ORDER BY result.score DESC";

    std::shared_ptr<sqlite3_stmt> statement = d->prepareStatement(sql);
    check(sqlite3_bind_text(statement.get(), 1, trimUnderscores(resultItem.baseId()).c_str(), -1,
            SQLITE_TRANSIENT));
    check(sqlite3_bind_text(statement.get(), 2,
            (resultItem.variantPostfix() == "_off") ? "off" : "on", -1, SQLITE_STATIC));
    check(sqlite3_bind_text(statement.get(), 3,
            trimUnderscores(resultItem.resultPostfix()).c_str(), -1, SQLITE_TRANSIENT));
    bindString(statement.get(), 4, deviceFilter, SQLITE_STATIC);
    std::vector<CompareResult> compareResults;
    while (check(sqlite3_step(statement.get())) == SQLITE_ROW) {
        compareResults.push_back(d->extractCompareResult(statement.get(), maxScore));
    }
    return compareResults;
}



std::vector<CompareResult> DataGateway::getResultsByDevice(
        const std::string &api,
        const std::string &deviceId)
{
    std::vector<CompareResult> compareResults;
    std::shared_ptr<sqlite3_stmt> statement = d->prepareStatement(
            "SELECT device.name, device.image, result.score, result.fps, result.api, "
            "        result.test_base, result.variant, result.sub_type, result._id "
            "FROM result JOIN device ON result.device_id = device._id "
            "WHERE result.api = ? AND device.name = ?");

    bindString(statement.get(), 1, api, SQLITE_STATIC);
    bindString(statement.get(), 2, deviceId, SQLITE_STATIC);
    while (check(sqlite3_step(statement.get())) == SQLITE_ROW) {
        compareResults.push_back(d->extractCompareResult(statement.get()));
    }
    return compareResults;
}



std::string DataGateway::getResource(const std::string& name)
{
    int size = 0;
    const char* data = ::getResource(name.c_str(), &size);
    if (data == nullptr) {
        throw DatabaseException("Cannot load resource: " + name, 0);
    }
    return std::string(data, size);
}



tfw::Descriptor DataGateway::getDescriptorByTestId(const std::string& testId)
{
	int size = 0;
	const char* data = ::getResource( (std::string("config/") + testId + ".json").c_str(), &size);
	if (data == nullptr) {
		throw DatabaseException("Cannot load resource: " + testId, 0);
	}
    tfw::Descriptor descriptor;
    descriptor.fromJsonString(std::string(data));
    return descriptor;
}



sqlite3* DataGateway::database()
{
    return d->database.get();
}



void DataGateway::create(const std::string& path)
{
    sqlite3* databasePointer = nullptr;
    check(sqlite3_open(path.c_str(), &databasePointer));
    d->database = std::shared_ptr<sqlite3>(databasePointer, &sqlite3_close);
    check(sqlite3_exec(d->database.get(), "PRAGMA foreign_keys = ON", nullptr, nullptr, nullptr));
}



void DataGateway::upgrade()
{
    Transaction transaction(d->database.get());

    int oldVersion = 0;
    try {
        std::shared_ptr<sqlite3_stmt> statement = d->prepareStatement(
            "SELECT number FROM Metadata WHERE key = 'version'");
        if (sqlite3_step(statement.get()) == SQLITE_ROW) {
            oldVersion = sqlite3_column_int(statement.get(), 0);
        }
    } catch (const DatabaseException&) {
        /* Ignore */
    }

    if (oldVersion <= 0) {
        check(sqlite3_exec(d->database.get(),
                "CREATE TABLE IF NOT EXISTS 'Metadata' ( "
                "    'key' TEXT PRIMARY KEY NOT NULL, "
                "    'number' INTEGER, "
                "    'text' TEXT "
                ")", nullptr, nullptr, nullptr));
        check(sqlite3_exec(d->database.get(),
                "CREATE TABLE IF NOT EXISTS 'Sessions' ( "
                "    'session_id' INTEGER PRIMARY KEY NOT NULL, "
                "    'is_finished' BOOLEAN NOT NULL, "
                "    'configuration_name' TEXT NOT NULL "
                ")", nullptr, nullptr, nullptr));
        check(sqlite3_exec(d->database.get(),
                "CREATE TABLE IF NOT EXISTS 'Results' ( "
                "    'result_id' TEXT NOT NULL, "
                "    'session_id' INTEGER NOT NULL, "
                "    'score' DOUBLE NOT NULL, "
                "    'raw' TEXT NOT NULL, "
                "    PRIMARY KEY(result_id, session_id), "
                "    FOREIGN KEY(session_id) REFERENCES Sessions(session_id) "
                ")", nullptr, nullptr, nullptr));
    }
    if (oldVersion <= 1) {
        check(sqlite3_exec(d->database.get(),
                "CREATE TABLE IF NOT EXISTS 'TestSelections' ( "
                "    'test_id' TEXT PRIMARY KEY NOT NULL, "
                "    'is_selected' INTEGER "
                ")", nullptr, nullptr, nullptr));
    }
    if (oldVersion <= 3) {
        check(sqlite3_exec(d->database.get(),
                "CREATE TABLE IF NOT EXISTS 'ServerMessages' ( "
                "    'message_timestamp' TEXT PRIMARY KEY NOT NULL, "
                "    'message' TEXT "
                ")", nullptr, nullptr, nullptr));
    }

    if (oldVersion <= 2) {
        sqlite3_stmt* statementPointer;
        check(sqlite3_prepare_v2(d->database.get(), "SELECT session_id, raw FROM Results", -1,
                &statementPointer, nullptr));
        std::shared_ptr<sqlite3_stmt> statement(statementPointer, &sqlite3_finalize);

        std::vector<std::pair<Session, tfw::ResultGroup> > resultGroups;
        while (check(sqlite3_step(statement.get()) == SQLITE_ROW)) {
            long long sessionId = sqlite3_column_int64(statement.get(), 0);
            Session session;
            session.setSessionId(sessionId);

            std::string raw = columnString(statement.get(), 0);
            tfw::Result result;
            result.fromJsonString(raw);

            tfw::ResultGroup resultGroup;
            resultGroup.addResult(result);
            resultGroups.push_back(std::make_pair(session, resultGroup));
        }
        for (size_t i = 0; i < resultGroups.size(); ++i) {
            addResults(resultGroups[i].first, resultGroups[i].second);
        }
    }
    transaction.commit();

    Transaction transaction2(d->database.get());

    std::shared_ptr<sqlite3_stmt> insertMetadata = d->prepareStatement(
            "INSERT OR REPLACE INTO Metadata VALUES(?, ?, ?)");
    check(sqlite3_bind_text(insertMetadata.get(), 1, "version", -1, SQLITE_STATIC));
    check(sqlite3_bind_int(insertMetadata.get(), 2, CURRENT_DATABASE_VERSION));
    check(sqlite3_bind_null(insertMetadata.get(), 3));
    check(sqlite3_step(insertMetadata.get()));

    transaction2.commit();
}



tfw::ResultGroup DataGateway::Private::extractResult(sqlite3_stmt* statement)
{
    std::string raw = columnString(statement, 1);
    tfw::ResultGroup result;
    result.setRowId(sqlite3_column_int64(statement, 0));
    std::string error;
    tfw::ResultGroup::fromJsonString(raw, &result, &error);
    return result;
}



CompareResult DataGateway::Private::extractCompareResult(sqlite3_stmt* statement, double max)
{
    CompareResult compareResult;
    compareResult.setDeviceName(columnString(statement, 0));
    compareResult.setDeviceImage(columnString(statement, 1));
    compareResult.setScore(sqlite3_column_double(statement, 2));
    compareResult.setMaxScore(max >= 0 ? max : sqlite3_column_double(statement, 2));
    compareResult.setFps(sqlite3_column_double(statement, 3));
    compareResult.setApi(columnString(statement, 4));
    compareResult.setTestBase(columnString(statement, 5));
    compareResult.setVariant(columnString(statement, 6));
    compareResult.setSubType(columnString(statement, 7));
    compareResult.setRowId(sqlite3_column_int64(statement, 8));
    return compareResult;
}
