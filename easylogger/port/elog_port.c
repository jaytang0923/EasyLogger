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

#include <elog.h>
#ifdef ELOG_FILE_ENABLE
#include <elog_file.h>
#endif
#ifdef QL_EC600U
    #include <hal_debug.h>
    static ql_mutex_t s_mutexLock;
#endif
/**
 * EasyLogger port initialize
 *
 * @return result
 */
ElogErrCode elog_port_init(void)
{
    ElogErrCode result = ELOG_NO_ERR;
    /* add your code here */
#ifdef QL_EC600U

    if (ql_rtos_mutex_create(&s_mutexLock))
    {
        result = ELOG_ERR_INITLOCK;
    }

#endif
#ifdef ELOG_FILE_ENABLE
    elog_file_init();
#endif
    return result;
}

/**
 * EasyLogger port deinitialize
 *
 */
void elog_port_deinit(void)
{
    /* add your code here */
#ifdef QL_EC600U
    ql_rtos_mutex_delete(s_mutexLock);
#endif
}

/**
 * output log port interface
 *
 * @param log output of log
 * @param size log size
 */
void elog_port_output(const char *log, size_t size)
{
    /* add your code here */
#ifdef ELOG_TERMINAL_ENABLE
    ELOG("%s", log);
#endif
#ifdef ELOG_FILE_ENABLE
    /* write the file */
    elog_file_write(log, size);
#endif
}

/**
 * output lock
 */
void elog_port_output_lock(void)
{
    /* add your code here */
#ifdef QL_EC600U
    ql_rtos_mutex_try_lock(s_mutexLock);
#endif
}

/**
 * output unlock
 */
void elog_port_output_unlock(void)
{
    /* add your code here */
#ifdef QL_EC600U
    ql_rtos_mutex_unlock(s_mutexLock);
#endif
}

/**
 * get current time interface
 *
 * @return current time
 */
const char *elog_port_get_time(void)
{
    /* add your code here */
#ifdef QL_EC600U
    uint32_t curtick = ql_rtos_get_system_tick();
#endif
    static char cur_system_time[16] = { 0 };
    sprintf(cur_system_time, "%06ld.%03ld", curtick / 1000, curtick % 1000);
    return cur_system_time;
}

/**
 * get current process name interface
 *
 * @return current process name
 */
const char *elog_port_get_p_info(void)
{
    /* add your code here */
    return "";
}

/**
 * get current thread name interface
 *
 * @return current thread name
 */
const char *elog_port_get_t_info(void)
{
    /* add your code here */
#ifdef QL_EC600U
    static ql_task_t s_task;
    static ql_task_status_t s_status;

    if (ql_rtos_task_get_current_ref(&s_task))
    {
        return "";
    }

    if (ql_rtos_task_get_status(s_task, &s_status))
    {
        return "";
    }

    return s_status.pcTaskName;
#endif
}
