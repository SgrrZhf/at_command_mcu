/**
  ******************************************************************************
  * @file    _write.c
  * @author  Zhang Huifeng
  * @version V1.0.0
  * @date    2017-04-23
  * @brief   重定向printf的底层输出
  *          在gcc-arm下编译有效，其它平台未测试过
  ******************************************************************************
  *
  *
  ******************************************************************************
  */

#include "unistd.h"
#include "stm32l1xx_hal.h"

extern UART_HandleTypeDef huart1;

/**
  * @brief  重定向printf底层输出函数
  * @param  无需关系
  * @retval 待发送的数据字节数
  */
int _write(int fd, char* ptr, int len)
{
    if (fd == STDOUT_FILENO || fd == STDERR_FILENO)
    {
        HAL_UART_Transmit(&huart1, (uint8_t *)ptr, len, len * 2);
    }
    return len;
}

