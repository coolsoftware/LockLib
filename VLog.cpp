//////////////////////////////////////////////////////////////////
//
// Copyright 2013 CoolSoftware. http://blog.coolsoftware.ru/
//
// Simple logging.
//
// You can freely use this program and code for your needs.
//
// Please, don't remove this copyright.
//
//////////////////////////////////////////////////////////////////

#include "VLog.h"
#include <time.h>
#include "VTime.h"
#include "VLock.h"

#pragma warning(disable : 4996)

//---------------------------------------------------------------------------

//LockRead(30**)
//LockWrite(31**)
//Lock(32**)

//-----------------------------------------------------------------------------

// Implement a simple logging mechanism. I still prefer the printf way of 
// building custom strings rather than the << method, so that is why I have
// here. Feel free to use your own system though!
namespace VLog
{
	// We need to synchronize access to the logging function, so it should only
	// be used to report unexpected errors and not log general information.
	VLock logLock;

	volatile LONG logInitialized = 0;
	
	_TCHAR logFileNameBuf[MAX_PATH];
	_TCHAR *logFileName = NULL;

	bool logOverwrite = false;
	bool logOverwritten = false;
	bool logDebugOutput = false;

	// This is our simple logging function. It is styled to work similarly to printf.

#ifdef _DEBUG
#define OUTPUT_DEBUG_STRING(buffer) { OutputDebugString(buffer); }
#define OUTPUT_DEBUG_LINE(buffer) { size_t l = _tcslen(buffer); if (l > 0 && l < sizeof(buffer)-1) { buffer[l] = '\n'; buffer[l+1] = '\0'; OutputDebugString(buffer); buffer[l] = '\0'; } }
#endif

#ifdef _UNICODE
	void LogErr(const _TCHAR * file, const _TCHAR * function, int line, const char * msg, ...)
	{
		if (!logInitialized) return;
		logLock.Lock(3211);
		__try
		{
			static char abuffer[8192];
			static _TCHAR buffer[8192+8];
			va_list args;
			va_start(args, msg);
			_vsnprintf_s(abuffer, 8192, 8191, msg, args);
			va_end(args);
			MultiByteToWideChar(CP_ACP, 0, abuffer, -1, buffer, 8192);
#ifdef _DEBUG
			OUTPUT_DEBUG_LINE(buffer);
#endif
			if (logFileName)
			{
				FILE * f;
				if (logOverwrite)
				{
					if (!logOverwritten)
					{
						f = _tfopen(logFileName, _T("wb"));
						logOverwritten = true;
					} else
						f = _tfopen(logFileName, _T("ab"));
					logOverwrite = false;
				} else
					f = _tfopen(logFileName, _T("ab"));
				if (f)
				{
#ifdef _DEBUG
					_ftprintf(f, _T("File: %s\nFunction: %s\nLine: %i\nError: %s\n"), file, function, line, buffer);
#else
					_ftprintf(f, _T("Error: %s\n"), buffer);
#endif
					fclose(f);
				}
			} else {
#ifdef _DEBUG
				_tprintf(_T("File: %s\nFunction: %s\nLine: %i\nError: %s\n"), file, function, line, buffer);
#else
				_tprintf(_T("Error: %s\n"), buffer);
#endif
			}
		}
		__except(EXCEPTION_EXECUTE_HANDLER)
		{
			//nothing to do
		}
		logLock.Unlock();
	}

	void LogErrDateTime(const _TCHAR * file, const _TCHAR * function, int line, const char * msg, ...)
	{
		if (!logInitialized) return;
		logLock.Lock(3212);
		__try
		{
			static char abuffer[8192];
			static _TCHAR buffer[8192+8];
			abuffer[0] = '[';
			_strdatetime_s(&abuffer[1], 18);
			abuffer[18] = ']';
			abuffer[19] = ' ';
			va_list args;
			va_start(args, msg);
			_vsnprintf_s(&abuffer[20], 8192-20, 8191-20, msg, args); //20 chars at the start of buffer is [mm/dd/yy hh:mm:ss] with space at the end
			va_end(args);
			MultiByteToWideChar(CP_ACP, 0, abuffer, -1, buffer, 8192);
#ifdef _DEBUG
			OUTPUT_DEBUG_LINE(buffer);
#endif
			if (logFileName)
			{
				FILE * f;
				if (logOverwrite)
				{
					if (!logOverwritten)
					{
						f = _tfopen(logFileName, _T("wb"));
						logOverwritten = true;
					} else
						f = _tfopen(logFileName, _T("ab"));
					logOverwrite = false;
				} else
					f = _tfopen(logFileName, _T("ab"));
				if (f)
				{
#ifdef _DEBUG
					_ftprintf(f, _T("File: %s\nFunction: %s\nLine: %i\nError: %s\n"), file, function, line, buffer);
#else
					_ftprintf(f, _T("Error: %s\n"), buffer);
#endif
					fclose(f);
				}
			} else {
#ifdef _DEBUG
				_tprintf(_T("File: %s\nFunction: %s\nLine: %i\nError: %s\n"), file, function, line, buffer);
#else
				_tprintf(_T("Error: %s\n"), buffer);
#endif
			}
		}
		__except(EXCEPTION_EXECUTE_HANDLER)
		{
			//nothing to do
		}
		logLock.Unlock();
	}

	void Log(const char * msg, ...)
	{
		if (!logInitialized) return;
		logLock.Lock(3213);
		__try
		{
			static char abuffer[8192];
			static _TCHAR buffer[8192+8];
			va_list args;
			va_start(args, msg);
			_vsnprintf_s(abuffer, 8192, 8191, msg, args);
			va_end(args);
			MultiByteToWideChar(CP_ACP, 0, abuffer, -1, buffer, 8192);
#ifdef _DEBUG
			OUTPUT_DEBUG_STRING(buffer);
#endif
			if (logFileName)
			{
				FILE * f;
				if (logOverwrite)
				{
					if (!logOverwritten)
					{
						f = _tfopen(logFileName, _T("wb"));
						logOverwritten = true;
					} else
						f = _tfopen(logFileName, _T("ab"));
					logOverwrite = false;
				} else
					f = _tfopen(logFileName, _T("ab"));
				if (f)
				{
					_ftprintf(f, _T("%s"), buffer);
					fclose(f);
				}
			} else {
				_tprintf(_T("%s"), buffer);
			}
		}
		__except(EXCEPTION_EXECUTE_HANDLER)
		{
			//nothing to do
		}
		logLock.Unlock();
	}

	void LogDateTime(const char * msg, ...)
	{
		if (!logInitialized) return;
		logLock.Lock(3214);
		__try
		{
			static char abuffer[8192];
			static _TCHAR buffer[8192+8];
			abuffer[0] = '[';
			_strdatetime_s(&abuffer[1], 18);
			abuffer[18] = ']';
			abuffer[19] = ' ';
			va_list args;
			va_start(args, msg);
			_vsnprintf_s(&abuffer[20], 8192-20, 8191-20, msg, args); //20 chars at the start of buffer is [mm/dd/yy hh:mm:ss] with space at the end
			va_end(args);
			MultiByteToWideChar(CP_ACP, 0, abuffer, -1, buffer, 8192);
#ifdef _DEBUG
			OUTPUT_DEBUG_LINE(buffer);
#endif
			if (logFileName)
			{
				FILE * f;
				if (logOverwrite)
				{
					if (!logOverwritten)
					{
						f = _tfopen(logFileName, _T("wb"));
						logOverwritten = true;
					} else
						f = _tfopen(logFileName, _T("ab"));
					logOverwrite = false;
				} else
					f = _tfopen(logFileName, _T("ab"));
				if (f)
				{
					_ftprintf(f, _T("%s\n"), buffer);
					fclose(f);
				}
			} else {
				_tprintf(_T("%s\n"), buffer);
			}
		}
		__except(EXCEPTION_EXECUTE_HANDLER)
		{
			//nothing to do
		}
		logLock.Unlock();
	}

	void LogDebugOutput(const char * msg, ...)
	{
		if (!logDebugOutput || !logDebugOutput) return;
		logLock.Lock(3215);
		__try
		{
			static char abuffer[8192];
			static _TCHAR buffer[8192+8];
			va_list args;
			va_start(args, msg);
			_vsnprintf_s(abuffer, 8192, 8191, msg, args);
			va_end(args);
			MultiByteToWideChar(CP_ACP, 0, abuffer, -1, buffer, 8192);
			OutputDebugString(buffer);
		}
		__except(EXCEPTION_EXECUTE_HANDLER)
		{
			//nothing to do
		}
		logLock.Unlock();
	}

	void Setup(const char * szLogFileName, bool bLogOverwrite, bool bLogDebugOutput)
	{
		if (szLogFileName)
		{
			logFileName = logFileNameBuf;
			mbstowcs(logFileName, szLogFileName, MAX_PATH); 
			logFileName[MAX_PATH-1] = '\0';
		} else {
			logFileName = NULL; 
		}
		logOverwrite = bLogOverwrite;
		logDebugOutput = bLogDebugOutput;
		InterlockedIncrement(&logInitialized);
	}
#endif //_UNICODE

	void LogErr(const _TCHAR * file, const _TCHAR * function, int line, const _TCHAR * msg, ...)
	{
		if (!logInitialized) return;
		logLock.Lock(3201);
		__try
		{
			static _TCHAR buffer[8192+8];
			va_list args;
			va_start(args, msg);
			_vsntprintf_s(buffer, 8192, 8191, msg, args);
			va_end(args);
#ifdef _DEBUG
			OUTPUT_DEBUG_LINE(buffer);
#endif
			if (logFileName)
			{
				FILE * f;
				if (logOverwrite)
				{
					if (!logOverwritten)
					{
						f = _tfopen(logFileName, _T("wb"));
						logOverwritten = true;
					} else
						f = _tfopen(logFileName, _T("ab"));
					logOverwrite = false;
				} else
					f = _tfopen(logFileName, _T("ab"));
				if (f)
				{
#ifdef _DEBUG
					_ftprintf(f, _T("File: %s\nFunction: %s\nLine: %i\nError: %s\n"), file, function, line, buffer);
#else
					_ftprintf(f, _T("Error: %s\n"), buffer);
#endif
					fclose(f);
				}
			} else {
#ifdef _DEBUG
				_tprintf(_T("File: %s\nFunction: %s\nLine: %i\nError: %s\n"), file, function, line, buffer);
#else
				_tprintf(_T("Error: %s\n"), buffer);
#endif
			}
		}
		__except(EXCEPTION_EXECUTE_HANDLER)
		{
			//nothing to do
		}
		logLock.Unlock();
	}

	void LogErrDateTime(const _TCHAR * file, const _TCHAR * function, int line, const _TCHAR * msg, ...)
	{
		if (!logInitialized) return;
		logLock.Lock(3202);
		__try
		{
			static _TCHAR buffer[8192+8];
			buffer[0] = '[';
			_tstrdatetime_s(&buffer[1], 18);
			buffer[18] = ']';
			buffer[19] = ' ';
			va_list args;
			va_start(args, msg);
			_vsntprintf_s(&buffer[20], 8192-20, 8191-20, msg, args);
			va_end(args);
#ifdef _DEBUG
			OUTPUT_DEBUG_LINE(buffer);
#endif
			if (logFileName)
			{
				FILE * f;
				if (logOverwrite)
				{
					if (!logOverwritten)
					{
						f = _tfopen(logFileName, _T("wb"));
						logOverwritten = true;
					} else
						f = _tfopen(logFileName, _T("ab"));
					logOverwrite = false;
				} else
					f = _tfopen(logFileName, _T("ab"));
				if (f)
				{
#ifdef _DEBUG
					_ftprintf(f, _T("File: %s\nFunction: %s\nLine: %i\nError: %s\n"), file, function, line, buffer);
#else
					_ftprintf(f, _T("Error: %s\n"), buffer);
#endif
					fclose(f);
				}
			} else {
#ifdef _DEBUG
				_tprintf(_T("File: %s\nFunction: %s\nLine: %i\nError: %s\n"), file, function, line, buffer);
#else
				_tprintf(_T("Error: %s\n"), buffer);
#endif
			}
		}
		__except(EXCEPTION_EXECUTE_HANDLER)
		{
			//nothing to do
		}
		logLock.Unlock();
	}

	void Log(const _TCHAR * msg, ...)
	{
		if (!logInitialized) return;
		logLock.Lock(3203);
		__try
		{
			static _TCHAR buffer[8192+8];
			va_list args;
			va_start(args, msg);
			_vsntprintf_s(buffer, 8192, 8191, msg, args);
			va_end(args);
#ifdef _DEBUG
			OUTPUT_DEBUG_STRING(buffer);
#endif
			if (logFileName)
			{
				FILE * f;
				if (logOverwrite)
				{
					if (!logOverwritten)
					{
						f = _tfopen(logFileName, _T("wb"));
						logOverwritten = true;
					} else
						f = _tfopen(logFileName, _T("ab"));
					logOverwrite = false;
				} else
					f = _tfopen(logFileName, _T("ab"));
				if (f)
				{
					_ftprintf(f, _T("%s"), buffer);
					fclose(f);
				}
			} else {
				_tprintf(_T("%s"), buffer);
			}
		}
		__except(EXCEPTION_EXECUTE_HANDLER)
		{
			//nothing to do
		}
		logLock.Unlock();
	}

	void LogDateTime(const _TCHAR * msg, ...)
	{
		if (!logInitialized) return;
		logLock.Lock(3204);
		__try
		{
			static _TCHAR buffer[8192+8];
			buffer[0] = '[';
			_tstrdatetime_s(&buffer[1], 18);
			buffer[18] = ']';
			buffer[19] = ' ';
			va_list args;
			va_start(args, msg);
			_vsntprintf_s(&buffer[20], 8192-20, 8191-20, msg, args); //20 chars at the start of buffer is [mm/dd/yy hh:mm:ss] with space at the end
			va_end(args);
#ifdef _DEBUG
			OUTPUT_DEBUG_LINE(buffer);
#endif
			if (logFileName)
			{
				FILE * f;
				if (logOverwrite)
				{
					if (!logOverwritten)
					{
						f = _tfopen(logFileName, _T("wb"));
						logOverwritten = true;
					} else
						f = _tfopen(logFileName, _T("ab"));
					logOverwrite = false;
				} else
					f = _tfopen(logFileName, _T("ab"));
				if (f)
				{
					_ftprintf(f, _T("%s\n"), buffer);
					fclose(f);
				}
			} else {
				_tprintf(_T("%s\n"), buffer);
			}
		}
		__except(EXCEPTION_EXECUTE_HANDLER)
		{
			//nothing to do
		}
		logLock.Unlock();
	}

	void LogDebugOutput(const _TCHAR * msg, ...)
	{
		if (!logDebugOutput || !logDebugOutput) return;
		logLock.Lock(3205);
		__try
		{
			static _TCHAR buffer[8192+8];
			va_list args;
			va_start(args, msg);
			_vsntprintf_s(buffer, 8192, 8191, msg, args);
			va_end(args);
			OutputDebugString(buffer);
		}
		__except(EXCEPTION_EXECUTE_HANDLER)
		{
			//nothing to do
		}
		logLock.Unlock();
	}

	// We need to call these in our main function before we use the Log system and
	// before the program exits.
	void Setup(const _TCHAR * szLogFileName, bool bLogOverwrite, bool bLogDebugOutput)
	{
		if (szLogFileName)
		{
			logFileName = logFileNameBuf;
			_tcsncpy(logFileName, szLogFileName, MAX_PATH);
			logFileName[MAX_PATH-1] = '\0';
		} else {
			logFileName = NULL; 
		}
		logOverwrite = bLogOverwrite;
		logDebugOutput = bLogDebugOutput;
		InterlockedIncrement(&logInitialized);
	}

	void Cleanup()
	{
		InterlockedDecrement(&logInitialized);
	}

	void Overwrite()
	{
		logOverwrite = true;
	}

	void DebugOutput()
	{
		logDebugOutput = true;
	}

	void DumpData(UINT16 count, UINT8 * data)
	{
		const _TCHAR * hexChars = _T("0123456789ABCDEF");

		Log(_T("\n"));

		if (!count) return;

		_TCHAR sz[80];

		memset(sz, 0, 80 * sizeof(_TCHAR));
		memset(sz, ' ', 49 * sizeof(_TCHAR));

		// Dump out the data in hex format
		UINT16 index;
		for (index = 0; index < count; index++)
		{
			UINT8 b = data[index];
			int i = (index % 16) * 3;
			sz[i] = hexChars[(b >> 4) & 0x0f];
			sz[i+1] = hexChars[b & 0x0f];
			sz[49 + (index % 16)] = b < 32 ? '.' : b >= 128 ? '.' : b;
			if (index + 1 < count && (index + 1) % 16 == 0)
			{
				Log(_T("%s\n"), sz);
				memset(sz, 0, 80 * sizeof(_TCHAR));
				memset(sz, ' ', 49 * sizeof(_TCHAR));
			}
		}
		Log(_T("%s\n\n"), sz);
	}
}

//-----------------------------------------------------------------------------
