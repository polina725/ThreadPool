#include "TaskPair.h"

TaskPair::TaskPair(LPTHREAD_START_ROUTINE funcAddr, LPVOID params)
{
	functionAddress = funcAddr;
	parameters = params;
}
