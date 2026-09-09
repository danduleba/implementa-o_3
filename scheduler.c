#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define TAM_NOME 32
#define LOGIN "dad"

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
    int fim_nome, fim_periodo, fim_deadline;
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
        if(sscanf(linha,  "%31s%n %31s%n %31s%n %31s %1s", tarefa.nome, &fim_nome, texto_periodo, &fim_periodo, texto_deadline, &fim_deadline, texto_burst, extra) != 4 ||
           !isspace((unsigned char)linha[fim_nome]) ||
           !isspace((unsigned char)linha[fim_periodo]) ||
           !isspace((unsigned char)linha[fim_deadline])){
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

void registrar_trecho(FILE *saida, Tarefa *tarefas, int tarefa, int inicio, int fim, char estado){
    int duracao = fim - inicio;
    if(duracao<=0){
        return;
    }
    if(tarefa == -1){
        fprintf(saida, "idle for %d units\n", duracao);
    }
    else{
        fprintf(saida, "[%s] for %d units - %c\n", tarefas[tarefa].nome,duracao,estado);
    }
}

void simular(Tarefa *tarefas, int quantidade, int tempo_total, const char *algoritimo, FILE *saida){
    int tarefa_atual=-2;
    int inicio_trecho =0;
    int tempo;
    int escolhida;
    int i;

    inicializar_tarefa(tarefas, quantidade);

    for(tempo = 0; tempo<tempo_total; tempo++){
        for(i=0; i<quantidade; i++){
            if(tarefas[i].ativa && tarefas[i].restante > 0 && tarefas[i].deadline_absoluto == tempo){
                if(tarefa_atual == i){
                    registrar_trecho(saida,tarefas, i, inicio_trecho, tempo, 'L');
                    tarefa_atual = -2;
                }
                tarefas[i].perdidas++;
                tarefas[i].restante = 0;
                tarefas[i].ativa=0;
            }
        }
        liberar_tarefas(tarefas,quantidade, tempo);
        escolhida = escolher_tarefa(tarefas, quantidade, algoritimo);
        if (escolhida != tarefa_atual) {
            if (tarefa_atual == -1) {
                registrar_trecho(saida, tarefas, -1,inicio_trecho, tempo, ' ');
            } 
            else if (tarefa_atual >= 0) {
                registrar_trecho(saida, tarefas, tarefa_atual,inicio_trecho, tempo, 'H');
            }

    tarefa_atual = escolhida;
    inicio_trecho = tempo;
}

        if(escolhida >=0){
            tarefas[escolhida].restante--;
            if(tarefas[escolhida].restante ==0){
                tarefas[escolhida].concluidas++;
                tarefas[escolhida].ativa = 0;
                registrar_trecho(saida, tarefas, escolhida, inicio_trecho, tempo +1, 'F');
                tarefa_atual = -2;
            }
        }
    }
    for(i=0; i<quantidade; i++){
        if(tarefas[i].ativa && tarefas[i].restante > 0 && tarefas[i].deadline_absoluto ==tempo_total){
            if(tarefa_atual == i){
                registrar_trecho(saida, tarefas, i, inicio_trecho, tempo_total, 'L');
                tarefa_atual = -2;
            }
            tarefas[i].perdidas++;
            tarefas[i].restante=0;
            tarefas[i].ativa = 0;
        }
    }
    if(tarefa_atual == -1){
        registrar_trecho(saida, tarefas, -1, inicio_trecho, tempo_total, ' ');
    }
    else if(tarefa_atual>=0){
        registrar_trecho(saida,tarefas,tarefa_atual,inicio_trecho,tempo_total, 'K');
    }
    for(i =0; i<quantidade; i++){
        if (tarefas[i].ativa && tarefas[i].restante > 0) {
        tarefas[i].mortas++;
        }
    }
    fprintf(saida, "\nLOST DEADLINES\n");

    for (i = 0; i < quantidade; i++) {
        fprintf(saida, "[%s] %d\n",tarefas[i].nome, tarefas[i].perdidas);
    }

    fprintf(saida, "\nCOMPLETE EXECUTION\n");

    for (i = 0; i < quantidade; i++) {
        fprintf(saida, "[%s] %d\n",tarefas[i].nome, tarefas[i].concluidas);
    }

    fprintf(saida, "\nKILLED\n");

    for (i = 0; i < quantidade; i++) {
        fprintf(saida, "[%s] %d\n",tarefas[i].nome, tarefas[i].mortas);
    }
}

int executar_simulacao(const char *algoritimo, Tarefa *tarefas, int quantidade, int tempo_total){
    FILE *saida;
    char nome_saida[64];

    snprintf(nome_saida, sizeof nome_saida, "%s_" LOGIN ".out", algoritimo);

    saida=fopen(nome_saida,"w");

    if(saida==NULL){
        fprintf(stderr, "nao foi possivel criar arquivo de saida\n");
        return 0;
    }
    if(fprintf(saida,"EXECUTION BY %s\n\n", strcmp(algoritimo, "rate")== 0 ? "RATE" : "EDF")<0){
        fclose(saida);
        return 0;
    }
    simular(tarefas,quantidade,tempo_total,algoritimo,saida);
    return fclose(saida) == 0;
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
    if (!executar_simulacao(argv[1], tarefas,quantidade, tempo_total)) {
    free(tarefas);
    return 1;
    }
    free(tarefas);
    return 0;
}