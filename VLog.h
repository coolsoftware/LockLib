//////////////////////////////////////////////////////////////////
//
// Copyright 2013-14 CoolSoftware. http://blog.coolsoftware.ru/
//
// Simple logging.
//
// You can freely use this program and code for your needs.
//
// Please, don't remove this copyright.
//
//////////////////////////////////////////////////////////////////

#pragma once

#ifndef V_LOG_H_
#define V_LOG_H_

//-----------------------------------------------------------------------------

#include <windows.h>
#include <string>
#include <tchar.h>

//-----------------------------------------------------------------------------

namespace VLog
{
	// This is our simple logging function. It is styled to work similarly to printf.

#ifdef _UNICODE
	void LogErr(const _TCHAR * file, const _TCHAR * function, int line, const char * msg, ...);
	void LogErrDateTime(const _TCHAR * file, const _TCHAR * function, int line, const char * msg, ...);
	void Log(const char * msg, ...);
	void LogDateTime(const char * msg, ...);
	void LogDebugOutput(const char * msg, ...);
	void Setup(const char * szLogFileName, bool bLogOverwrite, bool bLogDebugOutput);
#endif

	void LogErr(const _TCHAR * file, const _TCHAR * function, int line, const _TCHAR * msg, ...);
	void LogErrDateTime(const _TCHAR * file, const _TCHAR * function, int line, const _TCHAR * msg, ...);
	void Log(const _TCHAR * msg, ...);
	void LogDateTime(const _TCHAR * msg, ...);
	void LogDebugOutput(const _TCHAR * msg, ...);
	void DumpData(UINT16 count, UINT8 * data);
	void Setup(const _TCHAR * szLogFileName, bool bLogOverwrite, bool bLogDebugOutput);
	void Cleanup();
	void Overwrite();
	void DebugOutput();
}

//-----------------------------------------------------------------------------

// __VA_ARGS__ is only supported on VS2005 and above! We use a macro to cut down
// on having to type out the file, function, and line preprocessor macros each time.
#define LOGERR(msg, ...) VLog::LogErr(_T(__FILE__), _T(__FUNCTION__), __LINE__, msg, __VA_ARGS__)
#define LOGERRDT(msg, ...) VLog::LogErrDateTime(_T(__FILE__), _T(__FUNCTION__), __LINE__, msg, __VA_ARGS__)
#define LOG(msg, ...) VLog::Log(msg, __VA_ARGS__)
#define LOGDT(msg, ...) VLog::LogDateTime(msg, __VA_ARGS__)
#define DEBUGOUTPUT(msg, ...) VLog::LogDebugOutput(msg, __VA_ARGS__)
#define LOGDUMP(count, data) VLog::DumpData(count, data)

//-----------------------------------------------------------------------------

#endif //V_LOG_H_
