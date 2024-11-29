/**
 ****************************************************************************************
 *
 * @file user_fr_wps.c
 *
 * @brief User factory reset and wps task
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
#include "environ.h"
#include "nvedit.h"
#include "cma_debug.h"
#include "cma_gpio.h"
#include "cma_osal.h"
#include "user_hw_pin_config.h"
#include "user_fr_wps.h"

/* internal defines */
#define USER_FR_WPS_TASK_NAME       	"USER_FR_WPS"
#define USER_FR_WPS_TASK_STACK_SZ   	(256 * 4)
#define USER_FR_WPS_TASK_PRI        	OS_TASK_PRIORITY_USER

#define USER_FACTORY_RESET_INT          (1 << 0)
#define USER_FACTORY_RESET_RUN          (1 << 1)
#define USER_WPS_INT                    (1 << 2)
#define USER_RTC_WAKEUP2_PIN_INT        (1 << 3)

#define USER_FACTORY_RESET_TIME         5000  /* 5 sec */

typedef enum
{
    USER_FACTORY_STATE_PRESS,
    USER_FACTORY_STATE_RELEASE
} USER_FACTORY_STATE;

int32_t debug_level = LOG_INFO;

/* Local variable */
static OS_TASK xTask = NULL;
static OS_TIMER user_factory_reset_timer = NULL;

/* Local functions */
static CMA_STATUS_TYPE user_wps_setup(char *macaddr)
{
    char cmd_str[30];
    char reply[10];

    memset (cmd_str, 0, 30);
    memset (reply, 0, 10);

    sprintf (cmd_str, "wps_pbc %s", strlen (macaddr) == 17 ? macaddr : "any");

    da16x_cli_reply (cmd_str, NULL, reply);

    if (strcasecmp (reply, "OK") == 0)
    {
        return CMA_STATUS_OK;
    }

    return CMA_STATUS_FAIL;
}

static CMA_STATUS_TYPE user_factory_reset_data_to_default()
{
    int status;

    LOG(LOG_INFO, "\n Factory reset started...\n");

    status = da16x_clearenv(ENVIRON_DEVICE, "app");

    if (status == FALSE)
    {
        recovery_NVRAM ();
        LOG(LOG_ERR, "\n Failed factory reset...\n");
        return CMA_STATUS_FAIL;
    }

    reboot_func (SYS_REBOOT_POR);

    /* Wait for system-reboot */
    while (1)
        ;

    return CMA_STATUS_OK;
}

static void user_rtc_wakeup2_pin_int_handler(void *param)
{
    DA16X_UNUSED_ARG(param);

    if (xTask)
    {
        OS_TASK_NOTIFY_FROM_ISR(xTask, USER_RTC_WAKEUP2_PIN_INT, OS_NOTIFY_SET_BITS);
    }
}

static void user_wps_int_handler(void *param)
{
    DA16X_UNUSED_ARG(param);

    if (xTask)
    {
        OS_TASK_NOTIFY_FROM_ISR(xTask, USER_WPS_INT, OS_NOTIFY_SET_BITS);
    }
}

static void user_factory_reset_int_handler(void *param)
{
    DA16X_UNUSED_ARG(param);

    if (xTask)
    {
        OS_TASK_NOTIFY_FROM_ISR(xTask, USER_FACTORY_RESET_INT, OS_NOTIFY_SET_BITS);
    }
}

static void user_factory_reset_timer_callback(TimerHandle_t pxTime)
{
    DA16X_UNUSED_ARG(pxTime);

    if (xTask)
    {
        OS_TASK_NOTIFY_FROM_ISR(xTask, USER_FACTORY_RESET_RUN, OS_NOTIFY_SET_BITS);
    }
}

static void user_gpio_sets_interrupt(void)
{
    if (cma_gpio_set_rtc_wakeup_pin (CMA_RTC_WAKEUP2_PIN, CMA_INT_EDGE_ACTIVE_LOW, user_rtc_wakeup2_pin_int_handler)
            == CMA_STATUS_FAIL)
    {
        configASSERT(0);
    }

    user_factory_reset_timer = OS_TIMER_CREATE("USER_FT_RESET", portCONVERT_MS_2_TICKS(USER_FACTORY_RESET_TIME), pdTRUE,
                                               (void* )0, user_factory_reset_timer_callback);

    if (cma_gpio_set_func (USER_FACTORY_GPIO_FUNC_PIN) == CMA_STATUS_FAIL)
    {
        configASSERT(0);
    }

    if (cma_gpio_set_input (USER_FACTORY_GIPO_PORT, USER_FACTORY_GPIO_NUM, CMA_GPIO_HIGH_Z) == CMA_STATUS_FAIL)
    {
        configASSERT(0);
    }

    if (cma_gpio_set_interrupt (USER_FACTORY_GIPO_PORT, USER_FACTORY_GPIO_NUM, CMA_INT_EDGE_ACTIVE_LOW,
                                user_factory_reset_int_handler) == CMA_STATUS_FAIL)
    {
        configASSERT(0);
    }

    if (cma_gpio_set_func (USER_WPS_GPIO_FUNC_PIN) == CMA_STATUS_FAIL)
    {
        configASSERT(0);
    }

    if (cma_gpio_set_input (USER_WPS_GIPO_PORT, USER_WPS_GPIO_NUM, CMA_GPIO_HIGH_Z) == CMA_STATUS_FAIL)
    {
        configASSERT(0);
    }

    if (cma_gpio_set_interrupt (USER_WPS_GIPO_PORT, USER_WPS_GPIO_NUM, CMA_INT_EDGE_ACTIVE_LOW, user_wps_int_handler)
            == CMA_STATUS_FAIL)
    {
        configASSERT(0);
    }
}

static void user_fr_wps_task(void *arg)
{
    DA16X_UNUSED_ARG(arg);

    int8_t wdog_id;
    USER_FACTORY_STATE factory_rest_state = USER_FACTORY_STATE_RELEASE;

    LOG(LOG_INFO, "Start user_fr_wps_task!");

    /* Register pxp_reporter_task to be monitored by watchdog */
    wdog_id = da16x_sys_watchdog_register (pdFALSE);

    user_gpio_sets_interrupt ();

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

        if (notif & USER_FACTORY_RESET_INT)
        {
            if (factory_rest_state == USER_FACTORY_STATE_RELEASE)
            {
                if (cma_gpio_get_input (USER_FACTORY_GIPO_PORT, USER_FACTORY_GPIO_NUM) == CMA_GPIO_LEVEL_LOW)
                {
                    factory_rest_state = USER_FACTORY_STATE_PRESS;
                    if (cma_gpio_set_interrupt (USER_FACTORY_GIPO_PORT, USER_FACTORY_GPIO_NUM, CMA_INT_EDGE_ACTIVE_HIGH,
                                                user_factory_reset_int_handler) == CMA_STATUS_FAIL)
                    {
                        configASSERT(0);
                    }

                    OS_TIMER_START(user_factory_reset_timer, portCONVERT_MS_2_TICKS(USER_FACTORY_RESET_TIME));
                    LOG(LOG_INFO, "Factory reset : Pressed \n");
                }
                else
                {
                    LOG(LOG_INFO, "Factory reset : Already released \n");
                }
            }
            else if (factory_rest_state == USER_FACTORY_STATE_PRESS)
            {
                OS_TIMER_STOP(user_factory_reset_timer, 0);
                if (cma_gpio_set_interrupt (USER_FACTORY_GIPO_PORT, USER_FACTORY_GPIO_NUM, CMA_INT_EDGE_ACTIVE_LOW,
                                            user_factory_reset_int_handler) == CMA_STATUS_FAIL)
                {
                    configASSERT(0);
                }

                factory_rest_state = USER_FACTORY_STATE_RELEASE;
                LOG(LOG_INFO, "Factory reset : Released\n");
            }
        }

        if (notif & USER_FACTORY_RESET_RUN)
        {
            user_factory_reset_data_to_default ();
        }

        if (notif & USER_WPS_INT)
        {
            LOG(LOG_INFO, "WPS button pressed!");
            user_wps_setup (NULL);
        }

        if (notif & USER_RTC_WAKEUP2_PIN_INT)
        {
            LOG(LOG_INFO, "USER_RTC_WAKEUP2_PN_INT happens\n");
        }
    }
}

void user_fr_wps_init(void)
{
    if (xTask != NULL)
    {
        configASSERT(0);
    }

    if (pdPASS
            != OS_TASK_CREATE(USER_FR_WPS_TASK_NAME, user_fr_wps_task, NULL, USER_FR_WPS_TASK_STACK_SZ,
                              USER_FR_WPS_TASK_PRI, xTask))
    {
        LOG(LOG_ERR, "Failed to start User int process task!");
        configASSERT(0);
    }
}

