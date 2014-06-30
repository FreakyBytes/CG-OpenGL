#pragma once

//API
#ifndef GUI_API
  #ifdef __GUI_PRECOMPILED_H__
    #define GUI_API __declspec(dllexport)
  #else
    #define GUI_API __declspec(dllimport)
  #endif
#endif


#ifndef RtlZeroMemory
#include <memory.h>
#define RtlZeroMemory(Destination,Length) memset((Destination),0,(Length))
#endif

#ifndef ZeroMemory
#define ZeroMemory RtlZeroMemory
#endif

//windows typedef
typedef unsigned long DWORD, *PDWORD, *LPDWORD; 

#ifndef CALLBACK
#define CALLBACK __stdcall
#endif // !CALLBACK

#define USESHADER
//#define USELOD

