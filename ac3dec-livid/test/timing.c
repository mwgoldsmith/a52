/* 
 *  timing.c
 *
 *	Aaron Holtzman - May 1999
 *
 */

#include <stdio.h>
#include <sys/time.h>

void
timing_test_2(void (*func)(void*,void*),void *arg_1,void *arg_2,char name[])
{
	hrtime_t start, end;
	hrtime_t start_i, end_i;
	int i, iters = 10;

	printf("\nTiming %s 10 times\n",name);
	start = gethrtime();
	for (i = 0; i < iters; i++)
	{
		start_i = gethrtime();
		func(arg_1,arg_2);
		end_i = gethrtime();
		printf("Iteration %d - %lld nsec\n",i,end_i - start_i);
	}
	end = gethrtime();

	printf("Avg %s time = %lld nsec\n", name,(end - start) / iters);
}

