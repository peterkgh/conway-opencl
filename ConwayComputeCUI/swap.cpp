#include "stdafx.h"

bool swap_kernel_buffers(cl_command_queue *cmd_queue, unsigned int cqc, size_t *offy, size_t width, size_t height, cl_mem src, cl_mem dst)
{
	cl_int err;

	printf("Swapping kernel buffers on device\n");

	err = clEnqueueCopyBuffer(cmd_queue[0], src, dst, 0, 0, (width / 8) * height, 0, NULL, NULL);
	if (err)
	{
		printf("clEnqueueCopyBuffer failed! (Error: %d)\n", err);
		return false;
	}

	return true;
}

bool swap_kernel_memory_to_system(cl_command_queue *cmd_queue, unsigned int cqc, size_t sy, size_t ey, size_t *offy, size_t width, size_t height, cl_mem clmem, void *sysmem)
{
	cl_int err;

	printf("Swapping out cells from (0, "SZTF") to ("SZTF", "SZTF")\n", sy, width, ey);

	err = clEnqueueReadBuffer(cmd_queue[0], clmem, CL_TRUE, 0, (ey - sy) * (width / 8), &((unsigned char*)sysmem)[sy * (width / 8)], 0, NULL, NULL);
	if (err)
	{
		printf("clEnqueueReadBuffer failed! (Error: %d)\n", err);
		return false;
	}

	return true;
}

bool swap_system_memory_to_kernel(cl_command_queue *cmd_queue, unsigned int cqc, size_t sy, size_t ey, size_t *offy, size_t width, size_t height, cl_mem clmem, void *sysmem)
{
	cl_int err;
	size_t offset = 0;
	void *zero;
	bool zero_start = false;
	bool zero_end = false;

	if (sy > 0) 
		sy--;
	else
	{
		offset = width / 8;
		zero_start = true;
	}

	if (ey < height - 1)
		ey++;
	else
		zero_end = true;

	printf("Swapping in cells from (0, "SZTF") to ("SZTF", "SZTF")\n", sy, width, ey);

	err = clEnqueueWriteBuffer(cmd_queue[0], clmem, CL_TRUE, offset, (ey - sy) * (width / 8), &((unsigned char*)sysmem)[sy * (width / 8)], 0, NULL, NULL);
	if (err)
	{
		printf("clEnqueueWriteBuffer failed! (Error: %d)\n", err);
		return false;
	}

	if (zero_start || zero_end)
	{
		zero = malloc(width / 8);
		if (!zero) return false;

		memset(zero, 0, width / 8);

		if (zero_start)
		{
			printf("Writing "SZTF" bytes of zeros to offset: 0\n", (width/8));
			err = clEnqueueWriteBuffer(cmd_queue[0], clmem, CL_TRUE, 0, width / 8, zero, 0, NULL, NULL);
			if (err)
			{
				printf("clEnqueueWriteBuffer failed! (Error: %d)\n", err);
				return false;
			}
		}

		if (zero_end)
		{
			printf("Writing "SZTF" bytes of zeros to offset: "SZTF"\n", (width/8), ((ey - sy) * (width / 8) + offset));
			err = clEnqueueWriteBuffer(cmd_queue[0], clmem, CL_TRUE, ((ey - sy) * (width / 8) + offset), width / 8, zero, 0, NULL, NULL);
			if (err)
			{
				printf("clEnqueueWriteBuffer failed! (Error: %d)\n", err);
				return false;
			}
		}

		free(zero);
	}
	
	return true;
}