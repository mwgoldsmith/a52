#include <stdio.h>

int main (int argc, char ** argv)
{
    FILE * f1;
    FILE * f2;
    short buf1[512];
    short buf2[512];
    int i, j, total = 0, max = 0;
    long long err = 0, square = 0;

    if (argc != 3)
	return 1;
    f1 = fopen (argv[1], "rb");
    f2 = fopen (argv[2], "rb");
    if ((f1 == NULL) || (f2 == NULL)) {
	printf ("cannot open file %s\n", (f1 == NULL) ? argv[1] : argv[2]);
	return 1;
    }
    while (1) {
	i = fread (buf1, sizeof (short), 512, f1);
	j = fread (buf2, sizeof (short), 512, f2);
	if ((i < 512) || (j < 512))
	    break;
	for (i = 0; i < 512; i++) {
	    j = buf2[i] - buf1[i];
	    if (j) {
		err += j;
		square += j * j;
		if (j > max)
		    max = j;
		if (-j > max)
		    max = -j;
	    }
	}
	total += 512;
    }
    if (i < j)
	printf ("%s is too short\n", argv[1]);
    else if (j < i)
	printf ("%s is too short\n", argv[2]);
    else
	printf ("max error %d mean error %f mean square error %f\n",
		max, (float)err/total, (float)square/total);
    return (i != j) || (max > 1);
}
