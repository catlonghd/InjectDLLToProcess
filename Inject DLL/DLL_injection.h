#pragma once
#include <Windows.h>
#include <iostream>
#include <psapi.h>
#include <TlHelp32.h>
#include <tchar.h>

typedef void (*DLL_FUNC)(BYTE*, DWORD);
BOOL InjectDLL(DWORD dwPID, LPCSTR dllPath);
HMODULE GetRemoteModuleHandle(DWORD dwPID, BYTE* lpModuleName);
BOOL CallExtTest(BYTE* dllPath, DWORD dwPID, BYTE* szInput, DWORD dwSize);