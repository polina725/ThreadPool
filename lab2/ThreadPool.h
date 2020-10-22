#pragma once
#include <windows.h>
#include <queue>
#include "Logger.h"
#include "TaskPair.h"
#include "Thread.h"

class ThreadPool {
	public:
		ThreadPool(int maxThreadCount,int initThreadCount);
		int addTaskToQueue(LPTHREAD_START_ROUTINE functionAddr, LPVOID params);
		~ThreadPool();

	private:
		int maxCount;
		volatile LONG currentWorkingThreadCount;
		volatile int initializedThreadAmount;
		volatile LONG alive;
		int startInitValue;

		Logger logger;
		HANDLE ManagerThread;
		volatile LONG managerIsAlive;
		HANDLE* threadHandles;

		std::queue<Thread*> threadQueue;
		CRITICAL_SECTION taskQueueCrSection;
		CONDITION_VARIABLE taskQueueCnVariable;
		
		std::queue<TaskPair*> taskQueue;
		CRITICAL_SECTION threadQueueCrSection;
		CONDITION_VARIABLE threadQueueCnVariable;

		static DWORD WINAPI threadStart(ThreadPool* lpParam);
		static DWORD WINAPI startManager(ThreadPool* lpParam);
		bool allTaskExecuted();
};