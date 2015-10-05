/* A simple test harness for memory alloction. */

#include "mm_alloc.h"
#include <stdio.h>
int main(int argc, char **argv)
{

	int *data, *data2, *data3;

	data = (int*) mm_malloc(4);
	data2 = (int*) mm_malloc(8);
	data[0] = 1;
	data2[0] = 4;
	data = (int*)mm_realloc(data, 8);
	mm_free(data);
	mm_free(data2);
	data3 = (int*) mm_malloc(6);
	data3[0] = 3;
	mm_free(data3);
	printf("malloc sanity test successful!\n");
	return 0;
}
