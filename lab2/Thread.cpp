#include "Thread.h"

Thread::Thread()
{
	alive = TRUE;
	InitializeConditionVariable(&cnVariable);
	InitializeCriticalSectionAndSpinCount(&crSection,2000);
	taskForExecution = NULL;
}

Thread::~Thread()
{
	DeleteCriticalSection(&crSection);
}
