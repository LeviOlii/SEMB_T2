/**
  ******************************************************************************
  * @file    grafo_conexo.c
  * @brief   Graph connectivity module (DFS with UART row streaming)
  * @authors Levi Oliveira, Luís Fernando
  * @date    12/11/2025
  * @version 1.1
  *
  * @section PURPOSE Module Purpose
  * This module implements a memory-efficient iterative Depth-First Search (DFS)
  * to verify whether an undirected graph is connected. The graph is represented
  * as an adjacency matrix but **only one row is ever stored in RAM** at a time.
  *
  * The STM32F030 has only 8 KB of RAM; therefore the full 89×89 matrix cannot
  * be stored. This module requests each adjacency row on-demand from UART.
  *
  * @section DATA_STRUCTURES Internal Data Structures
  * The following global buffers are intentionally allocated here, since they
  * belong to the graph algorithm rather than to the main application:
  *
  *  - char linha[90]
  *      Stores one adjacency row (89 bytes + '\0' terminator).
  *
  *  - uint8_t visitado[89]
  *      Marks whether each vertex has been visited during DFS.
  *
  *  - uint8_t pilha[89]
  *      Stack used for the iterative DFS.
  *
  * Total RAM usage of the module: **~268 bytes**.
  *
  * @section ALGORITHM Algorithm Description
  * 1. Start DFS at vertex 0.
  * 2. While the stack is not empty:
  *      - Pop current vertex.
  *      - Request its adjacency row via UART.
  *      - For every neighbor marked '1':
  *            push it if not visited.
  * 3. After DFS, check whether all vertices were visited.
  *
  * Returns:
  *   1 → graph is connected
  *   0 → graph is not connected
  *  -1 → communication error (timeout or invalid byte)
  *
  * @section NOTE
  * This module does not deal with hardware initialization or application-level
  * flow; those responsibilities are handled in main.c.
  *
  ******************************************************************************
  */


#include "grafo_conexo.h"
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

/* Global variables (shared across functions) */
/** @brief Buffer to store the current adjacency row (89 bytes + '\0') */
char linha[VERTICES_MAX + 1];

/** @brief Array to mark visited vertices during DFS */
uint8_t visitado[VERTICES_MAX];

/** @brief Stack to store vertices to visit (iterative DFS) */
uint8_t pilha[VERTICES_MAX];

/**
 * @brief Verifies if the graph is connected using iterative DFS with UART streaming.
 *
 * The algorithm starts at vertex 0 and explores the graph by requesting adjacency
 * rows on demand via UART. Each row is processed to find unvisited neighbors.
 *
 * @param huart Pointer to UART handle (USART2)
 * @param num_vertices Number of vertices in the graph (89)
 * @return
 *   - 1 if graph is connected
 *   - 0 if not connected
 *   - -1 on communication error (timeout or invalid bit)
 *
 * @note Global variables affected: linha[], visitado[], pilha[]
 * @note Conforms to UM1785 (p. 568): HAL_UART_Receive() with timeout
 */
int verificarConectividade(UART_HandleTypeDef *huart, int num_vertices)
{
    int topo = -1;        // Stack top index (-1 = empty)
    int atual = 0;        // Current vertex being processed

    // Step 1: Initialize DFS from vertex 0
    pilha[++topo] = 0;    // Push vertex 0
    visitado[0] = 1;      // Mark as visited

    // Step 2: Main DFS loop (while stack is not empty)
    while (topo >= 0)
    {
        atual = pilha[topo--];  // Step 3: Pop current vertex

        // Step 4: Request adjacency row for current vertex
        imprimirMensagem(huart, "ENVIAR_LINHA:%d\n", atual);

        // Step 5: Receive 89 bytes (1 byte per byte)
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
                imprimirMensagem(huart, "Erro: byte invalido %c\r\n", byte);
                return -1;
            }
            linha[i] = byte;
        }
        linha[num_vertices] = '\0';  // Null-terminate string

        imprimirLinha(huart, atual);  // Debug: print received row

        // Step 6: Process neighbors and push unvisited ones
        for (int j = 0; j < num_vertices; j++)
        {
            if (linha[j] == '1' && !visitado[j])
            {
                visitado[j] = 1;      // Mark as visited
                pilha[++topo] = j;    // Push to stack
            }
        }
    }

    // Final check: all vertices visited?
    for (int i = 0; i < num_vertices; i++)
        if (!visitado[i]) return 0;

    return 1;  // Graph is connected
}

/**
 * @brief Prints a truncated version of the current adjacency row via UART.
 *
 * Shows first 50 bits + "..." if longer. Used for debugging.
 *
 * @param huart Pointer to UART handle
 * @param idx Index of the current vertex (for labeling)
 *
 * @note Uses global variable: linha[]
 */
void imprimirLinha(UART_HandleTypeDef *huart, int idx)
{
    char buffer[128];
    int len = snprintf(buffer, sizeof(buffer), "Linha %d: ", idx);

    // Print first 50 bits
    for (int i = 0; i < 50 && i < (int)strlen(linha); i++)
        len += snprintf(buffer + len, sizeof(buffer) - len, "%c", linha[i]);

    if (strlen(linha) > 50)
        len += snprintf(buffer + len, sizeof(buffer) - len, "...");

    snprintf(buffer + len, sizeof(buffer) - len, " (%d bits)\r\n", (int)strlen(linha));

    HAL_UART_Transmit(huart, (uint8_t*)buffer, strlen(buffer), HAL_MAX_DELAY);
}

/**
 * @brief Sends the final connectivity result via UART.
 *
 * @param huart Pointer to UART handle
 * @param conexo 1 if connected, 0 otherwise
 */
void imprimirResultado(UART_HandleTypeDef *huart, int conexo)
{
    const char *msg = conexo ?
        "Resultado: O grafo eh CONEXO.\r\n" :
        "Resultado: O grafo NAO eh conexo.\r\n";
    HAL_UART_Transmit(huart, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
}

/**
 * @brief Prints a formatted message via UART (printf-like).
 *
 * @param huart Pointer to UART handle
 * @param fmt Format string
 * @param ... Variable arguments
 */
void imprimirMensagem(UART_HandleTypeDef *huart, const char *fmt, ...)
{
    char buffer[128];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);
    HAL_UART_Transmit(huart, (uint8_t*)buffer, strlen(buffer), HAL_MAX_DELAY);
}
