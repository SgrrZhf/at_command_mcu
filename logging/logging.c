
#include "stm32l1xx_hal.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "logging.h"
#include <stdio.h>
#include <stdarg.h>

// Maximum time to wait for the mutex in a logging statement.
#define MAX_MUTEX_WAIT_MS 20
#define MAX_MUTEX_WAIT_TICKS ((MAX_MUTEX_WAIT_MS + portTICK_PERIOD_MS - 1) / portTICK_PERIOD_MS)

SemaphoreHandle_t PrintMutex;

void logging_write(const char *format, ...)
{
    if(PrintMutex == NULL)
    {
        PrintMutex = xSemaphoreCreateMutex();
        assert_param(PrintMutex != NULL);
    }

    if(xSemaphoreTake(PrintMutex, MAX_MUTEX_WAIT_TICKS) != pdPASS)
        return;
    va_list list;
    va_start(list, format);
    vprintf(format, list);
    va_end(list);
    xSemaphoreGive(PrintMutex);
}
