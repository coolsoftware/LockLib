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

#include "VRWLock.h"
#include "VLog.h"
#include <crtdbg.h>
#include <stdio.h>

#ifdef DEBUG_LOCK

CRITICAL_SECTION VRWLock::g_csRWLocks;
volatile LONG VRWLock::g_nRWLocks = 0;
VRWLockItem * VRWLock::g_lastRWLock = NULL;

#endif

VRWLock::VRWLock(LONG lMaxReaders, DWORD dwSpinCount, DWORD dwTimeout)
{
#ifdef DEBUG_LOCK
	m_lockItem = new VRWLockItem;
#endif

	m_lMaxReaders = lMaxReaders;
	m_dwSpinCount = dwSpinCount;
	m_dwTimeout = dwTimeout;

	m_lCounter = 0;
	m_lReadPosition = 0;
	m_lWritePosition = 0;

#ifdef DEBUG_LOCK
	m_lWaitingLock = 0;
	if (InterlockedIncrement(&g_nRWLocks) == 1)
	{
		InitializeCriticalSection(&g_csRWLocks);
	}
	EnterCriticalSection(&g_csRWLocks);
	__try
	{
		m_lockItem->m_pLock = this;
		m_lockItem->m_nextLock = NULL;
		m_lockItem->m_prevLock = g_lastRWLock;
		if (g_lastRWLock) g_lastRWLock->m_nextLock = m_lockItem;
		g_lastRWLock = m_lockItem;
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		DEBUGOUTPUT("[VRWLock] Error 1!\n");
	}
	LeaveCriticalSection(&g_csRWLocks);
	DEBUGOUTPUT("RW Locks: %i\n", g_nRWLocks);
#endif
}

VRWLock::~VRWLock()
{
#ifdef DEBUG_LOCK
	EnterCriticalSection(&g_csRWLocks);
	__try
	{
		if (m_lockItem != NULL)
		{
			if (m_lockItem->m_nextLock != NULL)
				m_lockItem->m_nextLock->m_prevLock = m_lockItem->m_prevLock;
			else
				g_lastRWLock = m_lockItem->m_prevLock;
			if (m_lockItem->m_prevLock != NULL)
				m_lockItem->m_prevLock->m_nextLock = m_lockItem->m_nextLock;
		}
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		DEBUGOUTPUT("[VRWLock] Error 2!\n");
	}
	LeaveCriticalSection(&g_csRWLocks);
	if (InterlockedDecrement(&g_nRWLocks) == 0)
	{
		DeleteCriticalSection(&g_csRWLocks);
	}
	DEBUGOUTPUT("RW Locks: %i\n", g_nRWLocks);
#endif
	
#ifdef DEBUG_LOCK
	if (m_lockItem != NULL) delete m_lockItem;
#endif
}

void VRWLock::LockRead(int lPosition, volatile LONG * lpThreadLock)
{
#ifdef DEBUG_LOCK
	InterlockedIncrement(&m_lWaitingLock);
#endif

	LONG lMaxReaders = m_lMaxReaders;
	DWORD dwLockAttempt = 0, dwSpinCount = m_dwSpinCount;
	for (;;) //бесконечный цикл ожидания освобождения ресурса
	{
		LONG n = InterlockedIncrement(&m_lCounter); 
		//в n - значение m_lCounter после инкремента
		if (n <= lMaxReaders) break; //писатель ничего не пишет - можно читать
		InterlockedDecrement(&m_lCounter);
		if (dwSpinCount != INFINITE)
		{
			if (dwLockAttempt >= dwSpinCount)
			{
				WaitForUnlock();
				dwLockAttempt = 0;
			}
			else
			{
				dwLockAttempt++;
			}
		}
	}

#ifdef DEBUG_LOCK
	InterlockedDecrement(&m_lWaitingLock);
#endif

#ifdef _DEBUG
	_ASSERT(m_lWritePosition == 0); //ошибка! не был вызван Unlock() после WriteLock()
#endif
	m_lReadPosition = lPosition;
	if (lpThreadLock) 
	{
#ifdef _DEBUG
		_ASSERT(*lpThreadLock == 0); //ошибка! значение *lpThreadLock должно быть = 0
#endif
		(*lpThreadLock)++;
	}
}

void VRWLock::LockWrite(int lPosition, volatile LONG * lpThreadLock)
{
#ifdef DEBUG_LOCK
	InterlockedIncrement(&m_lWaitingLock);
#endif

	LONG lMaxReaders = m_lMaxReaders;
	DWORD dwLockAttempt = 0, dwSpinCount = m_dwSpinCount;
	for (;;) //бесконечный цикл освобождения ресурса читателями/писателями
	{
		LONG n = InterlockedCompareExchange(&m_lCounter, (lMaxReaders+1), 0); 
		//в n - предыдущее значение m_lCounter, которое было ДО попытки заменить его на MAX_READERS+1 в InterlockedCompareExchange; 
		//если в m_lCounter был не 0, то это значение НЕ будет заменено на MAX_READERS+1, а останется прежним;
		//если в m_lCounter был 0, то никаких читателей/писателей не было, новое значение в m_lCounter будет MAX_READERS+1
		if (n == 0) break; 
		if (dwSpinCount != INFINITE)
		{
			if (dwLockAttempt >= dwSpinCount)
			{
				WaitForUnlock();
				dwLockAttempt = 0;
			}
			else
			{
				dwLockAttempt++;
			}
		}
	}

#ifdef DEBUG_LOCK
	InterlockedDecrement(&m_lWaitingLock);
#endif

#ifdef _DEBUG
	_ASSERT(m_lWritePosition == 0); //ошибка! не был вызван Unlock() после WriteLock()
#endif
	m_lWritePosition = lPosition;
	if (lpThreadLock) 
	{
#ifdef _DEBUG
		_ASSERT(*lpThreadLock == 0); //ошибка! значение *lpThreadLock должно быть = 0
#endif
		(*lpThreadLock) += (lMaxReaders+1);
	}
}

//ReLockWrite изменяет тип блокировки с блокировки читателем на блокировку писателем
void VRWLock::ReLockWrite(int lPosition, volatile LONG * lpThreadLock)
{
#ifdef DEBUG_LOCK
	InterlockedIncrement(&m_lWaitingLock);
#endif
	LONG lMaxReaders = m_lMaxReaders;
	DWORD dwLockAttempt = 0, dwSpinCount = m_dwSpinCount;
	LONG lComperand = 1; //начальное значение = 1, т.е. один читатель заблокировал ресурс и теперь он хочет изменить свой статус на писателя
	for (;;) //бесконечный цикл освобождения ресурса читателями/писателями
	{
		LONG n = InterlockedCompareExchange(&m_lCounter, (lMaxReaders+1), lComperand); 
		//в n - предыдущее значение m_lCounter, которое было ДО попытки заменить его на MAX_READERS+1 в InterlockedCompareExchange; 
		//если в m_lCounter был не lComperand, то это значение НЕ будет заменено на MAX_READERS+1, а останется прежним;
		//если в m_lCounter было значение lComperand (lComperand читателей), новое значение в m_lCounter будет MAX_READERS+1, что означает блокировку писателем
		if (n == lComperand) break;
		//если в m_lCounter был 0 и он не равен lComperand (lComperand = 1), то это означает, что перед вызовом ReLockWrite блокировки писателем не было, 
		//поэтому присваиваем lComperand = 0 (функция начинает работать как LockWrite)
		if (n == 0) { lComperand = 0; }
		if (dwSpinCount != INFINITE)
		{
			if (dwLockAttempt >= dwSpinCount)
			{
				WaitForUnlock();
				dwLockAttempt = 0;
			}
			else
			{
				dwLockAttempt++;
			}
		}
	}

#ifdef DEBUG_LOCK
	InterlockedDecrement(&m_lWaitingLock);
#endif

#ifdef _DEBUG
	_ASSERT(m_lWritePosition == 0); //ошибка! не был вызван Unlock() после WriteLock()
#endif
	m_lWritePosition = lPosition;
	if (lpThreadLock) 
	{
#ifdef _DEBUG
		_ASSERT(*lpThreadLock == lComperand);	//ошибка! значение *lpThreadLock должно быть = lComperand 
												//(1 если перед вызовом функции читатель заблокировал ресурс или 0 если блокировки читателем не было)
#endif
		(*lpThreadLock) += lMaxReaders;
	}
}

void VRWLock::Unlock(volatile LONG * lpThreadLock)
{
	if (lpThreadLock)
	{
#ifdef _DEBUG
		_ASSERT(*lpThreadLock >= 0); //ошибка! значение *lpThreadLock должно быть неотрицательным
#endif
		if (*lpThreadLock <= 0) return;
	}
	if (m_lWritePosition != 0)
	{
		//освобождаем блокировку писателем
		m_lWritePosition = 0;
		InterlockedExchangeAdd(&m_lCounter, -(m_lMaxReaders+1));
		if (lpThreadLock) 
		{
			(*lpThreadLock) -= (m_lMaxReaders+1);
#ifdef _DEBUG
			_ASSERT(*lpThreadLock == 0); //ошибка! значение *lpThreadLock должно быть = 0
#endif
		}
	}
	else
	{
		//освобождаем блокировку читателем
		InterlockedDecrement(&m_lCounter);
		if (lpThreadLock) 
		{
			(*lpThreadLock)--;
#ifdef _DEBUG
			_ASSERT(*lpThreadLock == 0); //ошибка! значение *lpThreadLock должно быть = 0
#endif
		}
	}
}

void VRWLock::WaitForUnlock()
{
	Sleep(m_dwTimeout);
}

void VRWLock::OutputDebugLocks()
{
#ifdef DEBUG_LOCK
	EnterCriticalSection(&g_csRWLocks);
	int n = 0, n1 = 0, n2 = 0, n3 = 0;
	VRWLockItem * pLockItem = g_lastRWLock;
	while (pLockItem)
	{
		VRWLock * pLock = pLockItem->m_pLock;
		if (pLock)
		{
			n++;
			__try
			{
				int wl = pLock->m_lWaitingLock;
				int rl = pLock->m_lCounter;
				int rp = pLock->m_lReadPosition;
				int wp = pLock->m_lWritePosition;
				if (wl != 0)
				{
					n2++;
					if (wp != 0)
					{
						n1++;
						DEBUGOUTPUT("RW Lock %i (0x%x): LOCKED FOR WRITE at %i, WAITING %i\n", n, pLock, wp, wl);
					}
					else
					if (rl != 0)
					{
						n3++;
						DEBUGOUTPUT("RW Lock %i (0x%x): %i LOCKS FOR READ (last at %i), WAITING %i\n", n, pLock, rl, rp, wl);
					}
					else
						DEBUGOUTPUT("RW Lock %i (0x%x): WAITING %i\n", n, pLock, wl);
				} 
				else 
				if (wp != 0)
				{
					n1++;
					DEBUGOUTPUT("RW Lock %i (0x%x): LOCKED FOR WRITE at %i\n", n, pLock, wp);
				}
				else
				if (rl != 0)
				{
					n3++;
					DEBUGOUTPUT("RW Lock %i (0x%x): %i LOCKS FOR READ (last at %i), WAITING %i\n", n, pLock, rl, rp, wl);
				}
			}
			__except(EXCEPTION_EXECUTE_HANDLER)
			{
				DEBUGOUTPUT("[VRWLock] Error 3!\n");
			}
		}
		pLockItem = pLockItem->m_prevLock;
	}
	if (n1 > 0 || n2 > 0 || n3 > 0)
	{
		DEBUGOUTPUT("%i RW locks for write, %i locks for read of %i. %i waits for lock.\n", n1, n3, n, n2);
	}
	LeaveCriticalSection(&g_csRWLocks);
#endif
}