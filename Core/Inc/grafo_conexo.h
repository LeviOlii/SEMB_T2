/**
  ******************************************************************************
  * @file    grafo_conexo.h
  * @brief   Header for graph connectivity verification (T2)
  * @authors Luís, Levi
  * @date    12/11/2025
  ******************************************************************************
  */

#ifndef GRAFO_CONEXO_H
#define GRAFO_CONEXO_H

#include "main.h"
#include <stdint.h>

/** Maximum number of vertices (fixed for T2) */
#define VERTICES_MAX 89

/**
 * @brief Runs a DFS-based connectivity test using streamed rows via UART.
 * @param huart UART handle (already initialized)
 * @param num_vertices Number of vertices
 * @return 1 if connected, 0 if not, -1 on UART error
 *
 * @note More detailed explanation can be found in the implementation (grafo_conexo.c).
 *
 */
int verificarConectividade(UART_HandleTypeDef *huart, int num_vertices);

/**
 * @brief Prints connectivity result.
 *
 * @note More detailed explanation can be found in the implementation (grafo_conexo.c).
 */
void imprimirResultado(UART_HandleTypeDef *huart, int conexo);

/**
 * @brief Prints formatted message
 * @note More detailed explanation can be found in the implementation (grafo_conexo.c).
 */
void imprimirMensagem(UART_HandleTypeDef *huart, const char *fmt, ...);

/**
 * @brief Prints current adjacency row (debug).
 * @note More detailed explanation can be found in the implementation (grafo_conexo.c).
 */
void imprimirLinha(UART_HandleTypeDef *huart, int idx);

#endif /* GRAFO_CONEXO_H */
