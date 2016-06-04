/* An horrible dummy malloc */

#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>

typedef struct s_block *t_block;

struct s_block {
	size_t size;
	t_block next;
	int free;
};

void* malloc(size_t size) {
	t_block b;

	b = sbrk(0);

	/* If sbrk fails, we return NULL */
	if (sbrk(sizeof(struct s_block) + size) == (void*) -1)
		return NULL;

	return b + sizeof(struct s_block);
}

int main() {
	// int k = (int) malloc(sizeof(int));
	// printf("%lu\n", sizeof(k));
	return 0;
}