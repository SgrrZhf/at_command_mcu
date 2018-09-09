
#include "stm32l1xx_hal.h"
#include "serial_bsp.h"
#include "main.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include <string.h>

xSemaphoreHandle SerialRxFinishSemphr;

uint8_t SerialBuffer[SERIAL_BUFFER_SIZE];
SerialDevice_t SerialDevice;

/**
  * @brief  初始化serial相关结构体,并开启uart dma接收
  * @param  omit
  * @retval none
  */
void serial_init(UART_HandleTypeDef *huart)
{
    SerialRxFinishSemphr = xSemaphoreCreateBinary();
    assert_param(SerialRxFinishSemphr != NULL);
    xSemaphoreTake(SerialRxFinishSemphr, 0);

    SerialDevice.UartHandle = huart;
    SerialDevice.PtrBuffer = SerialBuffer;
    SerialDevice.PutIndex = 0;
    SerialDevice.GetIndex = 0;
    SerialDevice.Cycles = 0;
    SerialDevice.BufferSize = SERIAL_BUFFER_SIZE;
    SerialDevice.InWaitings = 0;

    HAL_UART_Receive_DMA(huart, SerialDevice.PtrBuffer, SerialDevice.BufferSize);
    //__HAL_UART_ENABLE_IT(huart, UART_IT_IDLE);
}

/**
  * @brief  serial 反初始化
  * @param  none
  * @retval none
  */
void serial_deinit()
{
    vSemaphoreDelete(SerialRxFinishSemphr);
    HAL_UART_DMAStop(SerialDevice.UartHandle);

    SerialDevice.UartHandle = NULL;
    SerialDevice.PtrBuffer = NULL;
    SerialDevice.PutIndex = 0;
    SerialDevice.GetIndex = 0;
    SerialDevice.Cycles = 0;
    SerialDevice.BufferSize = 0;
    SerialDevice.InWaitings = 0;
}

/**
  * @brief  处理uart 接收 idle 中断
  * @param  omit
  * @retval none
  */
void serial_rxidle_handler(UART_HandleTypeDef *huart)
{
    uint16_t endIndex;
    portBASE_TYPE needYield = pdFALSE;

    /* Disable the UART IDLE interrupt and pause dma transfer */
    HAL_UART_DMAPause(huart);

    /* 处理环形缓冲区本次接收的起始位置与接收数量，并将putindex放在下一次接收的起始位置 */
    endIndex = SerialDevice.BufferSize - huart->hdmarx->Instance->CNDTR;
    if(endIndex >= SerialDevice.BufferSize)
        endIndex = 0;

    SerialDevice.GetIndex = SerialDevice.PutIndex;
    if(SerialDevice.Cycles == 0)
    {
        /* 首尾无交接 */
        SerialDevice.InWaitings = endIndex -  SerialDevice.PutIndex;
    }
    else
    {
        if(SerialDevice.PutIndex > endIndex && SerialDevice.Cycles == 1)
        {
            /* 首尾有交接，无溢出 */
            SerialDevice.InWaitings =
                SerialDevice.BufferSize - SerialDevice.PutIndex + endIndex;
        }
        else
        {
            /* 首尾有交接，且溢出 */
            SerialDevice.InWaitings = SerialDevice.BufferSize;
            SerialDevice.GetIndex = endIndex;
        }
    }
    SerialDevice.Cycles = 0;
    SerialDevice.PutIndex = endIndex;

    xSemaphoreGiveFromISR(SerialRxFinishSemphr, &needYield);
    if(needYield != pdFALSE)
    {
        taskYIELD();
    }

    HAL_UART_DMAResume(huart);

}

/**
  * @brief  dma完成一次循环接收处理
  * @param  none
  * @retval none
  */
void serial_rxcplt_handler()
{
    SerialDevice.Cycles++;
}

/**
  * @brief  串口写入数据
  * @param  data 待写入的数据存储位置
  *         length 待写入的数据长度（字节为单位）
  * @retval 0 成功
  *         1 失败
  */
int8_t serial_write(uint8_t *pData, uint16_t len)
{
    HAL_StatusTypeDef status;

    status = HAL_UART_Transmit_DMA(SerialDevice.UartHandle, pData, len);
    if(status != HAL_OK)
    {
        return 1;
    }
    return 0;
}

/**
  * @brief  串口读取数据
  * @param  dst,数据存储位置
  *         size 期望接收的数量（目前无用处）
  *         timeout 超时时间
  *         @retval 读取到的数据的长度
  */
uint16_t serial_read(uint8_t *dst, uint16_t size, uint32_t timeout)
{
    uint16_t firstCopyNum = 0;
    uint16_t readNum = 0;

    UNUSED(size);
    __HAL_UART_CLEAR_IDLEFLAG(SerialDevice.UartHandle);
    __HAL_UART_ENABLE_IT(SerialDevice.UartHandle, UART_IT_IDLE);
    if(xSemaphoreTake(SerialRxFinishSemphr, MS2TICK(timeout)) != pdPASS)
        goto serial_read_end;

    if(SerialDevice.InWaitings + SerialDevice.GetIndex > SerialDevice.BufferSize)
    {
        firstCopyNum = SerialDevice.BufferSize - SerialDevice.GetIndex;
        memcpy(dst, SerialDevice.PtrBuffer + SerialDevice.GetIndex, firstCopyNum);
        SerialDevice.InWaitings -= firstCopyNum;
        SerialDevice.GetIndex = 0;
        readNum += firstCopyNum;
    }
    memcpy(dst + firstCopyNum, SerialDevice.PtrBuffer + SerialDevice.GetIndex, SerialDevice.InWaitings);
    SerialDevice.GetIndex += SerialDevice.InWaitings;
    if(SerialDevice.GetIndex >= SerialDevice.BufferSize)
    {
        SerialDevice.GetIndex -= SerialDevice.BufferSize;
    }
    readNum += SerialDevice.InWaitings;
    SerialDevice.InWaitings = 0;

serial_read_end:
    __HAL_UART_DISABLE_IT(SerialDevice.UartHandle, UART_IT_IDLE);
    return readNum;
}
