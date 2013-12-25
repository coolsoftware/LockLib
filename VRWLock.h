//////////////////////////////////////////////////////////////////
//
// Copyright 2013 CoolSoftware. http://blog.coolsoftware.ru/
//
// Single-write-multiple-read.
//
// You can freely use this program and code for your needs.
//
// Please, don't remove this copyright.
//
//////////////////////////////////////////////////////////////////

#pragma once

#ifndef V_RW_LOCK_H_
#define V_RW_LOCK_H_

#include <Windows.h>
#include "VUncopyable.h"

#ifdef _DEBUG
#define DEBUG_LOCK
#endif

#ifdef DEBUG_LOCK

class VRWLock; //forward declaration

struct VRWLockItem
{
	VRWLock * m_pLock;
	VRWLockItem * m_nextLock;
	VRWLockItem * m_prevLock;
};

#endif

class VRWLock : private VUncopyable
{
private:
	LONG m_lMaxReaders;
	DWORD m_dwSpinCount;
	DWORD m_dwTimeout;

	volatile LONG m_lCounter;
	int m_lReadPosition;
	int m_lWritePosition;

#ifdef DEBUG_LOCK
	volatile LONG m_lWaitingLock;
	VRWLockItem * m_lockItem;

	static CRITICAL_SECTION g_csRWLocks;
	static volatile LONG g_nRWLocks;
	static VRWLockItem * g_lastRWLock;
#endif

	void WaitForUnlock();

public:
	explicit VRWLock(LONG lMaxReaders = 65535, DWORD dwSpinCount = 1000, DWORD dwTimeout = 5);
	~VRWLock();

	void LockRead(int lPosition, volatile LONG * lpThreadLock = NULL);
	void LockWrite(int lPosition, volatile LONG * lpThreadLock = NULL);
	void ReLockWrite(int lPosition, volatile LONG * lpThreadLock = NULL);
	void Unlock(volatile LONG * lpThreadLock = NULL);

	static void OutputDebugLocks();
};

class VReadLockPtr : private VUncopyable
{
private:
	VRWLock * m_pLock;
	volatile LONG * m_lpThreadLock;

public:
	VReadLockPtr(VRWLock * pLock, int lPosition, volatile LONG * lpThreadLock = NULL);
	~VReadLockPtr();
};

class VWriteLockPtr : private VUncopyable
{
private:
	VRWLock * m_pLock;
	volatile LONG * m_lpThreadLock;

public:
	VWriteLockPtr(VRWLock * pLock, int lPosition, volatile LONG * lpThreadLock = NULL);
	~VWriteLockPtr();
};

#endif //V_RW_LOCK_H_
