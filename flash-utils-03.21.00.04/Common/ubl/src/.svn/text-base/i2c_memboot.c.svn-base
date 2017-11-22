/* --------------------------------------------------------------------------
  FILE        : i2c_memboot.c                                                   
  PROJECT     : TI Booting and Flashing Utilities
  AUTHOR      : Daniel Allred
  DESC        : Module to boot the from an SPI flash device by finding the
                application (usually U-boot) and loading it to RAM.
----------------------------------------------------------------------------- */

#ifdef UBL_I2C_MEM

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
#include "i2c.h"
#include "i2c_mem.h"

// Device specific I2C info
#include "device_i2c.h"

// This module's header file
#include "i2c_memboot.h"

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

// structure for holding details about image stored in I2C memory
volatile SPIBOOT_HeaderObj  gSpiBoot;


/************************************************************
* Global Function Definitions                               *
************************************************************/

// Function to find out where the application is and copy to RAM
Uint32 I2C_MEM_BOOT_copy()
{
  I2C_MEM_InfoHandle hSpiMemInfo;

  DEBUG_printString("Starting I2C Memory Copy...\r\n");
  
  // Do device specific init for I2C
  DEVICE_I2CInit(DEVICE_I2CBOOT_PERIPHNUM);
  
  // SPI Memory Initialization (on SPI0)
  hSpiMemInfo = SPI_MEM_open(0);
  if (hSpiMemInfo == NULL)
    return E_FAIL;
    
  // Read data about Application starting at START_APP_BLOCK_NUM, Page 0
  // and possibly going until block END_APP_BLOCK_NUM, Page 0
  SPI_MEM_readBytes(hSpiMemInfo, DEVICE_SPI_APP_HDR_OFFSET, sizeof(SPIBOOT_HeaderObj), (Uint8 *) &gSpiBoot);
  
  if((gSpiBoot.magicNum & 0xFFFFFF00) == MAGIC_NUMBER_VALID)
  {
    // Valid magic number found
    DEBUG_printString("Valid magicnum, ");
    DEBUG_printHexInt(gSpiBoot.magicNum);
    DEBUG_printString(", found at offset ");
    DEBUG_printHexInt(DEVICE_SPI_APP_HDR_OFFSET);
    DEBUG_printString(".\r\n");
  }
  else
  {
    DEBUG_printString("Magicnum is missing. Aborting boot!\r\n");
    return E_FAIL;
  }
  
  if (SPI_MEM_readBytes(hSpiMemInfo, gSpiBoot.memAddress, gSpiBoot.appSize, (Uint8 *)gSpiBoot.ldAddress) != E_PASS)
  {
    DEBUG_printString("Application image reading failed. Aborting boot!\r\n");
    return E_FAIL;
  }
  
  // Application was read correctly, so set entrypoint
  gEntryPoint = gSpiBoot.entryPoint;

  return E_PASS;
}

/************************************************************
* Local Function Definitions                                *
************************************************************/


/***********************************************************
* End file                                                 *
***********************************************************/
#endif  // #ifdef UBL_SPI_MEM
