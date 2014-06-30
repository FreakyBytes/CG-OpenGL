#pragma once

//API
#ifndef TERRAIN_API
  #ifdef __TERRAIN_PRECOMPILED_H__
    #define TERRAIN_API __declspec(dllexport)
  #else
    #define TERRAIN_API __declspec(dllimport)
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
