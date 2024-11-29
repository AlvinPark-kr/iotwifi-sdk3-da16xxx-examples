/**
 ****************************************************************************************
 *
 * @file user_rtm.c
 *
 * @brief User retention memory Functions.
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
#include "da16x_dpm.h"
#include "util_api.h"
#include "cma_debug.h"
#include "cma_osal.h"
#include "cma_rtm_data.h"

static OS_MUTEX cma_rtm_data_mutex;

CMA_STATUS_TYPE cma_rtm_data_init(void)
{
    uint32_t ret;

    ret = dpm_user_rtm_pool_create ();

    if (ret == 1)
    {
        OS_MUTEX_CREATE(cma_rtm_data_mutex);
        OS_ASSERT(cma_rtm_data_mutex);
        return CMA_STATUS_OK;
    }
    else
        return CMA_STATUS_FAIL;
}

CMA_STATUS_TYPE cma_rtm_data_alloc_and_read(uint8_t *name, uint8_t **data, uint32_t len)
{
    uint32_t size, ret;

    OS_MUTEX_GET(cma_rtm_data_mutex, OS_MUTEX_FOREVER);
    size = user_rtm_get ((char*) name, data);

    if (size == 0)
    {
        ret = user_rtm_pool_allocate ((char*) name, (void**) data, len, 0);
        if (ret != 0)
        {
            LOG(LOG_ERR, "data alloc in rtm failed err = (0x%x)", ret);
            return CMA_STATUS_FAIL;
        }
        size = user_rtm_get ((char*) name, data);
    }
    OS_MUTEX_PUT(cma_rtm_data_mutex);

    if (size != len)
    {
        LOG(LOG_ERR, "the returned size (0x%x) is different from size of data (0x%x)", size, len);
        return CMA_STATUS_FAIL;
    }

    return CMA_STATUS_OK;
}
