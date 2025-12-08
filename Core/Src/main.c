/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Application entry point for T2 – Graph Connectivity Check
  * @authors        : Levi Oliveira, Luís Fernando
  * @date           : 20/11/2025
  * @version        : 1.1
  *
  * @section OVERVIEW Application Overview
  * This firmware runs on the STM32F030 (Nucleo-F030R8) and determines whether
  * an undirected graph is connected. The graph has up to 89 vertices and its
  * adjacency matrix is streamed on-demand via UART from a host computer.
  *
  * The embedded system:
  *  - Initializes hardware peripherals (clock, GPIO, USART2);
  *  - Requests the number of vertices via UART;
  *  - Requests each adjacency row dynamically;
  *  - Delegates the connectivity computation to the graph module
  *    (grafo_conexo.c);
  *  - Prints the final result on the serial terminal.
  *
  * @section CONTEXT Academic Context
  * This project corresponds to T2 from the Embedded Systems course.
  * The goal is to port an algorithm validated on PC (T1) to the STM32F030,
  * respecting strict memory constraints (8 KB RAM).
  *
  * T1 → Algorithm design and validation (unrestricted resources).
  * T2 → Embedded implementation under hardware limitations.
  *
  * Teacher: Elias Teodoro da Silva Júnior
  * Institute: Instituto Federal do Ceará - Campus Fortaleza
  *
  * @section USAGE How to Use
  * 1. Flash this firmware to a Nucleo-F030R8 board.
  * 2. Run the host script: `python generate_and_test.py`.
  * 3. Open a serial terminal (38400 baud).
  * 4. Observe the MCU requesting vertex count and adjacency rows automatically.
  * 5. The final result is printed: connected / not connected.
  *
  * @section IO_SPEC Input & Output (High-Level)
  * Input  → adjacency matrix streamed line-by-line via UART.
  * Output → textual indication of graph connectivity.
  *
  * @section RESPONSIBILITY Module Separation
  * - main.c handles:
  *     • hardware initialization
  *     • UART communication setup
  *     • high-level application flow
  *     • interaction with the host PC
  *
  * - grafo_conexo.c handles:
  *     • DFS-based connectivity algorithm
  *     • data structures used for DFS
  *     • RAM-efficient streaming of adjacency rows
  *
  * @section COPYRIGHT Copyright & License
  * Copyright (c) 2025 Levi Oliveira, Luís Fernando. All rights reserved.
  * Academic use and reproduction allowed with proper attribution.
  *
  * @section PLATFORM Target Platform
  * MCU            : STM32F030R8T6 (Cortex-M0, 48 MHz, 8 KB RAM, 64 KB Flash)
  * Toolchain      : STM32CubeIDE + HAL Library
  * Reference      : UM1785 – STM32F0 HAL User Manual (p. 7 and p. 568)
  *
  ******************************************************************************
  */

#include "main.h"
#include "grafo_conexo.h"
#include <string.h>
#include <stdio.h>

/* Private variables */
UART_HandleTypeDef huart2;

/* Function prototypes */
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);


/**
  * @brief  Application entry point
  * @retval int (never returns)
  *
  * Main execution flow:
  * 1. Initializes HAL, system clock, GPIO (unused) and USART2
  * 2. Sends startup banner and requests number of vertices of graph
  * 3. Receives "89" in ASCII format and converts to integer
  * 4. Executes streaming DFS connectivity test
  * 5. Prints final result
  * 6. Waits for next test (useful for automated validation)
  */
int main(void)
{

	/* MCU Configuration--------------------------------------------------------*/

	/* Reset of all peripherals, Initializes the Flash interface and the Systick. */
    HAL_Init();

    /* Configure the system clock */
    SystemClock_Config();

    /* Initialize all configured peripherals */
    MX_GPIO_Init();
    MX_USART2_UART_Init();

    /* Application startup -----------------------------------------------------*/

    /* Send welcome banner */
    imprimirMensagem(&huart2, "\r\n=== T2 - Grafos (STREAMING 89x89) ===\r\n");

    /* Request graph dimension from PC (Python script) */
    imprimirMensagem(&huart2, "ENVIAR_NUM_VERTICES:\n");

    /* === RECEIVE GRAPH DIMENSION (ASCII → integer) =========================*/

	/**
	 * @brief Buffer to receive the three ASCII digits that form the number of vertices
	 * Example: '8','9','\n' → "89"
	 */
    char vertices_str[4] = {0};

    /**
	 * Receive exactly 3 bytes (we only accept digits 0–9).
	 * This is a simple and robust way to get the dimension without needing a full parser.
	 */
    for (int i = 0; i < 3; i++)
    {
        uint8_t byte;
        /* Blocking receive – waits indefinitely (HAL_MAX_DELAY) */
        HAL_UART_Receive(&huart2, &byte, 1, HAL_MAX_DELAY);

        /* Filter only valid decimal digits */
        if (byte >= '0' && byte <= '9') vertices_str[i] = byte;
        /* Non-digit characters are ignored – tolerant to \r, \n, etc. */
    }

    /* Convert ASCII string to integer */
    int num_vertices = atoi(vertices_str);

    /* Input validation – safety check */
    if (num_vertices <= 0 || num_vertices > 89)
    {
        imprimirMensagem(&huart2, "Erro: número de vértices invalido %d\r\n", num_vertices);
        while(1); /* Halt on error */
    }

    imprimirMensagem(&huart2, "Grafo %dx%d (streaming)\r\n", num_vertices, num_vertices);

    /* === RUN DFS WITH UART STREAMING ========================================*/

	/**
	 * @brief Execute the connectivity test.
	 * The function requests adjacency rows on demand and uses only ~268 bytes RAM.
	 * Returns 1 (connected), 0 (not connected) or -1 (communication error).
	 */
    int conexo = verificarConectividade(&huart2, num_vertices);

    /* Print final result */
    imprimirResultado(&huart2, conexo);

    /* Ready for next graph (useful for automated testing) */
    imprimirMensagem(&huart2, "Aguardando proxima matriz...\r\n");

    /* Infinite loop – application never exits */
    while(1) HAL_Delay(100);
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL12;
  RCC_OscInitStruct.PLL.PREDIV = RCC_PREDIV_DIV1;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 38400;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  huart2.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart2.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief  GPIO Initialization (LED and User Button – not used in application)
  * @retval None
  *
  * GPIO pins are configured by CubeMX but never used in the T2 logic.
  * Kept for compatibility with standard Nucleo template.
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();

  HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);

  /* User button (B1) – configured but never read */
  GPIO_InitStruct.Pin = B1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

  /* On-board LED (LD2) – configured but never turned on */
  GPIO_InitStruct.Pin = LD2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LD2_GPIO_Port, &GPIO_InitStruct);
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  __disable_irq();
  while (1)
  {
  }
}

#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  */
void assert_failed(uint8_t *file, uint32_t line)
{
    /* User can add implementation to report the file name and line number */
}
#endif /* USE_FULL_ASSERT */
