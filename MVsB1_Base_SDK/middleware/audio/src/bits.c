#include "bits.h"

static unsigned char* bits_buffer_ptr;
unsigned long bits_cache;
static int   bits_bit_count;

void init_get_bits(unsigned char* buffer)
{
	bits_cache = (buffer[0] << 24) + (buffer[1] << 16) + (buffer[2] << 8) + (buffer[3] << 0);
	bits_bit_count = -16; 
	bits_buffer_ptr = buffer + 4; 
}

unsigned int get_bits(int n)
{
    unsigned short tmp;

	if(bits_bit_count >= 0)
	{ 
		bits_cache += ((bits_buffer_ptr[0] << 8) + bits_buffer_ptr[1]) << bits_bit_count; 
		bits_buffer_ptr += 2; 
		bits_bit_count -= 16; 
	}

	tmp = ((unsigned long)(bits_cache)) >> (32 - n);

	bits_cache <<= n;
	bits_bit_count += (n);	
	
	return tmp;
}


