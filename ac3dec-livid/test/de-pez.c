#include <stdio.h>

unsigned char buf[2013];

void main(void)
{
	FILE *in,*out;
	unsigned long int pes_length;
	unsigned long int extra_pes_bytes;
	unsigned long int user_data_length;

	in = fopen("foo.ac3","r");
	out = fopen("foo.ac3.out","w");

	while(!feof(in))
	{
		//Read pes header
		fread(buf,1,13,in);
		pes_length = (buf[4] << 8) + buf[5] + 6;
		extra_pes_bytes = buf[8];
		user_data_length = pes_length - 13 - extra_pes_bytes;
		printf("pes_length = %ld extra_pes_bytes = %ld user_data_length = %ld\n",
				pes_length,extra_pes_bytes,user_data_length);
		//Read extra pes bytes
		fread(buf,1,extra_pes_bytes,in);
		fread(buf,1,user_data_length,in);
		fwrite(buf,1,user_data_length,out);
	}

}

