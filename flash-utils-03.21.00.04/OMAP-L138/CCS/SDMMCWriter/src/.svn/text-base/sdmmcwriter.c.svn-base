/* --------------------------------------------------------------------------
    FILE        : sdmmcwriter.c 				                             	 	        
    PROJECT     : OMAP-L138 CCS SD/MMC Flashing Utility
    AUTHOR      : Daniel Allred
    DESC        : Main function for flashing the SD/MMC device on the OMAP-L138  
 ----------------------------------------------------------------------------- */

// C standard I/O library
#include "stdio.h"

// General type include
#include "tistdtypes.h"

// Device specific CSL
#include "device.h"

// Misc. utility function include
#include "util.h"

// Debug module
#include "debug.h"

// SDMMC memory driver include
#include "sdmmc.h"
#include "sdmmc_mem.h"
#include "device_sdmmc.h"
#include "sdmmcboot.h"

// This module's header file 
#include "sdmmcwriter.h"


/************************************************************
* Explicit External Declarations                            *
************************************************************/


/************************************************************
* Local Macro Declarations                                  *
************************************************************/


/************************************************************
* Local Typedef Declarations                                *
************************************************************/


/************************************************************
* Local Function Declarations                               *
************************************************************/

static Uint32 LOCAL_sdmmcwriter(void);
static Uint32 LOCAL_GetAndWriteFileData(SDMMC_MEM_InfoHandle hSdmmcMemInfo, String fileName, Uint32 destAddr, Bool useHeader);


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

  // Execute the SPI flashing
  status = LOCAL_sdmmcwriter();

  if (status != E_PASS)
  {
    DEBUG_printString("\tSD/MMC flashing failed!\r\n");
  }
  else
  {
    DEBUG_printString("\tSD/MMC boot preparation was successful!\r\n" );
  }
}


/************************************************************
* Local Function Definitions                                *
************************************************************/

static Uint32 LOCAL_sdmmcwriter()
{
  SDMMC_MEM_InfoHandle hSDMMCMemInfo;

  Int8	fileName[256];
  Uint32  baseAddress = 0;
  Bool  useHeaderForApp = FALSE;

  DEBUG_printString( "Starting " );
  DEBUG_printString( (String) devString );
  DEBUG_printString( " SDMMCWriter.\r\n" );
  
  // Initialize SD/MMC Memory Device
  hSDMMCMemInfo = SDMMC_MEM_open(DEVICE_SDMMCBOOT_PERIPHNUM, hDEVICE_SDMMC_config);
  if (hSDMMCMemInfo == NULL)
  {
    DEBUG_printString( "\tERROR: SDMMC Memory Initialization failed.\r\n" );
    return E_FAIL;
  }

  DEBUG_printString("Will you be writing a UBL image? (Y or y) \r\n");
  DEBUG_readString(fileName);
  fflush(stdin);

  if ((strcmp(fileName,"y") == 0) || (strcmp(fileName,"Y") == 0))
  {
    // Read the AIS file from host
    DEBUG_printString("Enter the binary AIS UBL file name (enter 'none' to skip): \r\n");
    DEBUG_readString(fileName);
    fflush(stdin);
    
    LOCAL_GetAndWriteFileData(hSDMMCMemInfo, fileName, baseAddress, FALSE);
    
    // Assume that the UBL will fit in the first block of the SPI flash
    baseAddress += 0x20000;
    useHeaderForApp = TRUE;
  }

  // Read the AIS file from host
  DEBUG_printString("Enter the application file name (enter 'none' to skip): \r\n");
  DEBUG_readString(fileName);
  fflush(stdin);
  
  if (LOCAL_GetAndWriteFileData(hSDMMCMemInfo, fileName, baseAddress, useHeaderForApp) != E_PASS)
  {
    DEBUG_printString("SDMMC Flashing Failed!");
    return E_FAIL;
  }

  return E_PASS;
}

static Uint32 LOCAL_GetAndWriteFileData(SDMMC_MEM_InfoHandle hSdmmcMemInfo, String fileName, Uint32 destAddr, Bool useHeader)
{
  FILE	*fPtr;
  Uint8	*appPtr, *appPtr2;
  Int32	fileSize = 0;
  SDMMCBOOT_HeaderObj sdmmcBoot;

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
    fileSize = 0;

    // Read file size
    fseek(fPtr,0,SEEK_END);
    fileSize = ftell(fPtr);

    // Check to make sure image was read correctly
    if(fileSize == 0)
    {
      DEBUG_printString("\tERROR: File read failed.\r\n");
      fclose (fPtr);
      return E_FAIL;
    }
    
    // Setup pointer in RAM
    appPtr = (Uint8 *) UTIL_allocMem(fileSize);

    fseek(fPtr,0,SEEK_SET);

    if (fileSize != fread(appPtr, 1, fileSize, fPtr))
    {
      DEBUG_printString("\tWARNING: File Size mismatch.\r\n");
    }

    fclose (fPtr);    
    
    // Erase the SPI flash to accomodate the file size
    if (SDMMC_MEM_readBytes( hSdmmcMemInfo, destAddr, 512, appPtr) != E_PASS)
    {
      DEBUG_printString("\tERROR: Reading SDMMC failed.\r\n");
      return E_FAIL;
    }
    
    while(1);
    
#if (0)
    // Check to make sure the app image will fit 
    else if ( fileSize > hSpiMemInfo->hMemParams->memorySize )
    {
      DEBUG_printString("\tERROR: File too big.. Closing program.\r\n");
      fclose (fPtr);
      exit(0);
    }

    // Setup pointer in RAM
    appPtr = (Uint8 *) UTIL_allocMem(fileSize);

    fseek(fPtr,0,SEEK_SET);

    if (fileSize != fread(appPtr, 1, fileSize, fPtr))
    {
      DEBUG_printString("\tWARNING: File Size mismatch.\r\n");
    }

    fclose (fPtr);
    
    DEBUG_printString("\tINFO: File read complete.\r\n");
    
    if (useHeader)
    {
      Uint8 *tempPtr = (Uint8 *) UTIL_allocMem(fileSize + sizeof(SPI_MEM_BOOT_HeaderObj));
      DEBUG_printString("Enter the app image load address (in hex): \r\n");
      DEBUG_readHexInt(&(spiMemBoot.ldAddress));  
      DEBUG_printString("Enter the app image entry point address (in hex): \r\n");
      DEBUG_readHexInt(&(spiMemBoot.entryPoint));

      spiMemBoot.appSize = fileSize;
      spiMemBoot.magicNum = UBL_MAGIC_BINARY_BOOT;
      spiMemBoot.memAddress = destAddr + sizeof(SPI_MEM_BOOT_HeaderObj); 
      
      UTIL_memcpy(tempPtr, spiMemBoot, sizeof(SPI_MEM_BOOT_HeaderObj));
      tempPtr += sizeof(SPI_MEM_BOOT_HeaderObj);
      UTIL_memcpy(tempPtr, appPtr, fileSize);
      fileSize += sizeof(SPI_MEM_BOOT_HeaderObj);
      appPtr = tempPtr;
      appPtr -= sizeof(SPI_MEM_BOOT_HeaderObj);
    }
    
    // Create spare buffer for erase and write verify
    appPtr2 = (Uint8 *) UTIL_allocMem(fileSize);

    // Erase the SPI flash to accomodate the file size
    if (SPI_MEM_eraseBytes( hSpiMemInfo, destAddr, fileSize ) != E_PASS)
    {
      DEBUG_printString("\tERROR: Erasing SPI failed.\r\n");
      return E_FAIL;
    }
    
    // Verify the SPI was actually erased
    if (SPI_MEM_verifyErase(hSpiMemInfo, destAddr, fileSize, appPtr2) != E_PASS)
    {
      DEBUG_printString("\tERROR: Verifying SPI data failed.\r\n");
      return E_FAIL;
    }
    
    // Make copy of the file data to compare against after the write
    memcpy(appPtr2, appPtr, fileSize);

    // Write the application data to the flash (note that writes are destructive)
    if (SPI_MEM_writeBytes( hSpiMemInfo, destAddr, fileSize, appPtr) != E_PASS)
    {
      DEBUG_printString("\tERROR: Writing SPI failed.\r\n");
      return E_FAIL;
    }

    // Verify the memory contents 
    if (SPI_MEM_verifyBytes(hSpiMemInfo, destAddr, fileSize, appPtr2, appPtr) != E_PASS)
    {
      DEBUG_printString("\tERROR: Verifying SPI data failed.\r\n");
      return E_FAIL;
    }
#endif    
  }  
  return E_PASS;
}


/***********************************************************
* End file                                                 *
***********************************************************/
