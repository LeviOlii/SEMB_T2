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
 * @brief Verifies graph connectivity using DFS streaming.
 * @param huart UART handle
 * @param num_vertices Number of vertices
 * @return 1 (connected), 0 (not), -1 (error)
 */
int verificarConectividade(UART_HandleTypeDef *huart, int num_vertices);

/**
 * @brief Prints connectivity result.
 */
void imprimirResultado(UART_HandleTypeDef *huart, int conexo);

/**
 * @brief Prints formatted message.
 */
void imprimirMensagem(UART_HandleTypeDef *huart, const char *fmt, ...);

/**
 * @brief Prints current adjacency row (debug).
 */
void imprimirLinha(UART_HandleTypeDef *huart, int idx);

#endif /* GRAFO_CONEXO_H */
