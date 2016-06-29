#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include "hfmalloc.c"

int main() {
	int* j = hfmalloc(sizeof(int) * 20);
	int* k = hfmalloc(sizeof(int));

	hffree(j);

	int* l = hfmalloc(sizeof(int) * 2);

	hffree(k);
	hffree(l);
}