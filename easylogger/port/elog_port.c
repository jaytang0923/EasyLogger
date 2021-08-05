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
#include <common.h>

static xSemaphoreHandle xSemaphore_elog = NULL; 
typedef struct
{
    Log_Channel channel;    //use to set the output channel
    uint8_t level;          //just use to save the elog.filter.level to file
} elog_cfg_str;

//default output to NONE.
static elog_cfg_str elogcfg = { LOG_CH_NONE, ELOG_LVL_ASSERT};

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
 * @fn          int loadelogconfig(void)
 * @brief       load elog config from ELOG_FILE_CFG
 * 
 * @param[in]   void  
 * @return      int
 */
int elog_port_loadconfig(void)
{
    elog_cfg_str readcfg;
    int ret;
    ret = readfile(ELOG_FILE_CFG, (void *)&readcfg, sizeof(readcfg));
    if(ret == sizeof(readcfg))
    {
        if(readcfg.channel <= LOG_CH_RTT)
        {
            elogcfg.channel = readcfg.channel;
        }
        if(readcfg.level <= ELOG_LVL_VERBOSE)
        {
            elog_set_filter_lvl(readcfg.level);
            elogcfg.level = readcfg.level;
        }
        dbg("elog_port_loadconfig channel:%d level:%d  %d %d\n",elogcfg.channel, elogcfg.level,readcfg.channel,readcfg.level);
        return 0;
    }
    return -1;
}


/**
 * EasyLogger port initialize
 *
 * @return result
 */
ElogErrCode elog_port_init(void) {
    ElogErrCode result = ELOG_NO_ERR;

    /* add your code here */
    //mkdir /log here
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

/**
 * @fn          int updateelogconfig(void)
 * @brief       save the elog config to ELOG_FILE_CFG
 * 
 * @param[in]   void  
 * @return      int
                0 : success
               -1 : fail
 */
static int updateelogconfig(void)
{
    int ret;    
    dbg("updateelogconfig");
    if(checkfileexist(ELOG_FILE_CFG) == 0)
    {
        //create
        ret = createfile(ELOG_FILE_CFG,(void*)&elogcfg, sizeof(elogcfg));
    }else{
        //update
        ret = updatefile(ELOG_FILE_CFG,(void*)&elogcfg, sizeof(elogcfg));
    }
    
    if(ret)
    {
        err("updateelogconfig error\n");
    }
    return ret;
}

int elog_port_setchannel(Log_Channel channel)
{
    int ret;
    Log_Channel oldchan = elogcfg.channel;
    if(elogcfg.channel == channel)
        return 0;
    
    elogcfg.channel = channel;
    ret = updateelogconfig();
    if(ret)
    {
        elogcfg.channel = oldchan;
    }
    return ret;
}
Log_Channel elog_port_getchannel(void)
{
    return elogcfg.channel;
}

int elog_port_setlevel(uint8_t lv)
{
    int ret;
    uint8_t oldlevel = elogcfg.level;
    if(elogcfg.level == lv)
    {
        //make sure the elog filter sync
        elog_set_filter_lvl(lv);
        return 0;
    }

    elogcfg.level = lv;
    elog_set_filter_lvl(lv);
    ret = updateelogconfig();
    if(ret)
    {
        elogcfg.level = oldlevel;
        elog_set_filter_lvl(oldlevel);
    }
    return ret;
}
uint8_t elog_port_getlevel(void)
{
    return elogcfg.level;
}

/**
 * output log port interface
 *
 * @param log output of log
 * @param size log size
 */
void elog_port_output(char *log, size_t size) {
    /* add your code here */
    static tick tksmax = 0;
    tick tks=get_tick(),takems;
    switch (elogcfg.channel)
    {
        case LOG_CH_RTT:
            //FIXME:RTT   not support %.*s
            if(size < ELOG_LINE_BUF_SIZE){
                log[size] = 0;
            }
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
