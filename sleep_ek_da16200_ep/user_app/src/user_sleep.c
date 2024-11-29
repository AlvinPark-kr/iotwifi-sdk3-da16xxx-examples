/**
 ****************************************************************************************
 *
 * @file user_sleep_app.c
 *
 * @brief User sleep app task
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
#include "command_net.h"
#include "da16x_sys_watchdog.h"
#include "sys_feature.h"
#include "cma_debug.h"
#include "cma_gpio.h"
#include "cma_osal.h"
#include "cma_sleep.h"
#include "user_hw_pin_config.h"
#include "user_sleep.h"

/* internal defines */
#define USER_SLEEP_TASK_NAME       	"USER_SLEEP"
#define USER_SLEEP_TASK_STACK_SZ   	(256 * 4)
#define USER_SLEEP_TASK_PRI        	OS_TASK_PRIORITY_USER

#define USER_SLEEP_TIME	            (10000)

#define USER_SLEEP2_INT             (1 << 2)
#define USER_SLEEP3_INT  	      	(1 << 3)


int32_t debug_level = 4;

/* Local variable */
static OS_TASK xTask = NULL;

/* Local functions */

static void user_sleep3_int_handler(void *param)
{
    DA16X_UNUSED_ARG(param);

    if (xTask) {
        OS_TASK_NOTIFY_FROM_ISR(xTask, USER_SLEEP3_INT, OS_NOTIFY_SET_BITS);
    }
}

static void user_sleep2_int_handler(void *param)
{
    DA16X_UNUSED_ARG(param);

    if (xTask) {
        OS_TASK_NOTIFY_FROM_ISR(xTask, USER_SLEEP2_INT, OS_NOTIFY_SET_BITS);
    }
}

static void user_gpio_sets_interrupt(void)
{
    if (cma_gpio_set_func(USER_SLEEP2_GPIO_FUNC_PIN) == CMA_STATUS_FAIL) {
        configASSERT(0);
    }

    if (cma_gpio_set_input(USER_SLEEP2_GIPO_PORT, USER_SLEEP2_GPIO_NUM, CMA_GPIO_PULL_UP) == CMA_STATUS_FAIL) {
        configASSERT(0);
    }

    if (cma_gpio_set_interrupt(USER_SLEEP2_GIPO_PORT, USER_SLEEP2_GPIO_NUM, CMA_INT_EDGE_ACTIVE_LOW,
    		user_sleep2_int_handler) == CMA_STATUS_FAIL) {
        configASSERT(0);
    }

    if (cma_gpio_set_func(USER_SLEEP3_GPIO_FUNC_PIN) == CMA_STATUS_FAIL) {
        configASSERT(0);
    }

    if (cma_gpio_set_input(USER_SLEEP2_GIPO_PORT, USER_SLEEP3_GPIO_NUM, CMA_GPIO_PULL_UP) == CMA_STATUS_FAIL) {
        configASSERT(0);
    }

    if (cma_gpio_set_interrupt(USER_SLEEP3_GIPO_PORT, USER_SLEEP3_GPIO_NUM, CMA_INT_EDGE_ACTIVE_LOW,
    		user_sleep3_int_handler) == CMA_STATUS_FAIL) {
        configASSERT(0);
    }
}

static void user_sleep_task(void *arg)
{
    DA16X_UNUSED_ARG(arg);
    int8_t wdog_id;
    BaseType_t ret __attribute__((unused));
    uint32_t notif;

    LOG(LOG_INFO, "Start User interrupt process task!");
    /* Register user_sleep_task to be monitored by watchdog */
    wdog_id = da16x_sys_watchdog_register(pdFALSE);

    user_gpio_sets_interrupt();

    for (;;) {
        /* Notify watchdog on each loop */
        da16x_sys_watchdog_notify(wdog_id);

        /* Suspend watchdog while blocking on OS_TASK_NOTIFY_WAIT() */
        da16x_sys_watchdog_suspend(wdog_id);

        /*
         * Wait for any of the notification bits, then clear them all
         */
        ret = OS_TASK_NOTIFY_WAIT(0, OS_TASK_NOTIFY_ALL_BITS, &notif, OS_TASK_NOTIFY_FOREVER);
        /* Blocks forever waiting for the task notification. Therefore, the return value must
         * always be OS_OK
         */
        configASSERT(ret == pdTRUE);

        LOG(LOG_DBG, ">>> notify: 0x%X\n", notif);

        /* Resume watchdog */
        da16x_sys_watchdog_notify_and_resume(wdog_id);


        if (notif & USER_SLEEP2_INT) {
        	cma_sleep_trigger(CMA_SLEEP_TYPE_2, USER_SLEEP_TIME);
        }

        if (notif & USER_SLEEP3_INT) {
        	cma_sleep_trigger(CMA_SLEEP_TYPE_3, USER_SLEEP_TIME);
        }
    }
}

void user_sleep_init(void)
{
	cma_sleep_init();

    if (xTask != NULL) {
        configASSERT(0);
    }

    if (pdPASS
        != OS_TASK_CREATE(USER_SLEEP_TASK_NAME, user_sleep_task, NULL, USER_SLEEP_TASK_STACK_SZ,
            USER_SLEEP_TASK_PRI, xTask)) {
        LOG(LOG_ERR, "Failed to start user interrupt process task!\n");
        configASSERT(0);
    }
}

