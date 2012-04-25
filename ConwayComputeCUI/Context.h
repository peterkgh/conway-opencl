#pragma once

#include "stdafx.h"

struct dispatcher_context
{
	cl_command_queue *cmd_queue;
	cl_uint cqc;
	cl_kernel kern;
	size_t columns, rows, print_each, runs;
	int gui_enabled;
	unsigned int *buff0;
	cl_mem src, dst, hght, wdth, *offy;
	size_t *y, swapoffy;
	cl_context context;
    int win_height;
};