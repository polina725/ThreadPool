#include "ThreadPool.h"

#define POOL_CAPACITY 100
#define DEFAULT_AMOUNT_OF_INITIALIZING_THREADS 3

ThreadPool::ThreadPool(int maxThreadCount,int initThreadCount)
{
	if (maxThreadCount <= 0 || maxThreadCount>100)
		maxCount = POOL_CAPACITY;
	else
		maxCount = maxThreadCount;
	if (initThreadCount < 3 || initThreadCount>=maxCount)
		initializedThraedAmount = DEFAULT_AMOUNT_OF_INITIALIZING_THREADS;
	else
		initializedThraedAmount = initThreadCount;
	currentWorkingThreadCount = 0;
	alive = true;
	logger = new Logger();

	InitializeCriticalSectionAndSpinCount(&threadQueueCrSection, 2000);
	InitializeConditionVariable(&threadQueueCnVariable);	
	
	InitializeCriticalSectionAndSpinCount(&taskQueueCrSection, 2000);
	InitializeConditionVariable(&taskQueueCnVariable);

	ManagerThread= CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE)startManager, this, 0, NULL);

	threadHandles = (HANDLE*)calloc(POOL_CAPACITY, sizeof(HANDLE));
	for (int i = 0; i < initThreadCount; i++) {
		threadHandles[i]= CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE)threadStart, this, 0, NULL);
	}

	logger->logAction(CREATED(maxCount, POOL_CAPACITY));
}

int ThreadPool::addTaskToQueue(LPTHREAD_START_ROUTINE functionAddr, LPVOID params)
{
	if (functionAddr == NULL)
		return -1;
	EnterCriticalSection(&taskQueueCrSection);
	taskQueue.push(new TaskPair(functionAddr,params));
	WakeConditionVariable(&taskQueueCnVariable);
	LeaveCriticalSection(&taskQueueCrSection);
	logger->logAction(TASK_ADDED);
	return 0;
}

DWORD WINAPI ThreadPool::threadStart(ThreadPool* pool)
{
	Thread* thread = new Thread();

	EnterCriticalSection(&pool->threadQueueCrSection);
	pool->threadQueue.push(thread);
	WakeConditionVariable(&pool->threadQueueCnVariable);
	LeaveCriticalSection(&pool->threadQueueCrSection);

	while (pool->alive) {

		EnterCriticalSection(&thread->crSection);
		while (thread->taskForExecution == NULL && pool->alive)
			SleepConditionVariableCS(&thread->cnVariable, &thread->crSection, INFINITE);
		if (!pool->alive) {
			LeaveCriticalSection(&thread->crSection);
			return 0;
		}
		TaskPair* task = thread->taskForExecution;
		thread->taskForExecution = NULL;
		pool->currentWorkingThreadCount--;
		LeaveCriticalSection(&thread->crSection);


		try {
			task->functionAddress(task->parameters);
		}
		catch (exception ex) {
			pool->logger->logAction(EXCEPTION_RAISED(GetCurrentThreadId(), ex.what()));
		}
		delete task;
	}
	return 0;
}

DWORD WINAPI ThreadPool::startManager(ThreadPool* pool)
{
	EnterCriticalSection(&pool->threadQueueCrSection);
	while (pool->threadQueue.size() < pool->initializedThraedAmount /*&& pool->alive*/)
		SleepConditionVariableCS(&pool->threadQueueCnVariable, &pool->threadQueueCrSection, INFINITE);
	LeaveCriticalSection(&pool->threadQueueCrSection);

	while (pool->alive) {

		EnterCriticalSection(&pool->taskQueueCrSection);
		while (pool->taskQueue.empty() && pool->alive)
			SleepConditionVariableCS(&pool->taskQueueCnVariable, &pool->taskQueueCrSection, INFINITE);
		if (!pool->alive) {
			LeaveCriticalSection(&pool->taskQueueCrSection);
			return 0;
		}
		TaskPair* task = pool->taskQueue.front();
		pool->taskQueue.pop();
		LeaveCriticalSection(&pool->taskQueueCrSection);
		
		EnterCriticalSection(&pool->threadQueueCrSection);
		if (pool->currentWorkingThreadCount < pool->initializedThraedAmount ) {
			while (pool->threadQueue.front()->taskForExecution != NULL && pool->alive) {
				Thread* thread = pool->threadQueue.front();
				pool->threadQueue.pop();
				pool->threadQueue.push(thread);
			}
			pool->threadQueue.front()->taskForExecution = task;
			WakeConditionVariable(&pool->threadQueue.front()->cnVariable);
		}
		else {
			pool->initializedThraedAmount++;
			pool->threadHandles[pool->initializedThraedAmount] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)threadStart, pool, 0, NULL);
			SleepConditionVariableCS(&pool->threadQueueCnVariable, &pool->threadQueueCrSection, INFINITE);
			pool->threadQueue.back()->taskForExecution = task;
			WakeConditionVariable(&pool->threadQueue.back()->cnVariable);
		}
		pool->currentWorkingThreadCount++;///////////
		LeaveCriticalSection(&pool->threadQueueCrSection);
	}
	return 0;
}

ThreadPool::~ThreadPool()
{
	Sleep(2000*initializedThraedAmount);
	alive = false;

	WakeAllConditionVariable(&threadQueueCnVariable);
	WakeAllConditionVariable(&taskQueueCnVariable);
	WaitForSingleObject(ManagerThread, INFINITE);

	for (int i = 0; i < threadQueue.size(); i++) {
		Thread* thread = threadQueue.front();
		WakeAllConditionVariable(&thread->cnVariable);
		threadQueue.pop();
		threadQueue.push(thread);
	}
	WaitForMultipleObjects(initializedThraedAmount, threadHandles, TRUE, INFINITE);
	while (!threadQueue.empty()) {
		DeleteCriticalSection(&threadQueue.front()->crSection);
		threadQueue.pop();
	}
	for (int i = 0; i < initializedThraedAmount; i++) {
		CloseHandle(threadHandles[i]);
	}

	DeleteCriticalSection(&threadQueueCrSection);
	DeleteCriticalSection(&taskQueueCrSection);
	CloseHandle(ManagerThread);
	logger->logAction(DESTROYED);
	delete logger;
}
