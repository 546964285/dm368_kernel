/* --------------------------------------------------------------------------
  FILE        : nandboot.c                                                   
  PROJECT     : TI Booting and Flashing Utilities
  AUTHOR      : Daniel Allred
  DESC        : Module to boot the from a NAND flash device by finding the
                application (usually U-boot) and loading it to RAM.
----------------------------------------------------------------------------- */

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

// NAND driver functions
#include "async_mem.h"
#include "device_async_mem.h"
#include "nand.h"
#include "device_nand.h"

// This module's header file
#include "nandboot.h"

/************************************************************
* Explicit External Declarations                            *
************************************************************/

extern __FAR__ Uint32 ASYNC_MEM_START;
extern __FAR__ Uint32 EXTERNAL_RAM_SIZE, EXTERNAL_RAM_START, EXTERNAL_RAM_END;
extern __FAR__ Uint32 INTERNAL_RAM_SIZE, INTERNAL_RAM_START, INTERNAL_RAM_END;
extern __FAR__ Uint32 STACK_START, STACK_SIZE;

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

// structure for holding details about UBL stored in NAND
volatile NANDBOOT_HeaderObj  gNandBoot;


/************************************************************
* Global Function Definitions                               *
************************************************************/

// Function to find out where the application is and copy to RAM
Uint32 NANDBOOT_copy()
{
  NAND_InfoHandle hNandInfo;
  Uint32 count,blockNum, baseAddr;
  Uint32 i;
  Uint8 *rxBuf = NULL;
  Uint32 block,page;
  Uint32 readError = E_FAIL;
  Bool failedOnceAlready = FALSE;

  // Allocate memory for maximum application size
  rxBuf = (Uint8*)UTIL_allocMem(APP_IMAGE_SIZE);
  blockNum = DEVICE_NAND_UBL_SEARCH_START_BLOCK;

  DEBUG_printString("Starting NAND Copy...\r\n");
  
  // NAND Initialization
  baseAddr = DEVICE_ASYNC_MEM_interfaces[DEVICE_ASYNC_MEM_NANDBOOT_INTERFACE].regionStarts[DEVICE_ASYNC_MEM_NANDBOOT_REGION];
  hNandInfo = NAND_open(baseAddr, (Uint8) DEVICE_emifBusWidth());
  if (hNandInfo == NULL)
  {
    DEBUG_printString( "\tERROR: NAND Memory Initialization failed.\r\n" );
    return E_FAIL;
  }
    
NAND_startAgain:
  if (blockNum > DEVICE_NAND_UBL_SEARCH_END_BLOCK)
  {
    return E_FAIL;  // NAND boot failed and return fail to main
  }

  // Read data about Application starting at START_APP_BLOCK_NUM, Page 0
  // and possibly going until block END_APP_BLOCK_NUM, Page 0
  for(count=blockNum; count <= DEVICE_NAND_UBL_SEARCH_END_BLOCK; count++)
  {    
    Uint32 magicNum;
    
    if(NAND_readPage(hNandInfo,count,0,rxBuf) != E_PASS)
    {
      continue;
    }

    magicNum = ((Uint32 *)rxBuf)[0];

    /* Valid magic number found */
    if ((magicNum == UBL_MAGIC_BINARY_BOOT) || (magicNum == 0xA1ACED66))
    {
      blockNum = count;
      DEBUG_printString("Valid magicnum, ");
      DEBUG_printHexInt(magicNum);
      DEBUG_printString(", found in block ");
      DEBUG_printHexInt(blockNum);
      DEBUG_printString(".\r\n");
      break;
    }
  }

  // Never found valid header in any page 0 of any of searched blocks
  if (count > DEVICE_NAND_UBL_SEARCH_END_BLOCK)
  {
    DEBUG_printString("No valid boot image found!\r\n");
    return E_FAIL;
  }

  // Fill in NandBoot header
  gNandBoot.entryPoint = *(((Uint32 *)(&rxBuf[4])));/* The first "long" is entry point for Application */
  gNandBoot.numPage = *(((Uint32 *)(&rxBuf[8])));   /* The second "long" is the number of pages */
  gNandBoot.block = *(((Uint32 *)(&rxBuf[12])));   /* The third "long" is the block where Application is stored in NAND */
  gNandBoot.page = *(((Uint32 *)(&rxBuf[16])));   /* The fourth "long" is the page number where Application is stored in NAND */
  gNandBoot.ldAddress = *(((Uint32 *)(&rxBuf[20])));   /* The fifth "long" is the Application load address */

  // Set the copy location to final run location
  rxBuf = (Uint8 *)gNandBoot.ldAddress;
    
  // Free temp memory rxBuf used to point to
  UTIL_setCurrMemPtr((void *)((Uint32)UTIL_getCurrMemPtr() - APP_IMAGE_SIZE));

NAND_retry:
  // Initialize block and page number to be used for read
  block = gNandBoot.block;
  page = gNandBoot.page;

  // Perform the actual copying of the application from NAND to RAM
  for(i=0;i<gNandBoot.numPage;i++)
  {
    // if page goes beyond max number of pages increment block number and reset page number
    if(page >= hNandInfo->pagesPerBlock)
    {
      page = 0;
      block++;
    }
    readError = NAND_readPage(hNandInfo,block,page++,(&rxBuf[i*(hNandInfo->dataBytesPerPage)]));  /* Copy the data */

    // We attempt to read the app data twice.  If we fail twice then we go look for a new
    // application header in the NAND flash at the next block.
    if(readError != E_PASS)
    {
      if(failedOnceAlready)
      {
        blockNum++;
        goto NAND_startAgain;
      }
      else
      {
        failedOnceAlready = TRUE;
        goto NAND_retry;
      }
    }
  }

  // Application was read correctly, so set entrypoint
  gEntryPoint = gNandBoot.entryPoint;

  return E_PASS;
}

/************************************************************
* Local Function Definitions                                *
************************************************************/


/***********************************************************
* End file                                                 *
***********************************************************/
