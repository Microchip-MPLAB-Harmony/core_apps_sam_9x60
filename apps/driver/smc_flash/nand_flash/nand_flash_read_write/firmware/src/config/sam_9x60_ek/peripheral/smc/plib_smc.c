/*******************************************************************************
  Company:
    Microchip Technology Inc.
    Memory System Service - SMC Initialization File

  File Name:
    plib_smc.c

  Summary:
    Static Memory Controller, SMC.
    This file contains the source code to initialize the SMC_6498 controller

  Description:
    SMC configuration interface
    The SMC System Memory interface provides a simple interface to manage 8-bit
    and 16-bit parallel devices.

*******************************************************************************/

/*******************************************************************************
* Copyright (C) 2019 Microchip Technology Inc. and its subsidiaries.
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
#include "plib_smc.h"
#include "device.h"
#include <stddef.h>         // NULL

/* Address for transferring command bytes, CLE A22 */
#define COMMAND_ADDR    0x400000
/* Address for transferring address bytes, ALE A21 */
#define ADDRESS_ADDR    0x200000

static void PMECC_Initialize( void );
static void PMERRLOC_Initialize( void );

// SMC =========================================================================
/*  Function:
        void SMC_Initialize( void )

    Summary:
        Initializes hardware and data for the given instance of the SMC module.

    Description:
        This function initializes the SMC timings according to the external
        parallel device requirements.

    Returns:
        None
*/
void SMC_Initialize( void )
{
    SFR_REGS->SFR_CCFG_EBICSA |= SFR_CCFG_EBICSA_EBI_CS3A_Msk | SFR_CCFG_EBICSA_NFD0_ON_D16_Msk;

    // Write protection disable
    SMC_REGS->SMC_WPMR = SMC_WPMR_WPKEY_PASSWD;

    // Chip Select 3 --- --- --- --- --- --- --- --- --- --- --- --- --- --- ---
    // Configure SETUP register
    SMC_REGS->SMC_CS_NUMBER[ 3 ].SMC_SETUP = SMC_SETUP_NWE_SETUP( 2 )
                                          | SMC_SETUP_NCS_WR_SETUP( 2 )
                                          | SMC_SETUP_NRD_SETUP( 2 )
                                          | SMC_SETUP_NCS_RD_SETUP( 2 )
                                          ;
    // Configure CYCLE register
    SMC_REGS->SMC_CS_NUMBER[ 3 ].SMC_CYCLE = SMC_CYCLE_NWE_CYCLE( 13 )
                                          | SMC_CYCLE_NRD_CYCLE( 13 )
                                          ;
    // Configure PULSE register
    SMC_REGS->SMC_CS_NUMBER[ 3 ].SMC_PULSE = SMC_PULSE_NWE_PULSE( 7 )
                                          | SMC_PULSE_NCS_WR_PULSE( 7 )
                                          | SMC_PULSE_NRD_PULSE( 7 )
                                          | SMC_PULSE_NCS_RD_PULSE( 7 )
                                          ;
    // Configure MODE register
    SMC_REGS->SMC_CS_NUMBER[ 3 ].SMC_MODE = SMC_MODE_EXNW_MODE_DISABLED
                                          | SMC_MODE_TDF_MODE_Msk
                                          | SMC_MODE_TDF_CYCLES( 1 )
                                          | SMC_MODE_DBW_BIT_8
                                          | SMC_MODE_BAT_BYTE_SELECT
                                          | SMC_MODE_WRITE_MODE_NWE_CTRL
                                          | SMC_MODE_READ_MODE_NRD_CTRL
                                          ;
    //- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- ---
    // Tampering does not clear keys
    // Disable safety report interrupt
    SMC_REGS->SMC_SRIER &= ~SMC_SRIER_SRIE_Msk;

    PMECC_Initialize();
    PMERRLOC_Initialize();
    return;
}

uint32_t SMC_DataAddressGet(uint8_t chipSelect)
{
    uint32_t dataAddress = 0;

    switch (chipSelect)
    {
        case 0:
            dataAddress = EBI_CS0_ADDR;
            break;

        case 1:
            dataAddress = EBI_CS1_ADDR;
            break;

        case 2:
            dataAddress = EBI_CS2_ADDR;
            break;

        case 3:
            dataAddress = EBI_CS3_ADDR;
            break;

        case 4:
            dataAddress = EBI_CS4_ADDR;
            break;

        case 5:
            dataAddress = EBI_CS5_ADDR;
            break;

        default:
            break;
    }
    return dataAddress;
}

inline void SMC_CommandWrite(uint32_t dataAddress, uint8_t command)
{
    /* Send command */
    *((volatile uint8_t *)(dataAddress | COMMAND_ADDR)) = command;
}

inline void SMC_CommandWrite16(uint32_t dataAddress, uint16_t command)
{
    /* Send command */
    *((volatile uint16_t *)(dataAddress | COMMAND_ADDR)) = command;
}

inline void SMC_AddressWrite(uint32_t dataAddress, uint8_t address)
{
    /* Send address */
    *((volatile uint8_t *)(dataAddress | ADDRESS_ADDR)) = address;
}

inline void SMC_AddressWrite16(uint32_t dataAddress, uint16_t address)
{
    /* Send address */
    *((volatile uint16_t *)(dataAddress | ADDRESS_ADDR)) = address;
}

inline void SMC_DataWrite(uint32_t dataAddress, uint8_t data)
{
    *((volatile uint8_t *)dataAddress) = data;
}

inline void SMC_DataWrite16(uint32_t dataAddress, uint16_t data)
{
    *((volatile uint16_t *)dataAddress) = data;
}

inline uint8_t SMC_DataRead(uint32_t dataAddress)
{
    return *((volatile uint8_t *)dataAddress);
}

inline uint16_t SMC_DataRead16(uint32_t dataAddress)
{
    return *((volatile uint16_t *)dataAddress);
}

// PMECC =======================================================================
void PMECC_Initialize( void )
{
    // Disable then configure the PMECC module
    PMECC_REGS->PMECC_CTRL = PMECC_CTRL_DISABLE_Msk    // disable PMECC Module
                        | PMECC_CTRL_RST_Msk
                        ;
    // PMECC interrupt disable
    PMECC_REGS->PMECC_IDR = PMECC_IDR_ERRID_Msk;
    // Configuration register
    PMECC_REGS->PMECC_CFG = PMECC_CFG_PAGESIZE(0x3)
                        | PMECC_CFG_BCH_ERR(0x1)
                        | PMECC_CFG_AUTO_Msk
                        ;
    // Spare area size register
    PMECC_REGS->PMECC_SAREA = PMECC_SAREA_SPARESIZE( 57 ); // 56 bytes
    // Start address register
    PMECC_REGS->PMECC_SADDR = 2;
    // End address register
    PMECC_REGS->PMECC_EADDR = 57;
    // Clock register (value in cycles)
    PMECC_REGS->PMECC_CLK = 0;
    // PMECC interrupt left disabled
    // PMECC module enable
    PMECC_REGS->PMECC_CTRL = PMECC_CTRL_ENABLE_Msk;

    return;
}

void PMECC_DataPhaseStart(bool writeEnable)
{
    PMECC_REGS->PMECC_CTRL = PMECC_CTRL_RST_Msk;

    if (writeEnable)
    {
        PMECC_REGS->PMECC_CFG &= ~PMECC_CFG_AUTO_Msk;
        PMECC_REGS->PMECC_CFG |= PMECC_CFG_NANDWR_Msk;
    }
    else
    {
        PMECC_REGS->PMECC_CFG &= ~PMECC_CFG_NANDWR_Msk;
        if ((PMECC_REGS->PMECC_CFG & PMECC_CFG_SPAREEN_Msk) != PMECC_CFG_SPAREEN_Msk)
        {
            PMECC_REGS->PMECC_CFG |= PMECC_CFG_AUTO_Msk;
        }
    }

    PMECC_REGS->PMECC_CTRL = PMECC_CTRL_DATA_Msk;
}

bool PMECC_StatusIsBusy(void)
{
    return (PMECC_REGS->PMECC_SR & PMECC_SR_BUSY_Msk);
}

uint8_t PMECC_ErrorGet(void)
{
    return (PMECC_REGS->PMECC_ISR & PMECC_ISR_ERRIS_Msk);
}

int16_t PMECC_RemainderGet(uint32_t sector, uint32_t remainderIndex)
{
    return ((volatile int16_t *)PMECC_REGS->PMECC_REM[sector].PMECC_REM)[remainderIndex];
}

uint8_t PMECC_ECCGet(uint32_t sector, uint32_t byteIndex)
{
    return ((volatile uint8_t *)PMECC_REGS->PMECC_ECC[sector].PMECC_ECC)[byteIndex];
}

// PMERRLOC ====================================================================
uint32_t PMERRLOC_ErrorLocationGet(uint8_t position)
{
    return PMERRLOC_REGS->PMERRLOC_EL[position];
}

void PMERRLOC_ErrorLocationDisable(void)
{
    PMERRLOC_REGS->PMERRLOC_ELDIS = PMERRLOC_ELDIS_DIS_Msk;
}

void PMERRLOC_SigmaSet(uint32_t sigmaVal, uint32_t sigmaNum)
{
    *(volatile uint32_t *)(&PMERRLOC_REGS->PMERRLOC_SIGMA0 + sigmaNum) = sigmaVal;
}

uint32_t PMERRLOC_ErrorLocationFindNumOfRoots(uint32_t sectorSizeInBits, uint32_t errorNumber)
{
    /* Configure and enable error location process */
    PMERRLOC_REGS->PMERRLOC_ELCFG = (PMERRLOC_REGS->PMERRLOC_ELCFG & ~PMERRLOC_ELCFG_ERRNUM_Msk) | PMERRLOC_ELCFG_ERRNUM(errorNumber);
    PMERRLOC_REGS->PMERRLOC_ELEN = sectorSizeInBits;

    while ((PMERRLOC_REGS->PMERRLOC_ELISR & PMERRLOC_ELISR_DONE_Msk) == 0);

    return ((PMERRLOC_REGS->PMERRLOC_ELISR & PMERRLOC_ELISR_ERR_CNT_Msk) >> PMERRLOC_ELISR_ERR_CNT_Pos);
}

void PMERRLOC_Initialize( void )
{
    // Disable then configure the PMERRLOC module
    PMERRLOC_REGS->PMERRLOC_ELDIS = PMERRLOC_ELDIS_DIS_Msk;   // disable PMERRLOC Module
    // PMERRLOC interrupt disable
    PMERRLOC_REGS->PMERRLOC_ELIDR = PMERRLOC_ELIDR_DONE_Msk;

    // PMERRLOC interrupt left disabled

    return;
}

/*******************************************************************************
 End of File
*/
