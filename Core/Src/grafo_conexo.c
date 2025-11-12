#include "grafo_conexo.h"
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

char linha[VERTICES_MAX + 1]; //o último seria o \0
char visitado[VERTICES_MAX];
char pilha[VERTICES_MAX];

int verificarConectividade(UART_HandleTypeDef *huart, int num_vertices)
{
    uint8_t visitado[VERTICES_MAX] = {0};
    uint8_t pilha[VERTICES_MAX];
    int topo = -1;
    int atual = 0;

    // Passo 1: Inicia DFS
    pilha[++topo] = 0;
    visitado[0] = 1;

    // Passo 2: Loop principal do DFS (enquanto pilha não vazia)
    while (topo >= 0)
    {
        atual = pilha[topo--]; // Passo 3: Desempilha vértice atual

        // Passo 4: Pede linha do `atual`
        imprimirMensagem(huart, "ENVIAR_LINHA:%d\n", atual);

        // Passo 5: Recebe a linha (89 bytes)
        for (int i = 0; i < num_vertices; i++)
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
        linha[num_vertices] = '\0';

        imprimirLinha(huart, atual);

        // Passo 6: Identifica vizinhos e empilha
        for (int j = 0; j < num_vertices; j++)
        {
            if (linha[j] == '1' && !visitado[j])
            {
                visitado[j] = 1;
                pilha[++topo] = j;
            }
        }
    }

    // Verifica conectividade
    for (int i = 0; i < num_vertices; i++)
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
