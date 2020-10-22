#include "Logger.h"

void Logger::logAction(string eventMessage)
{
	EnterCriticalSection(&crSection);
	if (outputFile.is_open()) {
		outputFile << GetTime()<<" --- "<<eventMessage;
	}
	LeaveCriticalSection(&crSection);
}

Logger::Logger()
{
	InitializeCriticalSectionAndSpinCount(&crSection, 2000);
	outputFile.open(filePath);
}

Logger::~Logger()
{
	outputFile.close();
	DeleteCriticalSection(&crSection);
}

string Logger::GetTime()
{
	char* tmp;
	time_t end_time =time(NULL);
	tmp = ctime(&end_time);
	tmp[strlen(tmp) - 1] = '\0';
	return tmp;
}

