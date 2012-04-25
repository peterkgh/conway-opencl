#pragma once

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

#include <sys/time.h>

#include <CL/opencl.h>

#include <GL/glut.h>