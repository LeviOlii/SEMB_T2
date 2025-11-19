/**
  ******************************************************************************
  * @file    grafo_conexo.c
  * @brief   T2 - Graph Connectivity Verification using DFS (Streaming UART)
  * @authors Luís, Levi
  * @date    12/11/2025
  *
  * @section DESCRIPTION
  * This application implements a **Depth-First Search (DFS)** algorithm to verify
  * if an undirected graph is connected. The graph is represented as an adjacency
  * matrix (89x89) and is **streamed line-by-line via UART** to fit in the STM32F030
  * limited RAM (8 KB).
  *
  * The algorithm uses **iterative DFS with stack** and requires **only 360 bytes**
  * of RAM: `linha[90]`, `visitado[89]`, `pilha[89]`.
  *
  * @section INPUT_OUTPUT
  * **Input (UART):**
  *  - `"89\n"` → number of vertices of the graph
  *  - `"101010..."` → 89-byte adjacency row (on demand)
  *
  * **Output (UART):**
  *  - `"Resultado: O grafo eh CONEXO.\r\n"` or `"NAO eh conexo.\r\n"`
  *
  * @section PLATFORM
  * - **Target:** STM32F030 (Nucleo-F030R8)
  * - **HAL:** STM32CubeF0 HAL (UM1785, p. 568)
  * - **Baudrate:** 38400, 8N1
  *
  * @section COPYRIGHT
  * Copyright (c) 2025 Luís & Levi. All rights reserved.
  * This code is for academic use only. Redistribution and commercial use prohibited.
  *
  * @section HOW_TO_USE
  * 1. Flash this code to STM32F030
  * 2. Connect UART2 (PA2/TX, PA3/RX) to PC
  * 3. Run Python script: `python generate_and_test.py`
  * 4. Observe results in Terminal
  *
  * @section DOXYGEN
  * This file is Doxygen-ready. Run:
  *   doxygen Doxyfile
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
 * @note Uses global: linha[]
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
