#include <windows.h>
#include <iostream>
#include "DLL_injection.h"


int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        std::cerr << "Invalid number of parameters!\n";
        return -1;
    }
    
    BYTE dllPath[MAX_PATH];
    GetFullPathNameA("DLLMaker.dll", MAX_PATH, (char*)dllPath, NULL);
    
    DWORD dwPID = atoi(argv[1]);
    
    HMODULE dllBaseAddr = (HMODULE)InjectDLL(dwPID, (LPCSTR)dllPath);
    if (!dllBaseAddr)
    {
        std::cout << "Inject DLL failed" << std::endl;
        return -1;
    }
   
    BYTE szInput[50] = "Hello from ExtTest\n";
    DWORD dwSize = strlen((const char*) szInput);
    
    // Call ExtTest function
    BOOL callResult = CallExtTest(dllPath, dwPID, szInput, dwSize);
    if (!callResult)
    {
        return -1;
    }

    std::cout << "Success!\n";
   
    return 0;
}
