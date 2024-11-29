/**
 ****************************************************************************************
 *
 * @file user_flash.c
 *
 * @brief User flash example task
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
#include "cma_flash.h"
#include "user_hw_pin_config.h"
#include "user_flash.h"

/* internal defines */
#define USER_FLASH_TASK_NAME               "USER_FLASH"
#define USER_FLASH_TASK_STACK_SZ           (256 * 4)
#define USER_FLASH_TASK_PRI                OS_TASK_PRIORITY_USER

#define USER_FLASH_WRITE_READ_INT       (1 << 0)

int32_t debug_level = LOG_INFO;

/* Local variable */
static OS_TASK xTask = NULL;

/* Local functions */

static void user_flash_write_read_int_handler(void *param)
{
    DA16X_UNUSED_ARG(param);

    if (xTask)
    {
        OS_TASK_NOTIFY_FROM_ISR(xTask, USER_FLASH_WRITE_READ_INT, OS_NOTIFY_SET_BITS);
    }
}

static void user_gpio_sets_interrupt(void)
{

    if (cma_gpio_set_func (USER_FLASH_WRITE_READ_GPIO_FUNC_PIN) == CMA_STATUS_FAIL)
    {
        configASSERT(0);
    }

    cma_gpio_set_input (USER_FLASH_WRITE_READ_GIPO_PORT, USER_FLASH_WRITE_READ_GPIO_NUM, CMA_GPIO_PULL_UP);

    if (cma_gpio_set_interrupt (USER_FLASH_WRITE_READ_GIPO_PORT, USER_FLASH_WRITE_READ_GPIO_NUM,
                                CMA_INT_EDGE_ACTIVE_LOW, user_flash_write_read_int_handler) == CMA_STATUS_FAIL)
    {
        configASSERT(0);
    }
}

static void user_flash_write_read_op(void)
{
    void *handle = NULL;
    uint32_t address;
    uint8_t *data1, *data2;
    uint32_t size, sector, offset;
    CMA_STATUS_TYPE ret;

    /*0x3be000 ~ 0x3ec000 */
    size = 0x100;
    offset = rand () % SF_SECTOR_SZ;
    sector = rand () % ((SFLASH_ALLOC_SIZE_USER / SF_SECTOR_SZ) - 1);
    address = SFLASH_USER_AREA_START + sector * SF_SECTOR_SZ + offset - size;

    LOG(LOG_INFO, "\r\noffset = 0x%x sector = 0x%x address = 0x%x", offset, sector, address);

    handle = cma_flash_open ();
    cma_assert(handle);

    data1 = OS_MALLOC(size);
    cma_assert(data1);
    memset (data1, 0x0, size);

    data2 = OS_MALLOC(size);
    cma_assert(data2);
    memset (data2, 0x0, size);

    for (uint16_t i = 0; i < size; i++)
    {
        *(data1 + i) = 0xAA; //rand() % 128;
    }

    ret = cma_flash_write (handle, address, data1, size);
    cma_assert(ret == CMA_STATUS_OK);

    ret = cma_flash_read (handle, address, data2, size);
    cma_assert(ret == CMA_STATUS_OK);

    ret = cma_flash_close (handle);
    cma_assert(ret == CMA_STATUS_OK);

    for (uint16_t i = 0; i < size; i++)
    {
        if ((i + 1) % 8 == 0)
        {
            LOG(LOG_DBG, "0x%x: %02X %02X %02X %02X %02X %02X %02X %02X", address + (i - 7), data2[i - 7], data2[i - 6],
                data2[i - 5], data2[i - 4], data2[i - 3], data2[i - 2], data2[i - 1], data2[i]);
        }

        if (*(data1 + i) != *(data2 + i))
        {
            LOG(LOG_ERR, "i = %d data1 = %d  data2 = %d", i, *(data1 + i), *(data2 + i));
            cma_assert(0);
        }
    }

    OS_FREE(data1);
    OS_FREE(data2);

    LOG(LOG_INFO, "Completed!");
}

static void user_flash_task(void *arg)
{
    DA16X_UNUSED_ARG(arg);

    int8_t wdog_id;

    LOG(LOG_INFO, "Start User int process task!");

    /* Register pxp_reporter_task to be monitored by watchdog */
    wdog_id = da16x_sys_watchdog_register (pdFALSE);
    if (wdog_id < 0)
        return;

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

        if (notif & USER_FLASH_WRITE_READ_INT)
        {
            user_flash_write_read_op ();
        }

    }

    da16x_sys_watchdog_unregister (wdog_id);
}

void user_flash_init(void)
{
    cma_flash_init ();

    configASSERT(xTask == NULL);

    if (pdPASS
            != OS_TASK_CREATE(USER_FLASH_TASK_NAME, user_flash_task, NULL, USER_FLASH_TASK_STACK_SZ,
                              USER_FLASH_TASK_PRI, xTask))
    {
        LOG(LOG_ERR, "Failed to start User flash process task!");
        configASSERT(0);
    }
}

