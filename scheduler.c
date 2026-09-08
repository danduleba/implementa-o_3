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
    int restante; 
    int deadline_absoluto;
    int concluidas;
    int perdidas;
    int mortas;
    int ativa;
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
        fprintf(stderr,"Não foi possivel abrir o arquivo\n");
        return 0;
    }
    if(fgets(linha, sizeof linha, arquivo) == NULL || !ler_positivo(linha, tempo_total)){
        fprintf(stderr, "tempo total invalido\n");
        fclose(arquivo);
        return 0;
    }
    *tarefas = NULL;
    *quantidade = 0;

    while (fgets(linha,sizeof linha, arquivo)!= NULL){ 
        if(sscanf(linha,  "%31s %31s %31s %31s %1s", tarefa.nome, texto_periodo, texto_deadline, texto_burst, extra) != 4){
            fprintf(stderr, "ERRO: linha de tarefa invalida\n");
            free(*tarefas);
            fclose(arquivo);
            return 0;
        }
        if(!ler_positivo(texto_periodo, &tarefa.periodo) || !ler_positivo(texto_deadline, &tarefa.deadline) || !ler_positivo(texto_burst,&tarefa.burst )){
            fprintf(stderr, "erro; valor de tarefa invalido\n");
            free(*tarefas);
            fclose(arquivo);
            return 0;
        }
        if(tarefa.burst>tarefa.deadline || tarefa.deadline > tarefa.periodo){
            fprintf(stderr, "erro: a tarefa deve respeitar C <= D <= P.\n");
            free(*tarefas);
            fclose(arquivo);
            return 0;
        }
        tarefa.ordem= *quantidade;
        novo = realloc(*tarefas,(size_t)(*quantidade+1) * sizeof **tarefas);
        if (novo == NULL) {
            fprintf(stderr, "Erro: falha na alocacao de memoria.\n");
            free(*tarefas);
            fclose(arquivo);
            return 0;
        }
        *tarefas = novo;
        (*tarefas)[*quantidade] = tarefa;
        (*quantidade)++;
    }
    fclose(arquivo);

    if (*quantidade == 0) {
        fprintf(stderr, "Erro: nenhuma tarefa encontrada.\n");
        free(*tarefas);
        *tarefas = NULL;
        return 0;
    }

    return 1;
    
}

int tem_prioridade(const Tarefa *a, const Tarefa *b, const char *algoritimo){
    int prioridade_a;
    int prioridade_b;

    if(strcmp(algoritimo, "rate") == 0){
        prioridade_a = a->periodo;
        prioridade_b = b->periodo;
    }
    else{
        prioridade_a = a->deadline_absoluto;
        prioridade_b = b->deadline_absoluto;
    }
    if(prioridade_a!=prioridade_b){
        return prioridade_a<prioridade_b;
    }
    return a->ordem < b->ordem;
}

int escolher_tarefa(Tarefa *tarefas, int quantidade, const char *algoritimo){
    int escolhida = -1;
    int i;
    for(i = 0; i < quantidade; i++){
        if(!tarefas[i].ativa || tarefas[i].restante ==0){
            continue;
        }
        if (escolhida == -1 || tem_prioridade(&tarefas[i], &tarefas[escolhida], algoritimo)){
            escolhida = i;
        }
    }
    return escolhida;
}
void liberar_tarefas(Tarefa *tarefas, int quantidade, int tempo){
    int i;

    for(i=0; i<quantidade; i++){
        if(tempo % tarefas[i]. periodo == 0){
            tarefas[i].restante = tarefas[i].burst;
            tarefas[i].deadline_absoluto = tempo + tarefas[i].deadline;
            tarefas[i].ativa = 1;
        }
    }
}

void inicializar_tarefa(Tarefa *tarefas, int quantidade){
    int i;
    for(i=0; i<quantidade; i++){
        tarefas[i].restante = 0;
        tarefas[i].deadline_absoluto = 0;
        tarefas[i].concluidas = 0;
        tarefas[i].perdidas = 0;
        tarefas[i].mortas = 0;
        tarefas[i].ativa = 0;

    }
}

void simular(Tarefa *tarefas, int quantidade, int tempo_total, const char *algoritimo){
    int tempo;
    int escolhida;
    int i;

    inicializar_tarefa(tarefas, quantidade);

    for(tempo = 0; tempo<tempo_total; tempo++){
        for(i=0; i<quantidade; i++){
            if(tarefas[i].ativa && tarefas[i].restante > 0 && tarefas[i].deadline_absoluto == tempo){
                tarefas[i].perdidas++;
                tarefas[i].restante = 0;
                tarefas[i].ativa=0;
            }
        }
        liberar_tarefas(tarefas,quantidade, tempo);
        escolhida = escolher_tarefa(tarefas, quantidade, algoritimo);

        if(escolhida >=0){
            tarefas[escolhida].restante--;
            if(tarefas[escolhida].restante ==0){
                tarefas[escolhida].concluidas++;
                tarefas[escolhida].ativa = 0;
            }
        }
    }
    for(i=0; i<quantidade; i++){
        if(tarefas[i].ativa && tarefas[i].restante > 0 && tarefas[i].deadline_absoluto ==tempo_total){
            tarefas[i].perdidas++;
            tarefas[i].restante=0;
            tarefas[i].ativa = 0;
        }
        if(tarefas[i].ativa && tarefas[i].restante >0){
            tarefas[i].mortas++;
        }
    }
}

int main(int argc, char *argv[]) {
    Tarefa *tarefas;
    int quantidade;
    int tempo_total;

    if (argc != 3) {
        fprintf(stderr, "Uso: %s rate|edf arquivo\n", argv[0]);
        return 1;
    }

    if (strcmp(argv[1], "rate") != 0 &&
        strcmp(argv[1], "edf") != 0) {
        fprintf(stderr, "Erro: use rate ou edf.\n");
        return 1;
    }

    if (!carregar_arquivo(argv[2], &tempo_total,&tarefas, &quantidade)) {
        return 1;
    }
    simular(tarefas, quantidade, tempo_total, argv[1]);
    free(tarefas);
    return 0;
}