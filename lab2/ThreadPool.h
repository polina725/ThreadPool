#pragma once
#include <windows.h>
#include <queue>
#include "TaskPair.h"
#include "Logger.h"

class ThreadPool {
	public:
		ThreadPool(int maxThreadCount);
		int addTaskToQueue(LPTHREAD_START_ROUTINE functionAddr, LPVOID params);
		~ThreadPool();

	private:
		int maxCount;
		int currentThreadCount;
		volatile LONG alive;
		HANDLE* threadHandles;
		Logger* logger;
		std::queue<TaskPair*> taskQueue;
		CRITICAL_SECTION ñriticalSection;
		CONDITION_VARIABLE conditionVariable;


		static DWORD WINAPI threadStart(ThreadPool* lpParam);
};