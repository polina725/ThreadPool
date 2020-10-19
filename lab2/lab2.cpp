﻿#include <windows.h>
#include <iostream>
#include <fstream>
#include "Logger.h"
#include "ThreadPool.h"

DWORD WINAPI ShortFunction(LPVOID lpParam);
DWORD WINAPI SomeFunction(LPVOID lpParam);
DWORD WINAPI OldFunction(LPVOID lpParam);

/// <summary>
/// ThreadPool to another thread
/// </summary>
/// <returns></returns>
int main()
{
    ThreadPool pool(10,4);
    for (int i = 0; i < 16; i++)
        pool.addTaskToQueue(SomeFunction, (LPVOID)i);
    Sleep(3000);
  //  pool.~ThreadPool();
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
    Sleep(100*(int)lpParam);
    std::cout << (int)lpParam << " " << threadId << std::endl;
    return 0;
}

DWORD WINAPI OldFunction(LPVOID lpParam)
{
    Sleep(5000);
    return 0;
}
