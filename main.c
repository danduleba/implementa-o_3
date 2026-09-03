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