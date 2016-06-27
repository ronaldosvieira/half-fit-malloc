#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <math.h>

#define align4(x) (((((x)-1)>>2)<<2)+4)
#define bindex(s) (int)floor(1.0*log10(s)/log10(2))
#define rindex(s) (int)floor(1.0*log10(s-1)/log10(2))+1

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
#define BLOCK_SIZE 40
#define WORD_SIZE 32

int amount_free_blocks[WORD_SIZE] = {0};
t_block free_blocks[WORD_SIZE] = {NULL};

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

int get_free_block(t_block b) {
	t_block temp = free_blocks[bindex(b->size)];
	int i = 0;

	while (temp && valid_addr(temp->data)) {
		++i;

		if (temp->ptr == b->ptr
			&& temp->size == b->size) {
			return i;
		}

		temp = (void*) *(temp->data);
	}

	return 0;
}

int push_free_block(t_block b) {
	int i = bindex(b->size);

	if (amount_free_blocks[i] < 11) {
		free_blocks[i] = b;
		amount_free_blocks[i]++;

		return 0;
	} else {
		return -1;
	}
}

t_block pop_free_block(t_block b) {
	return NULL;
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
	t_block best = NULL;

	while (b) {
		if (b->free && b->size >= size) {
			if (!best || b->size < best->size)
				best = b;
		}

		*last = b;
		b = b->next;
	}

	return best? best : b;
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

void* mymalloc(size_t size) {
	t_block b, last;
	size = align4(size);

	if (base_address) {
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

	new = mymalloc(number * size);

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
			printf("prev = %p\n", b->prev);
			printf("next = %p\n", b->next);
			printf("free = %d\n", b->free);
			printf("ptr  = %p\n", b->ptr);
			printf("data = %p\n", b->data);
			printf("content = %d\n", *((int*) b->data));
			printf("\n");
		}

		printf("BLOCO %d: null\n", i);
	}

	printf("\n### FIM DO HEAP ####\n");
}

int main() {
	int *k = (int*) mymalloc(sizeof(int));
	// int *j = (int*) malloc(sizeof(int) * 20);
	// int *i = (int*) malloc(sizeof(int));
	// int *h = (int*) malloc(sizeof(int));
	// int *g = (int*) malloc(sizeof(int));

	// *k = 11;
	// *j = 12;
	// *i = 13;
	// *h = 14;
	// *g = 15;

	// print_heap();

	// free(j);
	// free(h);

	// print_heap();

	// j = (int*) malloc(sizeof(int));

	// print_heap();

	t_block b = get_block(k);
	push_free_block(b);

	int res = get_free_block(b);

	printf("%d\n", res);

	return 0;
}