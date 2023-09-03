#include "pch.h"
#include "ExportFunc.h"

void ExtTest(BYTE* szInput, DWORD dwSize) 
{
    //MessageBoxA(0, (LPCSTR)"abc", (LPCSTR)"pwned", MB_OK);
    OutputDebugString((LPCWSTR)szInput);
}
    