
#ifndef __AT_COMMAND_IO_H
#define __AT_COMMAND_IO_H

#include "stm32l1xx_hal.h"

#define BUFFER_SIZE 256
#define PAYLOAD_SIZE 64

//AT指令列表,必须要和atCommand一一对应
typedef enum
{
    AT = 0,
    AT_CGMI,
    AT_CGMM,
    AT_CGSN,
    AT_CFUN,
    AT_CSQ,
    AT_NBAND,
    AT_CGACT,
    AT_CGATT,
    AT_CEREG,
    AT_CSCON,
    AT_CGPADDR,
    AT_NCONFIG,
    AT_CPSMS,
    AT_NMGS,
    AT_NNMI,
    AT_NCDP,
    AT_NMSTATUS,
    AT_NNMI_RESP,
    AT_CCLK,
    AT_NCCID,
    AT_S,
} AtCommandListTypedef;

/* At指令解析与打包的格式 */
typedef struct
{
    /* 为NULL表示该指令不具有相应的数据 */
    const char *Name;//指令名
    const char *pQueryTransmitFormat;//查询模式下应该具有的格式
    const char *pQueryReceiveFormat;//查询模式下接收的数据格式
    const char *pSetTransmitFormat;//设置模式下应该具有的发送格式
    const char *pSetReceiveFormat;//设置模式下接收的数据格式
} CommandTypedef;

/* AT命令执行结果 */
typedef enum
{
    AT_OK = 0,
    AT_ERROR,
    AT_NO_RESPONSE,
    AT_FORMAT_WRONG,
} AT_StatusTypedef;

AT_StatusTypedef at_test();
AT_StatusTypedef at_read_command(AtCommandListTypedef command, ...);
AT_StatusTypedef at_set_command(AtCommandListTypedef command, ...);
AT_StatusTypedef at_wait_command(AtCommandListTypedef command, uint32_t timeout,...);

#endif /* ifndef __AT_COMMAND_IO_H */
