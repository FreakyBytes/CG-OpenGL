#pragma once

//#define VCG_PRINT_SHADERLOG
//API
#ifndef GLUTILS_API
  #ifdef __GLUTILS_PRECOMPILED_H__
    #define GLUTILS_API __declspec(dllexport)
  #else
    #define GLUTILS_API __declspec(dllimport)
  #endif
#endif

//windows defines
//#ifndef BYTE
//#define BYTE unsigned char
//#endif

#ifndef RtlZeroMemory
#include <memory.h>
#define RtlZeroMemory(Destination,Length) memset((Destination),0,(Length))
#endif

#ifndef ZeroMemory
#define ZeroMemory RtlZeroMemory
#endif

#ifndef CALLBACK
#define CALLBACK __stdcall
#endif // !CALLBACK

//enable opengl error checking via CHelper::CheckForError
#define CHECKOPENGLERROR

//enable multithreading
//#define USETHREADING

//windows typedef
typedef unsigned long DWORD, *PDWORD, *LPDWORD; 
