#include "stdafx.h"

void
print_buffer(unsigned int *buff, size_t width, size_t height)
{
    size_t x, y;
    
    for (y = 0; y < height; y++)
    {
        for (x = 0; x < width; x++)
        {
           if (buff[(x + y * width) >> 5] & (1 << (31 - ((x + y * width) & 31))))
               printf("1");
           else
               printf("0");
        }
        printf("\n");
    }
}