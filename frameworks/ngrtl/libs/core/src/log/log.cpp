/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifdef NGLOG_ENABLE
#undef NGLOG_ENABLE
#endif
#define NGLOG_ENABLE NGLOG_SEVERITY_ANY

#include "ng/log.h"

#include <iostream>
#include <stdio.h>

#include "ng/mutex.h"

#if ANDROID
#include <android/log.h>
#endif

#ifdef NACL
#include <ppapi/utility/completion_callback_factory.h>
#include <ppapi/cpp/instance.h>
#include <ppapi/cpp/module.h>
#endif

#ifdef _WIN32
#include <Windows.h>
#endif

namespace ng { namespace log {


Logger::Logger(): _mutex(new mutex)
{}

Logger::~Logger()
{
	delete (mutex*)_mutex;
}

Logger* Logger::
	_theGlobal(NULL);

void Logger::
	setTheGlobal(Logger* logger)
{
	_theGlobal = logger;
}

Logger* Logger::
	theGlobal()
{
	return _theGlobal;
}

static const char* getSeverityString(ENUM(Severity) sev)
{
	switch(sev)
	{
	case Severity::any:
		return "ANY";
	case Severity::none:
		return "NONE";
	case Severity::fatal:
		return "FATAL";
	case Severity::error:
		return "ERROR";
	case Severity::warn:
		return "WARN ";
	case Severity::info:
		return "INFO ";
	case Severity::debug:
		return "DEBUG";
	case Severity::trace:
		return "TRACE";
	}
	return "UNKNW";
}

void Logger::
	logCore(ENUM(Severity) sev, const char* msg)
{
	//todo: add line header like date/time into Message

	Message logMsg(sev, msg);
	if ( _theGlobal == 0 || _theGlobal->_sinks.empty() )
	{
		static mutex s_mutex;
		unique_lock<mutex> lock(s_mutex);
		printf("%s\n", logMsg.full());
		fflush(stdout);
	} else
	{
		unique_lock<mutex> lock(*(mutex*)_theGlobal->_mutex);
		for(Sinks::iterator it = _theGlobal->_sinks.begin(); it != _theGlobal->_sinks.end(); ++it)
			(*it)->log(logMsg);
	}
}

void Logger::
	addSink(Sink* sink)
{
	_sinks.push_back(sink);
}

Message::
	Message(ENUM(Severity) sev, const char* msg)
	: _baseMsg(msg)
	, _sev(sev)
	, _fullMsg(nullptr)
{}

Message::
	~Message()
{
	delete _fullMsg;
}

const char* Message::
	full() const
{
	if ( !_fullMsg )
	{
		_fullMsg = new std::string;
		*_fullMsg += "[";
		*_fullMsg += getSeverityString(_sev);
		*_fullMsg += "]: ";
		*_fullMsg += _baseMsg;
	}
	return _fullMsg->c_str();
}

const char* Message::
	fullButSeverity() const
{
	return _baseMsg;
}

void Sink::
	log(const Message& lm)
{
	if ( lm.severity() >= _minSev )
		doLog(lm);
}
}

void LogSinkPrintf::
	doLog(const log::Message& lm)
{
	printf("%s\n", lm.full());
	fflush(stdout);
}

void LogSinkString::
	doLog(const log::Message& lm)
{
	_buf += lm.full();
	_buf += _lineSeparator;
}

void LogSinkCout::
	doLog(const log::Message& lm)
{
	std::cout << lm.full() << std::endl;
	std::cout.flush();
}

void LogSinkCerr::
	doLog(const log::Message& lm)
{
	std::cerr << lm.full() << std::endl;
	std::cerr.flush();
}

LogSinkFile::
	LogSinkFile(const char* fileName, bool bTruncate)
	: _fileName(fileName)
{
	if ( bTruncate )
		openForWtAndClose();
}

void LogSinkFile::
	init(const char* fileName, bool bTruncate)
{
	_fileName = fileName;
	if ( bTruncate )
		openForWtAndClose();
}

void LogSinkFile::
	doLog(const log::Message& lm)
{
	FILE* f = fopen(_fileName.c_str(), "at");
	if ( f != NULL )
	{
		fprintf(f, "%s\n", lm.full());
		fclose(f);
	}
}

void LogSinkFile::
	openForWtAndClose()
{
	FILE* f = fopen(_fileName.c_str(), "wt");
	if ( f != NULL )
		fclose(f);
}

#ifdef _WIN32

void LogSinkWindowsDebug::
doLog(const log::Message& lm)
{
#ifdef _DEBUG
    if (IsDebuggerPresent())
        OutputDebugStringA((std::string(lm.full()) + '\n').c_str());
    else
#endif
        LogSinkPrintf().log(lm);
}

typedef LogSinkWindowsDebug DefaultSink;
#elif defined ANDROID

int LogSinkLogcat::
	severity(ENUM(ng::log::Severity) value)
{
	switch(value)
	{
	case log::Severity::fatal:
		return ANDROID_LOG_FATAL;
	case log::Severity::error:
		return ANDROID_LOG_ERROR;
	case log::Severity::warn:
		return ANDROID_LOG_WARN;
	case log::Severity::info:
		return ANDROID_LOG_INFO;
	case log::Severity::debug:
		return ANDROID_LOG_DEBUG;
	case log::Severity::trace:
		return ANDROID_LOG_VERBOSE;
	default:
		return ANDROID_LOG_DEFAULT;
	}
}

void LogSinkLogcat::
doLog(const log::Message& lm)
{
	const char *tag = "_logcat_";

	const char *end = lm.fullButSeverity();
	do
	{
		const char *begin = end;
		while (*end && *end != '\n')
			++end;

		std::string line(begin, end);
		__android_log_print(severity(lm.severity()), tag, "%s", line.c_str());
	}
	while (*end++);
}

typedef LogSinkLogcat DefaultSink;
#elif defined NACL

void NaClLogOnMainThreadFunc(void* data, int32_t result)
{
	pp::Module::InstanceMap instances = pp::Module::Get()->current_instances();
	pp::Module::InstanceMap::iterator iter = instances.begin();
	if (iter!=instances.end())
	{
		iter->second->LogToConsole((PP_LogLevel)result,pp::Var((char*)data));
	}
	delete[] (char*)data;
}

void LogSinkNaCl::
	doLog(const log::Message& lm)
{
	PP_LogLevel level;
	switch(lm.severity())
	{
	case log::Severity::fatal:
	case log::Severity::error:
		level = PP_LOGLEVEL_ERROR;
		break;
	case log::Severity::warn:
		level = PP_LOGLEVEL_WARNING;
		break;
	case log::Severity::info:
	case log::Severity::debug:
		level = PP_LOGLEVEL_LOG;
		break;
	case log::Severity::trace:
	default:
		level = PP_LOGLEVEL_TIP;
	}
	
	char* buffer;
	const char* src = lm.full();
	buffer = new char[strlen(src)+1];
	strcpy(buffer,src);
	pp::Module::Get()->core()->CallOnMainThread(0,pp::CompletionCallback(&NaClLogOnMainThreadFunc,buffer),level);
}

typedef LogSinkNaCl DefaultSink;
#else
typedef LogSinkPrintf DefaultSink;
#endif


#ifndef NGLOG_DISABLE_DEFAULT_LOGGER
// default logger instance
struct DefaultLogger : public Logger {
	DefaultLogger()
	{
		addSink(&sink_);
		Logger::setTheGlobal(this);
	}
	DefaultSink sink_;
};
static DefaultLogger _defaultLogger;
#endif

namespace log {
Sink::
	Sink()
	: _minSev(Severity::any)
{
}

void Sink::
	setMinSeverity(ENUM(Severity) sev)
{
	_minSev = sev;
}
}

}
