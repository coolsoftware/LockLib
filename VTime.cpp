//////////////////////////////////////////////////////////////////
//
// Copyright 2013-14 CoolSoftware. http://blog.coolsoftware.ru/
//
// You can freely use this program and code for your needs.
//
// Please, don't remove this copyright.
//
//////////////////////////////////////////////////////////////////

#include "VTime.h"
#include <errno.h>

errno_t __cdecl _strdatetime_s(char * _Buf, size_t _SizeInBytes)
{
	if (!_Buf || _SizeInBytes <= 0) return EINVAL;
	_Buf[0] = '\0';
	if (_SizeInBytes < 18) return EINVAL;
	errno_t r = _strdate_s(_Buf, _SizeInBytes); // mm/dd/yy - 8 chars
	if (r != 0) return r;
	_Buf[8] = ' ';
	r = _strtime_s(&_Buf[9], _SizeInBytes-9); // hh:mm:ss - 8 chars
	return r;
}

#ifdef _UNICODE

errno_t __cdecl _tstrdatetime_s(_TCHAR * _Buf, size_t _SizeInBytes)
{
	if (!_Buf || _SizeInBytes <= 0) return EINVAL;
	_Buf[0] = '\0';
	if (_SizeInBytes < 18) return EINVAL;
	errno_t r = _tstrdate_s(_Buf, _SizeInBytes); // mm/dd/yy - 8 chars
	if (r != 0) return r;
	_Buf[8] = ' ';
	r = _tstrtime_s(&_Buf[9], _SizeInBytes-9); // hh:mm:ss - 8 chars
	return r;
}

#endif