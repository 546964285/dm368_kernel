/* --------------------------------------------------------------------------
  FILE        : sdmmcboot.c
  PROJECT     : TI Booting and Flashing Utilities
  AUTHOR      : Daniel Allred
  DESC        : Module to boot the from the SD/MMC peripheral by finding the
                application (usually U-boot) and loading it to RAM.
----------------------------------------------------------------------------- */

#ifdef UBL_SDMMC

// General type include
#include "tistdtypes.h"

// Debug I/O module
#include "debug.h"

// Misc utility module
#include "util.h"

// Main UBL module
#include "ubl.h"

// SD/MMC driver functions
#include "sdmmc.h"

// This module's header file
#include "sdmmcboot.h"

// Device specific file
#include "device_sdmmc.h"

/************************************************************
* Explicit External Declarations                            *
************************************************************/

// Entrypoint for application we are decoding out of flash
extern Uint32 gEntryPoint;
extern __FAR__ Uint32 EXTERNAL_RAM_SIZE, EXTERNAL_RAM_START, EXTERNAL_RAM_END;
extern __FAR__ Uint32 INTERNAL_RAM_SIZE, INTERNAL_RAM_START, INTERNAL_RAM_END;
extern __FAR__ Uint32 STACK_START, STACK_SIZE;

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

// structure for holding details about UBL stored in SDMMC
volatile SDMMCBOOT_HeaderObj  gSDMMCBoot;
Uint8 SDMMCBOOT(SDMMC_InfoHandle hSDMMCInfo);

/************************************************************
* Global Function Definitions                               *
************************************************************/

// Function to find out where the application is and copy to RAM
Uint32 SDMMCBOOT_copy()
{
  SDMMC_InfoHandle hSDMMCInfo;

   
  
  // SDMMC Initialization
  hSDMMCInfo = SDMMC_open(0,NULL);
  if (hSDMMCInfo == NULL)
    return E_FAIL;

   if( SDMMCBOOT(hSDMMCInfo) != E_PASS)
	   return E_FAIL;

  return E_PASS;
}



Uint8 SDMMCBOOT(SDMMC_InfoHandle hSDMMCInfo)
{	Uint32 count,blockNum;
	Uint32 magicNum,magicNumFound;   
	Uint8 *rxBuf;		/* receive buffer where the data read from NAND will be stored */
	Uint32 readError = E_FAIL;
	volatile SDMMC_Boot sdMMCBootDesc;
	Uint8 retry = 0;

  // Allocate memory for maximum application size
  rxBuf = (Uint8*)UTIL_allocMem(APP_IMAGE_SIZE); //0x100;
#ifdef OMAPL138
  blockNum = 1;
#else
  blockNum = DEVICE_SDMMC_UBL_SEARCH_START_BLOCK;
#endif

  
SDMMC_tryAgain:

	retry++;	
    // if retry value is > 2, MMCSD boot fails and starts USB boot mode 
	if(retry > 2)
	  return E_FAIL;
#if 0
	// PINMUX3 bit23
	*PINMUX3 |= 0x00800000;		
 	/* initialize MMCSD interface */
	// Drive GIO11 active low for MMC/SD Power Switch
    // GIO11 config bank bit11 = 0 for output
	CSL_GPIO_REGS->BANK[0].DIR &= 0xFFFFF7FF;
	// Clear GIO11
	CSL_GPIO_REGS->BANK[0].CLR_DATA = 0x800; 
	if(SDMMCInitCard() != E_PASS)
		goto SDMMC_tryAgain;
#endif
	/* read data about UBL from the block 1(to MMCSD_TRY_BLOCK_NUM+1)*/

	magicNumFound = 0;
	for(count=blockNum; count <= DEVICE_SDMMC_UBL_SEARCH_END_BLOCK; count++) {
		// reading 512 though only 32 is used
		readError = SDMMCSingleBlkRead(hSDMMCInfo, count*hSDMMCInfo->dataBytesPerBlk ,&rxBuf[0],512);	
                
		/* Read Error has occured */
		if(readError != E_PASS) {
			continue;
                }

		magicNum = *(((Uint32 *)rxBuf));

		/* Magic number found */
		if((magicNum & 0xFFFFFF00) == MAGIC_NUMBER_VALID)
		{
			blockNum = count;
			magicNumFound = 1;
			break;
		}
		
	}
	
	if(readError != E_PASS)
		goto SDMMC_tryAgain; /* MMC/SD boot failed.. Retry */
#if 0		
	/* When readError == E_PASS, check if magicNum is found */
	if(magicNumFound == 0) {
		goto SDMMC_tryAgain; /* MMC/SD boot failed.. Retry */
	}
#endif	
	
	/* entry point must be between 0x0020 and 0x37FC */
	sdMMCBootDesc.entryPoint = *(((Uint32 *)(&rxBuf[4])));/* The first "long" is entry point for Application */
	sdMMCBootDesc.numBlock  = *(((Uint32 *)(&rxBuf[8])));	 /* The second "long" is the number of blocks */
	sdMMCBootDesc.startBlock = *(((Uint32 *)(&rxBuf[12])));	 /* The third "long" is the block where Application is stored in MMC/SD */
	sdMMCBootDesc.ldAddress = *(((Uint32 *)(&rxBuf[16])));	 /* The fourth "long" is the load address of the Application */

  // If the application is already in binary format, then our 
  // received buffer can point to the specified load address
  // instead of the temp location used for storing an S-record
  // Checking for the UBL_MAGIC_DMA guarantees correct usage with the 
  // Spectrum Digital CCS flashing tool, flashwriter_nand.out

  if (magicNum == UBL_MAGIC_BIN_IMG)
  {
    // Set the copy location to final run location
    rxBuf = (Uint8 *)sdMMCBootDesc.ldAddress;
    
    // Free temp memory rxBuf used to point to
    UTIL_setCurrMemPtr((void *)((Uint32)UTIL_getCurrMemPtr() - (APP_IMAGE_SIZE>>1)));
  }

MMCSD_retry:	

	readError = SDMMCMultipleBlkRead(hSDMMCInfo,  (sdMMCBootDesc.startBlock*hSDMMCInfo->dataBytesPerBlk) ,(&rxBuf[0]), (sdMMCBootDesc.numBlock*hSDMMCInfo->dataBytesPerBlk));	/* Copy the data */
		
	if(readError != E_PASS) {
		if((magicNum & 0xFF) == UBL_MAGIC_SAFE) {
                        DEBUG_printString("error1\r\n");
			if(blockNum == (DEVICE_SDMMC_UBL_SEARCH_END_BLOCK)) {
				goto SDMMC_tryAgain; /* MMC/SD boot failed.. Retry */
			} else {
				/* Search from for the Magic number in next block */ 
				blockNum++;
				goto SDMMC_tryAgain; /* MMC/SD boot failed.. Retry */
			}
		}
	    else {               
                        DEBUG_printString("error2\r\n");
	        magicNum = UBL_MAGIC_SAFE;
			goto MMCSD_retry;
		}
	}	    				

  // Application was read correctly, so set entrypoint
	gEntryPoint = sdMMCBootDesc.entryPoint;
    return E_PASS;
}

/************************************************************
* Local Function Definitions                                *
************************************************************/


/***********************************************************
* End file                                                 *
***********************************************************/
#endif  // #ifdef UBL_SDMMC
