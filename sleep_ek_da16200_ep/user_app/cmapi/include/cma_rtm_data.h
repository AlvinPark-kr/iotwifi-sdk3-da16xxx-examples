/**
 ****************************************************************************************
 *
 * @file cma_rtm_data.h
 *
 * @brief User functions for managing data in retention memory.
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

#ifndef CMA_RTM_DATA_H_

#define CMA_RTM_DATA_H_

#include "da16x_types.h"
#include "da16200_ioconfig.h"
#include "cma_status.h"

/**
 ****************************************************************************************
 * @brief Initialize structure and area for user data in retention memory .
 *
 * @param[in] NONE
 *
 * @return Success or Fail
 ****************************************************************************************
 */
CMA_STATUS_TYPE cma_rtm_data_init(void);

/**
 ****************************************************************************************
 * @brief Read data user data.
 *
 * @param[in] unique name for the user data
 * @param[in] memory pointer of user data
 * @param[in] size of user data
 * @return allocated size
 ****************************************************************************************
 */
CMA_STATUS_TYPE cma_rtm_data_alloc_and_read(uint8_t *name, uint8_t **data, uint32_t len);

#endif /* CMA_RTM_DATA_H_ */
