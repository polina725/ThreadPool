#pragma once
#include <windows.h>
#include "TaskPair.h"

class Thread {
	public:
		Thread();
		~Thread();

		CRITICAL_SECTION crSection;
		CONDITION_VARIABLE cnVariable;
		TaskPair* taskForExecution;
		BOOL alive;
};