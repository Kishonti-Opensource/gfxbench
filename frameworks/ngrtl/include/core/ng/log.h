/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef NG_LOG_INCLUDED
#define NG_LOG_INCLUDED

#include <vector>

#include "ng/ngrtl_core_export.h"
#include "ng/macros.h"
#include "ng/format.h"

/* *** BASIC LOGGING ****

1. Simple Usage (print only to console):
Simply use the following macros to print log message to screen:

NGLOG_FATAL, NGLOG_ERROR, NGLOG_WARN, NGLOG_INFO, NGLOG_DEBUG, NGLOG_TRACE

These macros can be used like list

	NGLOG_ERROR("Print any variable %s or %s and don't add endline", 32.31, "doggydoggy");

Use %s for all types of arguments. No other formatting options are supported.

This log.h file is can be configured with macros to not to compile
certain NGLOG_... calls into the object file (for example, for release builds)

- if neither NGLOG_ENABLE nor _DEBUG is defined, nothing is compiled (NGLOG
     messages resolve to ((void)0) )

- if only _DEBUG is defined, everything will be compiled

- if NGLOG_ENABLE is defined to a value NGLOG_SEVERITY_XXX, where XXX is one of the following
	ANY, TRACE, DEBUG, INFO, WARN, ERROR, FATAL, NONE
	then only the NGLOG_XXX messages of that or stronger level will be compiled

2. Advanced Usage

Example: log to file and cout. To cout log only fatal and error messages.

	1. Create your logger and sink objects and connect them

		ng::Logger myLog; //the main logger class
		ng::Logger::setTheGlobal(&myLog); //will be the active one

		//create sink that printfs errors or fatal errors
		ng::LogSinkPrintf myPrintfSink;
		myPrintfSink.setMinSeverity(ng::log::Severity::error);

		//create sink that writes everything into a file
		ng::LogSinkFile myFileSink("c:/tmp/log.txt");

		//connect them to the logger
		myLog.addSink(&myPrintfSink);
		myLog.addSink(&myFileSink);


	2. Use the logging macros as above

		NGLOG_WARN("this one goes to the file only");
		NGLOG_ERROR("this one goes to the file and console");

	You can also write your own sinks, inherit from ng::log::Sink

*/

//If NGLOG_ENABLE not defined
//(1) log everything if _DEBUG
//(2) else log INFO, WARN, ERROR, FATAL
#ifndef NGLOG_ENABLE
#ifdef _DEBUG
#define NGLOG_ENABLE NGLOG_SEVERITY_ANY
#else
#define NGLOG_ENABLE NGLOG_SEVERITY_INFO
#endif
#endif

#define NGLOG_SEVERITY_ANY 0
#define NGLOG_SEVERITY_TRACE 1
#define NGLOG_SEVERITY_DEBUG 2
#define NGLOG_SEVERITY_INFO 3
#define NGLOG_SEVERITY_WARN 4
#define NGLOG_SEVERITY_ERROR 5
#define NGLOG_SEVERITY_FATAL 6
#define NGLOG_SEVERITY_NONE 7

#if NGLOG_ENABLE <= NGLOG_SEVERITY_TRACE
#define NGLOG_TRACE(x, ...) ::ng::Logger::log(::ng::log::Severity::trace, FORMATCSTR(x, __VA_ARGS__))
#else
#define NGLOG_TRACE(x, ...) ((void)0)
#endif

#if NGLOG_ENABLE <= NGLOG_SEVERITY_DEBUG
#define NGLOG_DEBUG(x, ...) ::ng::Logger::log(::ng::log::Severity::debug, FORMATCSTR(x, __VA_ARGS__))
#else
#define NGLOG_DEBUG(x, ...) ((void)0)
#endif

#if NGLOG_ENABLE <= NGLOG_SEVERITY_INFO
#define NGLOG_INFO(x, ...) ::ng::Logger::log(::ng::log::Severity::info, FORMATCSTR(x, __VA_ARGS__))
#else
#define NGLOG_INFO(x, ...) ((void)0)
#endif

#if NGLOG_ENABLE <= NGLOG_SEVERITY_WARN
#define NGLOG_WARN(x, ...) ::ng::Logger::log(::ng::log::Severity::warn, FORMATCSTR(x, __VA_ARGS__))
#else
#define NGLOG_WARN(x, ...) ((void)0)
#endif

#if NGLOG_ENABLE <= NGLOG_SEVERITY_ERROR
#define NGLOG_ERROR(x, ...) ::ng::Logger::log(::ng::log::Severity::error, FORMATCSTR(x, __VA_ARGS__))
#else
#define NGLOG_ERROR(x, ...) ((void)0)
#endif

#if NGLOG_ENABLE <= NGLOG_SEVERITY_FATAL
#define NGLOG_FATAL(x, ...) ::ng::Logger::log(::ng::log::Severity::fatal, FORMATCSTR(x, __VA_ARGS__))
#else
#define NGLOG_FATAL(x, ...) ((void)0)
#endif


namespace ng { namespace log {

//log severity
ENUM_CLASS(Severity)
{
	any = NGLOG_SEVERITY_ANY,
	trace = NGLOG_SEVERITY_TRACE,
	debug = NGLOG_SEVERITY_DEBUG,
	info = NGLOG_SEVERITY_INFO,
	warn = NGLOG_SEVERITY_WARN,
	error = NGLOG_SEVERITY_ERROR,
	fatal = NGLOG_SEVERITY_FATAL,
	none = NGLOG_SEVERITY_NONE
}; ENUM_CLASS_END

class Sink;

class NGRTL_EXPORT Logger
{
public:

	Logger();
	~Logger();
	static void log(ENUM(Severity) sev, const char* msg)
	{
		//we need this here in the header to use the value of NGLOG_ENABLE set for the client of the library
		//and not the value set when library was built
		if ( sev >= NGLOG_ENABLE )
			logCore(sev, msg);
	}

	void addSink(Sink* sink); //weak ptr

	//static methods
	static void setTheGlobal(Logger* logger);
	static Logger* theGlobal();

private:
	typedef std::vector<Sink*> Sinks; //weak ptrs
	Sinks _sinks;
	void *_mutex;

	static Logger* _theGlobal;

	static void logCore(ENUM(Severity) sev, const char* msg); //logs a message of severity sev
};

//assist in lazy construction of message
//Use this when you write your own sinks
//The input message is supplied in Message object to the sink
//The sink must extract the string from the message
//Default: it uses the full() string. But In some cases you forward the
//log message to a 3rd party logger device which may expect severity and baseMsg strings separately.
//In that case you may call the base() or fullButSeverity() and severity() methods
class NGRTL_EXPORT Message
{
public:
	Message(ENUM(Severity) sev, const char* baseMsg);
	~Message();

	const char* base() const //return the baseMsg argument of the ctor
		{ return _baseMsg; }
	const ENUM(Severity)& severity() const //return the severity argument of the ctor
		{ return _sev; }

	const char* full() const; //return full decorated message
	const char* fullButSeverity() const; //return full decorated message without the severity string

private:
	const char* _baseMsg;
	ENUM(Severity) _sev;
	mutable std::string* _fullMsg;
};

//Base class of all sinks
class NGRTL_EXPORT Sink
{
public:
	Sink();

	void log(const Message& lm);
	void setMinSeverity(ENUM(Severity) sev);

	virtual ~Sink() {}
private:
	virtual void doLog(const Message& lm) = 0;

	ENUM(Severity) _minSev;
};

} //namespace log

//lift some definitions up to namespace ng
using log::Logger;

class NGRTL_EXPORT LogSinkPrintf
	: public log::Sink
{
private:
	virtual void doLog(const log::Message& lm);
};

class NGRTL_EXPORT LogSinkString
	: public log::Sink
{
public:
	LogSinkString()
	: _lineSeparator("\n")
	{}
	LogSinkString(const char* lineSeparator)
	: _lineSeparator(lineSeparator)
	{}
	const char* c_str() const { return _buf.c_str(); }
	const std::string& string() const { return _buf; }
	void clear() { _buf.clear(); }
private:
	std::string _lineSeparator;
	std::string _buf;
	
	virtual void doLog(const log::Message& lm);
};

class NGRTL_EXPORT LogSinkCout
	: public log::Sink
{
private:
	virtual void doLog(const log::Message& lm);
};

class NGRTL_EXPORT LogSinkCerr
	: public log::Sink
{
private:
	virtual void doLog(const log::Message& lm);
};

class NGRTL_EXPORT LogSinkFile
	: public log::Sink
{
public:
	LogSinkFile() {}
	LogSinkFile(const char* fileName, bool bTruncate = true);
	void init(const char* fileName, bool bTruncate = true);

private:
	virtual void doLog(const log::Message& lm);

	void openForWtAndClose();
	std::string _fileName;
};

#ifdef _WIN32
class NGRTL_EXPORT LogSinkWindowsDebug
    : public log::Sink
{
private:
    virtual void doLog(const log::Message& lm);
};
#endif

#ifdef PLATFORM_ANDROID
class NGRTL_EXPORT LogSinkLogcat
	: public log::Sink
{
private:
	virtual void doLog(const log::Message& lm);
	int severity(ENUM(ng::log::Severity) value);
};
#endif

#if defined NACL
class NGRTL_EXPORT LogSinkNaCl
	: public log::Sink
{
private:
	virtual void doLog(const log::Message& lm);
};
#endif

} // namespace ng

#endif //NG_LOG_INCLUDED

