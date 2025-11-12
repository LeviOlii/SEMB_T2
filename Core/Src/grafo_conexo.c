#include "grafo_conexo.h"
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

char linha[90];
char visitado[VERTICES_MAX];
char pilha[VERTICES_MAX];

int verificarConectividade(UART_HandleTypeDef *huart, int dim)
{
    uint8_t visitado[89] = {0};
    uint8_t pilha[89];
    int topo = -1;
    int atual = 0;

    // Passo 1: Inicia DFS
    pilha[++topo] = 0;
    visitado[0] = 1;

    // Passo 2: Loop principal do DFS (enquanto pilha não vazia)
    while (topo >= 0)
    {
        atual = pilha[topo--]; // 1. Desempilha vértice atual

        // Passo 3: Pede linha do `atual`
        imprimirMensagem(huart, "ENVIAR_LINHA:%d\n", atual);

        // Passo 4: Recebe a linha (89 bits)
        for (int i = 0; i < dim; i++)
        {
            uint8_t byte;
            if (HAL_UART_Receive(huart, &byte, 1, 5000) != HAL_OK)
            {
                imprimirMensagem(huart, "Erro: timeout linha %d\r\n", atual);
                return -1;
            }
            if (byte != '0' && byte != '1')
            {
                imprimirMensagem(huart, "Erro: bit invalido %c\r\n", byte);
                return -1;
            }
            linha[i] = byte;
        }
        linha[dim] = '\0';

        imprimirLinha(huart, atual);

        // Passo 5: Identifica vizinhos e empilha
        for (int j = 0; j < dim; j++)
        {
            if (linha[j] == '1' && !visitado[j])
            {
                visitado[j] = 1;
                pilha[++topo] = j;
            }
        }
    }

    // Verifica conectividade
    for (int i = 0; i < dim; i++)
        if (!visitado[i]) return 0;
    return 1;
}

void imprimirLinha(UART_HandleTypeDef *huart, int idx)
{
    char buffer[128];
    int len = snprintf(buffer, sizeof(buffer), "Linha %d: ", idx);

    // Imprime os primeiros 50 bits + "..." se for muito grande
    for (int i = 0; i < 50 && i < strlen(linha); i++)
        len += snprintf(buffer + len, sizeof(buffer) - len, "%c", linha[i]);

    if (strlen(linha) > 50)
        len += snprintf(buffer + len, sizeof(buffer) - len, "...");

    snprintf(buffer + len, sizeof(buffer) - len, " (%d bits)\r\n", (int)strlen(linha));

    HAL_UART_Transmit(huart, (uint8_t*)buffer, strlen(buffer), HAL_MAX_DELAY);
}

void imprimirResultado(UART_HandleTypeDef *huart, int conexo)
{
    const char *msg = conexo ?
        "Resultado: O grafo eh CONEXO.\r\n" :
        "Resultado: O grafo NAO eh conexo.\r\n";
    HAL_UART_Transmit(huart, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
}

void imprimirMensagem(UART_HandleTypeDef *huart, const char *fmt, ...)
{
    char buffer[128];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);
    HAL_UART_Transmit(huart, (uint8_t*)buffer, strlen(buffer), HAL_MAX_DELAY);
}

/*
void imprimirGrafo(UART_HandleTypeDef *huart, int dim)
{
    imprimirMensagem(huart, "Matriz recebida [%dx%d]:\r\n", dim, dim);
    for (int i = 0; i < dim; i++)
    {
        char linha[128] = {0};
        int len = 0;
        for (int j = 0; j < dim; j++)
            len += snprintf(linha + len, sizeof(linha) - len, "%c ", grafo[i][j]);
        linha[len - 1] = '\r';
        linha[len] = '\n';
        HAL_UART_Transmit(huart, (uint8_t*)linha, len + 1, HAL_MAX_DELAY);
    }
    imprimirMensagem(huart, "Fim da matriz.\r\n");
}

int parse_and_fill_matrix(UART_HandleTypeDef *huart, char *str)
{
    char *p = str;
    int bits = 0;
    int dim = 0;

    imprimirMensagem(huart, "DEBUG: rx_data+1 = [%s]\r\n", str);

    // === 1. CONTAR TOTAL DE BITS (0s e 1s) ===
    for (char *temp = str; *temp != '\0'; temp++)
    {
        if (*temp == '0' || *temp == '1')
            bits++;
    }

    // === 2. CALCULAR DIMENSÃO: √bits ===
    for (dim = 1; dim <= VERTICES_MAX; dim++)
    {
        if (dim * dim == bits)
            break;
    }

    if (dim > VERTICES_MAX || dim * dim != bits)
    {
        imprimirMensagem(huart, "Erro: matriz nao quadrada (bits=%d)\r\n", bits);
        return 0;
    }

    // === 3. PREENCHER A MATRIZ ===
    p = str;
    int row = 0, col = 0;

    while (*p != '\0')
    {
        if (*p == '0' || *p == '1')
        {
            grafo[row][col++] = *p;

            if (col == dim)
            {
                col = 0;
                row++;
            }
        }
        p++;
    }

    return dim;  // ← RETORNA A DIMENSÃO (5)
}
*/
