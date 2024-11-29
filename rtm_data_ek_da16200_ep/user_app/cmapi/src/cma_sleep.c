/**
 ****************************************************************************************
 *
 * @file cma_sleep.c
 *
 * @brief User GPIO Functions.
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
#include "limits.h"
#include "cma_debug.h"
#include "cma_osal.h"
#include "cma_sleep.h"

extern void do_set_dpm_power_down(UINT64 usec, UCHAR retention);

static OS_MUTEX cma_sleep_mutex;

void cma_sleep_init(void)
{
    OS_MUTEX_CREATE(cma_sleep_mutex);
    OS_ASSERT(cma_sleep_mutex);
}

void cma_sleep_trigger(CMA_SLEEP_TYPE type, uint64_t wakeup_time)
{
    uint8_t retain;

    if (type == CMA_SLEEP_TYPE_2)
        retain = 0;
    else
        retain = 1;

    OS_MUTEX_GET(cma_sleep_mutex, OS_MUTEX_FOREVER);

    if (wakeup_time == 0)
        do_set_dpm_power_down (0x1FFFFF * 1000000ULL, retain);
    else
        do_set_dpm_power_down ((wakeup_time * 1000), retain); //usec

    OS_MUTEX_PUT(cma_sleep_mutex);
}

