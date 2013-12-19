//////////////////////////////////////////////////////////////////
//
// Copyright 2013 CoolSoftware. http://blog.coolsoftware.ru/
//
// You can freely use this program and code for your needs.
//
// Please, don't remove this copyright.
//
//////////////////////////////////////////////////////////////////

#pragma once

#ifndef V_TIME_H_
#define V_TIME_H_

#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <tchar.h>

errno_t __cdecl _strdatetime_s(char * _Buf, size_t _SizeInBytes);

template<size_t _Size> inline errno_t __cdecl _strdatetime_s(char (&_Buffer)[_Size])
{
	return _strdatetime_s(_Buffer, _Size);
}

#ifdef _UNICODE

errno_t __cdecl _tstrdatetime_s(_TCHAR * _Buf, size_t _SizeInBytes);

template<size_t _Size> inline errno_t __cdecl _tstrdatetime_s(_TCHAR (&_Buffer)[_Size])
{
	return _tstrdatetime_s(_Buffer, _Size);
}

#else

#define _tstrdatetime_s _strdatetime_s

#endif

#endif //V_TIME_H_