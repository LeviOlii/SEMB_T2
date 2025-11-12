//grafo conexo.h

#ifndef GRAFO_CONEXO_H
#define GRAFO_CONEXO_H

#include "main.h"
#include <stdint.h>

#define VERTICES_MAX 89

int inicializarGrafoMock(void);
int verificarConectividade(UART_HandleTypeDef *huart, int dim);
void imprimirResultado(UART_HandleTypeDef *huart, int conexo);
void imprimirMensagem(UART_HandleTypeDef *huart, const char *fmt, ...);
void imprimirLinha(UART_HandleTypeDef *huart, int idx);
/*int parse_and_fill_matrix(UART_HandleTypeDef *huart, char *str);
void imprimirGrafo(UART_HandleTypeDef *huart, int n);*/

#endif
