// ConwayComputeCUI.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

int start_simulation(void)
{
    cl_device_id *dev;
	cl_uint devc;
    cl_context context;
    cl_command_queue *cmd_queue;
    cl_mem src, dst, wdth, hght, *offy;
    cl_int err;
    cl_kernel kern;
    cl_program prog;
	cl_platform_id pform;
    size_t rows, columns, runs, print_each = 0, offsetx, offsety, *y, swapoffy;
	int gui_enabled, platform, device;
    unsigned int *buff0;
    unsigned int i;
	struct dispatcher_context *c;
	int random;
	char fname[1024];
	cl_uint cqc;
    
#if 1
    
	do
	{
		printf("Width: ");
		scanf(SZTF, &columns);
        
		if (columns % 32)
		{
			printf("Width must be a multiple of 32\n");
			continue;
		}
        
		if (columns == 0)
		{
			printf("Width must be > 0\n");
			continue;
		}
        
		break;
	}
	while (true);
    
	do
	{
		printf("Height: ");
		scanf(SZTF, &rows);
        
		if (rows == 0)
		{
			printf("Width must be > 0\n");
			continue;
		}
        
		break;
	}
	while (true);
    
    printf("Runs: ");
    scanf(SZTF, &runs);
    
	printf("Random? ");
	scanf("%d", &random);
    
	if (!random)
	{
		printf("File name: ");
		scanf("%s", fname);
        
		printf("Offset X: ");
		scanf(SZTF, &offsetx);
        
		printf("Offset Y: ");
		scanf(SZTF, &offsety);
	}
    
	printf("GUI? ");
	scanf("%d", &gui_enabled);
    
	if (!gui_enabled)
	{
		printf("Print after run: ");
		scanf(SZTF, &print_each);
	}
    
	printf("Platform index: ");
	scanf("%d", &platform);
    
	printf("Device index (-1 for all): ");
	scanf("%d", &device);
    
	do
	{
		printf("Swap offset: ");
		scanf(SZTF, &swapoffy);
        
		if (swapoffy == 0) swapoffy = rows;
        
		if (rows % swapoffy != 0)
		{
			printf("Swap offset must be a factor of the row count\n");
			continue;
		}
        
		break;
	}
	while (true);
#else
    columns = 512;
    rows = 512;
    runs = 1000;
	gui_enabled = 1;
    print_each = 0;
	random = 1;
    offsetx = 0;
    offsety = 0;
	platform = 0;
	device = 0;
	swapoffy = rows;
#endif
    
    err = get_devices(&dev, &devc, &pform, platform, device);
    if (err)
        return err;
    
    err = initialize_context_cmd_queue(dev, devc, pform, &context, &cmd_queue, &cqc);
    if (err)
        return err;
    
    err = load_kernel(context, dev, devc, &prog, &kern);
    if (err)
        return err;
    
    buff0 = (unsigned int*)malloc(rows * (columns / 8));
    if (!buff0)
        return -1;
    
	if (random)
	{
		srand((unsigned int)time(NULL));
		for (i = 0; i < (rows * (columns / 8)) / 4; i++)
		{
		    buff0[i] = rand() | (rand() << 16);
		}
	}
	else
	{
		memset(buff0, 0, rows * (columns / 8));
		if (!load_file_to_buffer(buff0, fname, offsetx, offsety, columns, rows))
			return -1;
	}
    
	err = compute_buffer_sizes(context, cmd_queue, cqc, kern, columns, rows, swapoffy, &y);
	if (err)
		return err;
    
    err = create_buffers(context, swapoffy * (columns / 8), columns, 
		swapoffy, &src, &dst, &wdth, &hght, &offy, y, buff0, devc);
    if (err)
        return err;
    
	if (gui_enabled)
	{
		if (!initialize_window())
			return -1;
	}
    
	c = (struct dispatcher_context *)malloc(sizeof(*c));
	if (!c)
		return -1;
    
	c->cmd_queue = cmd_queue;
	c->cqc = cqc;
	c->kern = kern;
	c->columns = columns;
	c->rows = rows;
	c->print_each = print_each;
	c->runs = runs;
	c->gui_enabled = gui_enabled;
	c->buff0 = buff0;
	c->src = src;
	c->dst = dst;
	c->wdth = wdth;
	c->hght = hght;
	c->offy = offy;
	c->y = y;
	c->swapoffy = swapoffy;
	c->context = context;
    c->win_height = rows;
    
	return start_dispatcher(c);
}