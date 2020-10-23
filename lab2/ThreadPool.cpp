#include "ThreadPool.h"

#define POOL_CAPACITY 100
#define DEFAULT_AMOUNT_OF_INITIALIZING_THREADS 3

ThreadPool::ThreadPool(int maxThreadCount,int initThreadCount)
{
	if (maxThreadCount <= 0 || maxThreadCount>100)
		maxCount = POOL_CAPACITY;
	else
		maxCount = maxThreadCount;
	if (initThreadCount < 3 || initThreadCount >= maxCount) {
		initializedThreadAmount = DEFAULT_AMOUNT_OF_INITIALIZING_THREADS;
		startInitValue = DEFAULT_AMOUNT_OF_INITIALIZING_THREADS;
	}
	else {
		initializedThreadAmount = initThreadCount;
		startInitValue = initThreadCount;
	}
	currentWorkingThreadCount = 0;
	alive = true;

	InitializeCriticalSectionAndSpinCount(&threadQueueCrSection, 2000);
	InitializeConditionVariable(&threadQueueCnVariable);	
	
	InitializeCriticalSectionAndSpinCount(&taskQueueCrSection, 2000);
	InitializeConditionVariable(&taskQueueCnVariable);

	ManagerThread= CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE)startManager, this, 0, NULL);
	managerIsAlive = TRUE;
	threadHandles = (HANDLE*)calloc(POOL_CAPACITY, sizeof(HANDLE));
	for (int i = 0; i < initThreadCount; i++) {
		threadHandles[i]= CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE)threadStart, this, 0, NULL);
	}

	logger.logAction(CREATED(initThreadCount, maxCount, POOL_CAPACITY));
}

int ThreadPool::addTaskToQueue(LPTHREAD_START_ROUTINE functionAddr, LPVOID params)
{
	if (functionAddr == NULL || !alive) {
		return -1;
	}
	if (initializedThreadAmount == maxCount)
		logger.logAction(OUT_OF_POOL_CAPACITY);
	EnterCriticalSection(&taskQueueCrSection);
	taskQueue.push(new TaskPair(functionAddr,params));
	WakeConditionVariable(&taskQueueCnVariable);
	LeaveCriticalSection(&taskQueueCrSection);

	logger.logAction(TASK_ADDED);
	return 0;
}

DWORD WINAPI ThreadPool::threadStart(ThreadPool* pool)
{
	Thread* thread = new Thread();

	EnterCriticalSection(&pool->threadQueueCrSection);
	pool->threadQueue.push(thread);
	WakeConditionVariable(&pool->threadQueueCnVariable);
	LeaveCriticalSection(&pool->threadQueueCrSection);

	pool->logger.logAction(INIT_THREAD(GetCurrentThreadId()));

	while (pool->managerIsAlive) {

		EnterCriticalSection(&thread->crSection);
		while (thread->taskForExecution == NULL && pool->managerIsAlive)
			SleepConditionVariableCS(&thread->cnVariable, &thread->crSection, INFINITE);
		if (!pool->managerIsAlive) {
			InterlockedExchange(&pool->currentWorkingThreadCount, pool->currentWorkingThreadCount - 1);
			LeaveCriticalSection(&thread->crSection);			
			if (thread->taskForExecution != NULL) {
				pool->logger.logAction("aaaaa1 " + std::to_string(GetCurrentThreadId()) + "\n");
			}
			return 0;
		}
		TaskPair* task = thread->taskForExecution;
		thread->taskForExecution = NULL;
		InterlockedExchange(&pool->currentWorkingThreadCount, pool->currentWorkingThreadCount - 1);
		LeaveCriticalSection(&thread->crSection);


		try {
				task->functionAddress(task->parameters);
		}
		catch (exception ex) {
			pool->logger.logAction(EXCEPTION_RAISED(GetCurrentThreadId(), ex.what()));
		}
		delete task;
	}
	if (thread->taskForExecution != NULL) {
		pool->logger.logAction("aaaaa1 " + std::to_string(GetCurrentThreadId()) + "\n");
	}
	return 0;
}

DWORD WINAPI ThreadPool::startManager(ThreadPool* pool)
{
	bool allTaskExecuted = false;

	EnterCriticalSection(&pool->threadQueueCrSection);
	while (pool->threadQueue.size() < pool->initializedThreadAmount)
		SleepConditionVariableCS(&pool->threadQueueCnVariable, &pool->threadQueueCrSection, INFINITE);
	LeaveCriticalSection(&pool->threadQueueCrSection);

	while (pool->alive || !pool->taskQueue.empty()) {
	
		EnterCriticalSection(&pool->taskQueueCrSection);
		while (pool->taskQueue.empty() && pool->alive)
			SleepConditionVariableCS(&pool->taskQueueCnVariable, &pool->taskQueueCrSection, 2000);
		if (!pool->alive && pool->taskQueue.empty()) {
			LeaveCriticalSection(&pool->taskQueueCrSection);
			while (!pool->allTaskExecuted()) { 
				//pool->logger.logAction("uuuuu1 " + std::to_string(GetCurrentThreadId()) + " " + std::to_string(pool->taskQueue.size()) + "\n"); 
				Sleep(4000); 
			}
			InterlockedExchange(&pool->managerIsAlive, FALSE);
			return 0;
		}
		TaskPair* task = pool->taskQueue.front();
		pool->taskQueue.pop();
		LeaveCriticalSection(&pool->taskQueueCrSection);
		
		EnterCriticalSection(&pool->threadQueueCrSection);
		if (pool->currentWorkingThreadCount < pool->initializedThreadAmount ) {
			while (pool->threadQueue.front()->taskForExecution != NULL) {
				Thread* thread = pool->threadQueue.front();
				pool->threadQueue.pop();
				pool->threadQueue.push(thread);
			}
			pool->threadQueue.front()->taskForExecution = task;
			WakeConditionVariable(&pool->threadQueue.front()->cnVariable);
			LeaveCriticalSection(&pool->threadQueueCrSection);
			//pool->logger.logAction("in " + std::to_string((int)task->parameters) + "\n");
		}
		else if (pool->initializedThreadAmount < pool->maxCount) {
			pool->initializedThreadAmount++;
			pool->threadHandles[pool->initializedThreadAmount] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)threadStart, pool, 0, NULL);
			SleepConditionVariableCS(&pool->threadQueueCnVariable, &pool->threadQueueCrSection, INFINITE);
			pool->threadQueue.back()->taskForExecution = task;
			WakeConditionVariable(&pool->threadQueue.back()->cnVariable);
			LeaveCriticalSection(&pool->threadQueueCrSection);
			pool->logger.logAction(NEW_THREAD(GetThreadId(pool->threadHandles[pool->initializedThreadAmount])));
		//	pool->logger.logAction("in1 " + std::to_string((int)task->parameters) + "\n");
		}
		InterlockedExchange(&pool->currentWorkingThreadCount, pool->currentWorkingThreadCount + 1);
		
	}
	while (!pool->allTaskExecuted()) { 
	//	pool->logger.logAction("uuuuu " + std::to_string(GetCurrentThreadId()) + " " + std::to_string(pool->taskQueue.size()) + "\n"); 
		Sleep(4000); 
	}
	InterlockedExchange(&pool->managerIsAlive, FALSE);
	return 0;
}

bool ThreadPool::allTaskExecuted()
{
	EnterCriticalSection(&threadQueueCrSection);
	bool allTaskExecuted = true;
	for (int i = 0; i < threadQueue.size(); i++) {
		Thread* thread = threadQueue.front();
		threadQueue.pop();
		threadQueue.push(thread);
		allTaskExecuted = allTaskExecuted && (thread->taskForExecution == NULL);
	}
	LeaveCriticalSection(&threadQueueCrSection);
	return allTaskExecuted;
}



ThreadPool::~ThreadPool()
{
	alive = false;
	WaitForSingleObject(ManagerThread, INFINITE);
	WakeAllConditionVariable(&threadQueueCnVariable);
	WakeAllConditionVariable(&taskQueueCnVariable);
//	std::cout << "Working " << currentWorkingThreadCount << std::endl;/////////////
	for (int i = 0; i < threadQueue.size(); i++) {
		Thread* thread = threadQueue.front();
		WakeAllConditionVariable(&thread->cnVariable);
		threadQueue.pop();
		threadQueue.push(thread);
	}
	WaitForMultipleObjects(initializedThreadAmount, threadHandles, TRUE, INFINITE);
	while (!threadQueue.empty()) {
		DeleteCriticalSection(&threadQueue.front()->crSection);
		threadQueue.pop();
	}
	for (int i = 0; i < initializedThreadAmount; i++) {
		CloseHandle(threadHandles[i]);
	}

	DeleteCriticalSection(&threadQueueCrSection);
	DeleteCriticalSection(&taskQueueCrSection);
	CloseHandle(ManagerThread);
	logger.logAction(DESTROYED);
}
