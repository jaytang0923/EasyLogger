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
#include <ProjectConfig.h>
#include <FreeRTOS.h>
#include <semphr.h>
#include <SEGGER_RTT.h>
#include <debug.h>

static xSemaphoreHandle xSemaphore_elog = NULL; 

#include "elog_file.h"

/**
 * EasyLogger flile log pulgin port initialize
 *
 * @return result
 */
ElogErrCode elog_file_port_init(void)
{
    ElogErrCode result = ELOG_NO_ERR;

    /* add your code here */
    xSemaphore_elog = xSemaphoreCreateMutex();
    if(xSemaphore_elog == NULL)
    {
        //err("xSemaphoreCreateMutex error\n");
        SEGGER_RTT_printf(0,"xSemaphoreCreateMutex elog file error\n");
        result = -1;
    }else
    {
        if(xSemaphoreGive(xSemaphore_elog) != pdTRUE)
        {
            SEGGER_RTT_printf(0, "warning: xSemaphoreGive elog file\n");
        }
    }
    return result;
}

/**
 * file log lock
 */
void elog_file_port_lock(void) {

    /* add your code here */
    if(xSemaphore_elog != NULL)
    {
        //0 for in case of this been call in isr.
        xSemaphoreTake(xSemaphore_elog,0);
    }
}

/**
 * file log unlock
 */
void elog_file_port_unlock(void) {

    /* add your code here */
    if(xSemaphore_elog != NULL)
    {
        xSemaphoreGive(xSemaphore_elog);
    }
}

/**
 * file log deinit
 */
void elog_file_port_deinit(void) {

    /* add your code here */

}
