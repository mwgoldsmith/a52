/*
 *
 *  matlab.c
 *
 * Aaron Holtzman - April 1999
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include "matlab.h"


matlab_file_t *matlab_open(char name[])
{
	matlab_file_t *mf;	

	mf = malloc(sizeof(matlab_file_t));
	if(!mf)
		return 0;

	mf->f = fopen(name,"w");
	if(!mf->f)
	{
		free(mf);
		return 0;
	}

  fprintf(mf->f,"s = [\n");

	return mf;
}

void matlab_close(matlab_file_t *mf)
{
  fprintf(mf->f,"];\n");
	fclose(mf->f);
	free(mf);
}

void matlab_write(matlab_file_t *mf,float x[], int length)
{
  int i;

  for(i=0;i< length;i++)
    fprintf(mf->f,"%5f;\n ",x[i]);
}

