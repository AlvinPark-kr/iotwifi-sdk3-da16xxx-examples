/**
 ****************************************************************************************
 *
 * @file user_blinky_rtos.c
 *
 * @brief User Blinky RTOS task
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
#include "cma_gpio.h"
#include "cma_osal.h"
#include "user_hw_pin_config.h"
#include "user_blinky_rtos.h"

/* internal defines */
#define USER_BLINKY_RTOS_TASK_NAME       "USER_BLINKY_RTOS"
#define USER_BLINKY_RTOS_TASK_STACK_SZ   (256 * 4)
#define USER_BLINKY_RTOS_TASK_PRI        OS_TASK_PRIORITY_USER

#define USER_BLINKY_RTOS_EV		    	(1 << 0)

#define USER_BLINKY_RTOS_TIME         	2000  /* 2 sec */

int32_t debug_level = 4;

/* Local variable */
static OS_TASK xTask = NULL;
static OS_TIMER user_blinky_rtos_timer = NULL;

static void user_blink_rtos_timer_callback(TimerHandle_t pxTime)
{
    DA16X_UNUSED_ARG(pxTime);

    if (xTask) {
        OS_TASK_NOTIFY_FROM_ISR(xTask, USER_BLINKY_RTOS_EV, OS_NOTIFY_SET_BITS);
    }
}

static CMA_STATUS_TYPE user_blinky_rtos_app_init(void)
{
	user_blinky_rtos_timer = OS_TIMER_CREATE("BLINKY_RTOS", portCONVERT_MS_2_TICKS(USER_BLINKY_RTOS_TIME), pdTRUE,
	        (void* )0, user_blink_rtos_timer_callback);

	OS_TIMER_START(user_blinky_rtos_timer, OS_TIMER_FOREVER);

	return CMA_STATUS_OK;
}

static void user_blinky_rtos_task(void *arg)
{
    DA16X_UNUSED_ARG(arg);

    int8_t wdog_id;

    if (user_blinky_rtos_app_init() == CMA_STATUS_FAIL) {
    	cma_assert(0);
    }

    /* Register pxp_reporter_task to be monitored by watchdog */
    wdog_id = da16x_sys_watchdog_register(pdFALSE);

    for (;;) {
        BaseType_t ret __attribute__((unused));
        uint32_t notif;

        /* Notify watchdog on each loop */
        da16x_sys_watchdog_notify(wdog_id);

        /* Suspend watchdog while blocking on OS_TASK_NOTIFY_WAIT() */
        da16x_sys_watchdog_suspend(wdog_id);

        /*
         * Wait on any of the notification bits, then clear them all
         */
        ret = OS_TASK_NOTIFY_WAIT(0, OS_TASK_NOTIFY_ALL_BITS, &notif, OS_TASK_NOTIFY_FOREVER);
        /* Blocks forever waiting for the task notification. Therefore, the return value must
         * always be OS_OK
         */
        configASSERT(ret == pdTRUE);

        if (notif & USER_BLINKY_RTOS_EV) {
            cma_gpio_set_output_level(USER_LED1_GPIO_PORT, USER_LED1_GPIO_NUM, CMA_GPIO_LEVEL_LOW);
            OS_DELAY(100); //1 sec
            cma_gpio_set_output_level(USER_LED1_GPIO_PORT, USER_LED1_GPIO_NUM, CMA_GPIO_LEVEL_HIGH);

            if (user_blinky_rtos_timer != NULL)
                OS_TIMER_START(user_blinky_rtos_timer, OS_TIMER_FOREVER);
        }

        /* Resume watchdog */
        da16x_sys_watchdog_notify_and_resume(wdog_id);

    }
}

void user_blinky_rtos_init(void)
{

    if (xTask != NULL) {
        configASSERT(0);
    }

    if (pdPASS
        != OS_TASK_CREATE(USER_BLINKY_RTOS_TASK_NAME, user_blinky_rtos_task, NULL, USER_BLINKY_RTOS_TASK_STACK_SZ,
            USER_BLINKY_RTOS_TASK_PRI, xTask)) {
        LOG(LOG_ERR, "Failed to start User master process task!");
        configASSERT(0);
    }
}

