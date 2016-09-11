/* sw -- script wrap v1.0
 * the module that can append the input script and other 
 * needed info to the module which can read and execute it */

// includes
#include "script_wrap.h"
#include "script_image_csv.h"
#include <stdbool.h>

// constants
const char INPUT_FILE[] = "-f";
const char INTERPRETER[] = "-e";
const char ICON_CHANGE[] = "-ic";

// functions
void read_script(wrap_data * wd);
void append_data(wrap_data * wd, char * out_file);
void encrypt_script(wrap_data * wd);
void dump_image(char * image_name);

// globals
char * out_file;

// code
int main(int argc, char * argv[])
{
	/* argument handling and main logic */
	int i;
	wrap_data script;
	bool has_input_file = false;
	bool has_interpreter = false;
	bool has_icon = false;
	char * icon_name;
	
	// manage arguments
	if (argc < 3)	// not enough arguments
	{
		printf("Use:\n%s -f <input script> -e <script interpreter> -ic <icon file>\n", argv[0]);
		puts("The -ic part is optional.");
		exit(1);
	}
		
	for (i = 1; i < argc; ++i)
	{
		if (0 == strcmp(argv[i], INPUT_FILE))
		{
			if (argv[i + 1] != '\0')	// do we have an input script file?
			{
				script.script_name = argv[i + 1];
				has_input_file = true;
			}		
		}
		else if (0 == strcmp(argv[i], INTERPRETER))
		{
			if (argv[i + 1] != '\0')	// do we have an interpreter command?
			{
				script.interpreter_name = argv[i + 1];
				has_interpreter = true;
			}
		}
		else if (0 == strcmp(argv[i], ICON_CHANGE))
		{
			if (argv[i + 1] != '\0')	// do we have an icon?
			{
				has_icon = true;
				icon_name = argv[i + 1];
			}
		}
	}
	
	// check arg errors
	if (!has_input_file)
	{
		printf("Err: bad input file name.\n");
		exit(1);
	}
	
	if (!has_interpreter)
	{
		printf("Err: missing interpreter.\n");
		exit(1);
	}
	
	// create the .exe
	dump_image(script.script_name);
	
	// set icon if any
	if (has_icon)
	{
		// allocate string
		char * ic_string = malloc(strlen(out_file) + strlen(icon_name) + 8);
		// prepare line
		strcpy(ic_string, "ic ");
		strcat(ic_string, out_file);
		strcat(ic_string, " ");
		strcat(ic_string, icon_name);
		
		// call ic
		system(ic_string);
		
		// free string
		free(ic_string);
	}
	
	read_script(&script);
	append_data(&script, out_file);
	puts("Done.");
	
	// free and go home
	free(out_file);
	return 0;
}

void dump_image(char * image_name)
{
	/* dumps the image_csv array into an exe file */
	FILE * f;
	
	f = fopen(image_name, "rb");
	if (NULL == f)	// do we actually have something to append?
	{
		printf("Err: unable to open %s\n", image_name);
		exit(1);
	
	}
	fclose(f);
	
	// prepare the .exe name
	int img_size;
	int len = strlen(image_name);
	char * exe_name = malloc(len + 8);
	char ext[] = ".exe";
	
	strcpy(exe_name, image_name);
	strcat(exe_name, ext);
	
	// make the exe_name global
	out_file = exe_name;
	
	f = fopen(out_file, "wb");
	if (NULL == f)
	{
		printf("Err: unable to open %s\n", out_file);
		exit(1);
	
	}
	
	// get the size in bytes
	img_size = sizeof(IMAGE_CSV) / sizeof(byte); 
	
	// write
	fwrite(IMAGE_CSV, 1, img_size, f);
	
	// close and go home
	fclose(f);
}

void read_script(wrap_data * wd)
{
	/* reads the script file into wd->script_data */
	int i;
	FILE * f;
	
	f = fopen(wd->script_name, "rb"); // open script
	if (NULL == f)
	{
		printf("Err: unable to open %s\n", wd->script_name);
		exit(1);
	}
	
	// get file size
	fseek(f, 0L, SEEK_END);
	wd->script_data_size = ftell(f);
	rewind(f);
	
	// allocate buffer for the script data
	wd->script_data = malloc(sizeof(byte) * wd->script_data_size);
	
	// read
	i = fread(wd->script_data, 1, wd->script_data_size, f);
	if (i != wd->script_data_size)
	{
		printf("Err: reading error.\n");
		free(wd->script_data);
		fclose(f);
		exit(1);
	}
	
	// close and go
	fclose(f);
}

void append_data(wrap_data * wd, char * out_file)
{
	/* appends name, inerpreter line and script data to the dumped .exe */
	FILE * f;
	
	// fill in rest of wrap_data
	wd->script_name_len = strlen(wd->script_name) + 1;				// for '\0'
	wd->interpreter_name_len = strlen(wd->interpreter_name) + 1;	// for '\0'
	
	// open output file
	f = fopen(out_file, "r+b");
	if (NULL == f)
	{
		printf("Err: unable to open %s\n", wd->script_name);
		exit(1);
	}
	
	// go to end and get offset
	fseek(f, 0L, SEEK_END);
	wd->offset = ftell(f);
	
	// encrypt
	encrypt_script(wd);
	
	// append
	fwrite(&(wd->script_name_len), 1, sizeof(int), f);
	fwrite(wd->script_name, 1, wd->script_name_len, f);
	fwrite(&(wd->interpreter_name_len), 1, sizeof(int), f);
	fwrite(wd->interpreter_name, 1, wd->interpreter_name_len, f);
	fwrite(&(wd->script_data_size), 1, sizeof(int), f);
	fwrite(wd->script_data, 1, wd->script_data_size, f);
	fwrite(&(wd->offset), 1, sizeof(int), f);
	
	// close and go
	fclose(f);
}

void encrypt_script(wrap_data * wd)
{
	/* simple xor encryption of the string and data parts */
	
	// get xor value
	wd->xor_value = get_xor_value(wd->script_data_size);
	
	// encrypt
	xor_byte_string(wd->script_name, wd->xor_value, wd->script_name_len);
	xor_byte_string(wd->interpreter_name, wd->xor_value, wd->interpreter_name_len);
	xor_byte_string(wd->script_data, wd->xor_value, wd->script_data_size);
}
