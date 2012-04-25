#pragma once

#if defined(__linux__)
#include "platform_lnx.h"
#elif defined(__APPLE__)
#include "platform_osx.h"
#elif defined(_WIN32)
#include "platform_win.h"
#else
#error Unsupported OS
#endif

#include "Context.h"

unsigned long long get_time(void);
bool initialize_window(void);
void update_gui(unsigned int *buff, size_t width, size_t height);
int start_dispatcher(struct dispatcher_context *c);