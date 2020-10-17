#pragma once
#include <windows.h>

class TaskPair {
	public:
		LPTHREAD_START_ROUTINE functionAddress;
		LPVOID parameters;
		TaskPair(LPTHREAD_START_ROUTINE funcAddr, LPVOID params);
};
