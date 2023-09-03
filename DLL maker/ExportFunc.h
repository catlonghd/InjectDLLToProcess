#pragma once
#ifdef TESTLIBRARY_EXPORTS
#define TESTLIBRARY_API __declspec(dllexport)
#else
#define TESTLIBRARY_API __declspec(dllimport)
#endif

extern "C" __declspec(dllexport) void __stdcall ExtTest(BYTE * szInput, DWORD dwSize);


