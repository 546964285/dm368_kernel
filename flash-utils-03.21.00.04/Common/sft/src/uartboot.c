/* --------------------------------------------------------------------------
  FILE        : uartboot.c                                                   
  PROJECT     : TI Booting and Flashing Utilities
  AUTHOR      : Daniel Allred
  DESC        : Implementation of the UART boot mode for the SFT
 ----------------------------------------------------------------------------- */

// General type include
#include "tistdtypes.h"

// Misc. utility function include
#include "util.h"

// Project specific debug functionality
#include "debug.h"

// Main SFT module
#include "sft.h"

// UART driver
#include "uart.h"

// Device module
#include "device.h"

// Flash type includes
#if defined(UBL_NOR)
  #include "device_async_mem.h"
  #include "nor.h"
  #include "norboot.h"
#elif defined(UBL_NAND)
  #include "device_async_mem.h"
  #include "nand.h"
  #include "device_nand.h"
  #include "nandboot.h"
#elif defined(UBL_SPI_MEM)
  #include "spi.h"
  #include "spi_mem.h"
  #include "device_spi.h"
  #include "spi_memboot.h"
#elif defined(UBL_I2C_MEM)
  #include "i2c.h"
  #include "i2c_mem.h"
  #include "12c_memboot.h"
#elif defined(UBL_SDMMC)
  #include "sdmmc.h"
  #include "sdmmcboot.h"
#elif defined(UBL_ONENAND)
  #include "device_async_mem.h"
  #include "onenand.h"
  #include "onenandboot.h"
#endif

// This module's header file
#include "uartboot.h"


/************************************************************
* Explicit External Declarations                            *
************************************************************/

extern __FAR__ Uint32 gEntryPoint;

extern __FAR__ Uint32 ASYNC_MEM_START;
extern __FAR__ Uint32 EXTERNAL_RAM_SIZE, EXTERNAL_RAM_START, EXTERNAL_RAM_END;
extern __FAR__ Uint32 INTERNAL_RAM_SIZE, INTERNAL_RAM_START, INTERNAL_RAM_END;
extern __FAR__ Uint32 STACK_START, STACK_SIZE;

extern __FAR__ UART_InfoHandle hUartInfo;


/************************************************************
* Local Macro Declarations                                  *
************************************************************/

// Define max UBL image size (DRAM size - 2048)
#define UBL_IMAGE_SIZE              (((Uint32)&INTERNAL_RAM_START) - ((Uint32)&STACK_SIZE))

// Define max app image size (1/8th of External RAM size)
#define APP_IMAGE_SIZE              (((Uint32)&EXTERNAL_RAM_SIZE) >> 3)


/************************************************************
* Local Typedef Declarations                                *
************************************************************/


/************************************************************
* Local Function Declarations                               *
************************************************************/

static Uint32 LOCAL_sendSequence(String s);
static Uint32 LOCAL_recvSequence(String s);
static Uint32 LOCAL_recvCommand(Uint32* bootCmd);
static Uint32 LOCAL_recvHeaderAndData(UARTBOOT_HeaderHandle ackHeader);
#if defined(UBL_NAND)
  //static Uint32 LOCAL_NANDWriteHeaderAndData(NAND_InfoHandle hNandInfo, Uint32 startBlock, Uint32 endBlock, NANDBOOT_HeaderHandle nandBoot, Uint8 *srcBuf);
  static Uint32 LOCAL_NANDWriteHeaderAndData(NAND_InfoHandle hNandInfo, NANDBOOT_HeaderHandle nandBoot, Uint8 *srcBuf);
  static Uint32 LOCAL_NANDWriteData(NAND_InfoHandle hNandInfo, Uint8 *srcBuf, Uint32 byteCnt);
  static Uint32 LOCAL_NAND_commands(Uint32 bootCmd);
#elif defined(UBL_NOR)
  static Uint32 LOCAL_NOR_commands(Uint32 bootCmd);
#elif defined(UBL_SPI_MEM)
  static Uint32 LOCAL_SPIMEM_commands(Uint32 bootCmd);
#elif defined(UBL_I2C_MEM)
  static Uint32 LOCAL_I2CMEM_commands(Uint32 bootCmd);
#elif defined(UBL_SDMMC)
  static Uint32 LOCAL_SDMMC_commands(Uint32 bootCmd);
#elif defined(UBL_ONENAND)
    status = LOCAL_ONENAND_commands(Uint32 bootCmd);  
#endif


/************************************************************
* Local Variable Definitions                                *
************************************************************/

#if defined(UBL_NAND)
  static Uint8     *hNandWriteBuf,*hNandReadBuf;
#elif defined(UBL_SPI_MEM)
  static Uint8     *hSpiReadBuf;
#endif  
  static UARTBOOT_HeaderObj  ackHeader;

/************************************************************
* Global Variable Definitions                               *
************************************************************/


/************************************************************
* Global Function Definitions                               *
************************************************************/

Uint32 UARTBOOT_copy(void)
{
  Uint32 bootCmd, status;

UART_tryAgain:
  
    // Receive the START command
    #ifndef SftLocalRecvSeq
    if(LOCAL_recvSequence("  START") != E_PASS)
    {
      goto UART_tryAgain;
    }
    #endif

  if (LOCAL_sendSequence("BOOTUBL") != E_PASS)
  {
    goto UART_tryAgain;
  }
  do 
  {
    // Receive the next BOOT command
    if(LOCAL_recvCommand(&bootCmd) != E_PASS)
    {
      goto UART_tryAgain;
    }

    // Send ^^^DONE\0 to indicate command was accepted
    if ( LOCAL_sendSequence("   DONE") != E_PASS )
    {
      goto UART_tryAgain;
    }
  
  #if defined(UBL_NAND)
    status = LOCAL_NAND_commands(bootCmd);
  #elif defined(UBL_NOR)
    status = LOCAL_NOR_commands(bootCmd);
  #elif defined(UBL_SPI_MEM)
    status = LOCAL_SPIMEM_commands(bootCmd);
  #elif defined(UBL_I2C_MEM)
    status = LOCAL_I2CMEM_commands(bootCmd);
  #elif defined(UBL_SDMMC)
    status = LOCAL_SDMMC_commands(bootCmd);
  #elif defined(UBL_ONENAND)
    status = LOCAL_ONENAND_commands(bootCmd);
  #else
    status = E_FAIL;
  #endif
    
    if (status != E_PASS)
    {
      // Send ^^^FAIL\0 to indicate command did not complete
      LOCAL_sendSequence("   FAIL");
      // then go try again
      goto UART_tryAgain;
    }
    else
    {
      // Send ^^^DONE\0 to indicate command was completed
      LOCAL_sendSequence("   DONE");
    }
  }
  while (bootCmd != UBL_MAGIC_FINISHED);

  // Exit successfully
  return E_PASS;
}

#if defined(UBL_NAND)
static Uint32 LOCAL_NAND_commands(Uint32 bootCmd)
{
  Uint32              i;
  Uint32              baseAddr;
  NANDBOOT_HeaderObj  nandBoot;
  NAND_InfoHandle     hNandInfo;

  baseAddr = DEVICE_ASYNC_MEM_interfaces[DEVICE_ASYNC_MEM_NANDBOOT_INTERFACE].regionStarts[DEVICE_ASYNC_MEM_NANDBOOT_REGION];
  hNandInfo = NAND_open(baseAddr, DEVICE_ASYNC_MEM_NANDBOOT_BUSWIDTH);
  if (hNandInfo == NULL)
  {
    DEBUG_printString( "\tERROR: NAND Memory Initialization failed.\r\n" );
    return E_FAIL;
  }
  
  // Unprotect all blocks we might need to touch
  NAND_unProtectBlocks(hNandInfo, DEVICE_NAND_RBL_SEARCH_START_BLOCK, DEVICE_NAND_UBL_SEARCH_END_BLOCK-1);
  
  // Allocate mem for write and read buffers (only once)
  hNandWriteBuf = UTIL_allocMem(hNandInfo->dataBytesPerPage);
  hNandReadBuf  = UTIL_allocMem(hNandInfo->dataBytesPerPage);
  
  // Clear buffers
  for (i=0; i < hNandInfo->dataBytesPerPage; i++)
  {
    hNandWriteBuf[i] = 0xFF;
    hNandReadBuf[i] = 0xFF;
  }

  switch(bootCmd)
  {
    case UBL_MAGIC_FLASH_NO_UBL:
    {
   
      // ------ Get Boot Image Data and Write it to Flash ------       
      // Get the UBL header and data
      if (LOCAL_recvHeaderAndData(&ackHeader) != E_PASS)
      {
        return E_FAIL;
      }
      // Write boot image to the NAND
      if (LOCAL_NANDWriteData(hNandInfo, ackHeader.imageBuff, ackHeader.byteCnt) != E_PASS)
      {
        return E_FAIL;
      }

      // Set the entry point to nowhere, since there isn't an appropriate binary image to run
      gEntryPoint = 0x0;
      break;    
    
    }
    case UBL_MAGIC_FLASH:
    {
      // ------ Get UBL Data and Write it to Flash ------       
      // Get the UBL header and data
      if (LOCAL_recvHeaderAndData(&ackHeader) != E_PASS)
      {
        return E_FAIL;
      }
        
      //DEBUG_printString("Writing UBL to NAND flash\r\n");
    #if defined(AIS_RBL)
      if (LOCAL_NANDWriteData(hNandInfo, ackHeader.imageBuff, ackHeader.byteCnt) != E_PASS)
      {
        LOCAL_sendSequence("   FAIL");
        return E_FAIL;
      }      
    #else // DAVINCI_RBL
      // Setup fixed elements of the NANDBOOT header that will be stored in flash for UBL
      nandBoot.magicNum = ackHeader.magicNum;
      nandBoot.entryPoint = ackHeader.startAddr;
      nandBoot.page = 1;                          // The page is always page 0 for the UBL header, so we use page 1 for data        
      nandBoot.ldAddress = ackHeader.loadAddr;    // This field doesn't matter for the UBL header      
      nandBoot.forceContigImage = TRUE;
      nandBoot.startBlock = DEVICE_NAND_RBL_SEARCH_START_BLOCK;
      nandBoot.endBlock = DEVICE_NAND_RBL_SEARCH_END_BLOCK;
        
      // Calculate the number of NAND pages needed to store the UBL image
      nandBoot.numPage = 0;
      while ( (nandBoot.numPage * hNandInfo->dataBytesPerPage) < (ackHeader.byteCnt))
      {
        nandBoot.numPage++;
      }     
    
      // Write multiple copies of the UBL to the appropriate RBL search blocks
      if (LOCAL_NANDWriteHeaderAndData(hNandInfo, &nandBoot, ackHeader.imageBuff) != E_PASS)
      {
        LOCAL_sendSequence("   FAIL");
        return E_FAIL;
      }      
    #endif
      
      LOCAL_sendSequence("SENDING");              
      // Return DONE when UBL flash operation has been completed
      if ( LOCAL_sendSequence("   DONE") != E_PASS )
      {
        return E_FAIL;
      }

      // ------ Get Application Data and Write it to Flash ------       
      // Get the application header and data
      if (LOCAL_recvHeaderAndData(&ackHeader) != E_PASS)
      {
        return E_FAIL;
      }
      
      // Setup fixed elements of the NANDBOOT header that will be stored in flash for APP
      nandBoot.magicNum         = ackHeader.magicNum;         // Rely on the host applciation to send over the right magic number (safe or bin)
      //nandBoot.magicNum         = UBL_MAGIC_BINARY_BOOT;
	  //nandBoot.magicNum         = UBL_MAGIC_DMA;
      nandBoot.entryPoint       = ackHeader.startAddr;      // Use the entrypoint received in ACK header
      nandBoot.page             = 1;                              // The page is always page 0 for the header, so we use page 1 for data
      nandBoot.ldAddress        = ackHeader.loadAddr;        // The load address is only important if this is a binary image
      nandBoot.forceContigImage = FALSE;
      nandBoot.startBlock       = DEVICE_NAND_UBL_SEARCH_START_BLOCK;
      nandBoot.endBlock         = DEVICE_NAND_UBL_SEARCH_END_BLOCK;      
      
      // Calculate the number of NAND pages needed to store the APP image
      nandBoot.numPage = 0;
      while ( (nandBoot.numPage * hNandInfo->dataBytesPerPage) < ackHeader.byteCnt )
      {
        nandBoot.numPage++;
      }

      // Write multiple copies of the APP to the appropriate UBL search blocks
      //DEBUG_printString("Writing APP to NAND flash\r\n");
      if (LOCAL_NANDWriteHeaderAndData(hNandInfo, &nandBoot, ackHeader.imageBuff) != E_PASS)
      {
        LOCAL_sendSequence("   FAIL");
        return E_FAIL;
      }

      // Set the entry point to nowhere, since there isn't an appropriate binary image to run
      gEntryPoint = 0x0;
      break;
    }  

#if defined(OMAPL137_v2) || defined(OMAPL137_v1)
 case UBL_MAGIC_FLASH_DSP:
    {
      // ------ Get DSP UBL Data and Write it to Flash ------       
      // Get the UBL header and data
      if (LOCAL_recvHeaderAndData(&ackHeader) != E_PASS)
      {
        return E_FAIL;
      }
        
      //DEBUG_printString("Writing UBL to NAND flash\r\n");
      if (LOCAL_NANDWriteData(hNandInfo, ackHeader.imageBuff, ackHeader.byteCnt) != E_PASS)
      {
        LOCAL_sendSequence("   FAIL");
        return E_FAIL;
      }      

      LOCAL_sendSequence("SENDING");              
      // Return DONE when UBL flash operation has been completed
      if ( LOCAL_sendSequence("   DONE") != E_PASS )
      {
        return E_FAIL;
      }

      // ------ Get ARM UBL Data and Write it to Flash ------       
      // Get the UBL header and data      
	 if (LOCAL_recvHeaderAndData(&ackHeader) != E_PASS)
      {
        return E_FAIL;
      }
      
      // Setup fixed elements of the NANDBOOT header that will be stored in flash for APP
    //  nandBoot.magicNum         = ackHeader.magicNum;         // Rely on the host applciation to send over the right magic number (safe or bin)
      nandBoot.magicNum         = UBL_MAGIC_BIN_IMG;
      nandBoot.entryPoint       = ackHeader.startAddr;      // Use the entrypoint received in ACK header
      nandBoot.page             = 1;                              // The page is always page 0 for the header, so we use page 1 for data
      nandBoot.ldAddress        = ackHeader.loadAddr;        // The load address is only important if this is a binary image
      nandBoot.forceContigImage = FALSE;
      nandBoot.startBlock       = DEVICE_NAND_ARMUBL_SEARCH_START_BLOCK;
      nandBoot.endBlock         = DEVICE_NAND_ARMUBL_SEARCH_END_BLOCK;      
      
      // Calculate the number of NAND pages needed to store the APP image
      nandBoot.numPage = 0;
      while ( (nandBoot.numPage * hNandInfo->dataBytesPerPage) < ackHeader.byteCnt )
      {
        nandBoot.numPage++;
      }

      // Write multiple copies of the APP to the appropriate UBL search blocks
      //DEBUG_printString("Writing APP to NAND flash\r\n");
      if (LOCAL_NANDWriteHeaderAndData(hNandInfo, &nandBoot, ackHeader.imageBuff) != E_PASS)
      {
        LOCAL_sendSequence("   FAIL");
        return E_FAIL;
      }

      LOCAL_sendSequence("SENDING");              
      // Return DONE when UBL flash operation has been completed
      if ( LOCAL_sendSequence("   DONE") != E_PASS )
      {
        return E_FAIL;
      }

      // ------ Get Application Data and Write it to Flash ------       
      // Get the application header and data
      if (LOCAL_recvHeaderAndData(&ackHeader) != E_PASS)
      {
        return E_FAIL;
      }
      
      // Setup fixed elements of the NANDBOOT header that will be stored in flash for APP
    //  nandBoot.magicNum         = ackHeader.magicNum;         // Rely on the host applciation to send over the right magic number (safe or bin)
      nandBoot.magicNum         = UBL_MAGIC_DMA;
      nandBoot.entryPoint       = ackHeader.startAddr;      // Use the entrypoint received in ACK header
      nandBoot.page             = 1;                              // The page is always page 0 for the header, so we use page 1 for data
      nandBoot.ldAddress        = ackHeader.loadAddr;        // The load address is only important if this is a binary image
      nandBoot.forceContigImage = FALSE;
      nandBoot.startBlock       = DEVICE_NAND_UBOOT_SEARCH_START_BLOCK;
      nandBoot.endBlock         = DEVICE_NAND_UBOOT_SEARCH_END_BLOCK;      
      
      // Calculate the number of NAND pages needed to store the APP image
      nandBoot.numPage = 0;
      while ( (nandBoot.numPage * hNandInfo->dataBytesPerPage) < ackHeader.byteCnt )
      {
        nandBoot.numPage++;
      }

      // Write multiple copies of the APP to the appropriate UBL search blocks
      //DEBUG_printString("Writing APP to NAND flash\r\n");
      if (LOCAL_NANDWriteHeaderAndData(hNandInfo, &nandBoot, ackHeader.imageBuff) != E_PASS)
      {
        LOCAL_sendSequence("   FAIL");
        return E_FAIL;
      }

      // Set the entry point to nowhere, since there isn't an appropriate binary image to run
      gEntryPoint = 0x0;
      break;
    }  
#endif //#if defined(OMAPL137_v2) || defined(OMAPL137_v1)
	
    case UBL_MAGIC_ERASE:
    {
      // Check if device is write protected
      if (NAND_isWriteProtected(hNandInfo))
      {
        DEBUG_printString("NAND is write protected!\r\n");
        return E_FAIL;
      }
    
      // Send SIZE keyword followed by number of bytes in flash to SFH
      if ( LOCAL_sendSequence("   SIZE") != E_PASS )
        return E_FAIL;
    
      // Send number of bytes
      DEBUG_printInt(1);
      LOCAL_sendSequence("");

      LOCAL_sendSequence("SENDING");

      // Erase all the pages of the device
      if (NAND_eraseBlocks(hNandInfo, DEVICE_NAND_RBL_SEARCH_START_BLOCK, DEVICE_NAND_UBL_SEARCH_END_BLOCK) != E_PASS)
      {
        LOCAL_sendSequence("   FAIL");
        return E_FAIL;
      }
  
      // Protect the device
      NAND_protectBlocks(hNandInfo);

      // Set the entry point for code execution
      // Go to reset in this case since no code was downloaded 
      gEntryPoint = 0x0; 
      break;
    }
    default:
    {
      return E_FAIL;
    }
  }
  
  // Return DONE when UBL flash operation has been completed
  if ( LOCAL_sendSequence("   DONE") != E_PASS )
  {
    return E_FAIL;
  }

  return E_PASS;
}

#elif defined(UBL_NOR)
static Uint32 LOCAL_NOR_commands(Uint32 bootCmd)
{
  Uint32              baseAddr;
  int          eraseSize, erase_chunksize, i, size, chunksize;
  NORBOOT_HeaderObj   norBoot;
  NOR_InfoHandle      hNorInfo;

  chunksize = 4096;
  // Initialize the NOR Flash
  baseAddr = DEVICE_ASYNC_MEM_interfaces[DEVICE_ASYNC_MEM_NORBOOT_INTERFACE].regionStarts[DEVICE_ASYNC_MEM_NORBOOT_REGION];
  hNorInfo = NOR_open(baseAddr, (Uint8)DEVICE_emifBusWidth() );
  if ( hNorInfo ==  NULL )
  {
    DEBUG_printString("NOR_open() failed!");
    return E_FAIL;
  }

  switch(bootCmd)
  {
    case UBL_MAGIC_FLASH_NO_UBL:
    {    
      // Get the boot image in binary form
      if ( LOCAL_recvHeaderAndData(&ackHeader) != E_PASS )
      {
        return E_FAIL;
      }
      
      // Erasing the Flash
      if ( NOR_erase(hNorInfo,hNorInfo->flashBase, ackHeader.byteCnt) != E_PASS )
      {
        return E_FAIL;
      }

  //Send in byte chunks
      for(i=0;i<=ackHeader.byteCnt;i+=chunksize)
      {
        if(ackHeader.byteCnt-i < chunksize)
        {
          size=ackHeader.byteCnt-i;
        }
        else
        {
          size=chunksize;
        }

        // Write the boot image data to the flash
        if (NOR_writeBytes(hNorInfo, hNorInfo->flashBase+i, size, (Uint32) ackHeader.imageBuff+i) != E_PASS)
        {
          DEBUG_printString("\tERROR: Writing NOR failed.\r\n");
          return E_FAIL;
        }
         LOCAL_sendSequence("SENDING");
      }
      if ( LOCAL_sendSequence("   DONE") != E_PASS )
      {
        return E_FAIL;
      }
      // Set the entry point for code execution
      gEntryPoint = hNorInfo->flashBase;
      break;
    }
    case UBL_MAGIC_FLASH:
    {
      Uint32 blockSize, blockAddr;
    
      // Get the secondary boot loader data in binary form
      if (LOCAL_recvHeaderAndData(&ackHeader) != E_PASS)
      {
        return E_FAIL;
      }

      // Erasing the flash
      if ( NOR_erase(hNorInfo,hNorInfo->flashBase, ackHeader.byteCnt) != E_PASS )
      {
        return E_FAIL;
      }
          
      // Write the secondary boot loader data to the start of the NOR flash
    //Send in byte chunks
      for(i=0;i<=ackHeader.byteCnt;i+=chunksize)
      {
        if(ackHeader.byteCnt-i < chunksize)
        {
          size=ackHeader.byteCnt-i;
        }
        else
        {
          size=chunksize;
        }

        // Write the boot image data to the flash
        if (NOR_writeBytes(hNorInfo, hNorInfo->flashBase+i, size, (Uint32) ackHeader.imageBuff+i) != E_PASS)
        {
          DEBUG_printString("\tERROR: Writing NOR failed.\r\n");
          return E_FAIL;
        }
         LOCAL_sendSequence("SENDING");
      }
     
      // Return DONE when UBL flash operation has been completed
      if ( LOCAL_sendSequence("   DONE") != E_PASS )
      {
        return E_FAIL;
      }
        
      // Get block size and base of block where UBL was written
      NOR_getBlockInfo(hNorInfo,hNorInfo->flashBase+ackHeader.byteCnt,&blockSize,&blockAddr);

      // Get the application header and data
      if (LOCAL_recvHeaderAndData(&ackHeader) != E_PASS)
      {
        return E_FAIL;
      }
        
      // Setup the NORBOOT header that will be stored in flash
      norBoot.magicNum = ackHeader.magicNum;
      norBoot.entryPoint = ackHeader.startAddr;
      norBoot.appSize = ackHeader.byteCnt;
      norBoot.ldAddress = ackHeader.loadAddr;

      // Erasing the Flash
      if ( NOR_erase(hNorInfo,(blockAddr + blockSize), (ackHeader.byteCnt + sizeof(NORBOOT_HeaderObj))) != E_PASS )
      {
        return E_FAIL;
      }
          
      // Write the NORBOOT header to the flash
      DEBUG_printString("Writing APP to NOR flash\r\n");
      if (NOR_writeBytes(hNorInfo,(blockAddr + blockSize), sizeof(NORBOOT_HeaderObj), (Uint32)&norBoot) != E_PASS )
      {
        return E_FAIL;
      }
      

  //Send in byte chunks
      for(i=0;i<=norBoot.appSize;i+=chunksize)
      {
        if(norBoot.appSize-i < chunksize)
        {
          size=norBoot.appSize-i;
        }
        else
        {
          size=chunksize;
        }

        // Write the boot image data to the flash
        if (NOR_writeBytes(hNorInfo,(blockAddr + blockSize + sizeof(NORBOOT_HeaderObj))+i, size, (Uint32) ackHeader.imageBuff+i) != E_PASS)
        {
          DEBUG_printString("\tERROR: Writing NOR failed.\r\n");
          return E_FAIL;
        }
      LOCAL_sendSequence("SENDING");
    }
     
      // Return DONE when UBL flash operation has been completed
      if ( LOCAL_sendSequence("   DONE") != E_PASS )
      {
        return E_FAIL;
      }


      // Set the entry point to nowhere, since there isn't an appropriate binary image to run */
      gEntryPoint = 0x0;
      break;
    }  
    case UBL_MAGIC_ERASE:
    {
      // Erasing the Flash 
    eraseSize = hNorInfo->flashSize;
    if ( LOCAL_sendSequence("   SIZE") != E_PASS )
        return E_FAIL;

    // Send number of bytes
    DEBUG_printInt(eraseSize);
    LOCAL_sendSequence("");
    erase_chunksize = 65536*2;

    for(i=0;i<=eraseSize;i+=erase_chunksize) {
      if(eraseSize-i < erase_chunksize) {
        size=eraseSize-i;
      }
      else
        size=erase_chunksize;

      // Erase a chunk of the flash
      if (NOR_erase(hNorInfo, hNorInfo->flashBase+i, size) != E_PASS)
      {
      DEBUG_printString("\tERROR: Erasing NOR failed.\r\n");
      return E_FAIL;
      }

       LOCAL_sendSequence("SENDING");
    }    

      // Set the entry point for code execution
      // Go to reset in this case since no code was downloaded
      gEntryPoint = 0x0; 
    
    if ( LOCAL_sendSequence("   DONE") != E_PASS )
        return E_FAIL;
      break;
    }
    default:
    {
      return E_FAIL;
    }
  }
  return E_PASS;
}

#elif defined(UBL_SPI_MEM)
static Uint32 LOCAL_SPIMEM_commands(Uint32 bootCmd)
{
  int i;
  int size;
  int eraseSize;
  int chunksize = 4096;
  int erase_chunksize = 65536*2;
  Uint32 imageSize;
  Uint8 *imageBuff,*imageBuff2;
  SPI_MEM_InfoHandle hSpiMemInfo;
  SPI_MEM_BOOT_HeaderObj spiBoot;
  
  // Initialize SPI Memory Device on device-specific SPI peripheral and chip-select
  hSpiMemInfo = SPI_MEM_open(DEVICE_SPIBOOT_PERIPHNUM, DEVICE_SPIBOOT_CSNUM, hDEVICE_SPI_config);
  if (hSpiMemInfo == NULL)
  {
    DEBUG_printString( "\tERROR: SPI Memory Initialization failed.\r\n" );
    return E_FAIL;
  }
  DEBUG_printString( "\tINFO: SPI Memory Initialization passed.\n" );

  switch(bootCmd)
  {
    case UBL_MAGIC_FLASH_NO_UBL:
    {
      // Read the application file from host
      if (LOCAL_recvHeaderAndData(&ackHeader) != E_PASS)
      {
        DEBUG_printString( "\tERROR: Header and data read failed.\r\n" );
        return E_FAIL;
      }

      // Erase the SPI memory to accomodate the boot image data
      if (SPI_MEM_eraseBytes( hSpiMemInfo, 0x00, ackHeader.byteCnt ) != E_PASS)
      {
        DEBUG_printString("\tERROR: Erasing SPI failed.\r\n");
        return E_FAIL;
      }

      // Make copy of the file data to compare against after the write
      imageBuff2 = (Uint8 *) UTIL_allocMem(ackHeader.byteCnt);
      UTIL_memcpy(imageBuff2, ackHeader.imageBuff, ackHeader.byteCnt);
      
      //Send in byte chunks
      for(i=0;i<=ackHeader.byteCnt;i+=chunksize)
      {
        if( (ackHeader.byteCnt-i) < chunksize)
        {
          size=(ackHeader.byteCnt-i);
        }
        else
        {
          size=chunksize;
        }

        // Write the boot image data to the flash
        if (SPI_MEM_writeBytes(hSpiMemInfo, 0x00+i, size, ackHeader.imageBuff+i) != E_PASS)
        {
          DEBUG_printString("\tERROR: Writing SPI failed.\r\n");
          return E_FAIL;
        }
         LOCAL_sendSequence("SENDING");
      }
    
      // Verify image was written correctly
      if (SPI_MEM_verifyBytes(hSpiMemInfo, 0x00, ackHeader.byteCnt, imageBuff2, ackHeader.imageBuff) != E_PASS)
      {
        DEBUG_printString("\tERROR: Data didnt match. Writing SPI failed.\r\n");
        return E_FAIL;
      }
      
      DEBUG_printString("\tSPI written correctly.\r\n");
      if ( LOCAL_sendSequence("   DONE") != E_PASS )
      {
        return E_FAIL;
      }

      // Set the entry point for code execution
      gEntryPoint = 0x00;
      
      break;
    }
    case UBL_MAGIC_FLASH:
    {
      DEBUG_printString( "Flashing UBL...\r\n" );
      
      // Get the UBL header and data from the host
      if (LOCAL_recvHeaderAndData(&ackHeader) != E_PASS)
      {
        return E_FAIL;
      }
      
    #if defined(AIS_RBL)

      // Erase the SPI memory to accomodate the boot image data
      if (SPI_MEM_eraseBytes( hSpiMemInfo, 0x00, ackHeader.byteCnt ) != E_PASS)
      {
        DEBUG_printString("\tERROR: Erasing SPI failed.\r\n");
        return E_FAIL;
      }

      // Make copy of the file data to compare against after the write
      imageBuff2 = (Uint8 *) UTIL_allocMem(ackHeader.byteCnt);
      UTIL_memcpy(imageBuff2, ackHeader.imageBuff, ackHeader.byteCnt);

      //Send in byte chunks
      for(i=0;i<=ackHeader.byteCnt;i+=chunksize) 
      {
        if( (ackHeader.byteCnt-i) < chunksize)
        {
          size=(ackHeader.byteCnt-i);
        }
        else
        {
          size=chunksize;
        }

        // Write the boot image data to the flash
        if (SPI_MEM_writeBytes(hSpiMemInfo, 0x00+i, size, ackHeader.imageBuff+i) != E_PASS)
        {
          DEBUG_printString("\tERROR: Writing SPI failed.\r\n");
          return E_FAIL;
        }
        
        LOCAL_sendSequence("SENDING");
      }

      // Verify image was written correctly
      if (SPI_MEM_verifyBytes(hSpiMemInfo, 0x00, ackHeader.byteCnt, imageBuff2, ackHeader.imageBuff) != E_PASS)
      {
        DEBUG_printString("\tERROR: Data didnt match. Writing SPI failed.\r\n");
        return E_FAIL;
      }
      DEBUG_printString("UBL flashed correctly.\r\n");    
    #else // DAVINCI_RBL
       
      // Setup the SPIBoot header
      spiBoot.magicNum = ackHeader.magicNum;
      spiBoot.appSize = ackHeader.byteCnt;
      spiBoot.entryPoint = ackHeader.startAddr;
      spiBoot.ldAddress = ackHeader.loadAddr;
      spiBoot.memAddress = sizeof(spiBoot);
      
      // Make copy of the file data to compare against after the write
      imageBuff = (Uint8 *) UTIL_allocMem(ackHeader.byteCnt + sizeof(SPI_MEM_BOOT_HeaderObj));
      imageSize = ackHeader.byteCnt + sizeof(SPI_MEM_BOOT_HeaderObj);
      
      UTIL_memcpy(imageBuff, &spiBoot, sizeof(SPI_MEM_BOOT_HeaderObj));
      UTIL_memcpy((imageBuff + sizeof(SPI_MEM_BOOT_HeaderObj)), ackHeader.imageBuff, ackHeader.byteCnt);

      // Erase the SPI memory to accomodate the boot image data
      if (SPI_MEM_eraseBytes( hSpiMemInfo, 0x00, imageSize ) != E_PASS)
      {
        DEBUG_printString("\tERROR: Erasing SPI failed.\r\n");
        return E_FAIL;
      }

      // Make copy of the file data to compare against after the write
      imageBuff2 = (Uint8 *) UTIL_allocMem(imageSize);
      UTIL_memcpy(imageBuff2, imageBuff, imageSize);
      
      //Send in byte chunks
      for(i=0;i<=imageSize;i+=chunksize)
      {
        if((imageSize-i) < chunksize)
        {
          size=(imageSize-i);
        }
        else
        {
          size=chunksize;
        }

        // Write the boot image data to the flash
        if (SPI_MEM_writeBytes(hSpiMemInfo, 0x00+i, size, imageBuff+i) != E_PASS)
        {
          DEBUG_printString("\tERROR: Writing SPI failed.\r\n");
          return E_FAIL;
        }

        LOCAL_sendSequence("SENDING");
      }

      DEBUG_printString("Finished writing to SPI, now verifying.\r\n");
      // Verify image was written correctly
      if (SPI_MEM_verifyBytes(hSpiMemInfo, 0x00, imageSize, imageBuff2, imageBuff) != E_PASS)
      {
        DEBUG_printString("ERROR: Data didnt match. Writing SPI failed.\r\n");
        return E_FAIL;
      }    
    #endif
    
      // Return DONE when UBL flash operation has been completed
      if ( LOCAL_sendSequence("   DONE") != E_PASS )
        return E_FAIL;

      // ------ Get Application Data and Write it to Flash ------
      // Get the Application image header and data
      if (LOCAL_recvHeaderAndData(&ackHeader) != E_PASS)
      {
        return E_FAIL;
      }
       
      // Setup the SPIBoot header
      spiBoot.magicNum = ackHeader.magicNum;
      spiBoot.appSize = ackHeader.byteCnt;
      spiBoot.entryPoint = ackHeader.startAddr;
      spiBoot.ldAddress = ackHeader.loadAddr;
      spiBoot.memAddress = hSpiMemInfo->hMemParams->blockSize + sizeof(SPI_MEM_BOOT_HeaderObj);

      // Make copy of the file data to compare against after the write
      imageBuff = (Uint8 *) UTIL_allocMem(ackHeader.byteCnt + sizeof(SPI_MEM_BOOT_HeaderObj));
      imageSize = ackHeader.byteCnt + sizeof(SPI_MEM_BOOT_HeaderObj);
      
      UTIL_memcpy(imageBuff, &spiBoot, sizeof(SPI_MEM_BOOT_HeaderObj));
      UTIL_memcpy((imageBuff + sizeof(SPI_MEM_BOOT_HeaderObj)), ackHeader.imageBuff, ackHeader.byteCnt);

      // Erase the SPI memory to accomodate the boot image data
      if (SPI_MEM_eraseBytes( hSpiMemInfo, hSpiMemInfo->hMemParams->blockSize, imageSize ) != E_PASS)
      {
        DEBUG_printString("\tERROR: Erasing SPI failed.\r\n");
        return E_FAIL;
      }

      // Make copy of the file data to compare against after the write
      imageBuff2 = (Uint8 *) UTIL_allocMem(imageSize);
      UTIL_memcpy(imageBuff2, imageBuff, imageSize);
      
      //Send in byte chunks
      for(i=0;i<=imageSize;i+=chunksize)
      {
        if(imageSize-i < chunksize)
        {
          size=imageSize-i;
        }
        else
        {
          size=chunksize;
        }

        // Write the boot image data to the flash
        if (SPI_MEM_writeBytes(hSpiMemInfo, hSpiMemInfo->hMemParams->blockSize+i, size, imageBuff+i) != E_PASS)
        {
          DEBUG_printString("\tERROR: Writing SPI failed.\r\n");
          return E_FAIL;
        }

        LOCAL_sendSequence("SENDING");
      }

      DEBUG_printString("Finished writing to SPI, now verifying.\r\n");
      // Verify image was written correctly
      if (SPI_MEM_verifyBytes(hSpiMemInfo, hSpiMemInfo->hMemParams->blockSize, imageSize, imageBuff2, imageBuff) != E_PASS)
      {
        DEBUG_printString("ERROR: Data didn't match. Writing SPI failed.\r\n");
        return E_FAIL;
      }
              
      // Return DONE when application flash operation has been completed
      if ( LOCAL_sendSequence("   DONE") != E_PASS )
        return E_FAIL;

      // Set the entry point to nowhere, since there isn't an appropriate binary image to run */
      gEntryPoint = 0x0;
      break;
    }
#if defined(OMAPL137_v2) || defined(OMAPL137_v1)
    case UBL_MAGIC_FLASH_DSP:
    {
      DEBUG_printString( "Flashing DSP UBL...\r\n" );
      
      // Get the UBL header and data from the host
      if (LOCAL_recvHeaderAndData(&ackHeader) != E_PASS)
      {
        return E_FAIL;
      }
      
      // Erase the SPI memory to accomodate the boot image data
      if (SPI_MEM_eraseBytes( hSpiMemInfo, 0x00, ackHeader.byteCnt ) != E_PASS)
      {
        DEBUG_printString("\tERROR: Erasing SPI failed.\r\n");
        return E_FAIL;
      }

      // Make copy of the file data to compare against after the write
      imageBuff2 = (Uint8 *) UTIL_allocMem(ackHeader.byteCnt);
      UTIL_memcpy(imageBuff2, ackHeader.imageBuff, ackHeader.byteCnt);

      //Send in byte chunks
      for(i=0;i<=ackHeader.byteCnt;i+=chunksize) 
      {
        if( (ackHeader.byteCnt-i) < chunksize)
        {
          size=(ackHeader.byteCnt-i);
        }
        else
        {
          size=chunksize;
        }

        // Write the boot image data to the flash
        if (SPI_MEM_writeBytes(hSpiMemInfo, 0x00+i, size, ackHeader.imageBuff+i) != E_PASS)
        {
          DEBUG_printString("\tERROR: Writing SPI failed.\r\n");
          return E_FAIL;
        }
        
        LOCAL_sendSequence("SENDING");
      }

      // Verify image was written correctly
      if (SPI_MEM_verifyBytes(hSpiMemInfo, 0x00, ackHeader.byteCnt, imageBuff2, ackHeader.imageBuff) != E_PASS)
      {
        DEBUG_printString("\tERROR: Data didnt match. Writing SPI failed.\r\n");
        return E_FAIL;
      }
      DEBUG_printString("UBL flashed correctly.\r\n");    
    
      // Return DONE when UBL flash operation has been completed
      if ( LOCAL_sendSequence("   DONE") != E_PASS )
        return E_FAIL;


      DEBUG_printString( "Flashing ARM UBL...\r\n" );
      
      // Get the ARM UBL header and data from the host
      if (LOCAL_recvHeaderAndData(&ackHeader) != E_PASS)
      {
        return E_FAIL;
      }

      // Erase the SPI memory to accomodate the boot image data
      if (SPI_MEM_eraseBytes( hSpiMemInfo, 0x2000, ackHeader.byteCnt ) != E_PASS)
      {
        DEBUG_printString("\tERROR: Erasing SPI failed.\r\n");
        return E_FAIL;
      }

      // Make copy of the file data to compare against after the write
      imageBuff2 = (Uint8 *) UTIL_allocMem(ackHeader.byteCnt);
      UTIL_memcpy(imageBuff2, ackHeader.imageBuff, ackHeader.byteCnt);

      //Send in byte chunks
      for(i=0;i<=ackHeader.byteCnt;i+=chunksize) 
      {
        if( (ackHeader.byteCnt-i) < chunksize)
        {
          size=(ackHeader.byteCnt-i);
        }
        else
        {
          size=chunksize;
        }

        // Write the boot image data to the flash
        if (SPI_MEM_writeBytes(hSpiMemInfo, 0x2000+i, size, ackHeader.imageBuff+i) != E_PASS)
        {
          DEBUG_printString("\tERROR: Writing SPI failed.\r\n");
          return E_FAIL;
        }
        
        LOCAL_sendSequence("SENDING");
      }

      // Verify image was written correctly
      if (SPI_MEM_verifyBytes(hSpiMemInfo, 0x2000, ackHeader.byteCnt, imageBuff2, ackHeader.imageBuff) != E_PASS)
      {
        DEBUG_printString("\tERROR: Data didnt match. Writing SPI failed.\r\n");
        return E_FAIL;
      }
      DEBUG_printString("UBL flashed correctly.\r\n");    

      // Return DONE when UBL flash operation has been completed
      if ( LOCAL_sendSequence("   DONE") != E_PASS )
        return E_FAIL;


      // ------ Get Application Data and Write it to Flash ------
      // Get the Application image header and data
      if (LOCAL_recvHeaderAndData(&ackHeader) != E_PASS)
      {
        return E_FAIL;
      }
       	   
      // Setup the SPIBoot header
      spiBoot.magicNum = ackHeader.magicNum;
      spiBoot.appSize = ackHeader.byteCnt;
      spiBoot.entryPoint = ackHeader.startAddr;
      spiBoot.ldAddress = ackHeader.loadAddr;
      spiBoot.memAddress = 0x8000 + sizeof(SPI_MEM_BOOT_HeaderObj);

      // Make copy of the file data to compare against after the write
      imageBuff = (Uint8 *) UTIL_allocMem(ackHeader.byteCnt + sizeof(SPI_MEM_BOOT_HeaderObj));
      imageSize = ackHeader.byteCnt + sizeof(SPI_MEM_BOOT_HeaderObj);
      
      UTIL_memcpy(imageBuff, &spiBoot, sizeof(SPI_MEM_BOOT_HeaderObj));
      UTIL_memcpy((imageBuff + sizeof(SPI_MEM_BOOT_HeaderObj)), ackHeader.imageBuff, ackHeader.byteCnt);

      // Erase the SPI memory to accomodate the boot image data
      if (SPI_MEM_eraseBytes( hSpiMemInfo, 0x8000, imageSize ) != E_PASS)
      {
        DEBUG_printString("\tERROR: Erasing SPI failed.\r\n");
        return E_FAIL;
      }

      // Make copy of the file data to compare against after the write
      imageBuff2 = (Uint8 *) UTIL_allocMem(imageSize);
      UTIL_memcpy(imageBuff2, imageBuff, imageSize);
      
      //Send in byte chunks
      for(i=0;i<=imageSize;i+=chunksize)
      {
        if(imageSize-i < chunksize)
        {
          size=imageSize-i;
        }
        else
        {
          size=chunksize;
        }

        // Write the boot image data to the flash
        if (SPI_MEM_writeBytes(hSpiMemInfo, 0x8000+i, size, imageBuff+i) != E_PASS)
        {
          DEBUG_printString("\tERROR: Writing SPI failed.\r\n");
          return E_FAIL;
        }

        LOCAL_sendSequence("SENDING");
      }

      DEBUG_printString("Finished writing to SPI, now verifying.\r\n");
      // Verify image was written correctly
      if (SPI_MEM_verifyBytes(hSpiMemInfo, 0x8000, imageSize, imageBuff2, imageBuff) != E_PASS)
      {
        DEBUG_printString("ERROR: Data didn't match. Writing SPI failed.\r\n");
        return E_FAIL;
      }
              
      // Return DONE when application flash operation has been completed
      if ( LOCAL_sendSequence("   DONE") != E_PASS )
        return E_FAIL;

      // Set the entry point to nowhere, since there isn't an appropriate binary image to run */
      gEntryPoint = 0x0;
      break;
    }
#endif //#if defined(OMAPL137_v2) || defined(OMAPL137_v1)	

    case UBL_MAGIC_ERASE:
    {
      eraseSize = hDEVICE_SPI_MEM_params->memorySize;
      //eraseSize = 8000;
      //DEBUG_printString("Erasing the spi flash!\r\n"); 

      // Send SIZE keyword followed by number of bytes in flash to SFH
      if ( LOCAL_sendSequence("   SIZE") != E_PASS )
          return E_FAIL;
    
      // Send number of bytes
      DEBUG_printInt(eraseSize);
      LOCAL_sendSequence("");
      erase_chunksize = 65536*2;
      //Erase in block chunks
      for(i=0;i<=eraseSize;i+=erase_chunksize)
      {
        if((eraseSize-i) < erase_chunksize)
        {
          size=eraseSize-i;
        }
        else
        {
          size=erase_chunksize;
        }
      
        // Erase a chunk of the flash
        if (SPI_MEM_eraseBytes(hSpiMemInfo, 0x00+i, size) != E_PASS)
        {
          DEBUG_printString("\tERROR: Erasing SPI failed.\r\n");
          return E_FAIL;
        }

        LOCAL_sendSequence("SENDING");
      }

      imageBuff2 = (Uint8 *) UTIL_allocMem(hDEVICE_SPI_MEM_params->memorySize);
      if (SPI_MEM_verifyErase(hSpiMemInfo, 0x00,  hDEVICE_SPI_MEM_params->memorySize, imageBuff2) != E_PASS)
      {
        DEBUG_printString("\tERROR: Erasing SPI failed.\r\n");
        return E_FAIL;
      }
      // Return DONE when UBL flash operation has been completed
      if ( LOCAL_sendSequence("   DONE") != E_PASS )
      {
        return E_FAIL;
      }
      break;
    }
    default:
    {
      return E_FAIL;
    }
  }
  return E_PASS;
}

#elif defined(UBL_I2C_MEM)
Uint32 LOCAL_I2CMEM_commands(Uint32 bootCmd)
{
  switch(bootCmd)
  {
    case UBL_MAGIC_FLASH:
    {
      break;
    }
    case UBL_MAGIC_ERASE:
    {
      break;
    }
    default:
    {
      return E_FAIL;
    }
  }
  return E_PASS;
}

#elif defined(UBL_SDMMC)

Uint32 LOCAL_SDMMC_commands(Uint32 bootCmd)
{
  switch(bootCmd)
  {
    case UBL_MAGIC_FLASH:
    {
      break;
    }
    case UBL_MAGIC_ERASE:
    {
      break;
    }
    default:
    {
      return E_FAIL;
    }
  }
  return E_PASS;
}

#elif defined(UBL_ONENAND)
Uint32 LOCAL_ONENAND_commands(Uint32 bootCmd)
{
  switch(bootCmd)
  {
    case UBL_MAGIC_FLASH:
    {
      break;
    }
    case UBL_MAGIC_ERASE:
    {
      break;
    }
    default:
    {
      return E_FAIL;
    }
  }
  
  return E_PASS;
}
#endif

/************************************************************
* Local Function Definitions                                *
************************************************************/

static Uint32 LOCAL_sendSequence(String s)
{
  return UART_sendString(hUartInfo, s, TRUE);
}

static Uint32 LOCAL_recvSequence(String s)
{
  return UART_checkSequence(hUartInfo, s, TRUE);
}

static Uint32 LOCAL_recvCommand(Uint32* bootCmd)
{
  if(UART_checkSequence(hUartInfo, "    CMD", TRUE) != E_PASS)
  {
    return E_FAIL;
  }

  if(UART_recvHexData(hUartInfo, 4,bootCmd) != E_PASS)
  {
    return E_FAIL;
  }

  return E_PASS;
}

static Uint32 LOCAL_recvHeaderAndData(UARTBOOT_HeaderHandle ackHeader)
{
  Uint32  error = E_PASS, recvLen;
  Uint32  maxImageSize,minStartAddr,maxStartAddr;
  
  // Issue command to host to send image
  if ( LOCAL_sendSequence("SENDIMG") != E_PASS)
  {
    return E_FAIL;
  }

  // Recv ACK command
  if(UART_checkSequence(hUartInfo, "    ACK", TRUE) != E_PASS)
  {
    return E_FAIL;
  }

  // Get the ACK header elements
  error =  UART_recvHexData( hUartInfo, 4, (Uint32 *) &(ackHeader->magicNum)  );
  error |= UART_recvHexData( hUartInfo, 4, (Uint32 *) &(ackHeader->startAddr) );
  error |= UART_recvHexData( hUartInfo, 4, (Uint32 *) &(ackHeader->byteCnt)   );
  error |= UART_recvHexData( hUartInfo, 4, (Uint32 *) &(ackHeader->loadAddr)  );  
  error |= UART_checkSequence( hUartInfo, "0000", FALSE);

  if(error != E_PASS)
  {
    return E_FAIL;
  }
  
  // Check if this is a UBL or APP image
#if !defined(AIS_RBL)
  // For Davinci-type devices, check if this is a UBL or APP image
  // and set limits appropriately
  if (ackHeader->loadAddr == 0x00000020)
  {
    maxImageSize = UBL_IMAGE_SIZE;
    minStartAddr = 0x0020;
    maxStartAddr = UBL_IMAGE_SIZE;
  }
  else
  {
    maxImageSize = APP_IMAGE_SIZE;
    minStartAddr = (Uint32) &EXTERNAL_RAM_START;
    maxStartAddr = (Uint32) &EXTERNAL_RAM_END;
  }
#else
  // For AIS-type devices, these checks don't matter
  minStartAddr = 0x00000000;
  maxStartAddr = 0xFFFFFFFF;
  maxImageSize = 0xFFFFFFFF;
#endif

  // Verify that the data size is appropriate
  if((ackHeader->byteCnt == 0) || (ackHeader->byteCnt > maxImageSize))
  {
    LOCAL_sendSequence(" BADCNT");  // trailing /0 will come along
    return E_FAIL;
  }

  // Verify application start address is in RAM (lower 16bit of appStartAddr also used 
  // to hold UBL entry point if this header describes a UBL)
  if( (ackHeader->startAddr < minStartAddr) || (ackHeader->startAddr > maxStartAddr) )
  {
    LOCAL_sendSequence("BADADDR");  // trailing /0 will come along
    return E_FAIL;
  }
  
  // Allocate space in DDR to store image
  ackHeader->imageBuff = (Uint8 *) UTIL_allocMem(ackHeader->byteCnt);

  // Send BEGIN command
  if (LOCAL_sendSequence("  BEGIN") != E_PASS)
    return E_FAIL;

  // Receive the data over UART
  recvLen = ackHeader->byteCnt;
  error = UART_recvStringN(hUartInfo, (String)ackHeader->imageBuff, &recvLen, FALSE );
  if ( (error != E_PASS) || (recvLen != ackHeader->byteCnt) )
  {
    DEBUG_printString("\r\nUART Receive Error\r\n");
    return E_FAIL;
  }

  //Return DONE when all data arrives
  if ( LOCAL_sendSequence("   DONE") != E_PASS )
      return E_FAIL;

  return E_PASS;
}

// Generic function to write a UBL or Application header and the associated data
#if defined(UBL_NAND)
static Uint32 LOCAL_NANDWriteData(NAND_InfoHandle hNandInfo, Uint8 *srcBuf, Uint32 byteCnt)
{
  Uint32    *ptr;
  Uint32    currBlockNum,currPageNum,pageCnt,i;
  Uint32    numPages,numBlks, numBlksRemaining;

  // Check if device is write protected
  if (NAND_isWriteProtected(hNandInfo))
  {
    DEBUG_printString("NAND is write protected!\r\n");
    return E_FAIL;
  }
  
  // Get total number of pages needed for each copy
  numPages = 0;
  while ( (numPages * hNandInfo->dataBytesPerPage)  < (byteCnt + 1) )
  {
    numPages++;
  }

  // Get total number of blocks needed for each copy
  numBlks = 0;
  while ( (numBlks * hNandInfo->pagesPerBlock)  < (numPages + 1) )
  {
    numBlks++;
  }
  
  // Init internal current block number counter
  currBlockNum = 1;

  // Unprotect all blocks of the device
  if (NAND_unProtectBlocks(hNandInfo, currBlockNum, (hNandInfo->numBlocks-1)) != E_PASS)
  {
    currBlockNum++;
    DEBUG_printString("Unprotect failed.\r\n");
    return E_FAIL;
  }
  // Go to first good block
  while (NAND_badBlockCheck(hNandInfo,currBlockNum) != E_PASS)
  {
    currBlockNum++;
    DEBUG_printString("Bad block at ");
    DEBUG_printHexInt(currBlockNum);
    DEBUG_printString("\r\n");
  }

  // Keep going while we have room to place another copy
  numBlksRemaining = numBlks;
   
  // Erase the block where the header goes and the data starts
  if (NAND_eraseBlocks(hNandInfo,currBlockNum,numBlks) != E_PASS)
  {
    // Attempt to mark block bad
    NAND_badBlockMark(hNandInfo, currBlockNum);
    currBlockNum++;
    DEBUG_printString("Erase failed\r\n");
    return E_FAIL;
  }
  
  // Clear write buffer
  ptr = (Uint32 *) hNandWriteBuf;
  for (i=0; i < hNandInfo->dataBytesPerPage >> 2; i++)
  {
    ptr[i] = 0xFFFFFFFF;
  }

  pageCnt = 0;
  currPageNum = 0;
  do
  {
    // Write the UBL or APP data on a per page basis
    if (NAND_writePage(hNandInfo, currBlockNum, currPageNum, srcBuf) != E_PASS)
    {
      // Attempt to mark block bad
      NAND_badBlockMark(hNandInfo, currBlockNum);
      currBlockNum++;
      DEBUG_printString("Write failed, skipping block!\r\n");
      srcBuf -= (hNandInfo->dataBytesPerPage * currPageNum);
      pageCnt -= currPageNum;
      currPageNum = 0;        
      continue;
    }

    UTIL_waitLoop(200);

    // Verify the page just written
    if (NAND_verifyPage(hNandInfo, currBlockNum, currPageNum, srcBuf, hNandReadBuf) != E_PASS)
    {
      // Attempt to mark block bad
      NAND_badBlockMark(hNandInfo, currBlockNum);
      currBlockNum++;
      DEBUG_printString("Write verify failed, skipping block!\r\n");
      srcBuf -= (hNandInfo->dataBytesPerPage * currPageNum);
      pageCnt -= currPageNum;
      currPageNum = 0;
      continue;
    }

    srcBuf += hNandInfo->dataBytesPerPage;
    pageCnt++;
    currPageNum++;

    // If we need to go the next block, or our image is complete, increment current block num
    if (currPageNum == hNandInfo->pagesPerBlock)
    {
      currBlockNum++;
      numBlksRemaining--;
      currPageNum = 0;
    }
  }
  while ( pageCnt < (numPages+1) );

  // Protect all blocks
  NAND_protectBlocks(hNandInfo);

  // We succeeded in writing all copies that would fit
  return E_PASS;
}

static Uint32 LOCAL_NANDWriteHeaderAndData(NAND_InfoHandle hNandInfo, NANDBOOT_HeaderHandle hNandBoot, Uint8 *srcBuf)
{
  Uint32    *ptr;
  Uint32    currBlockNum,currPageNum,pageCnt,i;
  Uint32    numBlks, numBlksRemaining;
  Uint8     *srcBufOrig; /* Pointer to the original buffer to write */	
  
  // Save the srcBuf pointer
  srcBufOrig = srcBuf;

  // Check if device is write protected
  if (NAND_isWriteProtected(hNandInfo))
  {
    DEBUG_printString("NAND is write protected!\r\n");
    return E_FAIL;
  }

  // Get total number of blocks needed for each copy
  numBlks = 0;
  while ( (numBlks * hNandInfo->pagesPerBlock)  < (hNandBoot->numPage + 1) )
  {
    numBlks++;
  }
  DEBUG_printString("Number of blocks needed for header and data: 0x");
  DEBUG_printHexInt(numBlks);
  DEBUG_printString("\r\n");

  // Init internal current block number counter
  currBlockNum = hNandBoot->startBlock; 

  // Go to first good block
  while (NAND_badBlockCheck(hNandInfo,currBlockNum) != E_PASS)
  {
    DEBUG_printString("NAND block ");
    DEBUG_printHexInt(currBlockNum);
    DEBUG_printString(" is bad!!!\r\n");
    currBlockNum++;
    // Now check to make sure we aren't already out of space
    if (currBlockNum > (hNandBoot->endBlock + numBlks - 1 ))
    {
      DEBUG_printString("No good blocks in allowed range!!!\r\n");
      return E_FAIL;
    }
  }

  DEBUG_printString("Attempting to start in block number 0x");
  DEBUG_printHexInt(currBlockNum);
  DEBUG_printString(".\n");

  // Keep going while we have room to place another copy
  do
  {
    srcBuf = srcBufOrig;
  
    numBlksRemaining = numBlks;
   
    // Erase the block where the header goes and the data starts
    if (NAND_eraseBlocks(hNandInfo,currBlockNum,numBlks) != E_PASS)
    {
      // Attempt to mark block bad
      NAND_badBlockMark(hNandInfo, currBlockNum);
      currBlockNum++;
      DEBUG_printString("Erase failed\r\n");
      continue;
    }
    
    // Clear write buffer
    ptr = (Uint32 *) hNandWriteBuf;
    for (i=0; i < hNandInfo->dataBytesPerPage >> 2; i++)
    {
      ptr[i] = 0xFFFFFFFF;
    }
    
    // Setup header to be written
    ptr[0] = hNandBoot->magicNum;
    ptr[1] = hNandBoot->entryPoint;
    ptr[2] = hNandBoot->numPage;
    ptr[3] = currBlockNum;  //always start data in current block
    ptr[4] = 1;      //always start data in page 1 (this header goes in page 0)
    ptr[5] = hNandBoot->ldAddress;

DEBUG_printString("Magicnum: 0x");
DEBUG_printHexInt(ptr[0]);
DEBUG_printString("\r\n");
DEBUG_printString("Entrypoint: 0x");
DEBUG_printHexInt(ptr[1]);
DEBUG_printString("\r\n");
DEBUG_printString("Numpage: 0x");
DEBUG_printHexInt(ptr[2]);
DEBUG_printString("\r\n");


    // Write the header to page 0 of the current blockNum
    DEBUG_printString("Writing header and image data to Block ");
    DEBUG_printHexInt(currBlockNum);
    DEBUG_printString(", Page ");
    DEBUG_printHexInt(0);
    DEBUG_printString("\r\n");

#ifdef DM35X_REVB
#define DM35X_REVC
#endif

#ifdef DM35X_REVC
    if (NAND_writePage_ubl_header(hNandInfo, currBlockNum, 0, hNandWriteBuf) != E_PASS)
    {
      // Attempt to mark block bad
      NAND_badBlockMark(hNandInfo, currBlockNum);
      currBlockNum++;
      DEBUG_printString("Write failed!\r\n");
      continue;
    }
#else
    if (NAND_writePage(hNandInfo, currBlockNum, 0, hNandWriteBuf) != E_PASS)
    {
      // Attempt to mark block bad
      NAND_badBlockMark(hNandInfo, currBlockNum);
      currBlockNum++;
      DEBUG_printString("Write failed!\r\n");
      continue;
    }
#endif
    
    UTIL_waitLoop(200);

    // Verify the page just written
    if (NAND_verifyPage(hNandInfo, currBlockNum, 0, hNandWriteBuf, hNandReadBuf) != E_PASS)
    {
      // Attempt to mark block bad
      NAND_badBlockMark(hNandInfo, currBlockNum);
      currBlockNum++;
      DEBUG_printString("Write verify failed!\r\n");
      continue;
    }

    pageCnt = 1;
    currPageNum = 1;
    //DEBUG_printString("Now writing UBL or app data!\r\n");
    do
    {
      // Write the UBL or APP data on a per page basis
      if (NAND_writePage(hNandInfo, currBlockNum, currPageNum, srcBuf) != E_PASS)
      {
        // Attempt to mark block bad
        NAND_badBlockMark(hNandInfo, currBlockNum);
        currBlockNum++;
        DEBUG_printString("Write failed, skipping block!\r\n");
        if ( (numBlksRemaining == numBlks) || (hNandBoot->forceContigImage) )
        {
          break;    // If we are still in the first block, we have to go rewrite the header too
        }
        else
        {
          srcBuf -= (hNandInfo->dataBytesPerPage * currPageNum);
          pageCnt -= currPageNum;
          currPageNum = 0;        
          continue;
        }
      }

      UTIL_waitLoop(200);

      // Verify the page just written
      if (NAND_verifyPage(hNandInfo, currBlockNum, currPageNum, srcBuf, hNandReadBuf) != E_PASS)
      {
        // Attempt to mark block bad
        NAND_badBlockMark(hNandInfo, currBlockNum);
        currBlockNum++;
        DEBUG_printString("Write verify failed, skipping block!\r\n");
        if ( (numBlksRemaining == numBlks) || (hNandBoot->forceContigImage) )
          break;    // If we are still in the first block, we have to go rewrite the header too
        else
        {
          srcBuf -= (hNandInfo->dataBytesPerPage * currPageNum);
          pageCnt -= currPageNum;
          currPageNum = 0;
          continue;
        }
      }

      srcBuf += hNandInfo->dataBytesPerPage;
      pageCnt++;
      currPageNum++;

      // If we need to go the next block, or our image is complete, increment current block num
      if ( (currPageNum == hNandInfo->pagesPerBlock) || (pageCnt >= (hNandBoot->numPage+1)) )
      {
        currBlockNum++;
        numBlksRemaining--;
        currPageNum = 0;
      }
    }
    while ( (pageCnt < (hNandBoot->numPage+1)) && ((currBlockNum + numBlksRemaining - 1)<=hNandBoot->endBlock) );
  } 
  while(0);

  // Protect all blocks
  NAND_protectBlocks(hNandInfo);

  // We succeeded in writing all copies that would fit
  return E_PASS;
}
#endif


/************************************************************
* End file                                                  *
************************************************************/


