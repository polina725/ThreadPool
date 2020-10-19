#include "Thread.h"

Thread::Thread()
{
	isAvailable = TRUE;
	isDead = FALSE;
	InitializeConditionVariable(&cnVariable);
	InitializeCriticalSectionAndSpinCount(&crSection,2000);
	taskForExecution = NULL;
}

Thread::~Thread()
{
	DeleteCriticalSection(&crSection);
}
