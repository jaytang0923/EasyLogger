/*
 * This file is part of the EasyLogger Library.
 *
 * Copyright (c) 2015, Armink, <armink.ztl@gmail.com>
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
 * Function: Portable interface for each platform.
 * Created on: 2015-04-28
 */
#include <stdio.h>
#include <string.h>
#include <ProjectConfig.h>
#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>
#include <SEGGER_RTT.h>
#include <debug.h>
#include <systick.h>
#include <elog_file.h>
#include <serial.h>
#include <elog.h>

static xSemaphoreHandle xSemaphore_elog = NULL; 

ElogErrCode elog_port_init_lock(void)
{
    ElogErrCode result = ELOG_NO_ERR;
    xSemaphore_elog = xSemaphoreCreateMutex();
    if(xSemaphore_elog == NULL)
    {
        //err("xSemaphoreCreateMutex error\n");
        SEGGER_RTT_printf(0,"xSemaphoreCreateMutex error\n");
        result = ELOG_ERR_INITLOCK;
    }else
    {
        if(xSemaphoreGive(xSemaphore_elog) != pdTRUE)
        {
            SEGGER_RTT_printf(0, "warning: xSemaphoreGive elog_port_init_lock\n");
        }
    }
    return result;
}

/**
 * EasyLogger port initialize
 *
 * @return result
 */
ElogErrCode elog_port_init(void) {
    ElogErrCode result = ELOG_NO_ERR;

    /* add your code here */
    result = elog_file_init();
    return result;
}

/**
 * EasyLogger port deinitialize
 *
 */
void elog_port_deinit(void) {

    /* add your code here */

}

static Log_Channel s_elogChannel = LOG_CH_NONE;
void elog_setchannel(Log_Channel channel)
{
    s_elogChannel = channel;
}
Log_Channel elog_getchannel(void)
{
    return s_elogChannel;
}
/**
 * output log port interface
 *
 * @param log output of log
 * @param size log size
 */
void elog_port_output(const char *log, size_t size) {
    /* add your code here */
    static tick tksmax = 0;
    tick tks=get_tick(),takems;
    switch (s_elogChannel)
    {
        case LOG_CH_RTT:
            SEGGER_RTT_printf(0,"%s",log);
        case LOG_CH_USB:
            usbserialwrite((uint8_t*)log, size);
        case LOG_CH_FILE:
            /* write the file */
            elog_file_write(log, size);
            takems = get_tick()-tks;
            if(takems > tksmax)
            {
                tksmax = takems;
                warn("elog MAX Take %dms\n",tksmax);
            }
            break;
        default:
            break;
    }
}

/**
 * output lock
 */
void elog_port_output_lock(void) {
    
    /* add your code here */
    if(xSemaphore_elog != NULL)
    {
        //0 for in case of this been call in isr.
        xSemaphoreTake(xSemaphore_elog,0);
    }
}

/**
 * output unlock
 */
void elog_port_output_unlock(void) {
    
    /* add your code here */
    if(xSemaphore_elog != NULL)
    {
        xSemaphoreGive(xSemaphore_elog);
    }
}

/**
 * get current time interface
 *
 * @return current time
 */
const char *elog_port_get_time(void) {
    
    /* add your code here */
    static char cur_system_time[10] = { 0 };
    //FIXME:check the max time
    sprintf(cur_system_time, "%06d.%03d", get_tick()/1000,get_tick()%1000);
    return cur_system_time;
}

/**
 * get current process name interface
 *
 * @return current process name
 */
const char *elog_port_get_p_info(void) {
    
    /* add your code here */
    return "";
}

/**
 * get current thread name interface
 *
 * @return current thread name
 */
const char *elog_port_get_t_info(void) {
    
    /* add your code here */
    TaskHandle_t handle =  xTaskGetCurrentTaskHandle();
    ELOG_ASSERT(handle != NULL);
    ELOG_ASSERT(pcTaskGetName(handle) != NULL);
    return pcTaskGetName(handle);
}
