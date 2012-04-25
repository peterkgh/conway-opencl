#pragma once

#include <mach/mach_time.h>

//PRINTF/SCANF Codes
#define SZTF "%zu"

//Thread stuff
#define THREAD_CALL
#define THREAD_RETURN void*

//Boolean
#define bool char
#define true 1
#define false 0

#include <pthread.h>

#include <OpenCL/OpenCL.h>

#include <GLUT/GLUT.h>