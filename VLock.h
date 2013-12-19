//////////////////////////////////////////////////////////////////
//
// Copyright 2013 CoolSoftware. http://blog.coolsoftware.ru/
//
// Exclusive lock.
//
// You can freely use this program and code for your needs.
//
// Please, don't remove this copyright.
//
//////////////////////////////////////////////////////////////////

#pragma once

#ifndef V_LOCK_H_
#define V_LOCK_H_

#include <Windows.h>

#ifdef _DEBUG
#define DEBUG_LOCK
#endif

#ifdef DEBUG_LOCK

class VLock; //forward declaration

struct VLockItem
{
	VLock * m_pLock;
	VLockItem * m_nextLock;
	VLockItem * m_prevLock;
};

#endif

class VLock
{
private:
	CRITICAL_SECTION m_csLock;

	int m_lLockPosition;

#ifdef DEBUG_LOCK
	volatile LONG m_lWaitingLock;
	VLockItem * m_lockItem;

	static CRITICAL_SECTION g_csLocks;
	static volatile LONG g_nLocks;
	static VLockItem * g_lastLock;
#endif

public:
	VLock();
	~VLock();

	void Lock(int lPosition, volatile LONG * lpThreadLock = NULL);
	void Unlock(volatile LONG * lpThreadLock = NULL);

	static void OutputDebugLocks();
};

#endif //V_LOCK_H_