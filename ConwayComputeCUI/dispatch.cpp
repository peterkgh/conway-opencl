#include "stdafx.h"

cl_int
dispatch_kernel(struct dispatcher_context *c)
{
    cl_int err;

	for (unsigned int i = 0; i < c->cqc; i++)
	{
		size_t global_work_size[2];

		global_work_size[0] = c->columns / 32;
		global_work_size[1] = ((i < (c->cqc - 1)) ? c->y[i+1] : c->swapoffy) - c->y[i];

		if ((err = setup_kernel_stack(c->kern, c->wdth, c->hght, c->offy[i], c->src, c->dst)))
			return -1;

		for (unsigned int j = 0; j < (c->rows / c->swapoffy); j++)
		{
			err = clEnqueueNDRangeKernel(c->cmd_queue[i], c->kern, 2, NULL, global_work_size, NULL, 0, NULL, NULL);
			if (err)
			{
				printf("clEnqueueNDRangeKernel failed! (Error: %d)\n", err);
				return err;
			}

			printf("Kernel entered command queue\n");

			if (!swap_kernel_memory_to_system(c->cmd_queue, c->cqc, 0, c->win_height, c->y, c->columns, c->win_height, c->dst, c->buff0))
				return -1;
	
			if (!swap_kernel_buffers(c->cmd_queue, c->cqc, c->y, c->columns, c->rows, c->dst, c->src))
				return -1;
		}
	}
    
    return 0;
}

THREAD_RETURN THREAD_CALL dispatcher(void *Context)
{
	struct dispatcher_context *c = (struct dispatcher_context *)Context;
	cl_int err;
	time_t t;
	size_t i;

	t = time(NULL);
    for (i = 0; i < c->runs; i++)
    {   
		printf("Starting run "SZTF"\n", (i+1));

        if ((err = dispatch_kernel(c)))
			break;
        
        if (c->gui_enabled || (c->print_each != 0 && ((i+1) % c->print_each) == 0))
        {
			if (c->gui_enabled)
				update_gui(c->buff0, c->columns, c->win_height);
			else
				print_buffer(c->buff0, c->columns, c->rows);
        }

		//fix case with 0 height window
		if (c->win_height == 0)
			c->win_height = 1;
    }

	for (unsigned int j = 0; j < c->cqc; j++)
		clFlush(c->cmd_queue[j]);

	for (unsigned int j = 0; j < c->cqc; j++)
		clFinish(c->cmd_queue[j]);

	printf("Final purge of command queue(s)\n");

	t = time(NULL) - t;

    printf("Completed "SZTF" runs of "SZTF"x"SZTF" in %lu seconds\n", i, c->columns, c->rows, t);

	for (unsigned int j = 0; j < c->cqc; j++)
		 clReleaseCommandQueue(c->cmd_queue[j]);

	clReleaseMemObject(c->src);
	clReleaseMemObject(c->dst);
	clReleaseKernel(c->kern);

	free(c->buff0);

	clReleaseContext(c->context);

	free(c);
	
	return (THREAD_RETURN)0;
}