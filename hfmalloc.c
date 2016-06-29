#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <math.h>

#define DEBUG 0

#if defined(DEBUG) && DEBUG > 0
	#define DEBUG_PRINT(fmt, args...) fprintf(stderr, "DEBUG: %s:%d:%s(): " fmt, \
    __FILE__, __LINE__, __func__, ##args)
#else
	#define DEBUG_PRINT(fmt, args...) /* Don't do anything in release builds */
#endif

#define align4(x) (((((x)-1)>>2)<<2)+4)
#define bindex(s) (int)floor(1.0*log10(s)/log10(2))
#define rindex(r) (int)floor(1.0*log10(r-1)/log10(2))+1

typedef struct s_block *t_block;

struct s_block {
	size_t size;
	t_block prev;
	t_block next;
	int free;
	void* ptr;
	char data[1];
};

#define BLOCK_SIZE 40
#define WORD_SIZE 32

int amount_free_blocks[WORD_SIZE] = {0};
t_block free_blocks[WORD_SIZE] = {NULL};

void *base_address = NULL;
t_block last = NULL;

/**
 * Obtém um ponteiro para o bloco associado a uma alocação de memória
 * @param p Endereço da memória alocada
 * @return Endereço do bloco
 */
t_block get_block(void* p) {
	char* tmp;
	tmp = p;

	return (p = tmp -= BLOCK_SIZE);
}

/**
 * Checa se um endereço corresponde a uma alocação de memória
 * feita pelo hfmalloc
 * @param p Endereço a ser checado
 * @return 1 caso o endereço corresponda a uma alocação de memória
 */
int valid_addr(void* p) {
	if (base_address) {
		if (p > base_address && p < sbrk(0)) {
			return p == (get_block(p))->ptr;
		}
	}

	return 0;
}

/**
 * Checa se um bloco está na lista de blocos livres
 * @param b Bloco a ser procurado
 * @return inteiro maior que zero, caso tenha sido feita a remoção
 		0 caso contrário
 */
int check_free_block(t_block b) {
	DEBUG_PRINT("**STARTING**\n");
	DEBUG_PRINT("t_block b = %p\n", b);

	if (!b || !valid_addr(b->data)) {
		DEBUG_PRINT("ERROR: b is not a valid block!\n");
		return -1;
	}

	int index = bindex(b->size);
	int i = 0;

	DEBUG_PRINT("bindex(b->size) = %d\n", index);

	t_block temp = free_blocks[index];
	DEBUG_PRINT("temp = %p\n", temp);

	while (i < amount_free_blocks[index] && valid_addr(temp->data)) {
		++i;

		if (temp->ptr == b->ptr
			&& temp->size == b->size) {
			DEBUG_PRINT("**END**\n\n");
			return i;
		}

		temp = *((int*) temp->ptr);
		DEBUG_PRINT("temp = %p\n", temp);
	}

	DEBUG_PRINT("**END**\n\n");
	return 0;
}

/**
 * Insere um bloco da lista de blocos livres, se possível.
 * Caso não seja possível, tenta inserir na posição de índice 
 imediatamente menor até que seja possível
 * @param b Bloco a ser inserido
 * @return 0 caso a inserção tenha sizo realizada com sucesso
 		-1 caso contrário
 */
int push_free_block(t_block b) {
	DEBUG_PRINT("**STARTING**\n");
	DEBUG_PRINT("t_block b = %p\n", b);

	if (!b || !valid_addr(b->data)) {
		DEBUG_PRINT("ERROR: b is not a valid block!\n");
		return -1;
	}

	int index = bindex(b->size);
	t_block temp = NULL;

	DEBUG_PRINT("bindex(b->size) = %d\n", index);

	if (amount_free_blocks[index] < 11) {
		DEBUG_PRINT("amount_free_blocks[%d] < 11\n", index);

		if (amount_free_blocks[index] == 0) {
			DEBUG_PRINT("amount_free_blocks[%d] == 0\n", index);

			free_blocks[index] = b;
		} else {
			DEBUG_PRINT("amount_free_blocks[%d] != 0\n", index);
			temp = free_blocks[index];
			DEBUG_PRINT("temp = %p\n", temp);

			int* c = (int*) b->ptr;
			*c = (int*) temp;

			free_blocks[index] = b;

			DEBUG_PRINT("c = %p, *c = %p\n", c, (void*) *c);
		}

		amount_free_blocks[index]++;

		DEBUG_PRINT("**END**\n\n");
		return 0;
	} else {
		DEBUG_PRINT("amount_free_blocks[%d] >= 11\n", index);

		DEBUG_PRINT("**END**\n\n");
		return -1;
	}
}

/**
 * Remove um bloco da lista de blocos livres, se possível
 * @param b Bloco a ser removido
 * @return inteiro maior que zero, caso tenha sido feita a remoção
 		0 caso contrário
 */
int remove_free_block(t_block b) {
	DEBUG_PRINT("**STARTING**\n");
	DEBUG_PRINT("t_block b = %p\n", b);

	if (!b || !valid_addr(b->data)) {
		DEBUG_PRINT("ERROR: b is not a valid block!\n");
		return -1;
	}

	int index = bindex(b->size);
	t_block temp = NULL;
	t_block last = NULL;
	int i = 0;

	if (amount_free_blocks[index]) {
		temp = free_blocks[index];
		DEBUG_PRINT("temp = %p\n", temp);

		while (i < amount_free_blocks[index] && valid_addr(temp->data)) {
			++i;

			if (temp->ptr == b->ptr
				&& temp->size == b->size) {
				DEBUG_PRINT("found at pos %d\n", i);

				if (i < amount_free_blocks[index]) {
					if (last) {
						DEBUG_PRINT("case: |_|->|b|->|_|\n");

						int* c = (int*) last->ptr;
						*c = (int*) *((int*) temp->ptr);
					} else {
						DEBUG_PRINT("case: |b|->|_|\n");

						free_blocks[index] = (t_block) *((int*) temp->ptr);
					}
				}

				amount_free_blocks[index]--;

				DEBUG_PRINT("**END**\n\n");
				return i;
			}

			last = temp;
			temp = *((int*) temp->ptr);
			DEBUG_PRINT("temp = %p\n", temp);
		}
	}

	DEBUG_PRINT("not found\n");
	DEBUG_PRINT("**END**\n\n");
	return 0;
}

/**
 * Busca um bloco livre de um determinado tamanho
 * na lista de blocos livres utilizando a estratégia
 * half fit
 * @param last Endereço do último bloco procurado
 * @param size Tamanho mínimo do blocos
 * @return Um bloco livre de tamanho >= size, caso exista.
 * 	NULL caso contrário.
 */
t_block pop_free_block(size_t size) {
	DEBUG_PRINT("**STARTING**\n");
	DEBUG_PRINT("size_t size = %lu\n", size);

	int index = rindex(size);
	t_block temp = NULL;
	int i = 0;

	do {
		if (amount_free_blocks[index + i]) {
			temp = free_blocks[index + i];

			DEBUG_PRINT("block = %p\n", temp);
			DEBUG_PRINT("size = %lu\n", temp->size);

			free_blocks[index + i] = (t_block) *((int*) temp->ptr);
			amount_free_blocks[index + i]--;
		}

		++i;
	} while (temp == NULL && index + i < 32);

	if (temp == NULL) DEBUG_PRINT("Not found on index %d\n", index + i);
	else DEBUG_PRINT("Found on index %d\n", index + i);

	DEBUG_PRINT("**END**\n\n");
	return temp;
}

/**
 * Divide um bloco em dois, caso o bloco seja grande o suficiente
 * @param b Bloco a ser dividido
 * @param size Tamanho alvo de um dos blocos
 */
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

/**
 * Aumenta o tamanho da heap
 * @param last Endereço do último bloco procurado
 * @param size Tamanho a ser requisitado
 * @return Um novo bloco com uma alocação de tamanho size
 */
t_block extend_heap(size_t size) {
	DEBUG_PRINT("**STARTING**\n");
	DEBUG_PRINT("size_t size = %lu\n", size);

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

	last = b;

	DEBUG_PRINT("**END**\n\n");
	return b;
}

/**
 * Realiza a fusão de um bloco com o bloco subsequente,
 * se este estiver livre
 * @param b Endereço do bloco a ser fundido
 * @return Endereço do bloco, independente se foi fundido
 */
t_block fusion(t_block b) {
	DEBUG_PRINT("**STARTING**\n");

	if (b->next && b->next->free) {
		DEBUG_PRINT("Block %p fused with %p\n", b, b->next);

		remove_free_block(b);
		remove_free_block(b->next);
		b->size += BLOCK_SIZE + b->next->size;
		b->next = b->next->next;

		DEBUG_PRINT("New size: %lu\n", b->size);

		if (b->next) b->next->prev = b;
	}

	DEBUG_PRINT("**END**\n\n");
	return b;
}

/**
 * Realiza uma alocação dinâmica na memória de tamanho size
 * @param size Tamanho da alocação a ser feita
 * @return Endereço da alocação realizada
 */
void* hfmalloc(size_t size) {
	t_block b;
	size = align4(size);

	if (!size || rindex(size) >= 32) return NULL;

	if (base_address) {
		b = pop_free_block(size);

		if (b) {
			if ((b->size - size) >= (BLOCK_SIZE + 4)) {
				remove_free_block(b);
				split_block(b, size);
				push_free_block(b);
			}

			b->free = 0;
		} else {
			b = extend_heap(size);

			if (!b) return NULL;
		}
	} else {
		b = extend_heap(size);
		
		if (!b) return NULL;

		base_address = b;
	}
	
	return b->data;
}

void* hfcalloc(size_t number, size_t size) {
	size_t *new;
	size_t s4, i;

	new = hfmalloc(number * size);

	if (new) {
		s4 = align4(number * size) << 2;

		for (i = 0; i < s4; i++) new[i] = 0;
	}

	return new;
}

/**
 * Realiza a desalocação de uma alocação feita
 * com o hfmalloc
 * @param p Endereço da alocação a ser desalocada
 */
void hffree(void* p) {
	DEBUG_PRINT("**STARTING**\n");
	DEBUG_PRINT("void* p = %p\n", p);

	t_block b;
	int still_exists = 1;

	if (valid_addr(p)) {
		DEBUG_PRINT("Valid block\n");

		b = get_block(p);
		b->free = 1;
		DEBUG_PRINT("Block set to free\n");

		if (b->prev && b->prev->free) {
			DEBUG_PRINT("Calling fusion with prev\n");
			b = fusion(b->prev);
		}

		if (b->next) {
			DEBUG_PRINT("Calling fusion with next\n");
			b = fusion(b);
		} else {
			DEBUG_PRINT("Unallocating p from heap\n");
			if (b->prev) {
				b->prev->next = NULL;
				last = b->prev;
			}
			else {
				base_address = NULL;
				last = NULL;
			}

			brk(b);

			still_exists = 0;
		}

		if (still_exists) {
			push_free_block(b);
			DEBUG_PRINT("Block added to freelist\n");
		}
	}

	DEBUG_PRINT("**END**\n\n");
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