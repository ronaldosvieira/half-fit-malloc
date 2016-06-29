#ifndef HFMALLOC_H_
#define HFMALLOC_H_

#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <math.h>

#define DEBUG 3

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

/**
 * Obtém um ponteiro para o bloco associado a uma alocação de memória
 * @param p Endereço da memória alocada
 * @return Endereço do bloco
 */
t_block get_block(void* p);

/**
 * Checa se um endereço corresponde a uma alocação de memória
 * feita pelo hfmalloc
 * @param p Endereço a ser checado
 * @return 1 caso o endereço corresponda a uma alocação de memória
 */
int valid_addr(void* p);

/**
 * Checa se um bloco está na lista de blocos livres
 * @param b Bloco a ser procurado
 * @return inteiro maior que zero, caso tenha sido feita a remoção
 		0 caso contrário
 */
int check_free_block(t_block b);

/**
 * Insere um bloco da lista de blocos livres, se possível.
 * Caso não seja possível, tenta inserir na posição de índice 
 imediatamente menor até que seja possível
 * @param b Bloco a ser inserido
 * @return 0 caso a inserção tenha sizo realizada com sucesso
 		-1 caso contrário
 */
int push_free_block(t_block b);

/**
 * Remove um bloco da lista de blocos livres, se possível
 * @param b Bloco a ser removido
 * @return inteiro maior que zero, caso tenha sido feita a remoção
 		0 caso contrário
 */
int remove_free_block(t_block b);

/**
 * Busca um bloco livre de um determinado tamanho
 * na lista de blocos livres utilizando a estratégia
 * half fit
 * @param last Endereço do último bloco procurado
 * @param size Tamanho mínimo do blocos
 * @return Um bloco livre de tamanho >= size, caso exista.
 * 	NULL caso contrário.
 */
t_block pop_free_block(size_t size);

/**
 * Divide um bloco em dois, caso o bloco seja grande o suficiente
 * @param b Bloco a ser dividido
 * @param size Tamanho alvo de um dos blocos
 */
void split_block(t_block b, size_t size);

/**
 * Aumenta o tamanho da heap
 * @param last Endereço do último bloco procurado
 * @param size Tamanho a ser requisitado
 * @return Um novo bloco com uma alocação de tamanho size
 */
t_block extend_heap(size_t size);

/**
 * Realiza a fusão de um bloco com o bloco subsequente,
 * se este estiver livre
 * @param b Endereço do bloco a ser fundido
 * @return Endereço do bloco, independente se foi fundido
 */
t_block fusion(t_block b);

/**
 * Realiza uma alocação dinâmica na memória de tamanho size
 * @param size Tamanho da alocação a ser feita
 * @return Endereço da alocação realizada
 */
void* hfmalloc(size_t size);

void* hfcalloc(size_t number, size_t size);

/**
 * Realiza a desalocação de uma alocação feita
 * com o hfmalloc
 * @param p Endereço da alocação a ser desalocada
 */
void hffree(void* p);

void print_heap();

#endif // HFMALLOC_H_