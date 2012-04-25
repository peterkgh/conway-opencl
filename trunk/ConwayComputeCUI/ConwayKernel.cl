#define ARRAY_INDEX(ww, xx, yy) (((xx) + ((yy) * (ww))) >> 5)
#define BIT_INDEX(xx) (31 - ((xx) & 31))

__kernel void
evaluate_bit(__constant size_t *width,
             __constant size_t *height,
             __constant size_t *offsety,
             __global unsigned int *src,
             __global unsigned int *dst)
{
    const size_t y = get_global_id(1) + *offsety;
    size_t x = (get_global_id(0) << 5);
    const size_t ex = x | 31;
    unsigned int buff = 0;

    for (; x <= ex;
         x++)
    {
        unsigned char i = 0;
        
        if (x > 0)
        {
            if (y > 0) i += (src[ARRAY_INDEX(*width, x-1, y-1)] & (1 << BIT_INDEX(x-1))) ? 1 : 0;
            
            i += (src[ARRAY_INDEX(*width, x-1, y)] & (1 << BIT_INDEX(x-1))) ? 1 : 0;
            
            if (y < *height - 1) i += (src[ARRAY_INDEX(*width, x-1, y+1)] & (1 << BIT_INDEX(x-1))) ? 1 : 0;
        }
        
        if (y > 0) i += (src[ARRAY_INDEX(*width, x, y-1)] & (1 << BIT_INDEX(x))) ? 1 : 0;
        
        if (y < *height - 1) i += (src[ARRAY_INDEX(*width, x, y+1)] & (1 << BIT_INDEX(x))) ? 1 : 0;
        
        if (x < *width - 1)
        {
            if (y > 0) i += (src[ARRAY_INDEX(*width, x+1, y-1)] & (1 << BIT_INDEX(x+1))) ? 1 : 0;
            
            i += (src[ARRAY_INDEX(*width, x+1, y)] & (1 << BIT_INDEX(x+1))) ? 1 : 0;
            
            if (y < *height - 1) i += (src[ARRAY_INDEX(*width, x+1, y+1)] & (1 << BIT_INDEX(x+1))) ? 1 : 0;
        }
        
        if (i == 3 || (i == 2 && (src[ARRAY_INDEX(*width, x, y)] & (1 << BIT_INDEX(x)))))
            buff |= (1 << BIT_INDEX(x));
    }

    dst[ARRAY_INDEX(*width, ex, y)] = buff;
}