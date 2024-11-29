/**
 ****************************************************************************************
 *
 * @file user_rtm_data.c
 *
 * @brief User rtm data task
 *
 * Copyright (c) 2016-2024 Renesas Electronics. All rights reserved.
 *
 * This software ("Software") is owned by Renesas Electronics.
 *
 * By using this Software you agree that Renesas Electronics retains all
 * intellectual property and proprietary rights in and to this Software and any
 * use, reproduction, disclosure or distribution of the Software without express
 * written permission or a license agreement from Renesas Electronics is
 * strictly prohibited. This Software is solely for use on or in conjunction
 * with Renesas Electronics products.
 *
 * EXCEPT AS OTHERWISE PROVIDED IN A LICENSE AGREEMENT BETWEEN THE PARTIES, THE
 * SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. EXCEPT AS OTHERWISE
 * PROVIDED IN A LICENSE AGREEMENT BETWEEN THE PARTIES, IN NO EVENT SHALL
 * RENESAS ELECTRONICS BE LIABLE FOR ANY DIRECT, SPECIAL, INDIRECT, INCIDENTAL,
 * OR CONSEQUENTIAL DAMAGES, OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF
 * USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE
 * OF THE SOFTWARE.
 *
 ****************************************************************************************
 */

#include "sdk_type.h"

#include "da16x_system.h"
#include "da16x_types.h"
#include "da16x_sys_watchdog.h"
#include "sys_feature.h"
#include "cma_debug.h"
#include "cma_osal.h"
#include "cma_sleep.h"
#include "cma_rtm_data.h"
#include "user_rtm_data.h"

/* internal defines */
#define USER_RTM_DATA_TASK_NAME       	"USER_RTM_DATA"
#define USER_RTM_DATA_TASK_STACK_SZ   	(256 * 4)
#define USER_RTM_DATA_TASK_PRI        	OS_TASK_PRIORITY_USER

#define USER_RTM_DATA_SLEEP_EV         	(1 << 0)

#define USER_BRTM_DATA_IDLE_TIME       	2000  /* 2 sec */
#define USER_BRTM_DATA_SLEEP_TIME       2000  /* 2 sec */

/* USER Buffer */
#define USER_BUFFER_TAG         "user_buff"
#define USER_BUFFER_LENGTH      (128)

struct user_buffer_t
{
    uint16_t idx;
};

int32_t debug_level = 4;

/* Local variable */
static OS_TASK xTask = NULL;
static OS_TIMER user_rtm_data_timer = NULL;

static struct user_buffer_t *g_user_buff = NULL;

/* Local functions */

static void user_rtm_data_timer_callback(TimerHandle_t pxTime)
{
    DA16X_UNUSED_ARG(pxTime);

    if (xTask)
    {
        OS_TASK_NOTIFY(xTask, USER_RTM_DATA_SLEEP_EV, OS_NOTIFY_SET_BITS);
    }
}

static CMA_STATUS_TYPE user_rtm_data_app_init(void)
{
    cma_rtm_data_alloc_and_read ((uint8_t*) USER_BUFFER_TAG, (uint8_t**) &g_user_buff, sizeof(struct user_buffer_t));
    if (g_user_buff == NULL)
    {
        cma_assert(0);
    }

    LOG(LOG_INFO, " Index = %d\n", g_user_buff->idx);

    user_rtm_data_timer = OS_TIMER_CREATE("BLINKY_RTOS", portCONVERT_MS_2_TICKS(USER_BRTM_DATA_IDLE_TIME), pdTRUE,
                                          (void* )0, user_rtm_data_timer_callback);

    OS_TIMER_START(user_rtm_data_timer, OS_TIMER_FOREVER);

    return CMA_STATUS_OK;
}

static void user_rtm_data_task(void *arg)
{
    DA16X_UNUSED_ARG(arg);

    int8_t wdog_id;

    LOG(LOG_INFO, "Start user rtm data task!");

    user_rtm_data_app_init ();

    /* Register pxp_reporter_task to be monitored by watchdog */
    wdog_id = da16x_sys_watchdog_register (pdFALSE);

    for (;;)
    {
        BaseType_t ret __attribute__((unused));
        uint32_t notif;

        /* Notify watchdog on each loop */
        da16x_sys_watchdog_notify (wdog_id);

        /* Suspend watchdog while blocking on OS_TASK_NOTIFY_WAIT() */
        da16x_sys_watchdog_suspend (wdog_id);

        /*
         * Wait on any of the notification bits, then clear them all
         */
        ret = OS_TASK_NOTIFY_WAIT(0, OS_TASK_NOTIFY_ALL_BITS, &notif, OS_TASK_NOTIFY_FOREVER);
        /* Blocks forever waiting for the task notification. Therefore, the return value must
         * always be OS_OK
         */
        configASSERT(ret == pdTRUE);

        /* Resume watchdog */
        da16x_sys_watchdog_notify_and_resume (wdog_id);

        if (notif & USER_RTM_DATA_SLEEP_EV)
        {
            g_user_buff->idx++;
            cma_sleep_trigger (CMA_SLEEP_TYPE_3, USER_BRTM_DATA_SLEEP_TIME);
        }
    }
}

void user_rtm_data_init(void)
{
    cma_sleep_init ();
    cma_rtm_data_init ();

    if (xTask != NULL)
    {
        configASSERT(0);
    }

    if (pdPASS
            != OS_TASK_CREATE(USER_RTM_DATA_TASK_NAME, user_rtm_data_task, NULL, USER_RTM_DATA_TASK_STACK_SZ,
                              USER_RTM_DATA_TASK_PRI, xTask))
    {
        LOG(LOG_ERR, "Failed to start user rtm data task!");
        configASSERT(0);
    }
}

