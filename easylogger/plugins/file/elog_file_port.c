/*
 * This file is part of the EasyLogger Library.
 *
 * Copyright (c) 2015-2019, Qintl, <qintl_linux@163.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * 'Software'), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED 'AS IS', WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * Function:  Portable interface for EasyLogger's file log pulgin.
 * Created on: 2019-01-05
 */
#include <stdio.h>
#include <string.h>
#include "elog_file.h"
#ifdef FREERTOS
    #include <ProjectConfig.h>
    #include <FreeRTOS.h>
    #include <semphr.h>
    #include <sys_littlefs.h>
    #include <SEGGER_RTT.h>
    #include <debug.h>
#elif defined QL_EC600U
    #include "ql_api_osi.h"
    #include <ql_fs.h>
    #include <hal_common.h>
#else
    #error "easy logger need define platform first"
#endif

#ifdef FREERTOS
    static xSemaphoreHandle xSemaphore_elogfile = NULL;
#elif defined QL_EC600U
    static ql_mutex_t s_mutexLock;
#endif
ElogErrCode elog_file_port_init_lock(void)
{
    ElogErrCode result = ELOG_NO_ERR;
#ifdef FREERTOS
    xSemaphore_elogfile = xSemaphoreCreateMutex();

    if (xSemaphore_elogfile == NULL)
    {
        //err("xSemaphoreCreateMutex error\n");
        SEGGER_RTT_printf(0, "xSemaphoreCreateMutex error\n");
        result = ELOG_ERR_INITLOCK;
    }
    else
    {
        if (xSemaphoreGive(xSemaphore_elogfile) != pdTRUE)
        {
            SEGGER_RTT_printf(0, "warning: xSemaphoreGive elog_file_port_init_lock\n");
        }
    }

#elif defined QL_EC600U

    if (ql_rtos_mutex_create(&s_mutexLock))
    {
        result = ELOG_ERR_INITLOCK;
    }

#endif
    return result;
}

/**
 * EasyLogger flile log pulgin port initialize
 *
 * @return result
 */
ElogErrCode elog_file_port_init(void)
{
    ElogErrCode result = ELOG_NO_ERR;

    /* add your code here */
    if (checkAndCreateDir(ELOG_FILE_DIR))
    {
        result = -1;
    }

    return result;
}

/**
 * file log lock
 */
void elog_file_port_lock(void)
{
    /* add your code here */
#ifdef FREERTOS
    if (xSemaphore_elogfile != NULL)
    {
        //0 for in case of this been call in isr.
        xSemaphoreTake(xSemaphore_elogfile, portMAX_DELAY);
    }

#elif defined QL_EC600U
    ql_rtos_mutex_try_lock(s_mutexLock);
#endif
}

/**
 * file log unlock
 */
void elog_file_port_unlock(void)
{
    /* add your code here */
#ifdef FREERTOS
    if (xSemaphore_elogfile != NULL)
    {
        xSemaphoreGive(xSemaphore_elogfile);
    }

#elif defined QL_EC600U
    ql_rtos_mutex_unlock(s_mutexLock);
#endif
}

/**
 * file log deinit
 */
void elog_file_port_deinit(void)
{
    /* add your code here */
#ifdef FREERTOS
#elif defined QL_EC600U
    ql_rtos_mutex_delete(s_mutexLock);
#endif
}
