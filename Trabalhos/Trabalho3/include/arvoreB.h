#ifndef H_ARVOREB_
#define H_ARVOREB_

typedef struct _arvoreB ARVOREB;
typedef struct _noB PAGINA;

#define VAZIO -1
#define PROMOVE -3
#define NAOPROMOVE -4
#define DEURUIM -5
#define ENCONTROU -2
#define M 6

int criarIndiceArvoreB(FILE* bin, FILE* indice);
int buscarIndice(FILE* indexBtree, int chave);
int inserirChave(ARVOREB *bTree, int chave, int Pr, FILE *index);
ARVOREB* criarCabecalhoArvoreB();
int buscaArvoreB(FILE* dataset, FILE* indexBtree, int chave);
int inserirRegistroArvoreB(FILE* bin, FILE* indice);

#endif