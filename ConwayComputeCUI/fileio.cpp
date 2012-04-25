#include "stdafx.h"

int
get_number(char* str, int* index)
{
	int num = 0, digits = 0;

	while(str[*index] <= '9' && str[*index] >= '0')
	{
		digits++;

		(*index)++;
	}

	*index = (*index) - digits;

	while (str[*index] <= '9' && str[*index] >= '0')
	{
		num += (((int)str[*index]) - ((int)'0')) * pow(10.0, digits-1);
		digits--;
		(*index)++;
	}

	return num;
}

bool
load_file_to_buffer_rle(unsigned int *buff, char *file, size_t offsetx, size_t offsety, size_t width, size_t height)
{
	FILE *f = fopen(file, "r");
	char chars[1024];
	size_t x = 0;
	size_t y = 0;
	bool ignore = false;

	if (!f) 
		return false;

	for (;;)
	{
		int length = fread(chars, 1, 1024, f);

		if (length != 1024)
		{
			if (!feof(f))
				return false;

			if (ferror(f))
				return false;

			if (length == 0)
				return true;
		}

		int i = 0;
		while (i < length)
		{
			int num;

			if (chars[i] == '\n' || chars[i] == '\r')
			{
				ignore = false;
				i++;
				continue;
			}

			if (ignore)
			{
				i++;
				continue;
			}
			else if (chars[i] == 'x' || chars[i] == '#')
			{
				ignore = true;
				i++;
				continue;
			}
			else if ((num = get_number(chars, &i)) != 0)
			{
				//Easy path; do nothing
			}
			else if (chars[i] == 'b' || chars[i] == 'o')
			{
				num = 1;
			}
			else if (chars[i] == '$')
			{
				y++;
				x = 0;
				i++;

				if (y >= height)
				{
					printf("Buffer must be at least "SZTF" units in height\n", (y+1));
					return false;
				}
				continue;
			}
			else if (chars[i] == '!')
			{
				return true;
			}

			for (int j = 0; j < num; j++)
			{
				if (x >= width)
				{
					printf("Buffer must be at least "SZTF" units in width\n", (x+1));
					return false;
				}

				if (chars[i] == 'o')
					buff[((x + offsetx) + (y + offsety) * width) >> 5] |= (1 << (31 - (((x + offsetx) + (y + offsety) * width) & 31)));

				x++;
			}

			i++;
		}
	}

	return true;
}

bool
load_file_to_buffer_plain_text(unsigned int *buff, char *file, size_t offsetx, size_t offsety, size_t width, size_t height)
{
	FILE *f = fopen(file, "r");
	char chars[1024];
	size_t x = 0;
	size_t y = 0;
	bool ignore = false;

	if (!f) 
		return false;

	for (;;)
	{
		int length = fread(chars, 1, 1024, f);

		if (length != 1024)
		{
			if (!feof(f))
				return false;

			if (ferror(f))
				return false;

			if (length == 0)
				return true;
		}

		for (int i = 0; i < length; i++)
		{
			if (chars[i] == '\n')
			{
				if (ignore)
				{
					ignore = false;
				}
				else
				{
					y++;
					x = 0;
				}

				continue;
			}

			if (ignore)
				continue;

			if ((x + offsetx) >= width || (y + offsety) >= height)
			{
				printf("Buffer too small! ("SZTF", "SZTF")\n", (x + offsetx), (y + offsety));
				return false;
			}

			if (chars[i] == '#' || chars[i] == '!')
			{
				ignore = true;
				continue;
			}
			else if (chars[i] == '1' || chars[i] == 'O' || chars[i] == '*')
			{
				buff[((x + offsetx) + (y + offsety) * width) >> 5] |= (1 << (31 - (((x + offsetx) + (y + offsety) * width) & 31)));
			}
			else if (chars[i] != '.' && chars[i] != '0' && chars[i] != '\r')
			{
				printf("Unrecognized character in file! (%c)\n", chars[i]);
				return false;
			}

			x++;
		}
	}

	return true;
}

bool
load_file_to_buffer(unsigned int *buff, char *fname, size_t offsetx, size_t offsety, size_t width, size_t height)
{
	if (strstr(fname, ".lif") || strstr(fname, ".cells"))
	{
		if (!load_file_to_buffer_plain_text(buff, fname, offsetx, offsety, width, height))
		{
			printf("Load plain text file failed!\n");
			return false;
		}
	}
	else if (strstr(fname, ".rle"))
	{
		if (!load_file_to_buffer_rle(buff, fname, offsetx, offsety, width, height))
		{
			printf("Load RLE file failed!\n");
			return false;
		}
	}
	else
	{
		return false;
	}

	return true;
}