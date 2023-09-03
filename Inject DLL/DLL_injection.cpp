#include "DLL_injection.h"

BOOL InjectDLL(DWORD dwPID, LPCSTR dllPath)
{
    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwPID);
    if (hProcess == NULL)
    {
        std::cerr << "Error in opening process: " << GetLastError();
        return FALSE;
    }

    LPVOID dllPathAddr = VirtualAllocEx(hProcess, NULL, strlen(dllPath) + 1, MEM_COMMIT, PAGE_READWRITE);
    if (dllPathAddr == NULL)
    {
        std::cerr << "Error in allocating address: " << GetLastError();
        return FALSE;
    }

    
    BOOL writeResult = WriteProcessMemory(hProcess, dllPathAddr, dllPath, strlen(dllPath) + 1, NULL);
    if (!writeResult)
    {
        std::cerr << "Error in writing to process memory: " << GetLastError();
        return FALSE;
    }

    // get address of LoadLibraryA function in kernel32 module
    FARPROC loadLibraryAddr = GetProcAddress(GetModuleHandle(TEXT("kernel32.dll")), "LoadLibraryA");
    if (loadLibraryAddr == NULL)
    {
        std::cerr << "Error in getting address of LoadLibraryA function in kernel32 module: " << GetLastError();
        return FALSE;
    }

    //Create a thread in process to call LoadLibraryA function with parameter is DLL path
    HANDLE hThread = CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)loadLibraryAddr, dllPathAddr, 0, NULL);
    if (hThread == NULL)
    {
        std::cerr << "Error in create remote thread: " << GetLastError();
        return FALSE;
    }
    
    //wait for process finish
    DWORD waitResult = WaitForSingleObject(hThread, INFINITE);
    if (waitResult != WAIT_OBJECT_0)
    {
        std::cerr << "Error in running thread: " << GetLastError();
        return FALSE;
    }
    

    DWORD loadResult;
    if (!GetExitCodeThread(hThread, &loadResult))
    {
        std::cerr << "Error in getting exit code of thread: " << GetLastError();
        return FALSE;
    }

    CloseHandle(hThread);
    VirtualFreeEx(hProcess, dllPathAddr, 0, MEM_RELEASE);
    CloseHandle(hProcess);

    return loadResult;
}


HMODULE GetRemoteModuleHandle(DWORD dwPID, BYTE* lpModuleName)
{
    LoadLibraryA((LPCSTR)lpModuleName);

    HMODULE hMods[1024];
    DWORD cbNeeded;
    unsigned int i;

    // Get a handle to the process.
    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS | PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, dwPID);
    if (hProcess == NULL)
    {
        CloseHandle(hProcess);
        std::cerr << "Error in opening process: " << GetLastError();
        return NULL;
    }

    // Get a list of all the modules in this process.
    if (EnumProcessModulesEx(hProcess, hMods, sizeof(hMods), &cbNeeded, LIST_MODULES_ALL))
    {
        for (i = 0; i < (cbNeeded / sizeof(HMODULE)); i++)
        {
            TCHAR szModName[MAX_PATH];

            // Get the full path to the module's file.
            if (GetModuleFileNameEx(hProcess, hMods[i], szModName, sizeof(szModName) / sizeof(TCHAR)))
            {
                // Compare if the module path name have the file name same as the target name
                if (_tcsstr(szModName, _T("DLLMaker.dll")))
                {
                    // Found a match
                    CloseHandle(hProcess);
                    return hMods[i];
                    
                }
            }
        }
    }
    
    CloseHandle(hProcess);
    return NULL;
}

BOOL CallExtTest(BYTE* dllPath, DWORD dwPID, BYTE* szInput, DWORD dwSize)
{
    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwPID);
    if (!hProcess) 
    {
        std::cerr << "Error in opening process: " << GetLastError();
        return false;
    }

    HMODULE hModule= GetRemoteModuleHandle(dwPID, dllPath);
    if (!hModule)
    {
        CloseHandle(hProcess);
        std::cerr << "Error in getting remote module handle: " << GetLastError();
        return FALSE;
    }

    // get address of ExtTest function in DLLMaker.dll module
    DLL_FUNC dllFunc = (DLL_FUNC)GetProcAddress(hModule, "ExtTest");
    if (!dllFunc)
    {
        CloseHandle(hProcess);
        std::cerr << "Error in getting address of ExtTest function in DLLMaker.dll module: " << GetLastError();
        return FALSE;
    }


    LPVOID funcAddr = VirtualAllocEx(hProcess, NULL, dwSize, MEM_COMMIT, PAGE_READWRITE);
    if (funcAddr == NULL)
    {
        CloseHandle(hProcess);
        std::cerr << "Error in allocating address: " << GetLastError();
        return FALSE;
    }

    BOOL writeResult = WriteProcessMemory(hProcess, funcAddr, szInput, dwSize, NULL);
    if (!writeResult)
    {
        VirtualFreeEx(hProcess, funcAddr, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        std::cerr << "Error in writing to process memory: " << GetLastError();
        return FALSE;
    }

    HANDLE hThread = CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)dllFunc, funcAddr, 0, NULL);
    if (hThread == NULL)
    {
        CloseHandle(hThread);
        VirtualFreeEx(hProcess, funcAddr, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        std::cerr << "Error in creating remote thread: " << GetLastError();
        return FALSE;
    }

    DWORD waitResult = WaitForSingleObject(hThread, INFINITE);
    if (waitResult != WAIT_OBJECT_0)
    {
        CloseHandle(hThread);
        VirtualFreeEx(hProcess, funcAddr, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        std::cerr << "Error in running thread: " << GetLastError();
        return FALSE;
    }

    CloseHandle(hThread);
    VirtualFreeEx(hProcess, funcAddr, 0, MEM_RELEASE);
    CloseHandle(hProcess);
    
    return TRUE;
}
