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

#include "VLock.h"
#include "VLog.h"
#include <crtdbg.h>
#include <stdio.h>

#ifdef DEBUG_LOCK

CRITICAL_SECTION VLock::g_csLocks;
volatile LONG VLock::g_nLocks = 0;
VLockItem * VLock::g_lastLock = NULL;

#endif

VLock::VLock()
{
#ifdef DEBUG_LOCK
	m_lockItem = new VLockItem;
#endif

	InitializeCriticalSection(&m_csLock);
	m_lLockPosition = 0;
	
#ifdef DEBUG_LOCK
	m_lWaitingLock = 0;
	if (InterlockedIncrement(&g_nLocks) == 1)
	{
		InitializeCriticalSection(&g_csLocks);
	}
	EnterCriticalSection(&g_csLocks);
	__try
	{
		m_lockItem->m_pLock = this;
		m_lockItem->m_nextLock = NULL;
		m_lockItem->m_prevLock = g_lastLock;
		if (g_lastLock) g_lastLock->m_nextLock = m_lockItem;
		g_lastLock = m_lockItem;
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		DEBUGOUTPUT("[VLock] Error 1!\n");
	}
	LeaveCriticalSection(&g_csLocks);
	DEBUGOUTPUT("Locks: %i\n", g_nLocks);
#endif
}

VLock::~VLock()
{
#ifdef DEBUG_LOCK
	EnterCriticalSection(&g_csLocks);
	__try
	{
		if (m_lockItem != NULL)
		{
			if (m_lockItem->m_nextLock != NULL)
				m_lockItem->m_nextLock->m_prevLock = m_lockItem->m_prevLock;
			else
				g_lastLock = m_lockItem->m_prevLock;
			if (m_lockItem->m_prevLock != NULL)
				m_lockItem->m_prevLock->m_nextLock = m_lockItem->m_nextLock;
		}
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		DEBUGOUTPUT("[VLock] Error 2!\n");
	}
	LeaveCriticalSection(&g_csLocks);
	if (InterlockedDecrement(&g_nLocks) == 0)
	{
		DeleteCriticalSection(&g_csLocks);
	}
	DEBUGOUTPUT("Locks: %i\n", g_nLocks);
#endif

	DeleteCriticalSection(&m_csLock);

#ifdef DEBUG_LOCK
	if (m_lockItem != NULL) delete m_lockItem;
#endif
}

void VLock::Lock(int lPosition, volatile LONG * lpThreadLock)
{
#ifdef DEBUG_LOCK
	InterlockedIncrement(&m_lWaitingLock);
#endif
	EnterCriticalSection(&m_csLock);
#ifdef DEBUG_LOCK
	InterlockedDecrement(&m_lWaitingLock);
#endif
#ifdef _DEBUG
	_ASSERT(m_lLockPosition == 0); //ошибка! не был вызван Unlock() после Lock()
#endif
	m_lLockPosition = lPosition;
	if (lpThreadLock) 
	{
#ifdef _DEBUG
		_ASSERT(*lpThreadLock == 0); //ошибка! значение *lpThreadLock должно быть = 0
#endif
		(*lpThreadLock)++;
	}
}

void VLock::Unlock(volatile LONG * lpThreadLock)
{
	if (lpThreadLock)
	{
#ifdef _DEBUG
		_ASSERT(*lpThreadLock >= 0); //ошибка! значение *lpThreadLock должно быть неотрицательным
#endif
		if (*lpThreadLock <= 0) return;
	}
#ifdef _DEBUG
	_ASSERT(m_lLockPosition != 0); //ошибка! не был вызван Lock() перед Unlock()
#endif
	m_lLockPosition = 0;
	if (lpThreadLock) 
	{
		(*lpThreadLock)--;
#ifdef _DEBUG
		_ASSERT(*lpThreadLock == 0); //ошибка! значение *lpThreadLock должно быть = 0
#endif
	}
	LeaveCriticalSection(&m_csLock);
}

void VLock::OutputDebugLocks()
{
#ifdef DEBUG_LOCK
	EnterCriticalSection(&g_csLocks);
	int n = 0, n1 = 0, n2 = 0;
	VLockItem * pLockItem = g_lastLock;
	while (pLockItem)
	{
		VLock * pLock = pLockItem->m_pLock;
		if (pLock)
		{
			n++;
			__try
			{
				int wl = pLock->m_lWaitingLock;
				int lp = pLock->m_lLockPosition;
				if (wl != 0)
				{
					n2++;
					if (lp != 0)
					{
						n1++;
						DEBUGOUTPUT("Lock %i (0x%x): LOCKED at %i, WAITING %i\n", n, pLock, lp, wl);
					}
					else
					{
						DEBUGOUTPUT("Lock %i (0x%x): WAITING %i\n", n, pLock, wl);
					}
				} 
				else 
				if (lp != 0)
				{
					n1++;
					DEBUGOUTPUT("Lock %i (0x%x): LOCKED at %i\n", n, pLock, lp);
				}
			}
			__except(EXCEPTION_EXECUTE_HANDLER)
			{
				DEBUGOUTPUT("[VLock] Error 3!\n");
			}
		}
		pLockItem = pLockItem->m_prevLock;
	}
	if (n1 > 0 || n2 > 0)
	{
		DEBUGOUTPUT("%i locks of %i. %i waits for lock.\n", n1, n, n2);
	}
	LeaveCriticalSection(&g_csLocks);
#endif
}

VLockPtr::VLockPtr(VLock * pLock, int lPosition, volatile LONG * lpThreadLock) 
	: m_pLock(pLock), m_lpThreadLock(lpThreadLock)
{
#ifdef _DEBUG
	_ASSERT(m_pLock != NULL);
#endif
	if (!m_pLock) return;
	m_pLock->Lock(lPosition, m_lpThreadLock);
}

VLockPtr::~VLockPtr()
{
	if (!m_pLock) return;
	m_pLock->Unlock(m_lpThreadLock);
}