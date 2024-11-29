/**
 ****************************************************************************************
 *
 * @file cma_flash.h
 *
 * @brief User Flash functions.
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

#ifndef CMA_FLASH_H_

#define CMA_FLASH_H_

#include "da16x_types.h"
#include "da16200_ioconfig.h"
#include "cma_status.h"

/**
 ****************************************************************************************
 * @brief Init resource for flash driver.
 *
 * @param[in] None
 *
 * @return Success or Fail.
 ****************************************************************************************
 */
CMA_STATUS_TYPE cma_flash_init(void);

/**
 ****************************************************************************************
 * @brief Delete resource for flash driver.
 *
 * @param[in] None
 *
 * @return Success or Fail.
 ****************************************************************************************
 */
CMA_STATUS_TYPE cma_flash_delete(void);

/**
 ****************************************************************************************
 * @brief Open flash driver.
 *
 * @param[in] None.
 *
 * @return handle pointer.
 ****************************************************************************************
 */
void* cma_flash_open(void);

/**
 ****************************************************************************************
 * @brief Write data to flash.
 *
 * @param[in] handle pointer.
 * @param[in] address of flash.
 * @param[in] data pointer.
 * @param[in] size of data.
 *
 * @return Success or Fail.
 ****************************************************************************************
 */
CMA_STATUS_TYPE cma_flash_write(void *handle, uint32_t startAddress, uint8_t *newData, uint32_t dataLength);

/**
 ****************************************************************************************
 * @brief read data from flash.
 *
 * @param[in] handle pointer.
 * @param[in] address of flash.
 * @param[in] data pointer.
 * @param[in] size of data.
 *
 * @return Success or Fail.
 ****************************************************************************************
 */
CMA_STATUS_TYPE cma_flash_read(void *handle, uint32_t startAddress, uint8_t *readData, uint32_t dataLength);

/**
 ****************************************************************************************
 * @brief erase data in flash.
 *
 * @param[in] handle pointer.
 * @param[in] address of flash.
 * @param[in] size of data.
 *
 * @return Success or Fail.
 ****************************************************************************************
 */
CMA_STATUS_TYPE cma_flash_erase(void *handle, uint32_t startAddress, uint32_t dataLength);

/**
 ****************************************************************************************
 * @brief close flash driver.
 *
 * @param[in] handle pointer.
 *
 * @return Success or Fail.
 ****************************************************************************************
 */
CMA_STATUS_TYPE cma_flash_close(void *handle);

#endif /* CMA_FLASH_H_ */
