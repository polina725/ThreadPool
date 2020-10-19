#pragma once
#include <windows.h>
#include <queue>
#include "TaskPair.h"
#include "Logger.h"
#include "Thread.h"

class ThreadPool {
	public:
		ThreadPool(int maxThreadCount,int initThreadCount);
		int addTaskToQueue(LPTHREAD_START_ROUTINE functionAddr, LPVOID params);
		~ThreadPool();

	private:
		int maxCount;
		volatile int currentWorkingThreadCount;
		volatile int initializedThraedAmount;
		volatile LONG alive;

		HANDLE* threadHandles;
		Logger* logger;
		HANDLE ManagerThread;

		std::queue<Thread*> threadQueue;
		CRITICAL_SECTION taskQueueCrSection;
		CONDITION_VARIABLE taskQueueCnVariable;
		
		std::queue<TaskPair*> taskQueue;
		CRITICAL_SECTION threadQueueCrSection;
		CONDITION_VARIABLE threadQueueCnVariable;

		static DWORD WINAPI threadStart(ThreadPool* lpParam);
		static DWORD WINAPI startManager(ThreadPool* lpParam);
};