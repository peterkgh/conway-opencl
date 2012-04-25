#pragma once

#include "stdafx.h"
#include "targetver.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

//PRINTF/SCANF Codes
#define SZTF "%Iu"

//Thread stuff
#define THREAD_CALL WINAPI
#define THREAD_RETURN DWORD

#include <cl/opencl.h>
#include <tchar.h>