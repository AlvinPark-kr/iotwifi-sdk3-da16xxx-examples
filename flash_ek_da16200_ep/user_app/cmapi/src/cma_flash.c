/**
 ****************************************************************************************
 *
 * @file cma_flash.c
 *
 * @brief User flash Functions.
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
#include "sflash.h"
#include "sys_image.h"
#include "environ.h"
#include "gpio.h"
#include "cma_osal.h"
#include "cma_debug.h"
#include "cma_flash.h"

#define CMA_SECTOR_SIZE 4096
#define CMA_PAGE_SIZE CMA_SECTOR_SIZE

OS_MUTEX cma_flash_mutex = NULL;

/*
 *****************************************************
 * Usage : init -> open -> read/write -> close -> delete
 * init -> (open -> read/write -> close -> open -> ...) -> delete
 ******************************************************
 */

CMA_STATUS_TYPE cma_flash_init(void)
{
    if (cma_flash_mutex == NULL)
    {
        OS_MUTEX_CREATE(cma_flash_mutex);
    }

    return CMA_STATUS_OK;
}

CMA_STATUS_TYPE cma_flash_delete(void)
{
    if (cma_flash_mutex != NULL)
    {
        OS_MUTEX_DELETE(cma_flash_mutex);
    }

    cma_flash_mutex = NULL;

    return CMA_STATUS_OK;
}

static void cmai_flash_enable_write(HANDLE handle, uint32_t address, uint32_t length)
{
    uint32_t ioctldata[8];
    uint32_t busmode;

    if (handle)
    {
        // write mode
        busmode = SFLASH_BUS_3BADDR | SFLASH_BUS_111;
        SFLASH_IOCTL (handle, SFLASH_BUS_CONTROL, &busmode);

        ioctldata[0] = address;
        ioctldata[1] = length;
        SFLASH_IOCTL (handle, SFLASH_SET_UNLOCK, ioctldata);
    }
}

static void cmai_flash_disable_write(HANDLE handle, uint32_t address, uint32_t length)
{
    uint32_t ioctldata[8];
    uint32_t busmode;

    if (handle)
    {
        ioctldata[0] = address;
        ioctldata[1] = length;
        SFLASH_IOCTL (handle, SFLASH_SET_LOCK, ioctldata);

        // read mode
        busmode = SFLASH_BUS_3BADDR | SFLASH_BUS_144;
        SFLASH_IOCTL (handle, SFLASH_BUS_CONTROL, &busmode);
    }
}

static uint32_t cmai_flash_read(HANDLE handle, uint32_t address, uint8_t *buffer, uint32_t length)
{
    uint32_t busmode;
    uint32_t ret = 0;

    if (handle)
    {
        busmode = SFLASH_BUS_3BADDR | SFLASH_BUS_144;
        SFLASH_IOCTL (handle, SFLASH_BUS_CONTROL, &busmode);

        ret = SFLASH_READ (handle, address, (void*) buffer, length);
    }

    return ret;
}

static uint32_t cmai_flash_write(HANDLE handle, uint32_t address, uint8_t *data, uint32_t length)
{
    uint32_t ret = 0;

    if (handle)
    {
        cmai_flash_enable_write (handle, address, length);

        ret = SFLASH_WRITE (handle, address, data, length);

        cmai_flash_disable_write (handle, address, length);
    }

    return ret;
}

static uint32_t cmai_flash_erase_sector(HANDLE handle, uint32_t address, uint32_t size)
{
    uint32_t ioctldata[8];
    uint32_t busmode;
    uint32_t ret = 0;

    if (handle)
    {
        busmode = SFLASH_BUS_3BADDR | SFLASH_BUS_111;
        SFLASH_IOCTL (handle, SFLASH_BUS_CONTROL, &busmode);

        cmai_flash_enable_write (handle, address, CMA_SECTOR_SIZE);

        ioctldata[0] = address;
        ioctldata[1] = size;
        size = SFLASH_IOCTL (handle, SFLASH_CMD_ERASE, ioctldata);

        cmai_flash_disable_write (handle, address, CMA_SECTOR_SIZE);
    }

    return ret;
}

/* Usage : open -> read/write -> close */
void* cma_flash_open(void)
{
    HANDLE handle;
    uint32_t ioctldata[8];
    uint32_t busmode;

    if (cma_flash_mutex == NULL)
        return NULL;

    da16x_environ_lock (TRUE);

    OS_MUTEX_GET(cma_flash_mutex, OS_MUTEX_FOREVER);

    handle = SFLASH_CREATE (SFLASH_UNIT_0);
    if (handle)
    {
        /* Setup bussel */
        ioctldata[0] = da16x_sflash_get_bussel ();
        SFLASH_IOCTL (handle, SFLASH_SET_BUSSEL, ioctldata);
        if (SFLASH_INIT (handle) == TRUE)
        {
            /* to prevent a reinitialization */
            if (da16x_sflash_setup_parameter ((UINT32*) ioctldata) == TRUE)
            {
                SFLASH_IOCTL (handle, SFLASH_SET_INFO, ioctldata);
            }
        }

        SFLASH_IOCTL (handle, SFLASH_CMD_WAKEUP, ioctldata);
        if (ioctldata[0] > 0)
        {
            ioctldata[0] = ioctldata[0] / 1000;
            if (ioctldata[0] > 100)
                OS_DELAY(ioctldata[0] / 100);
            else
                OS_DELAY(1);
        }

        busmode = SFLASH_BUS_3BADDR | SFLASH_BUS_144;
        SFLASH_IOCTL (handle, SFLASH_BUS_CONTROL, &busmode);
    }

    OS_MUTEX_PUT(cma_flash_mutex);
    return (void*) handle;
}

CMA_STATUS_TYPE cma_flash_write(void *handle, uint32_t startAddress, uint8_t *newData, uint32_t dataLength)
{
    // Calculate the number of sectors spanned by the data
    uint32_t endAddress = startAddress + dataLength - 1;
    uint32_t startSector = startAddress / CMA_SECTOR_SIZE;
    uint32_t endSector = endAddress / CMA_SECTOR_SIZE;
    CMA_STATUS_TYPE ret = CMA_STATUS_FAIL;

    // Buffers for page data
    uint8_t *pageBuffer; //[PAGE_SIZE];

    if (cma_flash_mutex == NULL || handle == NULL)
    {
        return CMA_STATUS_FAIL;
    }

    OS_MUTEX_GET(cma_flash_mutex, OS_MUTEX_FOREVER);

    pageBuffer = OS_MALLOC(CMA_PAGE_SIZE);
    if (pageBuffer)
    {
        for (uint32_t sector = startSector; sector <= endSector; sector++)
        {
            uint32_t sectorAddress = sector * CMA_SECTOR_SIZE;

            // Read and store data from the entire sector before erasing
            for (uint32_t page = 0; page < CMA_SECTOR_SIZE; page += CMA_PAGE_SIZE)
            {
                uint32_t pageAddress = sectorAddress + page;

                if (cmai_flash_read (handle, pageAddress, pageBuffer, CMA_PAGE_SIZE) == CMA_PAGE_SIZE)
                    ret = CMA_STATUS_OK;
                else
                    ret = CMA_STATUS_FAIL;

                // Calculate start and end of the new data within this page
                uint32_t pageStart = pageAddress;
                uint32_t pageEnd = pageAddress + CMA_PAGE_SIZE - 1;

                if (pageEnd < startAddress || pageStart > endAddress)
                {
                    // This page is completely outside the range of new data
                    continue;
                }

                // Determine the overlap and copy new data into the page buffer
                uint32_t newStartOffset = (pageStart < startAddress) ? (startAddress - pageStart) : 0;
                uint32_t newEndOffset = (pageEnd > endAddress) ? (endAddress - pageStart + 1) : CMA_PAGE_SIZE;
                uint32_t newDataOffset = (pageStart < startAddress) ? 0 : (pageStart - startAddress);

                memcpy (&pageBuffer[newStartOffset], &newData[newDataOffset], newEndOffset - newStartOffset);

                // Erase the sector once before writing the first page
                if (page == 0)
                {
                    if (cmai_flash_erase_sector (handle, sectorAddress, CMA_SECTOR_SIZE) != CMA_SECTOR_SIZE)
                        ret = CMA_STATUS_FAIL;
                    else
                        ret = CMA_STATUS_OK;
                }

                // Write updated page back to flash memory
                if (cmai_flash_write (handle, pageAddress, pageBuffer, CMA_PAGE_SIZE) == CMA_PAGE_SIZE)
                    ret = CMA_STATUS_OK;
                else
                    ret = CMA_STATUS_FAIL;
            }

            if (ret == CMA_STATUS_FAIL)
                break;
        }

        OS_FREE(pageBuffer);
    }

    OS_MUTEX_PUT(cma_flash_mutex);

    return ret;
}

CMA_STATUS_TYPE cma_flash_read(void *handle, uint32_t startAddress, uint8_t *readData, uint32_t dataLength)
{
    CMA_STATUS_TYPE ret = CMA_STATUS_FAIL;
    uint32_t offset;
    uint8_t *offset_data;

    if (cma_flash_mutex == NULL || handle == NULL)
    {
        return CMA_STATUS_FAIL;
    }

    OS_MUTEX_GET(cma_flash_mutex, OS_MUTEX_FOREVER);

    offset = startAddress % 4;

    if (offset != 0)
    {
        offset_data = OS_MALLOC(dataLength + offset);
        if (offset_data)
        {
            if (cmai_flash_read (handle, startAddress - offset, offset_data, dataLength + offset)
                    == (dataLength + offset))
                ret = CMA_STATUS_OK;
            else
                ret = CMA_STATUS_FAIL;

            memcpy (readData, offset_data + offset, dataLength);
            OS_FREE(offset_data);
        }
    }
    else
    {
        if (cmai_flash_read (handle, startAddress, readData, dataLength) == dataLength)
            ret = CMA_STATUS_OK;
        else
            ret = CMA_STATUS_FAIL;
    }

    OS_MUTEX_PUT(cma_flash_mutex);

    return ret;
}

CMA_STATUS_TYPE cma_flash_erase(void *handle, uint32_t startAddress, uint32_t dataLength)
{
    // Calculate the number of sectors spanned by the data
    uint32_t endAddress = startAddress + dataLength - 1;
    uint32_t startSector = startAddress / CMA_SECTOR_SIZE;
    uint32_t endSector = endAddress / CMA_SECTOR_SIZE;
    CMA_STATUS_TYPE ret = CMA_STATUS_FAIL;

    // Buffers for page data
    uint8_t *pageBuffer; //[PAGE_SIZE];

    if (cma_flash_mutex == NULL || handle == NULL)
    {
        return CMA_STATUS_FAIL;
    }

    OS_MUTEX_GET(cma_flash_mutex, OS_MUTEX_FOREVER);

    pageBuffer = OS_MALLOC(CMA_PAGE_SIZE);
    if (pageBuffer)
    {
        // Process each sector
        for (uint32_t sector = startSector; sector <= endSector; sector++)
        {
            uint32_t sectorAddress = sector * CMA_SECTOR_SIZE;

            // Read and store data from the entire sector before erasing
            for (uint32_t page = 0; page < CMA_SECTOR_SIZE; page += CMA_PAGE_SIZE)
            {
                uint32_t pageAddress = sectorAddress + page;

                if (cmai_flash_read (handle, pageAddress, pageBuffer, CMA_PAGE_SIZE) == CMA_PAGE_SIZE)
                    ret = CMA_STATUS_OK;
                else
                    ret = CMA_STATUS_FAIL;

                // Calculate start and end of the new data within this page
                uint32_t pageStart = pageAddress;
                uint32_t pageEnd = pageAddress + CMA_PAGE_SIZE - 1;

                if (pageEnd < startAddress || pageStart > endAddress)
                {
                    // This page is completely outside the range of new data
                    continue;
                }

                // Determine the overlap and copy new data into the page buffer
                uint32_t newStartOffset = (pageStart < startAddress) ? (startAddress - pageStart) : 0;
                uint32_t newEndOffset = (pageEnd > endAddress) ? (endAddress - pageStart + 1) : CMA_PAGE_SIZE;

                memset (&pageBuffer[newStartOffset], 0xFF, newEndOffset - newStartOffset);

                // Erase the sector once before writing the first page
                if (page == 0)
                {
                    if (cmai_flash_erase_sector (handle, sectorAddress, CMA_SECTOR_SIZE) != CMA_SECTOR_SIZE)
                        ret = CMA_STATUS_FAIL;
                    else
                        ret = CMA_STATUS_OK;
                }

                // Write updated page back to flash memory
                if (cmai_flash_write (handle, pageAddress, pageBuffer, CMA_PAGE_SIZE) == CMA_PAGE_SIZE)
                    ret = CMA_STATUS_OK;
                else
                    ret = CMA_STATUS_FAIL;
            }

            if (ret == CMA_STATUS_FAIL)
                break;
        }

        OS_FREE(pageBuffer);
    }

    OS_MUTEX_PUT(cma_flash_mutex);

    return ret;
}

CMA_STATUS_TYPE cma_flash_close(void *handle)
{
    UINT32 busmode;

    if (cma_flash_mutex == NULL || handle == NULL)
    {
        return CMA_STATUS_FAIL;
    }

    OS_MUTEX_GET(cma_flash_mutex, OS_MUTEX_FOREVER);

    busmode = SFLASH_BUS_3BADDR | SFLASH_BUS_144;
    SFLASH_IOCTL (handle, SFLASH_BUS_CONTROL, &busmode);
    SFLASH_CLOSE (handle);

    OS_MUTEX_PUT(cma_flash_mutex);

    da16x_environ_lock (FALSE);

    return CMA_STATUS_OK;
}
