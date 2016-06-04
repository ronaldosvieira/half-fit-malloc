/* An horrible dummy malloc */

#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>

#define align4(x) (((((x)-1)>>2)<<2)+4)

typedef struct s_block *t_block;

struct s_block {
	size_t size;
	t_block next;
	int free;
	char data[1];
};

#define BLOCK_SIZE sizeof(struct s_block)

void *base_address = NULL;

void split_block(t_block b, size_t size) {
	t_block new;

	new = (void*) b->data + size;

	new->size = b->size - size - BLOCK_SIZE;
	new->next = b->next;
	new->free = 1;

	b->size = size;
	b->next = new;
}

t_block find_block(t_block *last, size_t size) {
	t_block b = base_address;

	while (b && !(b->free && b->size >= size)) {
		*last = b;
		b = b->next;
	}

	return b;
}

t_block extend_heap(t_block last, size_t size) {
	t_block b;

	b = sbrk(0);

	if (sbrk(BLOCK_SIZE + size) == (void*) -1)
		return (NULL);

	b->size = size;
	b->next = NULL;
	b->free = 0;

	if (last) last->next = b;

	return (b);
}

void* malloc(size_t size) {
	t_block p;
	t_block last = NULL;
	size = align4(size);

	if (base_address != NULL) {
		p = find_block(&last, size);

		if (p != NULL) {
			if (p->size >= size + BLOCK_SIZE + 4) {
				split_block(p, size);
			}

			p->free = 0;
		} else {
			p = extend_heap(last, size);
		}
	} else {
		base_address = ((void*) sbrk(0));
		p = extend_heap(NULL, size);
	}

	/*if (((void*) sbrk(0)) == base_address) {
		p = extend_heap(last, size);
	} else {
		p = find_block(&last, size);
		if (p == NULL) p = extend_heap(last, size);
	}*/

	if (p == NULL) return NULL;
	else return p->data;
}

void print_heap() {
	t_block p = base_address;
	int i;

	printf("\n####### HEAP #######\n");

	if (sbrk(0) == base_address)
		printf("null\n");
	else {
		for (i = 1; p; i++, p = p->next) {
			printf("BLOCO %d:\n", i);
			printf("address = %p\n", p);
			printf("size = %lu\n", p->size);
			printf("next = %p\n", p->next);
			printf("free = %d\n", p->free);
			printf("data = %p\n", p->data);
			printf("\n");
		}

		printf("BLOCO %d: null\n", i);
	}

	printf("\n### FIM DO HEAP ####\n");
}

int main() {
	int *k = (int*) malloc(sizeof(int));
	int *j = (int*) malloc(sizeof(int));

	if (k) printf("inteiro k alocado com sucesso\n");
	else printf("problema na alocação do inteiro k\n");

	if (j) printf("inteiro j alocado com sucesso\n");
	else printf("problema na alocação do inteiro k\n");

	print_heap();

	return 0;
}