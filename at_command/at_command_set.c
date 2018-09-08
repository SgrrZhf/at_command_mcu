
#include <string.h>
#include "stm32l1xx_hal.h"
#include "at_command_set.h"
#include "serial_bsp.h"
#include "re.h"
#include "logging.h"
#include <stdarg.h>


uint8_t Buffer[BUFFER_SIZE];
char Payload[PAYLOAD_SIZE];
extern UART_HandleTypeDef huart3;

CommandTypedef AtCommand[] =
{
    /* name|query transmit format|query receive format|set tranmmit format|set receive format */
    {"AT",      "",         "",                             NULL,                       NULL    },
    {"CGMI",    "",         "%s",                           NULL,                       NULL    },
    {"CGMM",    "",         "%s",                           NULL,                       NULL    },
    {"CGSN",    "=1",       "+CGSN:%s",                     NULL,                       NULL    },
    {"CFUN",    "?",        "+CFUN:%d",                     "=%d",                      ""      },
    {"CSQ",     "",         "+CSQ:%d,%*d",                  NULL,                       NULL    },
    {"NBAND",   "?",        "+NBAND:%d",                    "=%d",                      ""      },
    {"CGACT",   "?",        "+CGACT:%*d,%d",                "=%d,%d",                   ""      },
    {"CGATT",   "?",        "+CGATT:%d",                    NULL,                       NULL    },
    {"CEREG",   "?",        "+CEREG:%*d,%d",                "=%d",                      ""      },
    {"CSCON",   "?",        "+CSCON:%*d,%d",                "=%d",                      ""      },
    {"CGPADDR", "",         "+CGPADDR:%*d,%d.%d.%d.%d",      NULL,                       NULL   },
    {"NCONFIG", "?",        "+NCONFIG:%*11s,%s",            "=%s,%s",                   ""      },
    {"CPSMS", "?", "+CPSMS:%d", "=%d", NULL},
    {"NMGS", NULL, NULL, "=%u,%s", ""},
    {"NNMI", "?", "+NNMI:%u", "=%u", ""},
    {"NCDP", "?", "+NCDP:%u.%u.%u.%u,%u", "=%u.%u.%u.%u,%u", ""},
    {"NMSTATUS", "?", "+NMSTATUS:%*s", NULL, NULL},
    {"NNMI", NULL, "+NNMI:%u,%s", NULL, NULL},
    {"CCLK", "?", "+CCLK:%u/%u/%u,%u:%u:%u+%u", NULL, NULL},
    {"NCCID", "?", "+NCCID:%s", NULL, NULL},
    {"S", "", NULL, "", NULL},
};

/* AT_OK的正则表达式对照组 */
static const char *atOkPattern = "\r\nOK\r\n";
/* AT_ERROR的正则表达式对照组 */
static const char *atErrorPattern = "\r\nERROR\r\n";


/**
  * @brief  at测试指令，用于检测设备是否在线
  * @param  none
  * @retval skip
  */
#include "FreeRTOS.h"
#include "task.h"
AT_StatusTypedef at_test()
{
    uint16_t len;
    char Payload[] = "AT\r\n";

    memset(Buffer, 0, BUFFER_SIZE);
    /* 简单的发送AT,在接收OK,没什么好说的 */
    serial_write((uint8_t *)Payload, strlen(Payload));
    len = serial_read(Buffer, BUFFER_SIZE , 1000);
    if ( len == 0)
        return AT_NO_RESPONSE;

    if(re_match(atOkPattern, (char *)Buffer) != -1)
        return AT_OK;

    if(re_match(atErrorPattern, (char *)Buffer) != -1)
        return AT_ERROR;

    return AT_FORMAT_WRONG;
}

/**
  * @brief  AT 查询指令或执行指令
    @param  command 指令名
            ... 额外的参数(与query receive format 匹配)
  * @retval skip
  */
AT_StatusTypedef at_read_command(AtCommandListTypedef command, ...)
{
    uint16_t len;
    size_t index;

    /* 从AT指令表中取出对应指令应该具有的发送格式，然后加上"AT+"前缀 */
    len = sprintf(Payload, "AT+%s%s\r\n",
                  AtCommand[command].Name, AtCommand[command].pQueryTransmitFormat);
    __Logging_D("write", "%s", Payload);
    serial_write((uint8_t *)Payload, len);
    /* 发送完成之后进入接收模式 */
    len = serial_read(Buffer, BUFFER_SIZE, 1000);
    Buffer[len] = '\0';
    __Logging_D("read", "%s", Buffer);

    /* 没有收到任何数据，返回 未响应 */
    /* 此处应该是指令格式有误，AT解释器不处理该指令，因检查指令拼写*/
    if(len == 0)
        return AT_NO_RESPONSE;

    /* 匹配到返回数据中有OK,认为指令执行成功 */
    if(re_match(atOkPattern, (char *)Buffer) != -1)
    {
        /* 将Buffer索引起点设置在2的位置，避开开头的\r\n转义字符 */
        index = 2;
        va_list list;
        va_start(list, command);
        vsscanf((char *)(Buffer + index), AtCommand[command].pQueryReceiveFormat, list);
        va_end(list);
        return AT_OK;
    }

    /* 匹配到返回数据中有ERROR */
    /* 此处正常情况下不会发生，若发生，应该是发送的指令格式有误，应该检查指令表 */
    if(re_match(atErrorPattern, (char *)Buffer) != -1)
        return AT_ERROR;

    /* 对于解析不了的返回格式错误，意味着传输过程中出现了意外 */
    /* 正常情况下此处也不该发生 */
    return AT_FORMAT_WRONG;
}

/**
  * @brief  AT 设置指令
  * @param  command，指令名
  *         ..., 额外的参数(与set tranmmit format匹配)
  * @retval skip
  */
AT_StatusTypedef at_set_command(AtCommandListTypedef command, ...)
{
    size_t len = 0;

    len += sprintf(Payload, "AT+%s", AtCommand[command].Name);
    va_list list;
    va_start(list, command);
    len += vsprintf(Payload + len, AtCommand[command].pSetTransmitFormat, list);
    va_end(list);
    len += sprintf(Payload + len, "\r\n");
    Payload[len] = '\0';
    __Logging_D("write","%s",Payload);
    serial_write((uint8_t *)Payload, len);
    len = serial_read(Buffer, BUFFER_SIZE, 5000);
    Buffer[len] = '\0';
    __Logging_D("read", "%s", Buffer);

    /* 这里的返回情况和At_ReadCommand一样 */
    if(len == 0)
        return AT_NO_RESPONSE;

    if(re_match(atOkPattern, (char *)Buffer) != -1)
    {
        return AT_OK;
    }
    if(re_match(atErrorPattern, (char *)Buffer) != -1)
        return AT_ERROR;

    return AT_FORMAT_WRONG;
}

/**
  * @brief  at 等待响应指令
  * @param  command 指令名
  *         timeout 设置超时时间
  * @retval -
  */
AT_StatusTypedef at_wait_command(AtCommandListTypedef command, uint32_t timeout, ...)
{
    uint16_t len;
    uint8_t index;

    len = serial_read(Buffer, BUFFER_SIZE, timeout);
    Buffer[len] = '\0';
    __Logging_D("read","%s",Buffer);

    if(len == 0)
        return AT_NO_RESPONSE;
    if(re_match(AtCommand[command].Name, (char *)Buffer) != -1)
    {
        index = 2;
        va_list list;
        va_start(list, timeout);
        vsscanf((char *)(Buffer + index), AtCommand[command].pQueryReceiveFormat, list);
        va_end(list);
        return AT_OK;
    }
    if(re_match(atErrorPattern, (char *)Buffer) != -1)
        return AT_ERROR;

    return AT_FORMAT_WRONG;
}
