/* An horrible dummy malloc */

#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>

#define align4(x) (((((x)-1)>>2)<<2)+4)

typedef struct s_block *t_block;

struct s_block {
	size_t size;
	t_block prev;
	t_block next;
	int free;
	void* ptr;
	char data[1];
};

// sizeof(struct s_block) nao vai retornar o numero correto
#define BLOCK_SIZE 20

void *base_address = NULL;

t_block get_block(void* p) {
	char* tmp;
	tmp = p;

	return (p = tmp -= BLOCK_SIZE);
}

int valid_addr(void* p) {
	if (base_address) {
		if (p > base_address && p < sbrk(0)) {
			return p == (get_block(p))->ptr;
		}
	}

	return 0;
}

void split_block(t_block b, size_t size) {
	t_block new;

	new = (t_block) (b->data + size);

	new->size = b->size - size - BLOCK_SIZE;
	new->next = b->next;
	new->prev = b;
	new->free = 1;
	new->ptr = new->data;

	b->size = size;
	b->next = new;

	if (new->next) {
		new->next->prev = new;
	}
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
		return NULL;

	b->size = size;
	b->next = NULL;
	b->prev = last;
	b->ptr = b->data;
	b->free = 0;

	if (last) last->next = b;

	return b;
}

t_block fusion(t_block b) {
	if (b->next && b->next->free) {
		b->size += BLOCK_SIZE + b->next->size;
		b->next = b->next->next;

		if (b->next) b->next->prev = b;
	}

	return b;
}

void* malloc(size_t size) {
	t_block b, last;
	size = align4(size);

	if (base_address ) {
		last = base_address;
		b = find_block(&last, size);

		if (b) {
			fusion(b);

			if ((b->size - size) >= (BLOCK_SIZE + 4)) {
				split_block(b, size);
			}

			b->free = 0;
		} else {
			b = extend_heap(last, size);

			if (!b) return NULL;
		}
	} else {
		b = extend_heap(NULL, size);
		
		if (!b) return NULL;

		base_address = b;
	}

	return b->data;
}

void* calloc(size_t number, size_t size) {
	size_t *new;
	size_t s4, i;

	new = malloc(number * size);

	if (new) {
		s4 = align4(number * size) << 2;

		for (i = 0; i < s4; i++) new[i] = 0;
	}

	return new;
}

void free(void* p) {
	t_block b;

	if (valid_addr(p)) {
		b = get_block(p);
		b->free = 1;

		if (b->prev && b->prev->free) {
			b = fusion(b->prev);
		}

		if (b->next) {
			fusion(b);
		} else {
			if (b->prev) b->prev->next = NULL;
			else base_address = NULL;

			brk(b);
		}
	}
}

void print_heap() {
	t_block b = base_address;
	int i;

	printf("\n####### HEAP #######\n");

	if (sbrk(0) == base_address)
		printf("null\n");
	else {
		for (i = 1; b; i++, b = b->next) {
			printf("BLOCO %d:\n", i);
			printf("address = %p\n", b);
			printf("size = %lu\n", b->size);
			printf("next = %p\n", b->next);
			printf("free = %d\n", b->free);
			printf("ptr  = %p\n", b->ptr);
			printf("data = %p\n", b->data);
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

	free(j);

	print_heap();

	return 0;
}