#pragma once

#include "stdafx.h"

//ConwayComputeCUI.cpp
int start_simulation(void);

//dispatch.cpp
cl_int dispatch_kernel(struct dispatcher_context *c);
THREAD_RETURN THREAD_CALL dispatcher(void *Context);

//fileio.cpp
bool load_file_to_buffer(unsigned int *buff, char *fname, size_t offsetx, size_t offsety, size_t width, size_t height);

//output.cpp
void print_buffer(unsigned int *buff, size_t width, size_t height);

//setup.cpp
cl_int setup_kernel_stack(cl_kernel kern, cl_mem wdth, cl_mem hght, cl_mem offy, cl_mem src, cl_mem dst);
cl_int load_kernel(cl_context context, cl_device_id *devices, unsigned int devc, cl_program *prog, cl_kernel *kern);
cl_int create_buffers(cl_context context, size_t src_length, size_t width, size_t height,
               cl_mem *src, cl_mem *dst, cl_mem *wdth, cl_mem *hght, cl_mem **offy, size_t *y, unsigned int *src_data, unsigned int c);
cl_int get_devices(cl_device_id **dev, unsigned int *devc, cl_platform_id *pform, int platform, int device);
cl_int initialize_context_cmd_queue(cl_device_id* dev, cl_uint devc, cl_platform_id platform, cl_context *context, cl_command_queue **cmd_queue, cl_uint *cqc);
cl_int compute_buffer_sizes(cl_context context, cl_command_queue* cmd_queue, cl_uint cqc, cl_kernel kern, size_t width, size_t height, size_t swapoffy, size_t **y);

//swap.cpp
bool swap_kernel_buffers(cl_command_queue *cmd_queue, unsigned int cqc, size_t *offy, size_t width, size_t height, cl_mem src, cl_mem dst);
bool swap_kernel_memory_to_system(cl_command_queue *cmd_queue, unsigned int cqc, size_t sy, size_t ey, size_t *offy, size_t width, size_t height, cl_mem clmem, void *sysmem);
bool swap_system_memory_to_kernel(cl_command_queue *cmd_queue, unsigned int cqc, size_t sy, size_t ey, size_t *offy, size_t width, size_t height, cl_mem clmem, void *sysmem);