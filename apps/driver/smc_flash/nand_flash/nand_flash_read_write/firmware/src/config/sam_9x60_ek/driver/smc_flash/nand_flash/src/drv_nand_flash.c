/******************************************************************************
  NAND FLASH Driver Implementation

  Company:
    Microchip Technology Inc.

  File Name:
    drv_nand_flash.c

  Summary:
    NAND FLASH Driver Interface Definition

  Description:
    The NAND FLASH Driver provides a interface to access the NAND FLASH peripheral on the SAM
    Devices. This file should be included in the project if NAND FLASH driver
    functionality is needed.
*******************************************************************************/

//DOM-IGNORE-BEGIN
/*******************************************************************************
* Copyright (C) 2020 Microchip Technology Inc. and its subsidiaries.
*
* Subject to your compliance with these terms, you may use Microchip software
* and any derivatives exclusively with Microchip products. It is your
* responsibility to comply with third party license terms applicable to your
* use of third party software (including open source software) that may
* accompany Microchip software.
*
* THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES, WHETHER
* EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED
* WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A
* PARTICULAR PURPOSE.
*
* IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE,
* INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND
* WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS
* BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE. TO THE
* FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN
* ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY,
* THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.
*******************************************************************************/
//DOM-IGNORE-END

// *****************************************************************************
// *****************************************************************************
// Section: Include Files
// *****************************************************************************
// *****************************************************************************

#include "driver/smc_flash/nand_flash/src/drv_nand_flash_local.h"
#include "driver/smc_flash/nand_flash/src/drv_nand_flash_pmecc.h"

// *****************************************************************************
// *****************************************************************************
// Section: Global objects
// *****************************************************************************
// *****************************************************************************

static DRV_NAND_FLASH_OBJECT gDrvNandFlashObj;
static DRV_NAND_FLASH_OBJECT *dObj = &gDrvNandFlashObj;
static DRV_NAND_FLASH_DATA gDrvNandFlashData;
static DRV_NAND_FLASH_DATA *dNandFlashData = &gDrvNandFlashData;

// *****************************************************************************
// *****************************************************************************
// Section: NAND FLASH Driver Local Functions
// *****************************************************************************
// *****************************************************************************

static void DRV_NAND_FLASH_DMA_CallbackHandler(SYS_DMA_TRANSFER_EVENT event, uintptr_t context)
{
    if (event == SYS_DMA_TRANSFER_COMPLETE)
    {
        dObj->transferStatus = DRV_NAND_FLASH_TRANSFER_COMPLETED;
    }
    else
    {
        dObj->transferStatus = DRV_NAND_FLASH_TRANSFER_ERROR_UNKNOWN;
    }
}

static void DRV_NAND_FLASH_ColumnAddressWrite(uint16_t columnAddress)
{
    uint16_t dataSize = dNandFlashData->nandFlashGeometry.pageSize;

    /* Check the data bus width */
    if (dNandFlashData->nandFlashGeometry.dataBusWidth == 16)
    {
        /* Divide by 2 for 16-bit address */
        columnAddress >>= 1;
    }

    /* Send column address */
    while (dataSize > 2)
    {
        if (dNandFlashData->nandFlashGeometry.dataBusWidth == 16)
        {
            dObj->nandFlashPlib->AddressWrite16(dNandFlashData->dataAddress, (columnAddress & 0xFF));
        }
        else
        {
            dObj->nandFlashPlib->AddressWrite(dNandFlashData->dataAddress, (columnAddress & 0xFF));
        }
        dataSize >>= 8;
        columnAddress >>= 8;
    }
}

static void DRV_NAND_FLASH_RowAddressWrite(uint32_t rowAddress)
{
    /* Calculate number of pages in Flash device */
    uint32_t numOfPages = dNandFlashData->nandFlashGeometry.deviceSize / dNandFlashData->nandFlashGeometry.pageSize;

    /* Send row address */
    while (numOfPages > 0)
    {
        if (dNandFlashData->nandFlashGeometry.dataBusWidth == 16)
        {
            dObj->nandFlashPlib->AddressWrite16(dNandFlashData->dataAddress, (rowAddress & 0xFF));
        }
        else
        {
            dObj->nandFlashPlib->AddressWrite(dNandFlashData->dataAddress, (rowAddress & 0xFF));
        }
        numOfPages >>= 8;
        rowAddress >>= 8;
    }
}

static void DRV_NAND_FLASH_DataWrite(uint32_t dataAddress, uint8_t *data, uint32_t size)
{
    uint32_t count = 0;
    uint16_t *data16 = 0;
    uint32_t dataSize16 = 0;

    if (dNandFlashData->nandFlashGeometry.dataBusWidth == 16)
    {
        data16 = (uint16_t *)data;
        dataSize16 = (size + 1) >> 1;

        /* Write page for 16-bit data bus */
        for (count = 0; count < dataSize16; count++)
        {
            dObj->nandFlashPlib->DataWrite16(dataAddress, data16[count]);
        }
    }
    else
    {
        /* Write page for 8-bit data bus */
        for (count = 0; count < size; count++)
        {
            dObj->nandFlashPlib->DataWrite(dataAddress, data[count]);
        }
    }
}

static void DRV_NAND_FLASH_DataRead(uint32_t dataAddress, uint8_t *data, uint32_t size)
{
    uint32_t count = 0;
    uint16_t *data16 = 0;
    uint32_t dataSize16 = 0;

    if (dNandFlashData->nandFlashGeometry.dataBusWidth == 16)
    {
        data16 = (uint16_t *)data;
        dataSize16 = (size + 1) >> 1;

        /* Read page for 16-bit data bus */
        for (count = 0; count < dataSize16; count++)
        {
            data16[count] = dObj->nandFlashPlib->DataRead16(dataAddress);
        }
    }
    else
    {
        /* Read page for 8-bit data bus */
        for (count = 0; count < size; count++)
        {
            data[count] = dObj->nandFlashPlib->DataRead(dataAddress);
        }
    }
}

static bool DRV_NAND_FLASH_PageRead(const DRV_HANDLE handle, uint16_t blockNum, uint16_t pageNum, uint8_t *data, uint8_t *spare)
{
    bool status = false;
    uint32_t columnAddress = 0;
    uint32_t rowAddress = 0;
    DRV_NAND_FLASH_TRANSFER_STATUS transferStatus = DRV_NAND_FLASH_TRANSFER_BUSY;

    if (handle == DRV_HANDLE_INVALID)
    {
        return status;
    }

    /* Row address of the page */
    rowAddress = blockNum * (dNandFlashData->nandFlashGeometry.blockSize / dNandFlashData->nandFlashGeometry.pageSize) + pageNum;

    /* Column address of the page */
    if (data)
    {
        columnAddress = 0;
    }
    else
    {
        columnAddress = dNandFlashData->nandFlashGeometry.pageSize;
    }

    /* Send read command for cycle 1 */
    dObj->nandFlashPlib->CommandWrite(dNandFlashData->dataAddress, NAND_FLASH_CMD_READ1);

    /* Send column address */
    DRV_NAND_FLASH_ColumnAddressWrite(columnAddress);

    /* Send row address */
    DRV_NAND_FLASH_RowAddressWrite(rowAddress);

    /* Send read page command for cycle 2 */
    dObj->nandFlashPlib->CommandWrite(dNandFlashData->dataAddress, NAND_FLASH_CMD_READ2);

    /* Read the ready status */
    while (transferStatus == DRV_NAND_FLASH_TRANSFER_BUSY)
    {
        transferStatus = DRV_NAND_FLASH_TransferStatusGet(handle);
    }

    if (transferStatus == DRV_NAND_FLASH_TRANSFER_COMPLETED)
    {
        /* Re-enable the data output mode by sending read mode command */
        dObj->nandFlashPlib->CommandWrite(dNandFlashData->dataAddress, NAND_FLASH_CMD_READ1);

        if (data)
        {
            if (DRV_NAND_FLASH_TX_RX_DMA_CH_IDX != SYS_DMA_CHANNEL_NONE)
            {
                SYS_DMA_ChannelTransfer(DRV_NAND_FLASH_TX_RX_DMA_CH_IDX,
                                       (const void *)dNandFlashData->dataAddress,
                                       (const void *)data,
                                        dNandFlashData->nandFlashGeometry.pageSize);

                dObj->transferStatus = DRV_NAND_FLASH_TRANSFER_BUSY;
                /* Wait for DMA transfer completion */
                while (dObj->transferStatus == DRV_NAND_FLASH_TRANSFER_BUSY);

                if (dObj->transferStatus != DRV_NAND_FLASH_TRANSFER_COMPLETED)
                {
                    return false;
                }
                /* Invalidate the data buffer to force the CPU to read from the main memory */
                SYS_CACHE_InvalidateDCache_by_Addr((uint32_t *)data, dNandFlashData->nandFlashGeometry.pageSize);
            }
            else
            {
                /* Read data page */
                DRV_NAND_FLASH_DataRead(dNandFlashData->dataAddress, data, dNandFlashData->nandFlashGeometry.pageSize);
            }
        }

        if (spare)
        {
            if (DRV_NAND_FLASH_TX_RX_DMA_CH_IDX != SYS_DMA_CHANNEL_NONE)
            {
                SYS_DMA_ChannelTransfer(DRV_NAND_FLASH_TX_RX_DMA_CH_IDX,
                                       (const void *)dNandFlashData->dataAddress,
                                       (const void *)spare,
                                        dNandFlashData->nandFlashGeometry.spareSize);

                dObj->transferStatus = DRV_NAND_FLASH_TRANSFER_BUSY;
                /* Wait for DMA transfer completion */
                while (dObj->transferStatus == DRV_NAND_FLASH_TRANSFER_BUSY);

                if (dObj->transferStatus != DRV_NAND_FLASH_TRANSFER_COMPLETED)
                {
                    return false;
                }
                /* Invalidate the spare buffer to force the CPU to read from the main memory */
                SYS_CACHE_InvalidateDCache_by_Addr((uint32_t *)spare, dNandFlashData->nandFlashGeometry.spareSize);
            }
            else
            {
                /* Read spare page */
                DRV_NAND_FLASH_DataRead(dNandFlashData->dataAddress, spare, dNandFlashData->nandFlashGeometry.spareSize);
            }
        }

        status = true;
    }

    return status;
}

static bool DRV_NAND_FLASH_PageWrite(const DRV_HANDLE handle, uint16_t blockNum, uint16_t pageNum, uint8_t *data, uint8_t *spare)
{
    uint32_t columnAddress = 0;
    uint32_t rowAddress = 0;
    DRV_NAND_FLASH_TRANSFER_STATUS transferStatus = DRV_NAND_FLASH_TRANSFER_BUSY;
    uint32_t spareDataAddress = 0;
    bool status = false;

    if (handle == DRV_HANDLE_INVALID)
    {
        return status;
    }

    /* Row address of the page */
    rowAddress = blockNum * (dNandFlashData->nandFlashGeometry.blockSize / dNandFlashData->nandFlashGeometry.pageSize) + pageNum;

    /* Column address of the page */
    if (data)
    {
        columnAddress = 0;
        spareDataAddress = dNandFlashData->dataAddress + dNandFlashData->nandFlashGeometry.pageSize;
    }
    else
    {
        columnAddress = dNandFlashData->nandFlashGeometry.pageSize;
        spareDataAddress = dNandFlashData->dataAddress;
    }

    /* Send page program command for cycle 1 */
    dObj->nandFlashPlib->CommandWrite(dNandFlashData->dataAddress, NAND_FLASH_CMD_PAGE_PROGRAM1);

    /* Send column address */
    DRV_NAND_FLASH_ColumnAddressWrite(columnAddress);

    /* Send row address */
    DRV_NAND_FLASH_RowAddressWrite(rowAddress);

    if (data)
    {
        if (DRV_NAND_FLASH_TX_RX_DMA_CH_IDX != SYS_DMA_CHANNEL_NONE)
        {
            /* Clean the data buffer to push the data to the main memory */
            SYS_CACHE_CleanDCache_by_Addr((uint32_t *)data, dNandFlashData->nandFlashGeometry.pageSize);
            SYS_DMA_ChannelTransfer(DRV_NAND_FLASH_TX_RX_DMA_CH_IDX,
                                   (const void *)data,
                                   (const void *)dNandFlashData->dataAddress,
                                    dNandFlashData->nandFlashGeometry.pageSize);

            dObj->transferStatus = DRV_NAND_FLASH_TRANSFER_BUSY;
            /* Wait for DMA transfer completion */
            while (dObj->transferStatus == DRV_NAND_FLASH_TRANSFER_BUSY);

            if (dObj->transferStatus != DRV_NAND_FLASH_TRANSFER_COMPLETED)
            {
                return false;
            }
        }
        else
        {
            /* Write data page */
            DRV_NAND_FLASH_DataWrite(dNandFlashData->dataAddress, data, dNandFlashData->nandFlashGeometry.pageSize);
        }
    }

    if (spare)
    {
        if (DRV_NAND_FLASH_TX_RX_DMA_CH_IDX != SYS_DMA_CHANNEL_NONE)
        {
            /* Clean the spare buffer to push the data to the main memory */
            SYS_CACHE_CleanDCache_by_Addr((uint32_t *)spare, dNandFlashData->nandFlashGeometry.spareSize);
            SYS_DMA_ChannelTransfer(DRV_NAND_FLASH_TX_RX_DMA_CH_IDX,
                                   (const void *)spare,
                                   (const void *)spareDataAddress,
                                    dNandFlashData->nandFlashGeometry.spareSize);

            dObj->transferStatus = DRV_NAND_FLASH_TRANSFER_BUSY;
            /* Wait for DMA transfer completion */
            while (dObj->transferStatus == DRV_NAND_FLASH_TRANSFER_BUSY);

            if (dObj->transferStatus != DRV_NAND_FLASH_TRANSFER_COMPLETED)
            {
                return false;
            }
        }
        else
        {
            /* Write spare page */
            DRV_NAND_FLASH_DataWrite(spareDataAddress, spare, dNandFlashData->nandFlashGeometry.spareSize);
        }
    }

    /* Send page program command for cycle 2 */
    dObj->nandFlashPlib->CommandWrite(dNandFlashData->dataAddress, NAND_FLASH_CMD_PAGE_PROGRAM2);

    /* Read the ready status */
    while (transferStatus == DRV_NAND_FLASH_TRANSFER_BUSY)
    {
        transferStatus = DRV_NAND_FLASH_TransferStatusGet(handle);
    }

    if (transferStatus == DRV_NAND_FLASH_TRANSFER_COMPLETED)
    {
        status = true;
    }

    return status;
}

static bool DRV_NAND_FLASH_PageReadPmecc(const DRV_HANDLE handle, uint16_t blockNum, uint16_t pageNum, uint8_t *data)
{
    bool status = false;
    uint32_t columnAddress = 0;
    uint32_t rowAddress = 0;
    DRV_NAND_FLASH_TRANSFER_STATUS transferStatus = DRV_NAND_FLASH_TRANSFER_BUSY;

    if (handle == DRV_HANDLE_INVALID)
    {
        return status;
    }

    /* Row address of the page */
    rowAddress = blockNum * (dNandFlashData->nandFlashGeometry.blockSize / dNandFlashData->nandFlashGeometry.pageSize) + pageNum;

    /* Send read command for cycle 1 */
    dObj->nandFlashPlib->CommandWrite(dNandFlashData->dataAddress, NAND_FLASH_CMD_READ1);

    /* Send column address */
    DRV_NAND_FLASH_ColumnAddressWrite(columnAddress);

    /* Send row address */
    DRV_NAND_FLASH_RowAddressWrite(rowAddress);

    /* Send read page command for cycle 2 */
    dObj->nandFlashPlib->CommandWrite(dNandFlashData->dataAddress, NAND_FLASH_CMD_READ2);

    /* Read the ready status */
    while (transferStatus == DRV_NAND_FLASH_TRANSFER_BUSY)
    {
        transferStatus = DRV_NAND_FLASH_TransferStatusGet(handle);
    }

    if (transferStatus == DRV_NAND_FLASH_TRANSFER_COMPLETED)
    {
        /* Re-enable the data output mode by sending read mode command */
        dObj->nandFlashPlib->CommandWrite(dNandFlashData->dataAddress, NAND_FLASH_CMD_READ1);

        /* Enable Read access and start data phase */
        dObj->nandFlashPlib->DataPhaseStart(0);

        if (DRV_NAND_FLASH_TX_RX_DMA_CH_IDX != SYS_DMA_CHANNEL_NONE)
        {
            SYS_DMA_ChannelTransfer(DRV_NAND_FLASH_TX_RX_DMA_CH_IDX,
                                   (const void *)dNandFlashData->dataAddress,
                                   (const void *)data,
                                   (dNandFlashData->nandFlashGeometry.pageSize + DRV_NAND_FLASH_PMECC_ECC_START_ADDR + DRV_NAND_FLASH_PMECC_ECC_SPARE_SIZE));

            dObj->transferStatus = DRV_NAND_FLASH_TRANSFER_BUSY;
            /* Wait for DMA transfer completion */
            while (dObj->transferStatus == DRV_NAND_FLASH_TRANSFER_BUSY);

            if (dObj->transferStatus != DRV_NAND_FLASH_TRANSFER_COMPLETED)
            {
                return false;
            }
            /* Invalidate the data buffer to force the CPU to read from the main memory */
            SYS_CACHE_InvalidateDCache_by_Addr((uint32_t *)data, (dNandFlashData->nandFlashGeometry.pageSize + DRV_NAND_FLASH_PMECC_ECC_START_ADDR + DRV_NAND_FLASH_PMECC_ECC_SPARE_SIZE));
        }
        else
        {
            /* Read data page */
            DRV_NAND_FLASH_DataRead(dNandFlashData->dataAddress, data,
            (dNandFlashData->nandFlashGeometry.pageSize + DRV_NAND_FLASH_PMECC_ECC_START_ADDR + DRV_NAND_FLASH_PMECC_ECC_SPARE_SIZE));
        }

        /* Wait until PMECC is not busy */
        while (dObj->nandFlashPlib->StatusIsBusy() == true);

        status = true;
    }

    return status;
}

static bool DRV_NAND_FLASH_PageReadWithPMECC(const DRV_HANDLE handle, uint16_t blockNum, uint16_t pageNum, uint8_t *data)
{
    bool status = false;
    uint32_t pmeccErrorStatus = 0;
    uint32_t count = 0;

    if (handle == DRV_HANDLE_INVALID)
    {
        return status;
    }

    /* Read Page */
    if (!DRV_NAND_FLASH_PageReadPmecc(handle, blockNum, pageNum, data))
    {
        return status;
    }

    /* Check PMECC Error */
    pmeccErrorStatus = dObj->nandFlashPlib->ErrorGet();
    if (pmeccErrorStatus)
    {
        /* Check if spare area is erased */
        DRV_NAND_FLASH_PageRead(handle, blockNum, pageNum, NULL, dNandFlashData->spareBuffer);

        for (count = 0; count < dNandFlashData->nandFlashGeometry.spareSize; count++)
        {
            if (dNandFlashData->spareBuffer[count] != 0xFF)
            {
                break;
            }
        }
        if (count == dNandFlashData->nandFlashGeometry.spareSize)
        {
            pmeccErrorStatus = 0;
        }
    }

    /* Perform bit correction in data buffer */
    if (pmeccErrorStatus && (DRV_NAND_FLASH_PmeccCorrection(pmeccErrorStatus, (uint32_t)data) == false))
    {
        status = false;
    }
    else
    {
        status = true;
    }
    return status;
}

static bool DRV_NAND_FLASH_PageWriteWithPMECC(const DRV_HANDLE handle, uint16_t blockNum, uint16_t pageNum, uint8_t *data)
{
    uint32_t columnAddress = 0;
    uint32_t rowAddress = 0;
    DRV_NAND_FLASH_TRANSFER_STATUS transferStatus = DRV_NAND_FLASH_TRANSFER_BUSY;
    uint32_t count = 0;
    uint32_t byteIndex = 0;
    uint32_t eccStartAddr = 0;
    bool status = false;

    if (handle == DRV_HANDLE_INVALID)
    {
        return status;
    }

    /* Row address of the page */
    rowAddress = blockNum * (dNandFlashData->nandFlashGeometry.blockSize / dNandFlashData->nandFlashGeometry.pageSize) + pageNum;

    /* ECC start address of the page */
    eccStartAddr = dNandFlashData->nandFlashGeometry.pageSize + DRV_NAND_FLASH_PMECC_ECC_START_ADDR;

    /* Enable Write access and start data phase */
    dObj->nandFlashPlib->DataPhaseStart(1);

    /* Send page program command for cycle 1 */
    dObj->nandFlashPlib->CommandWrite(dNandFlashData->dataAddress, NAND_FLASH_CMD_PAGE_PROGRAM1);

    /* Send column address */
    DRV_NAND_FLASH_ColumnAddressWrite(columnAddress);

    /* Send row address */
    DRV_NAND_FLASH_RowAddressWrite(rowAddress);

    if (DRV_NAND_FLASH_TX_RX_DMA_CH_IDX != SYS_DMA_CHANNEL_NONE)
    {
        /* Clean the data buffer to push the data to the main memory */
        SYS_CACHE_CleanDCache_by_Addr((uint32_t *)data, dNandFlashData->nandFlashGeometry.pageSize);
        SYS_DMA_ChannelTransfer(DRV_NAND_FLASH_TX_RX_DMA_CH_IDX,
                               (const void *)data,
                               (const void *)dNandFlashData->dataAddress,
                                dNandFlashData->nandFlashGeometry.pageSize);

        dObj->transferStatus = DRV_NAND_FLASH_TRANSFER_BUSY;
        /* Wait for DMA transfer completion */
        while (dObj->transferStatus == DRV_NAND_FLASH_TRANSFER_BUSY);

        if (dObj->transferStatus != DRV_NAND_FLASH_TRANSFER_COMPLETED)
        {
            return false;
        }
    }
    else
    {
        /* Write data page */
        DRV_NAND_FLASH_DataWrite(dNandFlashData->dataAddress, data, dNandFlashData->nandFlashGeometry.pageSize);
    }

    /* Send change write column (Random data input) command for cycle 1 */
    dObj->nandFlashPlib->CommandWrite(dNandFlashData->dataAddress, NAND_FLASH_CMD_CHANGE_WRITE_COLUMN);

    /* Send ECC start address */
    DRV_NAND_FLASH_ColumnAddressWrite(eccStartAddr);

    /* Wait until PMECC is not busy */
    while (dObj->nandFlashPlib->StatusIsBusy() == true);

    /* Read all ECC registers */
    for (count = 0; count < DRV_NAND_FLASH_PMECC_NUMBER_OF_SECTORS; count++)
    {
        for (byteIndex = 0; byteIndex < (DRV_NAND_FLASH_PMECC_ECC_SPARE_SIZE / DRV_NAND_FLASH_PMECC_NUMBER_OF_SECTORS); byteIndex++)
        {
            dNandFlashData->spareBuffer[count * (DRV_NAND_FLASH_PMECC_ECC_SPARE_SIZE / DRV_NAND_FLASH_PMECC_NUMBER_OF_SECTORS) + byteIndex] =
            dObj->nandFlashPlib->ECCGet(count, byteIndex);
        }
    }

    if (DRV_NAND_FLASH_TX_RX_DMA_CH_IDX != SYS_DMA_CHANNEL_NONE)
    {
        /* Clean the data buffer to push the data to the main memory */
        SYS_CACHE_CleanDCache_by_Addr((uint32_t *)dNandFlashData->spareBuffer, DRV_NAND_FLASH_PMECC_ECC_SPARE_SIZE);
        SYS_DMA_ChannelTransfer(DRV_NAND_FLASH_TX_RX_DMA_CH_IDX,
                               (const void *)dNandFlashData->spareBuffer,
                               (const void *)dNandFlashData->dataAddress,
                                DRV_NAND_FLASH_PMECC_ECC_SPARE_SIZE);

        dObj->transferStatus = DRV_NAND_FLASH_TRANSFER_BUSY;
        /* Wait for DMA transfer completion */
        while (dObj->transferStatus == DRV_NAND_FLASH_TRANSFER_BUSY);

        if (dObj->transferStatus != DRV_NAND_FLASH_TRANSFER_COMPLETED)
        {
            return false;
        }
    }
    else
    {
        /* Write spare page */
        DRV_NAND_FLASH_DataWrite(dNandFlashData->dataAddress, dNandFlashData->spareBuffer, DRV_NAND_FLASH_PMECC_ECC_SPARE_SIZE);
    }

    /* Send page program command for cycle 2 */
    dObj->nandFlashPlib->CommandWrite(dNandFlashData->dataAddress, NAND_FLASH_CMD_PAGE_PROGRAM2);

    /* Read the ready status */
    while (transferStatus == DRV_NAND_FLASH_TRANSFER_BUSY)
    {
        transferStatus = DRV_NAND_FLASH_TransferStatusGet(handle);
    }

    if (transferStatus == DRV_NAND_FLASH_TRANSFER_COMPLETED)
    {
        status = true;
    }

    return status;
}

// *****************************************************************************
// *****************************************************************************
// Section: NAND FLASH Driver Global Functions
// *****************************************************************************
// *****************************************************************************

bool DRV_NAND_FLASH_ResetFlash(const DRV_HANDLE handle)
{
    DRV_NAND_FLASH_TRANSFER_STATUS transferStatus = DRV_NAND_FLASH_TRANSFER_BUSY;

    if (handle == DRV_HANDLE_INVALID)
    {
        return false;
    }

    dObj->nandFlashPlib->CommandWrite(dNandFlashData->dataAddress, NAND_FLASH_CMD_RESET);

    /* Read the ready status */
    while (transferStatus == DRV_NAND_FLASH_TRANSFER_BUSY)
    {
        transferStatus = DRV_NAND_FLASH_TransferStatusGet(handle);
    }

    if (transferStatus != DRV_NAND_FLASH_TRANSFER_COMPLETED)
    {
        return false;
    }

    return true;
}

DRV_NAND_FLASH_TRANSFER_STATUS DRV_NAND_FLASH_TransferStatusGet(const DRV_HANDLE handle)
{
    uint8_t reg_status = 0;
    DRV_NAND_FLASH_TRANSFER_STATUS transferStatus = DRV_NAND_FLASH_TRANSFER_ERROR_UNKNOWN;

    if (handle == DRV_HANDLE_INVALID)
    {
        return transferStatus;
    }

    dObj->nandFlashPlib->CommandWrite(dNandFlashData->dataAddress, NAND_FLASH_CMD_READ_STATUS);

    reg_status = dObj->nandFlashPlib->DataRead(dNandFlashData->dataAddress);

    // Check Ready bit
    if (reg_status & (1 << 6))
    {
        // Check Fail bit
        if (reg_status & (1 << 0))
        {
            transferStatus = DRV_NAND_FLASH_TRANSFER_FAIL;
        }
        else
        {
            transferStatus = DRV_NAND_FLASH_TRANSFER_COMPLETED;
        }
    }
    else
    {
        transferStatus = DRV_NAND_FLASH_TRANSFER_BUSY;
    }

    return transferStatus;
}

bool DRV_NAND_FLASH_IdRead(const DRV_HANDLE handle, uint32_t *readId, uint8_t address)
{
    uint8_t *data = (uint8_t *)readId;
    uint8_t count = 0;

    if (handle == DRV_HANDLE_INVALID)
    {
        return false;
    }

    /* Send Read ID command */
    dObj->nandFlashPlib->CommandWrite(dNandFlashData->dataAddress, NAND_FLASH_CMD_READ_ID);

    /* Send Address */
    dObj->nandFlashPlib->AddressWrite(dNandFlashData->dataAddress, address);

    /* Read data */
    for (count = 0; count < sizeof(*readId); count++)
    {
        data[count] = dObj->nandFlashPlib->DataRead(dNandFlashData->dataAddress);
    }

    return true;
}

bool DRV_NAND_FLASH_FeatureSet(const DRV_HANDLE handle, uint8_t *featureData, uint8_t featureDataSize, uint8_t featureAddress)
{
    uint8_t count = 0;

    if (handle == DRV_HANDLE_INVALID)
    {
        return false;
    }

    /* Send set feature command */
    dObj->nandFlashPlib->CommandWrite(dNandFlashData->dataAddress, NAND_FLASH_CMD_SET_FEATURES);

    /* Send Address */
    dObj->nandFlashPlib->AddressWrite(dNandFlashData->dataAddress, featureAddress);

    /* Set features data */
    for (count = 0; count < featureDataSize; count++)
    {
        dObj->nandFlashPlib->DataWrite(dNandFlashData->dataAddress, featureData[count]);
    }

    return true;
}

bool DRV_NAND_FLASH_FeatureGet(const DRV_HANDLE handle, uint8_t *featureData, uint8_t featureDataSize, uint8_t featureAddress)
{
    uint8_t count = 0;

    if (handle == DRV_HANDLE_INVALID)
    {
        return false;
    }

    /* Send get feature command */
    dObj->nandFlashPlib->CommandWrite(dNandFlashData->dataAddress, NAND_FLASH_CMD_GET_FEATURES);

    /* Send Address */
    dObj->nandFlashPlib->AddressWrite(dNandFlashData->dataAddress, featureAddress);

    /* Get features data */
    for (count = 0; count < featureDataSize; count++)
    {
        featureData[count] = dObj->nandFlashPlib->DataRead(dNandFlashData->dataAddress);
    }

    return true;
}

bool DRV_NAND_FLASH_ParameterPageRead(const DRV_HANDLE handle, uint8_t *parameterPage, uint32_t size)
{
    DRV_NAND_FLASH_TRANSFER_STATUS transferStatus = DRV_NAND_FLASH_TRANSFER_BUSY;
    uint32_t count = 0;

    if (handle == DRV_HANDLE_INVALID)
    {
        return false;
    }

    if (size > 256)
    {
        size = 256;
    }

    /* Send Read Parameter Page command */
    dObj->nandFlashPlib->CommandWrite(dNandFlashData->dataAddress, NAND_FLASH_CMD_READ_PARAMETER_PAGE);

    /* Send Address */
    dObj->nandFlashPlib->AddressWrite(dNandFlashData->dataAddress, 0x00);

    /* Read the ready status */
    while (transferStatus == DRV_NAND_FLASH_TRANSFER_BUSY)
    {
        transferStatus = DRV_NAND_FLASH_TransferStatusGet(handle);
    }

    if (transferStatus == DRV_NAND_FLASH_TRANSFER_COMPLETED)
    {
        /* Re-enable the data output mode by sending read mode command */
        dObj->nandFlashPlib->CommandWrite(dNandFlashData->dataAddress, NAND_FLASH_CMD_READ1);

        /* Read Parameter Page */
        for (count = 0; count < size; count++)
        {
            parameterPage[count] = dObj->nandFlashPlib->DataRead(dNandFlashData->dataAddress);
        }
    }
    else
    {
        return false;
    }

    return true;
}

bool DRV_NAND_FLASH_GeometryGet(const DRV_HANDLE handle, DRV_NAND_FLASH_GEOMETRY *geometry)
{
    uint32_t numOfPages = 0;
    uint32_t numOfBlocks = 0;
    uint8_t flashParameter[116];
    bool status = false;

    if (handle == DRV_HANDLE_INVALID)
    {
        return status;
    }

    if (DRV_NAND_FLASH_ParameterPageRead(handle, flashParameter, sizeof(flashParameter)))
    {
        /* JEDEC Manufacturer ID */
        geometry->deviceId = flashParameter[64];

        /* Bus Width */
        geometry->dataBusWidth = (flashParameter[6] & 0x01) ? 16 : 8;

        /* Get number of data bytes per page */
        memcpy(&geometry->pageSize, &flashParameter[80], 4);

        /* Get number of spare bytes per page */
        memcpy(&geometry->spareSize, &flashParameter[84], 2);

        /* Get Block Size */
        memcpy(&numOfPages, &flashParameter[92], 4);
        geometry->blockSize = geometry->pageSize * numOfPages;

        /* Get Device Size */
        memcpy(&numOfBlocks, &flashParameter[96], 4);
        geometry->deviceSize = geometry->blockSize * numOfBlocks;

        /* Get number of logical units */
        geometry->numberOfLogicalUnits = flashParameter[100];

        /* Get number of bits of ECC correction */
        geometry->eccCorrectability = flashParameter[112];

        status = true;
    }

    return status;
}

bool DRV_NAND_FLASH_SkipBlock_BlockCheck(const DRV_HANDLE handle, uint16_t blockNum)
{
    uint8_t marker[2];
    bool status = false;

    if (handle == DRV_HANDLE_INVALID)
    {
        return status;
    }

    if (DRV_NAND_FLASH_PageRead(handle, blockNum, 0, NULL, dNandFlashData->spareBuffer))
    {
        marker[0] = dNandFlashData->spareBuffer[0];
        if (DRV_NAND_FLASH_PageRead(handle, blockNum, 1, NULL, dNandFlashData->spareBuffer))
        {
            marker[1] = dNandFlashData->spareBuffer[0];
            if (marker[0] == 0xFF && marker[1] == 0xFF)
            {
                /* Good block */
                status = true;
            }
        }
    }

    return status;
}

bool DRV_NAND_FLASH_SkipBlock_BlockErase(const DRV_HANDLE handle, uint16_t blockNum, bool disableBlockCheck)
{
    DRV_NAND_FLASH_TRANSFER_STATUS transferStatus = DRV_NAND_FLASH_TRANSFER_BUSY;
    uint32_t rowAddress = 0;
    bool status = false;

    if (handle == DRV_HANDLE_INVALID)
    {
        return status;
    }

    if (!disableBlockCheck)
    {
        if (!DRV_NAND_FLASH_SkipBlock_BlockCheck(handle, blockNum))
        {
            return status;
        }
    }

    /* Send block erase command for cycle 1 */
    dObj->nandFlashPlib->CommandWrite(dNandFlashData->dataAddress, NAND_FLASH_CMD_BLOCK_ERASE1);

    /* Calculate row address used for erase */
    rowAddress = blockNum * (dNandFlashData->nandFlashGeometry.blockSize / dNandFlashData->nandFlashGeometry.pageSize);

    /* Send row address */
    DRV_NAND_FLASH_RowAddressWrite(rowAddress);

    /* Send block erase command for cycle 2 */
    dObj->nandFlashPlib->CommandWrite(dNandFlashData->dataAddress, NAND_FLASH_CMD_BLOCK_ERASE2);

    /* Read the ready status */
    while (transferStatus == DRV_NAND_FLASH_TRANSFER_BUSY)
    {
        transferStatus = DRV_NAND_FLASH_TransferStatusGet(handle);
    }

    if (transferStatus == DRV_NAND_FLASH_TRANSFER_COMPLETED)
    {
        status = true;
    }

    return status;
}

bool DRV_NAND_FLASH_SkipBlock_BlockTag(const DRV_HANDLE handle, uint16_t blockNum, bool badBlock)
{
    bool status = false;
    uint8_t count = 0;

    if (handle == DRV_HANDLE_INVALID)
    {
        return status;
    }

    status = DRV_NAND_FLASH_SkipBlock_BlockCheck(handle, blockNum);

    if ((badBlock && (status == false)) || ((badBlock == false) && status))
    {
        return true;
    }

    status = DRV_NAND_FLASH_SkipBlock_BlockErase(handle, blockNum, true);
    if (status)
    {
        if (badBlock)
        {
            /* Tag bad block */
            memset(dNandFlashData->spareBuffer, 0xFF, sizeof(dNandFlashData->spareBuffer));
            dNandFlashData->spareBuffer[0] = 0xDE;

            for (count = 0; count < 2; count++)
            {
                status = DRV_NAND_FLASH_PageWrite(handle, blockNum, count, NULL, dNandFlashData->spareBuffer);
                if (status == false)
                {
                    return status;
                }
                dNandFlashData->spareBuffer[0] = 0xAD;
            }
        }
        else
        {
            /* Tag good block */
            status = DRV_NAND_FLASH_SkipBlock_BlockCheck(handle, blockNum);
        }
    }

    return status;
}

bool DRV_NAND_FLASH_SkipBlock_PageRead(const DRV_HANDLE handle, uint16_t blockNum, uint16_t pageNum, uint8_t *data, uint8_t *spare, bool disableBlockCheck)
{
    if (handle == DRV_HANDLE_INVALID)
    {
        return false;
    }

    if (!disableBlockCheck)
    {
        if (!DRV_NAND_FLASH_SkipBlock_BlockCheck(handle, blockNum))
        {
            return false;
        }
    }

    if ((DRV_NAND_FLASH_ENABLE_PMECC == 0) || spare)
    {
        if (!DRV_NAND_FLASH_PageRead(handle, blockNum, pageNum, data, spare))
        {
            return false;
        }
    }
    else
    {
        if (!DRV_NAND_FLASH_PageReadWithPMECC(handle, blockNum, pageNum, data))
        {
            return false;
        }
    }

    return true;
}

bool DRV_NAND_FLASH_SkipBlock_BlockRead(const DRV_HANDLE handle, uint16_t blockNum, uint8_t *data, bool disableBlockCheck)
{
    uint32_t count = 0;

    if (handle == DRV_HANDLE_INVALID)
    {
        return false;
    }

    if (!disableBlockCheck)
    {
        if (!DRV_NAND_FLASH_SkipBlock_BlockCheck(handle, blockNum))
        {
            return false;
        }
    }

    for (count = 0; count < (dNandFlashData->nandFlashGeometry.blockSize / dNandFlashData->nandFlashGeometry.pageSize); count++)
    {
        if (DRV_NAND_FLASH_ENABLE_PMECC == 0)
        {
            if (!DRV_NAND_FLASH_PageRead(handle, blockNum, count, data, 0))
            {
                return false;
            }
        }
        else
        {
            if (!DRV_NAND_FLASH_PageReadWithPMECC(handle, blockNum, count, data))
            {
                return false;
            }
        }
        data = data + dNandFlashData->nandFlashGeometry.pageSize;
    }

    return true;
}

bool DRV_NAND_FLASH_SkipBlock_PageWrite(const DRV_HANDLE handle, uint16_t blockNum, uint16_t pageNum, uint8_t *data, uint8_t *spare, bool disableBlockCheck)
{
    if (handle == DRV_HANDLE_INVALID)
    {
        return false;
    }

    if (!disableBlockCheck)
    {
        if (!DRV_NAND_FLASH_SkipBlock_BlockCheck(handle, blockNum))
        {
            return false;
        }
    }

    if ((DRV_NAND_FLASH_ENABLE_PMECC == 0) || spare)
    {
        if (!DRV_NAND_FLASH_PageWrite(handle, blockNum, pageNum, data, spare))
        {
            return false;
        }
    }
    else
    {
        if (!DRV_NAND_FLASH_PageWriteWithPMECC(handle, blockNum, pageNum, data))
        {
            return false;
        }
    }

    return true;
}

bool DRV_NAND_FLASH_SkipBlock_BlockWrite(const DRV_HANDLE handle, uint16_t blockNum, uint8_t *data, bool disableBlockCheck)
{
    uint32_t count = 0;

    if (handle == DRV_HANDLE_INVALID)
    {
        return false;
    }

    if (!disableBlockCheck)
    {
        if (!DRV_NAND_FLASH_SkipBlock_BlockCheck(handle, blockNum))
        {
            return false;
        }
    }

    for (count = 0; count < (dNandFlashData->nandFlashGeometry.blockSize / dNandFlashData->nandFlashGeometry.pageSize); count++)
    {
        if (DRV_NAND_FLASH_ENABLE_PMECC == 0)
        {
            if (!DRV_NAND_FLASH_PageWrite(handle, blockNum, count, data, 0))
            {
                return false;
            }
        }
        else
        {
            if (!DRV_NAND_FLASH_PageWriteWithPMECC(handle, blockNum, count, data))
            {
                return false;
            }
        }
        data = data + dNandFlashData->nandFlashGeometry.pageSize;
    }

    return true;
}

DRV_HANDLE DRV_NAND_FLASH_Open( const SYS_MODULE_INDEX drvIndex, const DRV_IO_INTENT ioIntent )
{
    DRV_NAND_FLASH_GEOMETRY geometry;

    if ((dObj->status != SYS_STATUS_READY) ||
        (dObj->nClients >= DRV_NAND_FLASH_CLIENTS_NUMBER))
    {
        return DRV_HANDLE_INVALID;
    }

    /* Reset NAND Flash */
    if (!DRV_NAND_FLASH_ResetFlash((DRV_HANDLE)drvIndex))
    {
        return DRV_HANDLE_INVALID;
    }

    /* Store the NAND Flash data */
    if (DRV_NAND_FLASH_GeometryGet((DRV_HANDLE)drvIndex, &geometry))
    {
        dNandFlashData->nandFlashGeometry.deviceId = geometry.deviceId;
        dNandFlashData->nandFlashGeometry.dataBusWidth = geometry.dataBusWidth;
        dNandFlashData->nandFlashGeometry.pageSize = geometry.pageSize;
        dNandFlashData->nandFlashGeometry.spareSize = geometry.spareSize;
        dNandFlashData->nandFlashGeometry.blockSize = geometry.blockSize;
        dNandFlashData->nandFlashGeometry.deviceSize = geometry.deviceSize;
        dNandFlashData->nandFlashGeometry.numberOfLogicalUnits = geometry.numberOfLogicalUnits;
        dNandFlashData->nandFlashGeometry.eccCorrectability = geometry.eccCorrectability;
        if (!DRV_NAND_FLASH_PmeccDescSetup(geometry.pageSize, geometry.spareSize, dObj))
        {
            return DRV_HANDLE_INVALID;
        }
        /* Setup data bus width with DMA System Service */
        if (DRV_NAND_FLASH_TX_RX_DMA_CH_IDX != SYS_DMA_CHANNEL_NONE)
        {
            if (dNandFlashData->nandFlashGeometry.dataBusWidth == 16)
            {
                SYS_DMA_DataWidthSetup(DRV_NAND_FLASH_TX_RX_DMA_CH_IDX, SYS_DMA_WIDTH_16_BIT);
            }
            else
            {
                SYS_DMA_DataWidthSetup(DRV_NAND_FLASH_TX_RX_DMA_CH_IDX, SYS_DMA_WIDTH_8_BIT);
            }
        }
    }
    else
    {
        return DRV_HANDLE_INVALID;
    }

    dObj->nClients++;

    dObj->ioIntent = ioIntent;

    return ((DRV_HANDLE)drvIndex);
}

void DRV_NAND_FLASH_Close( const DRV_HANDLE handle )
{
    if ( (handle != DRV_HANDLE_INVALID) &&
         (dObj->nClients > 0))
    {
        dObj->nClients--;
    }
}

SYS_MODULE_OBJ DRV_NAND_FLASH_Initialize
(
    const SYS_MODULE_INDEX drvIndex,
    const SYS_MODULE_INIT *const init
)
{
    DRV_NAND_FLASH_INIT *nandFlashInit = NULL;

    /* Check if the instance has already been initialized. */
    if (dObj->inUse)
    {
        return SYS_MODULE_OBJ_INVALID;
    }

    dObj->status = SYS_STATUS_UNINITIALIZED;

    /* Indicate that this object is in use */
    dObj->inUse = true;
    dObj->nClients  = 0;

    /* Assign to the local pointer the init data passed */
    nandFlashInit = (DRV_NAND_FLASH_INIT *)init;

    /* Initialize the attached memory device functions */
    dObj->nandFlashPlib = nandFlashInit->nandFlashPlib;

    /* Get the data address of NAND Flash */
    dNandFlashData->dataAddress = dObj->nandFlashPlib->DataAddressGet(DRV_NAND_FLASH_CS);

    dObj->transferStatus  = DRV_NAND_FLASH_TRANSFER_COMPLETED;

    /* Register call-backs with the DMA System Service */
    if (DRV_NAND_FLASH_TX_RX_DMA_CH_IDX != SYS_DMA_CHANNEL_NONE)
    {
        SYS_DMA_ChannelCallbackRegister(DRV_NAND_FLASH_TX_RX_DMA_CH_IDX, DRV_NAND_FLASH_DMA_CallbackHandler, 0);
    }

    dObj->status = SYS_STATUS_READY;

    /* Return the driver index */
    return drvIndex;
}

SYS_STATUS DRV_NAND_FLASH_Status( const SYS_MODULE_INDEX drvIndex )
{
    /* Return the driver status */
    return (gDrvNandFlashObj.status);
}
