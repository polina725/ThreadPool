#include "Logger.h"

void Logger::logAction(string eventMessage)
{
	if (outputFile.is_open()) {
		outputFile << GetTime()<<" --- "<<eventMessage;
	}
}

Logger::Logger()
{
	outputFile.open(filePath);
}

Logger::~Logger()
{
	outputFile.close();
	cout << "kek\n";
}

string Logger::GetTime()
{
	char* tmp;
	time_t end_time =time(NULL);
	tmp = ctime(&end_time);
	tmp[strlen(tmp) - 1] = '\0';
	return tmp;
}

