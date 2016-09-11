/* the module that can read and execute a script appended to it's end v1.0 */

// includes
#include "script_wrap.h"
#include <windows.h>

// constants
const char extract[] = "-x";

// functions
char * get_image_name(char * arg_zero);
void get_data(wrap_data * wd);
void extract_script(wrap_data * wd);
void execute_script(wrap_data * wd);
void decrypt_script(wrap_data * wd);
void dump_data(wrap_data * wd);

// globals
char * image_name;
int arg_count;
char ** arg_v;

// code
int main(int argc, char * argv[])
{
	/* main logic */
	
	// make argument info global
	arg_count = argc;
	arg_v = argv;
	
	wrap_data appended_data; // our data struct
	
	image_name = get_image_name(argv[0]); // get the proper name of itself
	get_data(&appended_data); // read the appended data
	
	// check if we only want to dump
	if (2 == argc && (0 == strcmp(argv[1], extract)))
	{
		dump_data(&appended_data);
		exit(0);
	}
	
	// extract and run
	extract_script(&appended_data);
	execute_script(&appended_data);
	
	// free and go home
	free(appended_data.script_name);
	free(appended_data.interpreter_name);
	free(appended_data.script_data);
	
	return 0;
}

char * get_image_name(char * arg_zero)
{
	/* adds .exe to the end of arg[0] if needed 
	 * so we have the full file name */
	int	len = strlen(arg_zero);
	char ext[] = ".exe";
	char * compare = arg_zero + len - 4; // .exe + '\0'

	// do we need to add?
	if (0 != strcmp(compare, ext))
		strcat(arg_zero, ext);
	
	return arg_zero;
}

void get_data(wrap_data * wd)
{
	/* read and decrypt the appended data */
	FILE * f;

	f = fopen(image_name, "rb");
	if (NULL == f)
	{
		printf("Err: unable to open %s\n", wd->script_name);
		exit(1);
	}
	
	// get offset
	fseek(f, -4L, SEEK_END);
	fread(&(wd->offset), 1, sizeof(int), f);
	
	// go to offset
	fseek(f, (long int)wd->offset, SEEK_SET);
	
	// get script name len
	fread(&(wd->script_name_len), 1, sizeof(int), f);
	
	// allocate memory and get script name
	wd->script_name = malloc(wd->script_name_len * sizeof(byte));
	fread(wd->script_name, 1, wd->script_name_len, f);

	// get interpreter name len
	fread(&(wd->interpreter_name_len), 1, sizeof(int), f);
	
	// allocate memory and get interpreter name
	wd->interpreter_name = malloc(wd->interpreter_name_len * sizeof(byte));
	fread(wd->interpreter_name, 1, wd->interpreter_name_len, f);
	
	// get script size
	fread(&(wd->script_data_size), 1, sizeof(int), f);
	
	// allocate memory and get script data
	wd->script_data = malloc(wd->script_data_size * sizeof(byte));
	fread(wd->script_data, 1, wd->script_data_size, f);
	
	decrypt_script(wd);
	
	fclose(f);
}

void decrypt_script(wrap_data * wd)
{
	/* xor decryption of the string and data parts */
	
	// get xor value
	wd->xor_value = get_xor_value(wd->script_data_size);
	
	// decrypt
	xor_byte_string(wd->script_name, wd->xor_value, wd->script_name_len);
	xor_byte_string(wd->interpreter_name, wd->xor_value, wd->interpreter_name_len);
	xor_byte_string(wd->script_data, wd->xor_value, wd->script_data_size);
}

void extract_script(wrap_data * wd)
{
	/* place the script file in the tmp folder */

	// get temp folder
	char * tmp_path = getenv("TEMP");
	
	// prepare the full file name
	char * full_name = malloc(strlen(tmp_path) + wd->script_name_len + 5);
	strcpy(full_name, tmp_path);
	strcat(full_name, "\\");
	strcat(full_name, wd->script_name);
	
	// make it the new script name and recalculate length
	free(wd->script_name);			// free current name
	wd->script_name = full_name;
	wd->script_name_len = strlen(wd->script_name);
	
	FILE * f;
	
	f = fopen(wd->script_name, "wb");
	if (NULL == f)
	{
		printf("Err: unable to open %s\n", wd->script_name);
		exit(1);
	}
	
	fwrite(wd->script_data, 1, wd->script_data_size, f);
	
	fclose(f);
}

void dump_data(wrap_data * wd)
{
	/* dump the appended data in a text file */
	
	// report text
	char fext[] = ".txt";
	char name[] = "Name: ";
	char run_line[] = "Run line: ";
	char script_start[] = "Script: ";
	
	// prepare the file name
	char * out_file_name = malloc(wd->script_name_len + strlen(fext) + 2);
	strcpy(out_file_name, wd->script_name);
	strcat(out_file_name, fext);
	
	FILE * f;
	
	f = fopen(out_file_name, "w");
	if (NULL == f)
	{
		printf("Err: unable to open %s\n", out_file_name);
		exit(1);
	}
	
	// write
	fprintf(f, "%s\n%s\n\n", name, wd->script_name);
	fprintf(f, "%s\n%s %s\n\n", run_line, wd->interpreter_name, wd->script_name);
	fprintf(f, "%s\n", script_start);

	fclose(f);
	
	// reopen for binary append to add the script
	f = fopen(out_file_name, "ab");
	if (NULL == f)
	{
		printf("Err: unable to open %s\n", out_file_name);
		exit(1);
	}
	
	fwrite(wd->script_data, 1, wd->script_data_size, f);
	
	// free and close
	free(out_file_name);
	fclose(f);
}

void execute_script(wrap_data * wd)
{
	/* launch the scrip in the background */
	
	int i, result;	// get length of arguments
	for (i = 1, result = 0; i < arg_count; ++i)
		result += strlen(arg_v[i]);
	
	// prepare the start line
	char * start_line = malloc(wd->interpreter_name_len + wd->script_name_len + result + arg_count + 2);
	strcpy(start_line, wd->interpreter_name);
	strcat(start_line, " ");
	strcat(start_line, wd->script_name);
	
	// add in the arguments
	for (i = 1; i < arg_count; ++i)
	{
		strcat(start_line, " ");
		strcat(start_line, arg_v[i]);
	}
	
	// launch the process
	STARTUPINFO si;
    PROCESS_INFORMATION pi;

	// zero out
    ZeroMemory( &si, sizeof(si) );
    si.cb = sizeof(si);
    ZeroMemory( &pi, sizeof(pi) );
	
	if( !CreateProcess(NULL,	// no module name (use command line)
        start_line,				// command line
        NULL,					// process handle not inheritable
        NULL,					// thread handle not inheritable
        FALSE,					// set handle inheritance to FALSE
        0,						// no creation flags
        NULL,					// use parent's environment block
        NULL,					// use parent's starting directory 
        &si,					// pointer to STARTUPINFO structure
        &pi)					// pointer to PROCESS_INFORMATION structure
    ) 
    {
        printf("Err: process could not be created.\n");
        exit(1);
    }
	
	// close process and thread handles. 
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
	
	// free memory
	free(start_line);
}
