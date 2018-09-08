
#ifndef __SERIAL_BSP_H
#define __SERIAL_BSP_H

#include "stm32l1xx_hal.h"

#define SERIAL_BUFFER_SIZE 256

typedef struct{
    UART_HandleTypeDef *UartHandle;
    uint8_t *PtrBuffer; //缓冲区首地址
    uint16_t PutIndex;  //下一次接收时所处缓冲区的位置
    uint16_t GetIndex;  //读取数据的起始位置
    uint16_t BufferSize;//环形缓冲区大小
    uint16_t InWaitings;//等待读取的数据个数
    uint16_t Cycles;    //接收一帧时跨越缓冲器尾首次数
}SerialDevice_t;

void serial_rxcplt_handler();
void serial_rxidle_handler(UART_HandleTypeDef *huart);
void serial_init(UART_HandleTypeDef *huart);
int8_t serial_write(uint8_t *pData, uint16_t len);
uint16_t serial_read(uint8_t *dst, uint16_t size, uint32_t timeout);

#endif /* ifndef  */
