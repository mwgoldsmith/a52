/*
 *
 *  matlab.h
 *
 * Aaron Holtzman - April 1999
 *
 */

typedef struct matlab_file_s
{
	FILE *f;
} matlab_file_t;

void matlab_write(matlab_file_t *out_fp,float x[], int length);
matlab_file_t *matlab_open(char name[]);
void matlab_close(matlab_file_t *mf);

