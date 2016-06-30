#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "hfmalloc.c"
#include "algorithms/bestfit.c"
#include "algorithms/firstfit.c"
#include "algorithms/worstfit.c"
#include "algorithms/quickfit.c"

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
		//printf("######################### malloc %d\n", i);
		alocs[i] = (int*) (*mallocf)(sizeof(int));
	}

	for (i = 0; i < n; i++) {
		//printf("######################### free %d\n", i);
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

double test3(int n, void* (*mallocf)(size_t), void (*freef)(size_t)) {
	int* alocs[n];
	int i = 0;
	int j = 0;
	int r;

	while (i < n || j < n) {
		r = (int) rand() % 2;
		//printf("######################### %d %d\n", i, j);

		if ((r || j >= i) && i < n) {
			alocs[i++] = (int*) (*mallocf)(sizeof(int));
		} else {
			(*freef)(alocs[j++]);
		}
	}
}

double test4(int n, void* (*mallocf)(size_t), void (*freef)(size_t)) {
	int* alocs[n];
	int i = 0;
	int j = 0;
	int r;

	while (i < n || j < n) {
		r = (int) rand() % 2;
		//printf("######################### %d %d\n", i, j);

		if ((r || j >= i) && i < n) {
			alocs[i++] = (int*) (*mallocf)(sizeof(int) * ((rand() % 32767) + 4));
		} else {
			(*freef)(alocs[j++]);
		}
	}
}

int main() {
	srand(time(NULL));
	int i, j, t;
	double media = 0.0;
	
	int NUM_MALLOCS = 6;
	int NUM_ITER = 10;
	int NUM_TESTES = 3;
	int NUM_ALLOC = 100;

	char* nomes[] = {"UNIX", "Half fit", "Quick fit", 
					"First fit", "Best fit", "Worst fit"};
	void* mallocs[] = {&malloc, &hfmalloc, &qfmalloc, 
					&ffmalloc, &bfmalloc, &wfmalloc};
	void* frees[] = {&free, &hffree, &qffree, 
					&fffree, &bffree, &wffree};

	char* n_testes[] = {"N alocações tam. fixo sequenciais",
						"N alocações tam. variado sequenciais",
						"N alocações tam. fixo aleatórias",
						"N alocações tam. variado aleatórias"};
	void* f_testes[] = {&test1, &test2, &test3, &test4};

	FILE *f;
    f = fopen("testes.csv", "a");

	for (t = 0; t < NUM_TESTES; t++) {
		printf("\n##### %s #####\n\n", n_testes[t]);

		for (i = 0; i < NUM_MALLOCS; i++) {
			media = 0.0;

			for (j = 0; j < NUM_ITER; j++) {
				media += measure(NUM_ALLOC, f_testes[t], mallocs[i], frees[i]);
			}

			printf("%s = %lf\n", 
					nomes[i],
					media / NUM_ITER);
			/*fprintf(f, "%s;%d;%d;%d;%lf\n", nomes[i], t, 
					50, NUM_ALLOC, media / NUM_ITER);*/
		}

		printf("\n#####################\n\n");
	}

	int fclose(fp);
}