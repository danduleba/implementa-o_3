#!/usr/bin/env bash

LOGIN="dad"
TOTAL=0
PASSARAM=0
FALHARAM=0
PASTA_TESTE=$(mktemp -d)

finalizar() {
    chmod -R u+w "$PASTA_TESTE" 2>/dev/null
    rm -rf "$PASTA_TESTE"
}

trap finalizar EXIT

passou() {
    TOTAL=$((TOTAL + 1))
    PASSARAM=$((PASSARAM + 1))
    printf '[PASSOU] %s\n' "$1"
}

falhou() {
    TOTAL=$((TOTAL + 1))
    FALHARAM=$((FALHARAM + 1))
    printf '[FALHOU] %s\n' "$1"
}

echo "========================================"
echo "TESTES DO ESCALONADOR"
echo "========================================"
echo

if [ -f Makefile ] &&
   make clean >/dev/null 2>&1 &&
   make >/dev/null 2>&1 &&
   [ -x scheduler ]; then
    passou "Makefile: make clean e make"
else
    falhou "Makefile: make clean e make"
fi

if gcc -Wall -Wextra -pedantic -std=c11 \
       scheduler.c -o scheduler \
       >"$PASTA_TESTE/compilacao.txt" 2>&1; then

    if [ -s "$PASTA_TESTE/compilacao.txt" ]; then
        falhou "Compilacao sem avisos"
    else
        passou "Compilacao sem erros e sem avisos"
    fi
else
    falhou "Compilacao do scheduler"
fi

if [ ! -x scheduler ]; then
    echo
    echo "Nao foi possivel continuar sem o executavel."
    exit 1
fi

cp scheduler "$PASTA_TESTE/scheduler"

cat > "$PASTA_TESTE/voo.txt" <<'EOF'
100
ATT 20 12 8
NAV 50 30 15
EOF

cat > "$PASTA_TESTE/esperado_rate.out" <<'EOF'
EXECUTION BY RATE

[ATT] for 8 units - F
[NAV] for 12 units - H
[ATT] for 8 units - F
[NAV] for 2 units - L
idle for 10 units
[ATT] for 8 units - F
idle for 2 units
[NAV] for 10 units - H
[ATT] for 8 units - F
[NAV] for 5 units - F
idle for 7 units
[ATT] for 8 units - F
idle for 12 units

LOST DEADLINES
[ATT] 0
[NAV] 1

COMPLETE EXECUTION
[ATT] 5
[NAV] 1

KILLED
[ATT] 0
[NAV] 0
EOF

cat > "$PASTA_TESTE/esperado_edf.out" <<'EOF'
EXECUTION BY EDF

[ATT] for 8 units - F
[NAV] for 15 units - F
[ATT] for 8 units - F
idle for 9 units
[ATT] for 8 units - F
idle for 2 units
[NAV] for 10 units - H
[ATT] for 8 units - F
[NAV] for 5 units - F
idle for 7 units
[ATT] for 8 units - F
idle for 12 units

LOST DEADLINES
[ATT] 0
[NAV] 0

COMPLETE EXECUTION
[ATT] 5
[NAV] 2

KILLED
[ATT] 0
[NAV] 0
EOF

rm -f "$PASTA_TESTE/rate_${LOGIN}.out"

(
    cd "$PASTA_TESTE" || exit 1
    ./scheduler rate voo.txt >stdout.txt 2>stderr.txt
)
STATUS=$?

if [ "$STATUS" -eq 0 ]; then
    passou "Rate: codigo de retorno correto"
else
    falhou "Rate: codigo de retorno correto"
fi

if [ ! -s "$PASTA_TESTE/stdout.txt" ]; then
    passou "Rate: nenhuma mensagem em stdout"
else
    falhou "Rate: nenhuma mensagem em stdout"
fi

if [ ! -s "$PASTA_TESTE/stderr.txt" ]; then
    passou "Rate: nenhuma mensagem em stderr"
else
    falhou "Rate: nenhuma mensagem em stderr"
fi

if [ -f "$PASTA_TESTE/rate_${LOGIN}.out" ]; then
    passou "Rate: arquivo rate_${LOGIN}.out criado"
else
    falhou "Rate: arquivo rate_${LOGIN}.out criado"
fi

if cmp -s \
   "$PASTA_TESTE/esperado_rate.out" \
   "$PASTA_TESTE/rate_${LOGIN}.out"; then
    passou "Rate: saida exatamente igual ao enunciado"
else
    falhou "Rate: saida exatamente igual ao enunciado"
fi

rm -f "$PASTA_TESTE/edf_${LOGIN}.out"

(
    cd "$PASTA_TESTE" || exit 1
    ./scheduler edf voo.txt >stdout.txt 2>stderr.txt
)
STATUS=$?

if [ "$STATUS" -eq 0 ]; then
    passou "EDF: codigo de retorno correto"
else
    falhou "EDF: codigo de retorno correto"
fi

if [ ! -s "$PASTA_TESTE/stdout.txt" ]; then
    passou "EDF: nenhuma mensagem em stdout"
else
    falhou "EDF: nenhuma mensagem em stdout"
fi

if [ ! -s "$PASTA_TESTE/stderr.txt" ]; then
    passou "EDF: nenhuma mensagem em stderr"
else
    falhou "EDF: nenhuma mensagem em stderr"
fi

if [ -f "$PASTA_TESTE/edf_${LOGIN}.out" ]; then
    passou "EDF: arquivo edf_${LOGIN}.out criado"
else
    falhou "EDF: arquivo edf_${LOGIN}.out criado"
fi

if cmp -s \
   "$PASTA_TESTE/esperado_edf.out" \
   "$PASTA_TESTE/edf_${LOGIN}.out"; then
    passou "EDF: saida exatamente igual ao esperado"
else
    falhou "EDF: saida exatamente igual ao esperado"
fi

cat > "$PASTA_TESTE/empate.txt" <<'EOF'
4
A 10 10 2
B 10 10 2
EOF

cat > "$PASTA_TESTE/empate_rate_esperado.out" <<'EOF'
EXECUTION BY RATE

[A] for 2 units - F
[B] for 2 units - F

LOST DEADLINES
[A] 0
[B] 0

COMPLETE EXECUTION
[A] 1
[B] 1

KILLED
[A] 0
[B] 0
EOF

rm -f "$PASTA_TESTE/rate_${LOGIN}.out"

(
    cd "$PASTA_TESTE" || exit 1
    ./scheduler rate empate.txt >stdout.txt 2>stderr.txt
)

if cmp -s \
   "$PASTA_TESTE/empate_rate_esperado.out" \
   "$PASTA_TESTE/rate_${LOGIN}.out"; then
    passou "Desempate pela ordem do arquivo"
else
    falhou "Desempate pela ordem do arquivo"
fi

cat > "$PASTA_TESTE/killed.txt" <<'EOF'
3
A 10 10 5
EOF

cat > "$PASTA_TESTE/killed_esperado.out" <<'EOF'
EXECUTION BY RATE

[A] for 3 units - K

LOST DEADLINES
[A] 0

COMPLETE EXECUTION
[A] 0

KILLED
[A] 1
EOF

rm -f "$PASTA_TESTE/rate_${LOGIN}.out"

(
    cd "$PASTA_TESTE" || exit 1
    ./scheduler rate killed.txt >stdout.txt 2>stderr.txt
)

if cmp -s \
   "$PASTA_TESTE/killed_esperado.out" \
   "$PASTA_TESTE/rate_${LOGIN}.out"; then
    passou "Tarefa morta no final da simulacao"
else
    falhou "Tarefa morta no final da simulacao"
fi

cat > "$PASTA_TESTE/campo_faltando.txt" <<'EOF'
100
ATT 20 12
EOF

cat > "$PASTA_TESTE/nao_numerico.txt" <<'EOF'
100
ATT vinte 12 8
EOF

cat > "$PASTA_TESTE/nao_positivo.txt" <<'EOF'
100
ATT 20 12 0
EOF

cat > "$PASTA_TESTE/c_maior_d.txt" <<'EOF'
100
ATT 20 5 8
EOF

cat > "$PASTA_TESTE/d_maior_p.txt" <<'EOF'
100
ATT 20 30 8
EOF

cat > "$PASTA_TESTE/campo_extra.txt" <<'EOF'
100
ATT 20 12 8 EXTRA
EOF

cat > "$PASTA_TESTE/tempo_invalido.txt" <<'EOF'
zero
ATT 20 12 8
EOF

cat > "$PASTA_TESTE/sem_tarefas.txt" <<'EOF'
100
EOF

testar_erro() {
    DESCRICAO="$1"
    shift

    rm -f "$PASTA_TESTE/rate_${LOGIN}.out"
    rm -f "$PASTA_TESTE/edf_${LOGIN}.out"
    : > "$PASTA_TESTE/stdout.txt"
    : > "$PASTA_TESTE/stderr.txt"

    (
        cd "$PASTA_TESTE" || exit 1
        ./scheduler "$@" >stdout.txt 2>stderr.txt
    )
    STATUS=$?

    if [ "$STATUS" -ne 0 ] &&
       [ ! -s "$PASTA_TESTE/stdout.txt" ] &&
       [ -s "$PASTA_TESTE/stderr.txt" ] &&
       [ ! -e "$PASTA_TESTE/rate_${LOGIN}.out" ] &&
       [ ! -e "$PASTA_TESTE/edf_${LOGIN}.out" ]; then
        passou "$DESCRICAO"
    else
        falhou "$DESCRICAO"
    fi
}

testar_erro "Sem argumentos"
testar_erro "Quantidade insuficiente de argumentos" rate
testar_erro "Quantidade excessiva de argumentos" rate voo.txt extra
testar_erro "Algoritmo invalido" fifo voo.txt
testar_erro "Arquivo inexistente" rate inexistente.txt
testar_erro "Campo faltando" rate campo_faltando.txt
testar_erro "Valor nao numerico" rate nao_numerico.txt
testar_erro "Valor nao positivo" rate nao_positivo.txt
testar_erro "Restricao C maior que D" rate c_maior_d.txt
testar_erro "Restricao D maior que P" rate d_maior_p.txt
testar_erro "Campo extra" rate campo_extra.txt
testar_erro "Tempo total invalido" rate tempo_invalido.txt
testar_erro "Arquivo sem tarefas" rate sem_tarefas.txt

rm -f "$PASTA_TESTE/rate_${LOGIN}.out"
mkdir "$PASTA_TESTE/rate_${LOGIN}.out"

(
    cd "$PASTA_TESTE" || exit 1
    ./scheduler rate voo.txt >stdout.txt 2>stderr.txt
)
STATUS=$?

if [ "$STATUS" -ne 0 ] &&
   [ ! -s "$PASTA_TESTE/stdout.txt" ] &&
   [ -s "$PASTA_TESTE/stderr.txt" ]; then
    passou "Falha ao criar arquivo de saida"
else
    falhou "Falha ao criar arquivo de saida"
fi

rmdir "$PASTA_TESTE/rate_${LOGIN}.out"

echo
echo "========================================"
echo "RESUMO"
echo "========================================"
echo "Total:     $TOTAL"
echo "Passaram:  $PASSARAM"
echo "Falharam:  $FALHARAM"
echo

if [ "$FALHARAM" -eq 0 ]; then
    echo "RESULTADO FINAL: TODOS OS TESTES PASSARAM"
    exit 0
else
    echo "RESULTADO FINAL: EXISTEM TESTES COM FALHA"
    exit 1
fi