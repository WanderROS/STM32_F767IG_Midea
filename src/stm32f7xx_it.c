/**
 ******************************************************************************
 * @file    Templates/stm32f7xx_it.c
 * @author  MCD Application Team
 * @version V1.0.2
 * @date    18-November-2015
 * @brief   Main Interrupt Service Routines.
 *          This file provides template for all exceptions handler and
 *          peripherals interrupt service routine.
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; COPYRIGHT(c) 2015 STMicroelectronics</center></h2>
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *   1. Redistributions of source code must retain the above copyright notice,
 *      this list of conditions and the following disclaimer.
 *   2. Redistributions in binary form must reproduce the above copyright notice,
 *      this list of conditions and the following disclaimer in the documentation
 *      and/or other materials provided with the distribution.
 *   3. Neither the name of STMicroelectronics nor the names of its contributors
 *      may be used to endorse or promote products derived from this software
 *      without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "stm32f7xx_it.h"
#include "main.h"
#include "bsp/bsp_debug_usart.h"
#include "bsp/bsp_device_usart.h"
#include "bsp/bsp_wifi_usart.h"
#include <stdlib.h>
/** @addtogroup STM32F7xx_HAL_Examples
 * @{
 */

/** @addtogroup Templates
 * @{
 */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/******************************************************************************/
/*            Cortex-M7 Processor Exceptions Handlers                         */
/******************************************************************************/

/**
 * @brief   This function handles NMI exception.
 * @param  None
 * @retval None
 */
void NMI_Handler(void)
{
}

/**
 * @brief  This function handles Hard Fault exception.
 * @param  None
 * @retval None
 */
void HardFault_Handler(void)
{
  /* Go to infinite loop when Hard Fault exception occurs */
  while (1)
  {
  }
}

/**
 * @brief  This function handles Memory Manage exception.
 * @param  None
 * @retval None
 */
void MemManage_Handler(void)
{
  /* Go to infinite loop when Memory Manage exception occurs */
  while (1)
  {
  }
}

/**
 * @brief  This function handles Bus Fault exception.
 * @param  None
 * @retval None
 */
void BusFault_Handler(void)
{
  /* Go to infinite loop when Bus Fault exception occurs */
  while (1)
  {
  }
}

/**
 * @brief  This function handles Usage Fault exception.
 * @param  None
 * @retval None
 */
void UsageFault_Handler(void)
{
  /* Go to infinite loop when Usage Fault exception occurs */
  while (1)
  {
  }
}

/**
 * @brief  This function handles SVCall exception.
 * @param  None
 * @retval None
 */
void SVC_Handler(void)
{
}

/**
 * @brief  This function handles Debug Monitor exception.
 * @param  None
 * @retval None
 */
void DebugMon_Handler(void)
{
}

/**
 * @brief  This function handles PendSVC exception.
 * @param  None
 * @retval None
 */
void PendSV_Handler(void)
{
}

/**
 * @brief  This function handles SysTick Handler.
 * @param  None
 * @retval None
 */
void SysTick_Handler(void)
{
  HAL_IncTick();
}

/******************************************************************************/
/*                 STM32F7xx Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32f7xx.s).                                               */
/******************************************************************************/

extern uint8_t ucUartRecvTemp;
// 接收字符串最大缓冲区
#define DEBUG_RECV_BUFFER_SIZE 1024
uint8_t ucDebugRecvBuffer[DEBUG_RECV_BUFFER_SIZE];
uint16_t ulDebugRecvCount = 0;
uint16_t ulDebugRecvSize;
uint8_t ucDebugRecvReady = FALSE;
void DEBUG_USART_IRQHandler(void)
{
  if (__HAL_UART_GET_IT(&UartHandle, UART_IT_RXNE) != RESET)
  {
    HAL_UART_Receive(&UartHandle, (uint8_t *)&ucUartRecvTemp, 1, 1000);
    // 接收数据超过缓冲区大小
    if (ulDebugRecvCount >= DEBUG_RECV_BUFFER_SIZE)
    {
      ulDebugRecvCount = 0;
      ucDebugRecvReady = FALSE;
      printf("串口接收一行内容超过缓冲区大小,放弃处理.\n");
    }
    else
    {
      ucDebugRecvBuffer[ulDebugRecvCount] = ucUartRecvTemp;
      // 回车
      if (ucDebugRecvBuffer[ulDebugRecvCount] == '\n')
      {
        // 当前串口接收字符串内容长度
        ulDebugRecvSize = ulDebugRecvCount;
        ucDebugRecvBuffer[ulDebugRecvSize + 1] = '\0';
        ulDebugRecvCount = 0;
        ucDebugRecvReady = TRUE;
      }
      else
      {
        ulDebugRecvCount++;
      }
    }
  }

  HAL_UART_IRQHandler(&UartHandle);
}

/**
 * CRC 校验
 */
unsigned char orderCheckSum(char *buffer, int len)
{
  unsigned char checksum = 0;
  for (int i = 0; i < len - 2; i++)
  {
    checksum += buffer[1 + i];
  }
  checksum = ~checksum + 1;
  return checksum;
}
enum Recv_Status
{
  START = 0,
  RECVING,
  END
} device_status,
    wifi_status;

uint8_t ucTemp;
// 串口设备接收缓冲区
char *device_recv_buffer;
// 设备帧接收长度
unsigned char device_recv_length;
unsigned char device_recv_cur_index;
// 接收字符串最大缓冲区
#define DEVICE_RECV_BUFFER_SIZE 256
uint8_t ucDeviceRecvBuffer[DEVICE_RECV_BUFFER_SIZE];
uint16_t ulDeviceRecvSize;
uint8_t ucDeviceRecvReady = FALSE;
void DEVICE_USART_IRQHandler(void)
{
  if (__HAL_UART_GET_IT(&UartDeviceHandle, UART_IT_RXNE) != RESET)
  {
    HAL_UART_Receive(&UartDeviceHandle, (uint8_t *)&ucTemp, 1, 1000);
    if (device_status == RECVING)
    {
      device_recv_cur_index++;
      device_recv_buffer[device_recv_cur_index] = ucTemp;
      if (device_recv_cur_index == device_recv_length - 1)
      {
        device_status = END;
      }
      if (device_status == END)
      {
        unsigned char checkSum = orderCheckSum(device_recv_buffer, device_recv_length);
        if (checkSum == device_recv_buffer[device_recv_length - 1])
        {
          for (int i = 0; i < device_recv_length; ++i)
          {
            ucDeviceRecvBuffer[i] = device_recv_buffer[i];
          }
          ulDeviceRecvSize = device_recv_length;
          ucDeviceRecvReady = TRUE;
        }
        free(device_recv_buffer);
        device_recv_buffer = NULL;
        device_recv_cur_index = 0;
        device_recv_length = 0;
      }
    }
    // 收到了0xAA,并且接收数组为空，说明是帧长度字节
    if (device_status == START)
    {
      device_recv_buffer = (char *)malloc(ucTemp + 1);
      device_status = RECVING;
      device_recv_length = ucTemp + 1;
      device_recv_buffer[0] = 0xAA;
      device_recv_buffer[1] = ucTemp;
      device_recv_cur_index = 1;
    }
    // 收到了 MSmart 帧头;
    if (0xAA == ucTemp)
    {
      device_status = START;
    }
  }
  HAL_UART_IRQHandler(&UartDeviceHandle);
}

uint8_t ucWiFiTemp;
// 串口wifi接收缓冲区
char *wifi_recv_buffer;
// WIFI帧接收长度
unsigned char wifi_recv_length;
unsigned char wifi_recv_cur_index;
// 接收字符串最大缓冲区
#define WIFI_RECV_BUFFER_SIZE 256
uint8_t ucWifiRecvBuffer[WIFI_RECV_BUFFER_SIZE];
uint16_t ulWifiRecvSize;
uint8_t ucWifiRecvReady = FALSE;
void UartWiFi_USART_IRQHandler(void)
{
  if (__HAL_UART_GET_IT(&UartWiFiHandle, UART_IT_RXNE) != RESET)
  {
    HAL_UART_Receive(&UartWiFiHandle, (uint8_t *)&ucWiFiTemp, 1, 1000);
    if (wifi_status == RECVING)
    {
      wifi_recv_cur_index++;
      wifi_recv_buffer[wifi_recv_cur_index] = ucWiFiTemp;
      if (wifi_recv_cur_index == wifi_recv_length - 1)
      {
        wifi_status = END;
      }
      if (wifi_status == END)
      {
        unsigned char checkSum = orderCheckSum(wifi_recv_buffer, wifi_recv_length);
        if (checkSum == wifi_recv_buffer[wifi_recv_length - 1])
        {
          for (int i = 0; i < wifi_recv_length; ++i)
          {
            ucWifiRecvBuffer[i] = wifi_recv_buffer[i];
          }
          ulWifiRecvSize = wifi_recv_length;
          ucWifiRecvReady = TRUE;
        }
        free(wifi_recv_buffer);
        wifi_recv_buffer = NULL;
        wifi_recv_cur_index = 0;
        wifi_recv_length = 0;
      }
    }
    // 收到了0xAA,并且接收数组为空，说明是帧长度字节
    if (wifi_status == START)
    {
      wifi_recv_buffer = (char *)malloc(ucWiFiTemp + 1);
      wifi_status = RECVING;
      wifi_recv_length = ucWiFiTemp + 1;
      wifi_recv_buffer[0] = 0xAA;
      wifi_recv_buffer[1] = ucWiFiTemp;
      wifi_recv_cur_index = 1;
    }
    // 收到了 MSmart 帧头;
    if (0xAA == ucWiFiTemp)
    {
      wifi_status = START;
    }
  }
  HAL_UART_IRQHandler(&UartWiFiHandle);
}
/**
 * @brief  This function handles PPP interrupt request.
 * @param  None
 * @retval None
 */
/*void PPP_IRQHandler(void)
{
}*/

/**
 * @}
 */

/**
 * @}
 */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
