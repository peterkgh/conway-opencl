#include "stdafx.h"

char * load_program_source(const char *filename)
{ 
	
	struct stat statbuf;
	FILE *fh; 
	char *source;
	size_t charsread;
	
	fh = fopen(filename, "r");
	if (fh == 0)
		return 0; 

	stat(filename, &statbuf);
	source = (char *) malloc(statbuf.st_size + 1);
	charsread = fread(source, sizeof(char), statbuf.st_size, fh);
	source[charsread] = 0; 

	fclose(fh);

	return source; 
}

cl_int
setup_kernel_stack(cl_kernel kern, cl_mem wdth, cl_mem hght, cl_mem offy, cl_mem src, cl_mem dst)
{
    cl_int err;
    
    err = clSetKernelArg(kern, 0, sizeof(wdth), &wdth);
    if (err)
    {
        printf("clSetKernelArg #1 failed! (Error: %d)\n", err);
        return err;
    }
    
    err = clSetKernelArg(kern, 1, sizeof(hght), &hght);
    if (err)
    {
        printf("clSetKernelArg #2 failed! (Error: %d)\n", err);
        return err;
    }
    
    err = clSetKernelArg(kern, 2, sizeof(offy), &offy);
    if (err)
    {
        printf("clSetKernelArg #3 failed! (Error: %d)\n", err);
        return err;
    }

	err = clSetKernelArg(kern, 3, sizeof(src), &src);
    if (err)
    {
        printf("clSetKernelArg #4 failed! (Error: %d)\n", err);
        return err;
    }

	err = clSetKernelArg(kern, 4, sizeof(dst), &dst);
    if (err)
    {
        printf("clSetKernelArg #5 failed! (Error: %d)\n", err);
        return err;
    }
 
    printf("Kernel stack is ready\n");
    
    return 0;
}

cl_int
load_kernel(cl_context context, cl_device_id *devices, unsigned int devc, cl_program *prog, cl_kernel *kern)
{
    cl_int err;
    char* source = load_program_source("ConwayKernel.cl");
    
    *prog = clCreateProgramWithSource(context, 1, (const char**)&source,
                                      NULL, &err);
    if (*prog == NULL)
    {
        printf("clCreateProgramWithSource failed! (Error: %d)\n", err);
        return err;
    }
    
    err = clBuildProgram(*prog, devc, devices, NULL, NULL, NULL);
	if (err)
	{
		printf("clBuildProgram failed! (Error: %d)\n", err);
		size_t length;
		char buffer[2048];
		clGetProgramBuildInfo(*prog, devices[0], CL_PROGRAM_BUILD_LOG, sizeof(buffer), buffer, &length);
		printf("Build log: %s\n", buffer);
		return err;
	}
    
    *kern = clCreateKernel(*prog, "evaluate_bit", &err);
    if (*kern == NULL)
    {
        printf("clCreateKernel failed! (Error: %d)\n", err);
        return err;
    }
    
    printf("Kernel build completed successfully\n");
    
    return 0;
}

cl_int
create_buffers(cl_context context, size_t src_length, size_t width, size_t height,
               cl_mem *src, cl_mem *dst, cl_mem *wdth, cl_mem *hght, cl_mem **offy,
               size_t *y, unsigned int *src_data, unsigned int c)
{
    cl_int err;
    size_t total_length = 0;
	cl_mem *h;
	cl_mem *oy;

	h = (cl_mem*)malloc(c * sizeof(cl_mem));
	if (h == NULL)
	{
		printf("Memory allocation failed!\n");
		return -1;
	}

	oy = (cl_mem*)malloc(c * sizeof(cl_mem));
	if (oy == NULL)
	{
		printf("Memory allocation failed!\n");
		return -1;
	}

    *src = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
                          src_length, src_data, &err);
    if (*src == NULL)
    {
        printf("clCreateBuffer failed! (Error: %d)\n", err);
        return err;
    }
    
    printf("Allocated source buffer: "SZTF" bytes\n", src_length);
    
    total_length += src_length;
    
    *dst = clCreateBuffer(context, CL_MEM_READ_WRITE, src_length,
                          NULL, &err);
    if (*dst == NULL)
    {
        printf("clCreateBuffer #2 failed! (Error: %d)\n", err);
        return err;
    }
    
    printf("Allocated target buffer: "SZTF" bytes\n", src_length);
    
    total_length += src_length;

	*wdth = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
						   sizeof(width), &width, &err);
	if (*wdth == NULL)
	{
		printf("clCreateBuffer #3 failed! (Error: %d)\n", err);
		return err;
	}
 
	total_length += sizeof(width);

	printf("Allocated width buffer: "SZTF" bytes\n", sizeof(width));

	*hght = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
						   sizeof(height), &height, &err);
	if (*hght == NULL)
	{
		printf("clCreateBuffer #4 failed! (Error: %d)\n", err);
		return err;
	}
 
	total_length += sizeof(height);

	printf("Allocated height buffer: "SZTF" bytes\n", sizeof(height));

	for (unsigned int i = 0; i < c; i++)
	{
		oy[i] = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(y[i]), 
							   &y[i], &err);
		if (oy[i] == NULL)
		{
			printf("clCreateBuffer #6 failed! (Error: %d)\n", err);
			return err;
		}
    
		printf("Allocated Y offset buffer for device %d: "SZTF" bytes\n", i, sizeof(y[i]));

		total_length += sizeof(y[i]);
	}
    
    printf("Total buffer allocation: "SZTF" bytes\n", total_length);

	*offy = oy;
    
    return 0;
}

cl_int
get_devices(cl_device_id **dev, unsigned int *devc, cl_platform_id *pform, int platform, int device)
{
    cl_int err;
    cl_char vendor[64] = {0};
    cl_char model[64] = {0};
    cl_uint platform_count;
    cl_platform_id* platforms;
	cl_uint *device_count;
	cl_device_id *devices;

    err = clGetPlatformIDs(0, NULL, &platform_count);
    if (err)
    {
        printf("clGetPlatformIDs #1 failed! (Error: %d)\n", err);
        return err;
    }

	printf("Found %d OpenCL implementation(s)\n", platform_count);

	if ((unsigned int)platform >= platform_count)
	{
		printf("Invalid platform index specified!\n");
		return -1;
	}
    
    platforms = (cl_platform_id*)malloc(platform_count * sizeof(cl_platform_id));
    if (!platforms)
    {
        printf("Failed to allocate memory\n");
        return -1;
    }
    
    err = clGetPlatformIDs(platform_count, platforms, NULL);
    if (err)
    {
        printf("clGetPlatformIDs #2 failed! (Error: %d)\n", err);
        return err;
    }

	platforms[0] = platforms[platform];
	platform_count = 1;

	device_count = (cl_uint*)malloc(platform_count * sizeof(cl_uint));
	if (!device_count)
	{
		printf("Failed to allocate memory\n");
		free(platforms);
		return -1;
	}

	*devc = 0;
    
    for (unsigned int i = 0; i < platform_count; i++)
    {
        char name[32];
        
        if (!clGetPlatformInfo(platforms[i], CL_PLATFORM_VENDOR, sizeof(name), name, NULL))
        {
            printf("Using %s platform\n", name);
        }
        else
        {
            printf("Invalid platform information\n");
        }
        
        err = clGetDeviceIDs(platforms[i], CL_DEVICE_TYPE_ALL, 0, NULL, &device_count[i]);
        if (err == CL_DEVICE_NOT_FOUND)
        {
            printf("Failed to find a supported OpenCL device on current platform!\n");
            break;
        }

		printf("Found %d device(s) on the selected platform\n", device_count[i]);

		*devc += device_count[i];
    }

	if (device != -1 && (unsigned int)device >= *devc)
	{
		printf("Invalid device index!\n");
		free(device_count);
		free(platforms);
		return -1;
	}

	devices = (cl_device_id*)malloc((*devc) * sizeof(cl_device_id));
	if (!devices)
	{
		printf("Failed to allocate memory\n");
		free(device_count);
		free(platforms);
		return -1;
	}
    
	unsigned int current_device_count = 0;
	for (unsigned int i = 0; i < platform_count; i++)
    {
        err = clGetDeviceIDs(platforms[i], CL_DEVICE_TYPE_ALL, device_count[i], &devices[current_device_count], NULL);
        if (err)
        {
            printf("Failed to find a supported OpenCL device!\n");
            break;
		}

		current_device_count += device_count[i];
    }

	if (device != -1)
	{
		devices[0] = devices[device];
		*devc = 1;
	}

	for (unsigned int j = 0; j < *devc; j++)
	{
		if (!clGetDeviceInfo(devices[j], CL_DEVICE_VENDOR, sizeof(vendor), vendor, NULL) &&
		    !clGetDeviceInfo(devices[j], CL_DEVICE_NAME, sizeof(model), model, NULL))
			printf("Using OpenCL device: %s %s\n", vendor, model);
	}

	*dev = devices;
	*pform = platforms[0];

    free(platforms);
	free(device_count);
    
    return 0;
}

cl_int
initialize_context_cmd_queue(cl_device_id* dev, cl_uint devc, cl_platform_id platform, cl_context *context, cl_command_queue **cmd_queue, cl_uint *cqc)
{
    cl_int err;
    cl_command_queue *cmdq;
	cl_context_properties props[] = {CL_CONTEXT_PLATFORM, (cl_context_properties)platform, 0};

	cmdq = (cl_command_queue*)malloc(devc * sizeof(cl_command_queue));
	if (cmdq == NULL)
	{
		printf("Memory allocation failed!\n");
		return -1;
	}

    *context = clCreateContext(props, devc, dev, NULL, NULL, &err);
    if (*context == NULL)
    {
        printf("clCreateContext failed! (Error: %d)\n", err);
		free(cmdq);
        return err;
    }
    
	for (unsigned int i = 0; i < devc; i++)
	{
		cmdq[i] = clCreateCommandQueue(*context, dev[i], 0, &err);
		if (cmdq[i] == NULL)
		{
			printf("clCreateCommandQueue failed! (Error: %d)\n", err);
            free(cmdq);
			return err;
		}
 
		printf("Created command queue for device %d\n", i);
	}

	*cmd_queue = cmdq;
	*cqc = devc;
    
    return 0;
}

cl_int
compute_buffer_sizes(cl_context context, cl_command_queue* cmd_queue, cl_uint cqc, cl_kernel kern, size_t width, size_t height, size_t swapoffy, size_t **y)
{
	size_t *yy, offset = 0;
	cl_mem wdth, hght, *offy, src, dst;
	unsigned long long *delays;
	double total = 0;

	yy = (size_t*)malloc(cqc * sizeof(*yy));
	if (yy == NULL)
	{
		printf("Memory allocation failed!\n");
		return -1;
	}

	//simple case with only 1 device
	if (cqc == 1)
	{
		yy[0] = 0;
		*y = yy;
		return 0;
	}

	delays = (unsigned long long*)malloc(cqc * sizeof(*delays));
	if (delays == NULL)
	{
		printf("Memory allocation failed!\n");
		return -1;
	}

	if (create_buffers(context, swapoffy * (width / 8), width,
		swapoffy, &src, &dst, &wdth, &hght, &offy, &offset, NULL, 1))
		return -1;

	if (setup_kernel_stack(kern, wdth, hght, offy[0], src, dst))
		return -1;

	unsigned int execs = 1;

	while (TRUE)
	{
		unsigned int i;

		for (i = 0; i < cqc; i++)
		{
			size_t global_work_size[2] = {width / 32, swapoffy};

			//Dry run first to allow the buffers to get into memory
			if (clEnqueueNDRangeKernel(cmd_queue[i], kern, 2, NULL, global_work_size, NULL, 0, NULL, NULL))
				return -1;
			clFinish(cmd_queue[i]);

			//Now the real deal
			delays[i] = get_time();
			for (unsigned int j = 0; j < execs; j++)
			{
				if (clEnqueueNDRangeKernel(cmd_queue[i], kern, 2, NULL, global_work_size, NULL, 0, NULL, NULL))
					return -1;
			}
			clFinish(cmd_queue[i]);
			delays[i] = get_time() - delays[i];

			if (delays[i] == 0) break;

			printf("Device %d took %lu time units to complete the timing run\n", i, (long unsigned int)delays[i]);
		}

		if (delays[i] == 0)
		{
			printf("Device %d completed %d executions in 0 time units\n", i, execs);
			execs++;
			printf("Retrying timing runs with %d executions\n", execs);
			continue;
		}

		break;
	}

	for (unsigned int i = 0; i < cqc; i++)
		total += delays[i];

	size_t current_offset = 0;
	for (unsigned int i = 0; i < cqc; i++)
	{
		yy[i] = current_offset;

		printf("Device %d will take Y offset: "SZTF"\n", i, yy[i]);

		current_offset += (1 - (delays[i] / total)) * height;
	}

	*y = yy;

	free(delays);

	clReleaseMemObject(src);
	clReleaseMemObject(dst);
	clReleaseMemObject(hght);
	clReleaseMemObject(wdth);
	clReleaseMemObject(offy[0]);

	free(offy);

	return 0;
}