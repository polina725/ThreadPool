#include "ThreadPool.h"

#define POOL_CAPACITY 100

ThreadPool::ThreadPool(int maxThreadCount)
{
	if (maxThreadCount <= 0)
		maxCount = 3;
	else
		maxCount = maxThreadCount;
	currentThreadCount = maxCount;
	alive = true;
	logger = new Logger();

	threadHandles = (HANDLE*)calloc(POOL_CAPACITY, sizeof(HANDLE));
	for (int i = 0; i < maxCount; i++) {
		threadHandles[i] = CreateThread(
										NULL,                   // default security attributes
										0,                      // use default stack size  
										(LPTHREAD_START_ROUTINE)threadStart,       // thread function name ???
										this,          // argument to thread function  ??
										0,                      // use default creation flags 
										NULL);
	}

	InitializeCriticalSectionAndSpinCount(&ñriticalSection, 2000);
	InitializeConditionVariable(&conditionVariable);

	logger->logAction(CREATED(maxCount, POOL_CAPACITY));
}

int ThreadPool::addTaskToQueue(LPTHREAD_START_ROUTINE functionAddr, LPVOID params)
{
	if (functionAddr == NULL)
		return -1;
	EnterCriticalSection(&ñriticalSection);

	if (taskQueue.size() == maxCount) {
		logger->logAction(NEW_THREAD);
		currentThreadCount++;
		threadHandles[currentThreadCount] = CreateThread(
										NULL,                   // default security attributes
										0,                      // use default stack size  
										(LPTHREAD_START_ROUTINE)threadStart,       // thread function name ???
										this,         // argument to thread function  ??
										0,                      // use default creation flags 
										NULL);
	}
	taskQueue.push(new TaskPair(functionAddr,params));
	logger->logAction(TASK_ADDED);
	
	WakeConditionVariable(&conditionVariable);

	LeaveCriticalSection(&ñriticalSection);
	return 0;
}

DWORD WINAPI ThreadPool::threadStart(ThreadPool* pool)
{
	TaskPair* task;
	do {
		EnterCriticalSection(&pool->ñriticalSection);

		while (pool->taskQueue.empty() && pool->alive)
			SleepConditionVariableCS(&pool->conditionVariable, &pool->ñriticalSection, INFINITE);

		if (!pool->alive) {
			LeaveCriticalSection(&pool->ñriticalSection);
			return 0;
		}
		task = pool->taskQueue.front();
		pool->taskQueue.pop();

		LeaveCriticalSection(&pool->ñriticalSection);
		if (task->functionAddress != NULL) 
			try {
				task->functionAddress(task->parameters);
			}
			catch (exception ex) {
				pool->logger->logAction(EXCEPTION_RAISED(GetCurrentThreadId(),ex.what()));
			}

	} while (pool->alive);
	return 0;
}

ThreadPool::~ThreadPool()
{
	InterlockedCompareExchange(&alive, FALSE, TRUE);	
	WakeAllConditionVariable(&conditionVariable);
	WaitForMultipleObjects(currentThreadCount, threadHandles, TRUE, 5000);
	for (int i = 0; i < currentThreadCount; i++)
		CloseHandle(threadHandles[i]);
	DeleteCriticalSection(&ñriticalSection);
	logger->logAction(DESTROYED);
	delete logger;
}
