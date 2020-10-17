#include <windows.h>
#include <iostream>
#include <fstream>
#include "Logger.h"
#include "ThreadPool.h"

DWORD WINAPI ShortFunction(LPVOID lpParam);
DWORD WINAPI SomeFunction(LPVOID lpParam);
DWORD WINAPI OldFunction(LPVOID lpParam);


int main()
{
    ThreadPool pool(4);
    SomeFunction(NULL);
    pool.addTaskToQueue(ShortFunction,NULL);
    pool.addTaskToQueue(SomeFunction,NULL);
    pool.addTaskToQueue(SomeFunction,NULL);
    Sleep(1000);
    return 0;
}

DWORD WINAPI ShortFunction(LPVOID lpParam)
{
    std::string fName = "D:\\users.xml";
    std::ofstream myfile;
    myfile.open(fName);
    myfile << "Writing this to a file.\n";
    myfile.close();
    return 0;
}

DWORD WINAPI SomeFunction(LPVOID lpParam)
{
    DWORD procId = GetCurrentProcessId();
    DWORD threadId =  GetCurrentThreadId();
    throw "aaaaa";
    Sleep(500);
    std::cout << procId << " + " << threadId << std::endl;
    return 0;
}

DWORD WINAPI OldFunction(LPVOID lpParam)
{
    Sleep(5000);
    return 0;
}
