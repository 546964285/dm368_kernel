/* --------------------------------------------------------------------------
    FILE        : spiwriter.c 				                             	 	        
    PROJECT     : OMAP-L138 CCS SPI Flashing Utility
    AUTHOR      : Daniel Allred
    DESC        : Main function for flashing the SPI device on the OMAP-L138  
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

// SPI memory driver include
#include "spi_mem.h"
#include "device_spi.h"
#include "spi_memboot.h"

// Common UBL header
#include "ubl.h"

// This module's header file 
#include "spiwriter.h"


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

static Uint32 LOCAL_spiwriter(void);
static Uint32 LOCAL_GetAndWriteFileData(SPI_MEM_InfoHandle hSpiMemInfo, String fileName, Uint32 destAddr, Bool useHeader);


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

  // Init memory alloc pointer to start of external memory
  UTIL_setCurrMemPtr(0);

  // System init
  if (DEVICE_init() !=E_PASS)
  {
    exit();
  }

  // Execute the SPI flashing
  status = LOCAL_spiwriter();

  if (status != E_PASS)
  {
    DEBUG_printString("\tSPI flashing failed!\r\n");
  }
  else
  {
    DEBUG_printString("\tSPI boot preparation was successful!\r\n" );
  }
}


/************************************************************
* Local Function Definitions                                *
************************************************************/

static Uint32 LOCAL_spiwriter()
{
  SPI_MEM_InfoHandle hSpiMemInfo;

  Int8	fileName[256];
  Uint32  baseAddress = 0;
  Bool  useHeaderForApp = FALSE;

  DEBUG_printString( "Starting ");
  DEBUG_printString( (String) devString);
  DEBUG_printString( " SPIWriter.\r\n");
  
  // Prep device for SPI writing (pinmux/PSC)
  DEVICE_SPIInit(DEVICE_SPIBOOT_PERIPHNUM);
  
  // Initialize SPI Memory Device
  hSpiMemInfo = SPI_MEM_open(DEVICE_SPIBOOT_PERIPHNUM, DEVICE_SPIBOOT_CSNUM, hDEVICE_SPI_config);
  if (hSpiMemInfo == NULL)
  {
    DEBUG_printString( "\tERROR: SPI Memory Initialization failed.\r\n" );
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
    
    LOCAL_GetAndWriteFileData(hSpiMemInfo, fileName, baseAddress, FALSE);
    
    // Assume that the UBL will fit in the first block of the SPI flash
    baseAddress += hSpiMemInfo->hMemParams->blockSize;
    useHeaderForApp = TRUE;
  }

  // Read the AIS file from host
  DEBUG_printString("Enter the application file name (enter 'none' to skip): \r\n");
  DEBUG_readString(fileName);
  fflush(stdin);
  
  if (LOCAL_GetAndWriteFileData(hSpiMemInfo, fileName, baseAddress, useHeaderForApp) != E_PASS)
  {
    DEBUG_printString("SPI Flashing Failed!");
    return E_FAIL;
  }
  
  return E_PASS;
}


static Uint32 LOCAL_GetAndWriteFileData(SPI_MEM_InfoHandle hSpiMemInfo, String fileName, Uint32 destAddr, Bool useHeader)
{
  FILE	*fPtr;
  Uint8	*appPtr, *appPtr2;
  Int32	fileSize = 0;
  SPI_MEM_BOOT_HeaderObj spiMemBoot;

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
      
      UTIL_memcpy(tempPtr, &spiMemBoot, sizeof(SPI_MEM_BOOT_HeaderObj));
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
  }
  return E_PASS;
}


/***********************************************************
* End file                                                 *
***********************************************************/
