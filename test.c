#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "hfmalloc.c"

double measure(int arg, void (*func)(int, void*, void*), void* (*mallocf)(size_t), void (*freef)(size_t)) {
	clock_t t0, t1;

	t0 = clock();

	func(arg, mallocf, freef);

	t1 = clock();

	return (double) (t1 - t0) / CLOCKS_PER_SEC;
}

double test1(int n, void* (*mallocf)(size_t), void (*freef)(size_t)) {
	int* alocs[n];
	int i;

	for (i = 0; i < n; i++) {
		alocs[i] = (int*) (*mallocf)(sizeof(int));
	}

	for (i = 0; i < n; i++) {
		(*freef)(alocs[i]);
	}
}

double test2(int n, void* (*mallocf)(size_t), void (*freef)(size_t)) {
	int* alocs[n];
	int i, r;

	for (i = 0; i < n; i++) {
		r = 1 << (rand() % 32);
		alocs[i] = (int*) (*mallocf)(sizeof(int) * r);
	}

	for (i = 0; i < n; i++) {
		(*freef)(alocs[i]);
	}
}

double test3_hf(int n) {
	int* alocs[n];
	int i = 0;
	int j = 0;
	int r;

	while (i < n || j < n) {
		r = (int) rand() % 2;

		if ((r || j >= i) && i < n) {
			alocs[i++] = (int*) malloc(sizeof(int));
		} else {
			free(alocs[j++]);
		}
	}
}

double test3(int n) {
	int* alocs[n];
	int i = 0;
	int j = 0;
	int r;

	while (i < n || j < n) {
		r = (int) rand() % 2;

		if ((r || j >= i) && i < n) {
			alocs[i++] = (int*) malloc(sizeof(int));
		} else {
			free(alocs[j++]);
		}
	}
}

int main() {
	srand(time(NULL));
	int i;
	int NUM_MALLOCS = 2;

	char* nomes[2] = {"Malloc", "HFMalloc"};
	void* mallocs[2] = {&malloc, &hfmalloc};
	void* frees[2] = {&free, &hffree};

	printf("\nTest1\n");

	for (i = 0; i < NUM_MALLOCS; i++) {
		printf("%s = %lf\n", 
			nomes[i],
			measure(100, &test1, mallocs[i], frees[i]));
	}

	printf("\nTest2\n");

	for (i = 0; i < NUM_MALLOCS; i++) {
		printf("%s = %lf\n", 
			nomes[i],
			measure(100, &test2, mallocs[i], frees[i]));
	}

	printf("\nTest3\n");

	for (i = 0; i < NUM_MALLOCS; i++) {
		printf("%s = %lf\n", 
			nomes[i],
			measure(100, &test3, mallocs[i], frees[i]));
	}
}