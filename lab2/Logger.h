#pragma once
#include <string>
#include <iostream>
#include <fstream>
#include <ctime>
using namespace std;

//#define MESSAGE_COUNT 4
#define CREATED(num,capacity) "Created "+std::to_string(num)+" threads. Pool capacity: "+std::to_string(capacity)+"\n"
#define DESTROYED "ThreadPool was destroyed\n"
#define NEW_THREAD "New thread was created cause no threads were available\n"
#define OUT_OF_POOL_CAPACITY "ThreadPool limit was reached. No threads was created\n"
#define TASK_ADDED "New task was added\n"
#define EXCEPTION_RAISED(threadID,message) "At thread "+std::to_string(threadID)+" was thrown\nException message:\n"+message+"\n"

class Logger {
	public:
		void logAction(string eventMessage);
		Logger();
		~Logger();
	private:
		const string filePath = "D:\\3course\\5sem\\OSaSP\\lab2\\logger.txt";
		ofstream outputFile;

		string GetTime();
};