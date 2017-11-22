/* --------------------------------------------------------------------------
  FILE        : spi_memboot.c                                                   
  PROJECT     : TI Booting and Flashing Utilities
  AUTHOR      : Daniel Allred
  DESC        : Module to boot the from an SPI flash device by finding the
                application (usually U-boot) and loading it to RAM.
----------------------------------------------------------------------------- */

#ifdef UBL_SPI_MEM

// General type include
#include "tistdtypes.h"
// Debug I/O module
#include "debug.h"

// Device specific functions
#include "device.h"

// Misc utility module
#include "util.h"

// Main UBL module
#include "ubl.h"

// SPI driver functions
#include "spi.h"
#include "spi_mem.h"

// Device specific SPI info
#include "device_spi.h"

// This module's header file
#include "spi_memboot.h"

/************************************************************
* Explicit External Declarations                            *
************************************************************/

// Entrypoint for application we are getting out of flash
extern Uint32 gEntryPoint;


/************************************************************
* Local Macro Declarations                                  *
************************************************************/


/************************************************************
* Local Typedef Declarations                                *
************************************************************/


/************************************************************
* Local Function Declarations                               *
************************************************************/


/************************************************************
* Local Variable Definitions                                *
************************************************************/


/************************************************************
* Global Variable Definitions                               *
************************************************************/

// Structure for holding details about image stored in SPI memory
volatile SPI_MEM_BOOT_HeaderObj  gSpiMemBoot;


/************************************************************
* Global Function Definitions                               *
************************************************************/

// Function to find out where the application is and copy to RAM
Uint32 SPI_MEM_BOOT_copy()
{
  SPI_MEM_InfoHandle hSpiMemInfo;
  
  Uint32 currMemAddr = 0;

  DEBUG_printString("Starting SPI Memory Copy...\r\n");
  
  // Do device specific init for SPI 
  DEVICE_SPIInit(DEVICE_SPIBOOT_PERIPHNUM);
  
  // SPI Memory Initialization
  hSpiMemInfo = SPI_MEM_open(DEVICE_SPIBOOT_PERIPHNUM, DEVICE_SPIBOOT_CSNUM, hDEVICE_SPI_config);
  if (hSpiMemInfo == NULL)
    return E_FAIL;
    
  // Read data about Application starting at start of memory and searching
  // at the start of each memory block
  while (currMemAddr < hSpiMemInfo->hMemParams->memorySize)
  {
    currMemAddr += hSpiMemInfo->hMemParams->blockSize;
  
    SPI_MEM_readBytes(hSpiMemInfo, currMemAddr, sizeof(SPI_MEM_BOOT_HeaderObj), (Uint8 *) &gSpiMemBoot);
  
    if (gSpiMemBoot.magicNum == UBL_MAGIC_BINARY_BOOT)
    {
      // Valid magic number found
      DEBUG_printString("Valid magicnum, ");
      DEBUG_printHexInt(gSpiMemBoot.magicNum);
      DEBUG_printString(", found at offset ");
      DEBUG_printHexInt(currMemAddr);
      DEBUG_printString(".\r\n");
      break;
    } 
  }

  if (currMemAddr >= hSpiMemInfo->hMemParams->memorySize)
  {
    DEBUG_printString("No magic number found.\r\n");
    return E_FAIL;
  }
  
  if (SPI_MEM_readBytes(hSpiMemInfo, gSpiMemBoot.memAddress, gSpiMemBoot.appSize, (Uint8 *)gSpiMemBoot.ldAddress) != E_PASS)
  {
    DEBUG_printString("Application image reading failed.\r\n");
    return E_FAIL;
  }
  
  // Application was read correctly, so set entrypoint
  gEntryPoint = gSpiMemBoot.entryPoint;

  return E_PASS;
}

/************************************************************
* Local Function Definitions                                *
************************************************************/


/***********************************************************
* End file                                                 *
***********************************************************/
#endif  // #ifdef UBL_SPI_MEM
