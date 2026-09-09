# Escalonamento de tarefas

Simulador em C dos algoritmos Rate Monotonic (rate) e Earliest Deadline First
(edf), ambos preemptivos. Empates são resolvidos pela ordem das tarefas no
arquivo de entrada.

## Arquivos

- `scheduler.c`: leitura e validação da entrada, escalonamento e geração da saída.
- `Makefile`: compilação e remoção do executável.
- `testar.sh`: testes de compilação, escalonamento, formato da saída e entradas inválidas.
- `voo.txt`: entrada de exemplo.
- `esperado_edf.out`: saída esperada do exemplo com EDF.

## Ambiente

Projeto em C11, compilado e testado em Linux (Debian GNU/Linux 13).
Requer GCC, Make e Bash para executar os testes.

## Compilar

```sh
make
```

Gera somente o executável `scheduler`.

## Executar

```sh
./scheduler rate voo.txt
./scheduler edf voo.txt
```

O resultado é gravado em `rate_dad.out` ou `edf_dad.out`, no diretório atual.
A execução normal não imprime no terminal. Entradas inválidas geram mensagem
em `stderr`, código de saída diferente de zero e não criam arquivo de saída.

A primeira linha da entrada contém o tempo total da simulação. As demais
contêm nome, período, deadline e tempo de execução de cada tarefa:

```text
100
ATT 20 12 8
NAV 50 30 15
```

Os valores numéricos devem ser inteiros positivos e respeitar `C <= D <= P`,
onde C é o tempo de execução, D é o deadline relativo e P é o período.

Na saída, `F` indica conclusão, `H` indica preempção, `L` indica perda de
deadline e `K` indica encerramento da tarefa ao terminar a simulação.

## Testar

```sh
./testar.sh
```

O script executa 28 verificações, incluindo comparações byte por byte das
saídas esperadas.

## Limpar

```sh
make clean
```

Remove o executável `scheduler`.
