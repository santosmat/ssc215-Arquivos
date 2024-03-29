/* Criado por Mateus de Souza Santos 11366913
    e Wellington Matos Amaral 11315054
*/

#include <stdio.h>
#include <stdlib.h>
#include <registro.h>
#include <string.h>
#include <binarionatela.h>


struct registroCabecalho{
    unsigned char status;           // Sempre que uma modificacao inicia assume `0`, se concluir com sucesso assume `1`
    int RRNproxRegistro;            // Byteoffset do proximo registro
    int numeroRegistrosInseridos;   // Inicialmente vale 0 e incrementa sempre que existir um novo registro
    int numeroRegistrosRemovidos;   // Os registros ser'ao logicamente removidos, incrementam sempre que isso ocorrer
    int numeroRegistrosAtualizados; // Sempre que alguma informacao em um registro for atualizada, ele sera incrmentado
    char lixo[111];                 // Armazena apenas `$` para manter o padrao de tamanho de 128 bites
};

/* Organizacao hibrida de registros, haverao campos de tamanho fixo, campo de tamanho variavel 
 * (indicando o tamanho do campo) e delimitador entre registros `#`
 */

struct registro{

    // Campos de tamanho fixo: tamanho maximo de 23 bytes
    int idNascimento;               // código sequencial que identifica univocamente cada registro do arquivo de dados)
    int idadeMae;                   // idade da mãe do bebê
    char dataNascimento[11];        // formato AAAA-MM-DD; data de nascimento do bebê
    char sexoBebe;                  // sexo  do  bebê pode assumir os valores ‘0’(ignorado), ‘1’(masculino) e ‘2’(feminino)
    char estadoMae[3];              // sigla  do  estado  da cidade de residência da mãe
    char estadoBebe[3];             // sigla  do  estado  da cidade na qual o bebê nasceu
    int tam_CidadeMae;              // tamanho do campo cidadeMae
    int tam_CidadeBebe;             // tamanho do campo cidadeBebe

    // Campos  de  tamanho  variável: tamanho  máximo  de 105 bytes incluindo  os espaços reservados para os indicadores de tamanho
    char* cidadeMae;                // cidade de residência da mãe
    char* cidadeBebe;               // cidade na qual o bebê nasceu
};


CABECALHO* lerCabecalhoBin(FILE* bin){
    CABECALHO *aux = calloc(1, sizeof(CABECALHO)); 
    
    if(aux == NULL) return aux;
    
    fread(&(aux->status), sizeof(unsigned char), 1, bin);
    fread(&(aux->RRNproxRegistro), sizeof(int), 1, bin);
    fread(&(aux->numeroRegistrosInseridos), sizeof(int), 1, bin);
    fread(&(aux->numeroRegistrosRemovidos), sizeof(int), 1, bin);
    fread(&(aux->numeroRegistrosAtualizados), sizeof(int), 1, bin);
    fread(&(aux->lixo), sizeof(char), 111, bin);

    return aux;
}

int criaBinario(FILE *src, FILE* dest){
    CABECALHO *header;
    REGISTRO regLido;
    unsigned char lixo = '$';
    int i = 0, lastID = -1;
    
    if((header = calloc(1, sizeof(CABECALHO))) == NULL) return ERRO;
 
    fseek(src, 0, SEEK_END);     // Ponteiro vai pro final do arquivo csv

    if(ftell(src) == 0){         // Verifica se o arquivo tem 0 byte
        criarCabecalhobin(header, dest);
        return SUCESSO;
    }

    fseek(src, 0, SEEK_SET);     // Volta o ponteiro para o inicio do arquivo

    regLido = lerRegistro(src);

    free(regLido.cidadeMae);
    free(regLido.cidadeBebe);

    while(1){
        
        //byteoffset do prox registro
        fseek(dest, (header->RRNproxRegistro * 128) + 128, SEEK_SET);

        //armazena os dados
        regLido = lerRegistro(src);
        
        if(lastID == regLido.idNascimento) break;

        lastID = regLido.idNascimento;

        fwrite(&regLido.tam_CidadeMae, sizeof(int), 1, dest);
        fwrite(&regLido.tam_CidadeBebe, sizeof(int), 1, dest);
        fwrite(regLido.cidadeMae, sizeof(char), regLido.tam_CidadeMae, dest);
        fwrite(regLido.cidadeBebe, sizeof(char), regLido.tam_CidadeBebe, dest);
        int ate = 105 - 8- regLido.tam_CidadeMae - regLido.tam_CidadeBebe;
        for (i = 0; i < ate; i++) fwrite(&lixo, sizeof(char), 1, dest);
        fwrite(&regLido.idNascimento, sizeof(int), 1, dest);
        fwrite(&regLido.idadeMae, sizeof(int), 1, dest);
        fwrite(&regLido.dataNascimento, sizeof(char), 10, dest);
        fwrite(&regLido.sexoBebe, sizeof(char), 1, dest);
        fwrite(&regLido.estadoMae, sizeof(char), 2, dest);
        fwrite(&regLido.estadoBebe, sizeof(char), 2, dest);

        free(regLido.cidadeMae);
        free(regLido.cidadeBebe);

        //atualiza o header
        header->numeroRegistrosInseridos++;
        header->RRNproxRegistro ++;
        i++;
    }


    free(regLido.cidadeMae);
    free(regLido.cidadeBebe);

    //armazenar o header
    criarCabecalhobin(header, dest);
    
    return SUCESSO;
}

int criarCabecalhobin(CABECALHO* header, FILE* dest){
    char lixo = '$';

    header->status = '1';
    fseek(dest, 0, SEEK_SET);
    fwrite(&header->status, sizeof(char), 1, dest);
    fwrite(&header->RRNproxRegistro, sizeof(int), 1, dest);
    fwrite(&header->numeroRegistrosInseridos, sizeof(int),1, dest);
    fwrite(&header->numeroRegistrosAtualizados, sizeof(int), 1, dest);
    fwrite(&header->numeroRegistrosRemovidos, sizeof(int), 1, dest);
    for(int i = 0; i < 111; i++) fwrite(&lixo, sizeof(char), 1, dest);

    free(header); 
    
    return SUCESSO;
}

REGISTRO lerRegistro(FILE *csv){
    REGISTRO reg;

    int k = 0, j = 0;

    char entrada[140], str[8][128];

    fgets(entrada, 140, csv);

    for(int i = 0; i < 8; i++){
        j = 0;
        while(entrada[k] != '\n' && entrada[k] != ',' && entrada[k] != 10 ){
            str[i][j] = entrada[k];
            k++, j++;
        } 
        

        if(entrada[k] == ',' || entrada[k] != '\n' ||  entrada[k] == 10) str[i][j] = '\0', k++;
        trim(str[i]);
    }

    reg.tam_CidadeMae = strlen(str[0]);
    reg.cidadeMae = calloc(sizeof(char), reg.tam_CidadeMae);
    strncpy(reg.cidadeMae, str[0], reg.tam_CidadeMae);
    
    reg.tam_CidadeBebe = strlen(str[1]);
    reg.cidadeBebe = calloc(sizeof(char), reg.tam_CidadeBebe);
    strncpy(reg.cidadeBebe, str[1], reg.tam_CidadeBebe);
    reg.idNascimento = atoi(str[2]);
    reg.idadeMae = atoi(str[3]);
    if(reg.idadeMae == 0) reg.idadeMae = -1;
    strncpy(reg.dataNascimento, str[4], 10);
    if(reg.dataNascimento[0]=='\0'){
        for(int i = 1; i < 10; i++){
            reg.dataNascimento[i] = '$';
        }       
    }
    if(str[5][0] == '\0') reg.sexoBebe = '0';
    else reg.sexoBebe = str[5][0];
    strncpy(reg.estadoMae, str[6], 2); 
    if(reg.estadoMae[0] == '\0') reg.estadoMae[1] = '$';
    strncpy(reg.estadoBebe, str[7], 2);
    if(reg.estadoBebe[0] == '\0') reg.estadoBebe[1] = '$';

    return reg;
}

int lerBinario(FILE *bin){
    CABECALHO *header;
    REGISTRO aux;
    
    int i;

    if((header = lerCabecalhoBin(bin)) == NULL || header->status == '0'){
        free(header);
        return ERRO;
    }


    if(header->numeroRegistrosInseridos == 0){
        printf("Registro inexistente.\n");
        return SUCESSO;
    }
    
    fseek(bin, 128, SEEK_SET);

    for(i = 0; i < header->numeroRegistrosInseridos; i++){
        fseek(bin, 128 + (128 * i), SEEK_SET);
        imprimirRegistroBin(bin);
    }
    
    free(header);

    return SUCESSO;
}

int imprimirRegistroBin(FILE *bin){
    REGISTRO aux;
    char sexo[10];

    fread(&(aux.tam_CidadeMae), sizeof(int), 1, bin);

    if(aux.tam_CidadeMae == -1) return regDeletado;

    fread(&(aux.tam_CidadeBebe), sizeof(int), 1, bin);
    
    aux.cidadeMae = calloc(aux.tam_CidadeMae + 1, sizeof(char));
    aux.cidadeBebe = calloc(aux.tam_CidadeBebe + 2, sizeof(char));

    fread(aux.cidadeMae, sizeof(char), aux.tam_CidadeMae, bin);
    aux.cidadeMae[aux.tam_CidadeMae] = '\0';
    fread(aux.cidadeBebe, sizeof(char), aux.tam_CidadeBebe, bin);
    aux.cidadeBebe[aux.tam_CidadeBebe] = '\0';
    if(aux.tam_CidadeBebe == 0) strcpy(aux.cidadeBebe, "-");

    fseek(bin, 105 - ( 8 + aux.tam_CidadeMae + aux.tam_CidadeBebe ), SEEK_CUR);

    fread(&(aux.idNascimento), sizeof(int), 1, bin);
    fread(&(aux.idadeMae), sizeof(int), 1, bin);
    fread(&(aux.dataNascimento), sizeof(char), 10, bin);
    aux.dataNascimento[10] = '\0';
    if(strlen(aux.dataNascimento) == 0) strcpy(aux.dataNascimento, "-");
    fread(&(aux.sexoBebe), sizeof(char), 1, bin);
    fread(&(aux.estadoMae), sizeof(char), 2, bin);
    aux.estadoMae[2] = '\0';
    fread(&(aux.estadoBebe), sizeof(char), 2, bin);
    aux.estadoBebe[2] = '\0';
    if(strlen(aux.estadoBebe) == 0) strcpy(aux.estadoBebe, "-");
    

    if(aux.sexoBebe == '0') strcpy(sexo, "IGNORADO");
    if(aux.sexoBebe == '1') strcpy(sexo, "MASCULINO"); 
    if(aux.sexoBebe == '2') strcpy(sexo, "FEMININO"); 

    printf("Nasceu em %s/%s, em %s, um bebê de sexo %s.\n", 
    aux.cidadeBebe, aux.estadoBebe, aux.dataNascimento, sexo);

    free(aux.cidadeMae);
    free(aux.cidadeBebe);

    return SUCESSO;
}

// Remover registro

/* Qualquer combinação de campopode ser usada como forma de  busca. Os  valores  
 dos campos  do  tipo string devem  ser  especificados  entre  aspas duplas  (").
 a  função scan_quote_string eh usada para manipular string ""
 Os dados devem ser  mostrados no  mesmo formato  definido  para  a  funcionalidade 
 Registros marcados como logicamente removidos  não devem ser exibidos.*/

// Pesquisa por itens gravados no arquivo

/* O usuario pode fazer qualquer combinacao de campos do registro,
* receber diversos itens na chamada da funcao, elas sera usadas para criar um "fitro"
* e retornar todos os itens que correspondem a ele
*/


// Busca por RRN

/*  Percorrer o arquivo ate encontrar o RRN, caso ele exista, imprimir as informacoes deste
* caso nao exista, retornar erro, caso ja tenha sido deletado, retornar erro
*/


// Remocao logica de registro

/* 
* Mudar o status para inconsistente
* Deve-se mudar o valor do primeiro campo para -1 neste caso, TamCidadeMae
* todos os outros campos continuam sem alteracoes
* 
* voltar o status para consistente
*/

// Iserir elementos adicionais     

/* 
* Mudar o status do para inconsistente
* Pode ser inserido em locais onde ja foram removidos registros
* nao deve ser tratado truncamento
* 
* valores nulos na entrada, devem ser identificados como NULL
* 
* mudar o status para consistente
* caso retorne erro imprimir "Falha no processamento do arquivo"
*/ 

// Atualizar registro

/* 
* mudar o status para inconsistente
* O usuario pode atualizar as informacoes de um registro, ele ira fornecer o RRN do
* item, nao deve tratar os caracteres antigos que estejam no ristro anteriormente
* 
* caso o RRN nao exista, o programa deve continuar normalmente apenas retornar para a main
* mudar o status para consistente
*/
