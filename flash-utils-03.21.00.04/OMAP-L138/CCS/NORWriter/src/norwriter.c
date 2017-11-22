/* --------------------------------------------------------------------------
  FILE        : norwriter.c 				                             	 	        
  PROJECT     : TI Booting and Flashing Utilities
  AUTHOR      : Daniel Allred, Joe Coombs
  DESC        : CCS NOR Flashing Utility for OMAP-L138
 ----------------------------------------------------------------------------- */

// C standard I/O library
#include <stdio.h>

// General type include
#include "tistdtypes.h"

// Device specific CSL
#include "device.h"

// This module's header file 
#include "norwriter.h"

// NOR driver include
#include "nor.h"

// Misc. utility function include
#include "util.h"

// Debug module
#include "debug.h"


/************************************************************
* Explicit External Declarations                            *
************************************************************/


/************************************************************
* Local Macro Declarations                                  *
************************************************************/

#define	LOCAL_norBase	(0x60000000)


/************************************************************
* Local Typedef Declarations                                *
************************************************************/


/************************************************************
* Local Function Declarations                               *
************************************************************/

static Uint32 norwriter(void);

#pragma DATA_SECTION(NORStart,".aemif_mem");
VUint32 __FAR__ NORStart;
/************************************************************
* Local Variable Definitions                                *
************************************************************/


/************************************************************
* Global Variable Definitions
************************************************************/


/************************************************************
* Global Function Definitions                               *
************************************************************/

void main( void )
{
  Uint32 status;

  // Init memory alloc pointer to start of DDR heap
  UTIL_setCurrMemPtr(0);

  // System init
  if (DEVICE_init() !=E_PASS)
  {
    exit();
  }

  // Execute the NOR flashing
  status = norwriter();

  if (status != E_PASS)
  {
    DEBUG_printString("\tNOR flashing failed!\r\n");
  }
  else
  {
    DEBUG_printString("\tNOR boot preparation was successful!\r\n" );
  }
}


/************************************************************
* Local Function Definitions                                *
************************************************************/

static Uint32 norwriter()
{
  NOR_InfoHandle hNorInfo;

  FILE	*fPtr;
  Uint8	*appPtr;
  Uint8    *tmp;
  Int32	appFileSize = 0;
  Int8	fileName[256];
  Uint32  baseAddress = 0;
  NOR_BOOT *norBoot;
  Uint32   blockSize, blockAddr;

  DEBUG_printString("Starting ");
  DEBUG_printString((String)devString);
  DEBUG_printString(" NORWriter.\r\n");

  // Initialize NOR Flash
  hNorInfo = NOR_open((Uint32)LOCAL_norBase, DEVICE_BUSWIDTH_16BIT );
  if (hNorInfo == NULL)
  {
    DEBUG_printString( "\tERROR: NOR Initialization failed.\r\n" );
    return E_FAIL;
  }

  // Set base address to start putting data at
  baseAddress = hNorInfo->flashBase;

  // Read the UBL file from host
  DEBUG_printString("Enter the binary AIS application file name (enter 'none' to skip): \r\n");
  DEBUG_readString(fileName);
  fflush(stdin);

  if (strcmp(fileName,"none") != 0)
  {
    // Open an File from the hard drive
    fPtr = fopen(fileName, "rb");
    if(fPtr == NULL)
    {
      DEBUG_printString("\tERROR: File ");
      DEBUG_printString(fileName);
      DEBUG_printString(" open failed.\r\n");
      return E_FAIL;
    }

    // Initialize the pointer
    appFileSize = 0;

    // Read file size
    fseek(fPtr,0,SEEK_END);
    appFileSize = ftell(fPtr);

    // Check to make sure image was read correctly
    if(appFileSize == 0)
    {
      DEBUG_printString("\tERROR: File read failed.\r\n");
      fclose (fPtr);
      return E_FAIL;
    }
    // Check to make sure the app image will fit 
    else if ( appFileSize > hNorInfo->flashSize )
    {
      DEBUG_printString("\tERROR: File too big.. Closing program.\r\n");
      fclose (fPtr);
    }

    // Setup pointer in RAM
    appPtr = (Uint8 *) UTIL_allocMem(appFileSize);

    fseek(fPtr,0,SEEK_SET);

    if (appFileSize != fread(appPtr, 1, appFileSize, fPtr))
    {
      DEBUG_printString("\tWARNING: File Size mismatch.\r\n");
    }

    fclose (fPtr);
    
    DEBUG_printString("\tINFO: File read complete.\r\n");

    // Erase the NOR flash to accomodate the file size
    if (NOR_erase( hNorInfo, baseAddress, appFileSize ) != E_PASS)
    {
      DEBUG_printString("\tERROR: Erasing NOR failed.\r\n");
      return E_FAIL;
    }

    // Write the application data to the flash  
    if (NOR_writeBytes( hNorInfo, baseAddress, appFileSize, (Uint32)(appPtr)) != E_PASS)
    {
      DEBUG_printString("\tERROR: Writing NOR failed.\r\n");
      return E_FAIL;
    }
  }

 // Set base address to start putting data at
   /* Get NOR block size */
  NOR_getBlockInfo(hNorInfo, (Uint32)&NORStart, &blockSize, &blockAddr);
  baseAddress = hNorInfo->flashBase + blockSize;
  
  // Read the UBL file from host
  DEBUG_printString("Enter u-boot file name (enter 'none' to skip): \r\n");
  DEBUG_readString(fileName);
  fflush(stdin);

  if (strcmp(fileName,"none") != 0)
  {
    // Open an File from the hard drive
    fPtr = fopen(fileName, "rb");
    if(fPtr == NULL)
    {
      DEBUG_printString("\tERROR: File ");
      DEBUG_printString(fileName);
      DEBUG_printString(" open failed.\r\n");
      return E_FAIL;
    }

    // Initialize the pointer
    appFileSize = 0;

    // Read file size
    fseek(fPtr,0,SEEK_END);
    appFileSize = ftell(fPtr);

    // Check to make sure image was read correctly
    if(appFileSize == 0)
    {
      DEBUG_printString("\tERROR: File read failed.\r\n");
      fclose (fPtr);
      return E_FAIL;
    }
    // Check to make sure the app image will fit 
    else if ( appFileSize > hNorInfo->flashSize )
    {
      DEBUG_printString("\tERROR: File too big.. Closing program.\r\n");
      fclose (fPtr);
    }

    // Setup pointer in RAM
    tmp = appPtr = (Uint8 *) UTIL_allocMem(appFileSize);
    norBoot = (NOR_BOOT*) appPtr;
	 norBoot->magicNum = UBL_MAGIC_BINARY_BOOT;
	 norBoot->entryPoint = 0xc1080000;
	 norBoot->ldAddress = 0xc1080000;
	 norBoot->appSize = appFileSize;
	 tmp += sizeof(NOR_BOOT);


    fseek(fPtr,0,SEEK_SET);

    if (appFileSize != fread(tmp, 1, appFileSize, fPtr))
    {
      DEBUG_printString("\tWARNING: File Size mismatch.\r\n");
    }

    fclose (fPtr);
    
    DEBUG_printString("\tINFO: File read complete.\r\n");

    // Erase the NOR flash to accomodate the file size
    if (NOR_erase( hNorInfo, baseAddress, appFileSize ) != E_PASS)
    {
      DEBUG_printString("\tERROR: Erasing NOR failed.\r\n");
      return E_FAIL;
    }

    // Write the application data to the flash  
    if (NOR_writeBytes( hNorInfo, baseAddress, appFileSize, (Uint32)(appPtr)) != E_PASS)
    {
      DEBUG_printString("\tERROR: Writing NOR failed.\r\n");
      return E_FAIL;
    }
    
  /*  
    // Verify memory contents
    {
      // temp: check flash mem
      int i;

      DEBUG_printString("Checking flash contents... ");
      for (i = 0; i < appFileSize; i++)
      {
        if (*(Uint8 *)(LOCAL_norBase + i) != *(appPtr + i))
        {
          DEBUG_printString("Failed! (");
          DEBUG_printHexInt(i);
          DEBUG_printString(" bytes successful)\r\n");
          //return E_FAIL;
        }	
      }
      DEBUG_printString("Passed! (");
      DEBUG_printHexInt(appFileSize);
      DEBUG_printString(" bytes)\r\n");
    }*/
  }
  return E_PASS;
}


/***********************************************************
* End file                                                 *
***********************************************************/
