/* common functions and data for sw.c nd script.c */

// includes
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// typedefs and structs
typedef unsigned char byte;

struct wrap_data
{
	int offset;					// offset at which the appended data starts
	byte xor_value;				// all bytes of script_data_size added together for simple encryption
	int script_name_len;		// the length of the script name - not xored
	byte * script_name;			// the script file name
	int interpreter_name_len;	// the length of the interpreter name - not xore
	byte * interpreter_name;	// the interpreter name
	int script_data_size;		// size of the script data in bytes
	byte * script_data;			// the script itself
};

typedef struct wrap_data wrap_data;

// functions
void xor_byte_string(byte * str, byte xor_value, int len);
byte get_xor_value(int in_value);

void xor_byte_string(byte * str, byte xor_value, int len)
{
	/* xor every str index with xor_value */
	int i;
	for (i = 0; i < len; ++i)
		str[i] ^= xor_value;
}

byte get_xor_value(int in_value)
{
	/* add all the bytes of in_value together */
	byte result = 0;
	
	result += (byte)(in_value & 0xFF);
	result += (byte)((in_value >> 8) & 0xFF);
	result += (byte)((in_value >> 16) & 0xFF);
	result += (byte)((in_value >> 24) & 0xFF);
	
	return result;
}