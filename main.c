#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define TAM_NOME 32

typedef struct {
    char nome[TAM_NOME];
    int periodo;
    int deadline;
    int burst;
    int ordem;
}Tarefa;

int ler_positivo(const char *texto, int *valor){
    char *fim;
    long numero;

    errno =0;
    numero = strtol(texto,&fim,10);

    if(errno != 0 || fim == texto || numero < 1 || numero > INT_MAX){
        return 0;
    }
    while (isspace((unsigned char)*fim)){
        fim++;
    }
    if(*fim != '\0'){
        return 0;
    }
    *valor = (int)numero;
    return 1;
}

int carregar_arquivo(const char *nome_arquivo, int *tempo_total, Tarefa **tarefas, int *quantidade){
    FILE *arquivo;
    char linha[256];
    char texto_periodo[32];
    char texto_deadline[32];
    char texto_burst[32];
    char extra[2];
    Tarefa tarefa;
    Tarefa *novo;

    arquivo = fopen(nome_arquivo, "r");

    if(arquivo==NULL){
        printf(stderr,"Não foi possivel abrir o arquivo\n");
        return 0;
    }
    if(fgets(linha, sizeof linha, arquivo) == NULL || !ler_positivo(linha, tempo_total)){
        fprintf(stderr, "tempo total invalido\n");
        return 0;
    }
    *tarefas = NULL;
    *quantidade = 0;

    while (fgets(linha,sizeof linha, arquivo)!= NULL){ 
        if(sscanf(linha,  "%31s %31s %31s %31s %31s"))
    }
    
}