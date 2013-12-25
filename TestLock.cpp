//////////////////////////////////////////////////////////////////
//
// Copyright 2013 CoolSoftware. http://blog.coolsoftware.ru/
//
// Test locks.
//
// You can freely use this program and code for your needs.
//
// Please, don't remove this copyright.
//
//////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <process.h>
#include "VLog.h"
#include "VLock.h"
#include "VRWLock.h"

HANDLE hExitEvent = 0;

BOOL __stdcall ConsoleHandler(DWORD ConsoleEvent)
{
	switch(ConsoleEvent)
	{
		case CTRL_LOGOFF_EVENT:
		case CTRL_C_EVENT:
		case CTRL_BREAK_EVENT:
		case CTRL_CLOSE_EVENT:
		case CTRL_SHUTDOWN_EVENT:
		{
			SetEvent(hExitEvent);
			return TRUE;
		}
	}
	return FALSE;
}

struct ThreadInfo
{
	volatile LONG lThreadLock;
	int nThread;
	HANDLE hThread;
};
/*
VLock lock;

unsigned int __stdcall LockThreadProc(void * lpParam)
{
	lock.Lock(1, reinterpret_cast<volatile LONG *>(lpParam)); //lock resource

	//do something here

	lock.Unlock( reinterpret_cast<volatile LONG *>(lpParam)); //unlock resource

	//contuinue working

	return 0;
}

unsigned int __stdcall LockPtrThreadProc(void * lpParam)
{
	{
		VLockPtr lockptr(&lock, 1, reinterpret_cast<volatile LONG *>(lpParam)); //lock resource

		//do something here

	} //unlock will be done here

	//contuinue working

	return 0;
}

void main1(int argc, char* argv[])
{
	volatile LONG lThreadLock = 0; //initialize with zero
	//create thread
	HANDLE hThread = (HANDLE)_beginthreadex(NULL, 0, LockThreadProc, (void*)&lThreadLock, CREATE_SUSPENDED, NULL);
	//start thread
	::ResumeThread(hThread);
	//wait for 5 seconds
	if(::WaitForSingleObject(hThread, 5000) == WAIT_TIMEOUT)
	{
		//terminate thread
		::TerminateThread(hThread, 0);
		//release lock
		lock.Unlock(&lThreadLock);
	}
	//close thread handle
	::CloseHandle(hThread);
}
*/
//#define TEST_LOCK_TIME	INFINITE
#define TEST_LOCK_TIME	2000

#define TEST_RW_LOCK

#ifndef TEST_RW_LOCK

VLock lock;

unsigned int __stdcall TestLockThreadProc(void * lpParam)
{
	struct ThreadInfo * ti = reinterpret_cast<struct ThreadInfo*>(lpParam);
	if (!ti)
	{
		LOGERRDT("ThreadInfo is empty");
		return 0;
	}
	LOGDT("Thread %i (%d) Locking...", ti->nThread, ti->hThread);
	//lock.Lock(1, &ti->lThreadLock);
	{
		VLockPtr plock(&lock, 1, &ti->lThreadLock);
		LOGDT("Thread %i (%d) Locked.", ti->nThread, ti->hThread);
		::WaitForSingleObject(hExitEvent, TEST_LOCK_TIME);
	}
	//lock.Unlock(&ti->lThreadLock);
	LOGDT("Thread %i (%d) Completed.", ti->nThread, ti->hThread);
	return 0;
}

#else //ifdef TEST_RW_LOCK

VRWLock rwlock;

unsigned int __stdcall TestReadLockThreadProc(void * lpParam)
{
	struct ThreadInfo * ti = reinterpret_cast<struct ThreadInfo*>(lpParam);
	if (!ti)
	{
		LOGERRDT("ThreadInfo is empty");
		return 0;
	}
	LOGDT("Read Thread %i (%d) Locking...", ti->nThread, ti->hThread);
	//rwlock.LockRead(1, &ti->lThreadLock);
	{
		VReadLockPtr plock(&rwlock, 2, &ti->lThreadLock);
		LOGDT("Read Thread %i (%d) Locked.", ti->nThread, ti->hThread);
		::WaitForSingleObject(hExitEvent, TEST_LOCK_TIME);
	}
	//rwlock.Unlock(&ti->lThreadLock);
	LOGDT("Read Thread %i (%d) Completed.", ti->nThread, ti->hThread);
	return 0;
}

unsigned int __stdcall TestWriteLockThreadProc(void * lpParam)
{
	struct ThreadInfo * ti = reinterpret_cast<struct ThreadInfo*>(lpParam);
	if (!ti)
	{
		LOGERRDT("ThreadInfo is empty");
		return 0;
	}
	LOGDT("Write Thread %i (%d) Locking...", ti->nThread, ti->hThread);
	//rwlock.LockWrite(3, &ti->lThreadLock);
	{
		VWriteLockPtr plock(&rwlock, 3, &ti->lThreadLock);
		LOGDT("Write Thread %i (%d) Locked.", ti->nThread, ti->hThread);
		::WaitForSingleObject(hExitEvent, TEST_LOCK_TIME);
	}
	//rwlock.Unlock(&ti->lThreadLock);
	LOGDT("Write Thread %i (%d) Completed.", ti->nThread, ti->hThread);
	return 0;
}

#endif //ifdef TEST_RW_LOCK

const int cTestThreads = 10;

struct ThreadInfo threads[cTestThreads] = { 0 };

#ifdef TEST_RW_LOCK
void RWFunc(const VRWLock&)
{
}
#endif

int _tmain(int argc, _TCHAR* argv[])
{
#ifdef TEST_RW_LOCK
	{
		VRWLock rwlock1;
		//VRWLock rwlock2 = rwlock1;
		//RWFunc(1);
	}
#endif

	VLog::Setup((const _TCHAR*)NULL, FALSE, FALSE);

	hExitEvent = ::CreateEvent(NULL, TRUE, FALSE, NULL);

	::SetConsoleCtrlHandler(ConsoleHandler, TRUE);

	bool bAllThreadsStarted = true;

	for (int i = 0; i < cTestThreads; i++)
	{
		LOGDT("Starting Thread %i...", i+1);

		threads[i].lThreadLock = 0;
		threads[i].nThread = i+1;
#ifndef TEST_RW_LOCK	
		threads[i].hThread = (HANDLE)_beginthreadex(NULL, 0, TestLockThreadProc, &threads[i], CREATE_SUSPENDED, NULL);
#else //ifdef TEST_RW_LOCK
		//if (i % 2 == 0) // read-write-read-write-read-write-...
		//if (i < cTestThreads / 2) // write-write-write-...-read-read-read-...
		if (i >= cTestThreads / 2) // read-read-read-...-write-write-write-...
			threads[i].hThread = (HANDLE)_beginthreadex(NULL, 0, TestWriteLockThreadProc, &threads[i], CREATE_SUSPENDED, NULL);
		else
			threads[i].hThread = (HANDLE)_beginthreadex(NULL, 0, TestReadLockThreadProc, &threads[i], CREATE_SUSPENDED, NULL);
#endif //ifdef TEST_RW_LOCK
		if (threads[i].hThread)
		{
			LOGDT("Thread %i created (%d)", i+1, threads[i].hThread);
			::ResumeThread(threads[i].hThread);
		}
		else
		{
			LOGERRDT("Thread %i was not able to be created. GetLastError returned [%i]", i+1, GetLastError());
			bAllThreadsStarted = false;
			break;
		}
	}

	bool bTerminated = false;

	if (bAllThreadsStarted)
	{
		LOGDT("Test started. Waiting...");
		//int n = cTestThreads;
		while (!(bTerminated = ::WaitForSingleObject(hExitEvent, 5000) != WAIT_TIMEOUT))
		{
			bool bThreadTerminated = false;
			for (int i = 0; i < cTestThreads; i++)
			{
				if (threads[i].hThread != NULL && 
					threads[i].lThreadLock > 0)
				{
					bThreadTerminated = true;
					LOGDT("Terminating Thread %i (%d)...", i+1, threads[i].hThread);
					::TerminateThread(threads[i].hThread, 0);
					::CloseHandle(threads[i].hThread);
					threads[i].hThread = NULL;
#ifndef TEST_RW_LOCK
					lock.Unlock(&threads[i].lThreadLock);
#else //ifdef TEST_RW_LOCK
					rwlock.Unlock(&threads[i].lThreadLock);
#endif
					LOGDT("Thread %i Terminated. Lock released.", i+1);
					//n--;
					break;
				}
			}
			if (!bThreadTerminated)
			{
				//LOGERRDT("Locked thread not found!");
				break;
			}
			//if (n <= 0) break;
			LOGDT("Waiting...");
		}
		LOGDT("Test completed.");
	}
	else
	{
		SetEvent(hExitEvent);
	}

	for (int i = 0; i < cTestThreads; i++)
	{
		if (threads[i].hThread)
		{
			DWORD dwResult = ::WaitForSingleObject(threads[i].hThread, 5000);
			if (dwResult == WAIT_TIMEOUT)
			{
				LOGERRDT("Thread %i did not properly terminate in %i seconds.", i+1, 5);
				::TerminateThread(threads[i].hThread, 0);
			}
			::CloseHandle(threads[i].hThread);
		}
	}

	if (!bTerminated)
	{
		LOGDT("Press Ctrl+Break to stop.");
		::WaitForSingleObject(hExitEvent, INFINITE);
	}

	::SetConsoleCtrlHandler(ConsoleHandler, FALSE);

	::CloseHandle(hExitEvent);

	return 0;
}

