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
    ThreadPool pool(15,10);
    pool.addTaskToQueue(ShortFunction, NULL);
    for (int i = 0; i < 30; i++) {
        pool.addTaskToQueue(SomeFunction, (LPVOID)i);
        Sleep(100);
    }
   // Sleep(2000);
    return 0;
}

DWORD WINAPI ShortFunction(LPVOID lpParam)
{
    std::string fName = "..\\tmp.txt";
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
