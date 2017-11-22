/* --------------------------------------------------------------------------
    FILE        : nandtester.c 				                             	 	        
    PURPOSE     : NAND writer main program
    PROJECT     : Dm644x CCS NAND Flashing Utility
    AUTHOR      : Daniel Allred
    DESC	      : CCS-based utility to flash the DM644x in preparation for 
                  NAND booting
 ----------------------------------------------------------------------------- */

// C standard I/O library
#include "stdio.h"

// General type include
#include "tistdtypes.h"

// Device specific CSL
#include "device.h"

// This module's header file 
#include "nandtester.h"

// NAND driver include
#include "nand.h"

// Misc. utility function include
#include "util.h"


/************************************************************
* Explicit External Declarations                            *
************************************************************/

extern __FAR__ Uint32 NANDStart, ASYNC_CS2_END;

/************************************************************
* Local Macro Declarations                                  *
************************************************************/

#define NANDWIDTH_8


/************************************************************
* Local Typedef Declarations                                *
************************************************************/


/************************************************************
* Local Function Declarations                               *
************************************************************/

static Uint32 nandtester(void);
static Uint32 LOCAL_writeData(NAND_InfoHandle hNandInfo, Uint8 numErr, Uint8 *srcBuf, Uint32 totalPageCnt);

/************************************************************
* Global Variable Definitions
************************************************************/

// Global variables for page buffers 
static Uint8* gNandTx;
static Uint8* gNandRx;

#if (1)
#else
  #pragma DATA_SECTION(ais,".testImage");
  #include "../../GNU/AISutils/t3_dsp_nandtest.h"
#endif


/************************************************************
* Global Function Definitions                               *
************************************************************/

void main( void )
{
  int status;

  // Init memory alloc pointer
  UTIL_setCurrMemPtr(0);

  // System init
  if (DEVICE_init() !=E_PASS)
  {
    exit();
  }

  // Execute the NAND flashing
  status = nandtester();

  if (status != E_PASS)
  {
    DEBUG_printString("\n\nNAND ECC test failed!\r\n");
  }
  else
  {
    DEBUG_printString( "\n\nNAND ECC test passed!\r\n" );
  }
}


/************************************************************
* Local Function Definitions                                *
************************************************************/

static Uint32 nandtester()
{
  Uint32 numPagesAIS;

  NAND_InfoHandle  hNandInfo;

  FILE	*fPtr;
  Uint8	*aisPtr;
  Int32	aisFileSize = 0,aisAllocSize = 0;
  Int8  fileName[256];
  Int32 i=0;
  Uint32 numErr;

  DEBUG_printString("Starting ");
  DEBUG_printString((String)devString);
  DEBUG_printString(" NANDWriter_ECC_Test.\r\n");

  // Initialize NAND Flash
#if defined(NANDWIDTH_8)
  hNandInfo = NAND_open((Uint32)&NANDStart, DEVICE_BUSWIDTH_8BIT );
#elif defined(NANDWIDTH_16)
  hNandInfo = NAND_open((Uint32)&NANDStart, DEVICE_BUSWIDTH_16BIT );
#else
  #error "Must define one of NANDWIDTH_8 or NANDWIDTH_16"
#endif
  
  if (hNandInfo == NULL)
  {
    DEBUG_printString( "\tERROR: NAND Initialization failed.\r\n" );
    return E_FAIL;
  }
  
  DEBUG_printString("Enter seed for the random number generator:");
  DEBUG_readHexInt(&numErr);
  srand(numErr);
  
  DEBUG_printString("How many bits of error do you want to test in each sector?\r\n");
  DEBUG_readHexInt(&numErr);

  // Read the file from host
#if (1)  
  DEBUG_printString("Enter the binary AIS file name to flash (enter 'none' to skip) :\r\n");
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

    // Read file size
    fseek(fPtr,0,SEEK_END);
    aisFileSize = ftell(fPtr);

    if(aisFileSize == 0)
    {
      DEBUG_printString("\tERROR: File read failed.. Closing program.\r\n");
      fclose (fPtr);
      return E_FAIL;
	  }

	  numPagesAIS = 0;
	  while ( (numPagesAIS * hNandInfo->dataBytesPerPage)  < aisFileSize )
	  {
		  numPagesAIS++;
	  }

    //We want to allocate an even number of pages.
    aisAllocSize = numPagesAIS * hNandInfo->dataBytesPerPage;

    // Setup pointer in RAM
    aisPtr = (Uint8 *) UTIL_allocMem(aisAllocSize);

    // Clear memory
	  for (i=0; i<aisAllocSize; i++)
	    aisPtr[i]=0xFF;

    // Go to start of file
    fseek(fPtr,0,SEEK_SET);

    // Read file data
    if (aisFileSize != fread(aisPtr, 1, aisFileSize, fPtr))
    {
      DEBUG_printString("\tWARNING: File Size mismatch.\r\n");
    }

    // Close file
	  fclose (fPtr);
    
    
    
	  if (LOCAL_writeData(hNandInfo,numErr,aisPtr,numPagesAIS) != E_PASS)
	  {
		  printf("\tERROR: Write failed.\r\n");
		  return E_FAIL;
	  }
  }
#else
    aisFileSize = sizeof(ais);
	  numPagesAIS = 0;
	  while ( (numPagesAIS * hNandInfo->dataBytesPerPage)  < aisFileSize )
	  {
		  numPagesAIS++;
	  }
    
    aisPtr = (Uint8 *) ais;
    
    if (LOCAL_writeData(hNandInfo,numErr,aisPtr,numPagesAIS) != E_PASS)
	  {
		  printf("\tERROR: Write failed.\r\n");
		  return E_FAIL;
	  }

#endif
  
	return E_PASS;
}

// Generic function to write a UBL or Application header and the associated data
static Uint32 LOCAL_writeData(NAND_InfoHandle hNandInfo, Uint8 numErr, Uint8 *srcBuf, Uint32 totalPageCnt)
{
  Uint32     blockNum,pageNum,pageCnt;
  Uint32     numBlks;
  Uint32     i,j;
  Uint8      *spareBytesBuff, *spareBytesPtr, *dataPtr, *srcBuf2, *dataPtr2;

  gNandTx = (Uint8 *) UTIL_allocMem(NAND_MAX_PAGE_SIZE);
  gNandRx = (Uint8 *) UTIL_allocMem(NAND_MAX_PAGE_SIZE);

  for (i=0; i<NAND_MAX_PAGE_SIZE; i++)  
  {
    gNandTx[i]=0xff;
	  gNandRx[i]=0xff;
  }	
	
  // Get total number of blocks needed
  numBlks = 0;
  while ( (numBlks*hNandInfo->pagesPerBlock)  < totalPageCnt )
  {
    numBlks++;
  }
  DEBUG_printString("Number of blocks needed for data: ");
  DEBUG_printHexInt(numBlks);
  DEBUG_printString("\r\n");

  // Start in block 1 (leave block 0 alone)
  blockNum = 1;

  // Unprotect all blocks of the device
	if (NAND_unProtectBlocks(hNandInfo, blockNum, (hNandInfo->numBlocks-1)) != E_PASS)
	{
		blockNum++;
		DEBUG_printString("Unprotect failed.\r\n");
    return E_FAIL;
	}
  
  // Allocate memory for the sparebytes to be stored when they are read back
  spareBytesBuff = (Uint8 *) UTIL_allocMem(totalPageCnt * hNandInfo->spareBytesPerPage);
  srcBuf2 = (Uint8 *) UTIL_allocMem(totalPageCnt * hNandInfo->dataBytesPerPage);
  memcpy(srcBuf2,srcBuf,totalPageCnt * hNandInfo->dataBytesPerPage);
  
  while (blockNum < hNandInfo->numBlocks)
  {
    // Find first good block
    //while (NAND_badBlockCheck(hNandInfo,blockNum) != E_PASS)
    //{
    //  blockNum++;
    //}

    // Erase the current block
    NAND_eraseBlocks(hNandInfo,blockNum,1);
    NAND_reset(hNandInfo);

	  // Start writing in page 0 of current block
    pageNum = 0;
    pageCnt = 0;
    blockNum = 1;

    // Setup data pointer
    dataPtr = srcBuf;

    // Start page writing loop
    do
    {
      DEBUG_printString((Uint8 *)"Writing image data to block ");
      DEBUG_printHexInt(blockNum);
      DEBUG_printString((Uint8 *)", page ");
      DEBUG_printHexInt(pageNum);
      DEBUG_printString((Uint8 *)"\r\n");

		  // Write the AIS image data to the NAND device
      if (NAND_writePage(hNandInfo, blockNum,  pageNum, dataPtr) != E_PASS)
      {
        DEBUG_printString("Write failed. Marking block as bad...\n");
        NAND_reset(hNandInfo);
        return E_FAIL;
      }
    
      UTIL_waitLoop(200);
		
      // Verify the page just written
      if (NAND_verifyPage(hNandInfo, blockNum, pageNum, dataPtr, gNandRx) != E_PASS)
      {
        DEBUG_printString("Verify failed. Marking block as bad...\n");
        NAND_reset(hNandInfo);
        return E_FAIL;
      }
		
      pageNum++;
      pageCnt++;
      dataPtr +=  hNandInfo->dataBytesPerPage;
  
      if (pageNum == hNandInfo->pagesPerBlock)
      {
        // A block transition needs to take place; go to next good block
        blockNum++;
        
        // Erase the current block
        NAND_eraseBlocks(hNandInfo,blockNum,1);

        pageNum = 0;
		  }
	  } while (pageCnt < totalPageCnt);
    
    // Start spare bytes reading
    spareBytesPtr = spareBytesBuff;
    pageNum = 0;
    pageCnt = 0;
    blockNum = 1;
    do
    {
      // Get good ECC from the spare bytes
      if (NAND_readSpareBytesOfPage(hNandInfo, blockNum, pageNum, spareBytesPtr) != E_PASS)
      {
        DEBUG_printString("Read spare bytes failed\n");
        return E_FAIL;
      }
		  
      UTIL_waitLoop(200);
		
      pageNum++;
      pageCnt++;
      spareBytesPtr += hNandInfo->spareBytesPerPage;
        
      if (pageNum == hNandInfo->pagesPerBlock)
      {
        // A block transition needs to take place; go to next good block
        blockNum++;
        pageNum = 0;
		  }
	  } while (pageCnt < totalPageCnt);

    // Introduce errors into page data
    dataPtr2 = srcBuf2;
    pageCnt = 0;
    do
    {
      Uint8 *currDataPtr;
    
      for (i = 0; i< hNandInfo->numOpsPerPage; i++)
      {
        currDataPtr = dataPtr2 + (i*hNandInfo->dataBytesPerOp);
        
        for (j=0; j<numErr; j++)
        {
          Uint16 bit = rand() % 4096;
          currDataPtr[bit >> 3] = currDataPtr[bit >> 3] ^ (0x1 << (bit & 0x7));
          /*DEBUG_printString("Error introduced at Byte: ");
          DEBUG_printHexInt((Uint32) (currDataPtr + (bit >> 3) - srcBuf2));
          DEBUG_printString(",Bit: ");
          DEBUG_printHexInt((Uint32) (bit & 0x7));
          DEBUG_printString(".\r\n");*/
        }
      }

      pageCnt++;
      dataPtr2 +=  hNandInfo->dataBytesPerPage;
  
    } while (pageCnt < totalPageCnt);
    
    // Writing data back with errors in it, but with correct ECC
    spareBytesPtr = spareBytesBuff;
    dataPtr = srcBuf;
    dataPtr2 = srcBuf2;
    pageNum = 0;
    pageCnt = 0;
    blockNum = 1;
    
    NAND_eraseBlocks(hNandInfo,blockNum,1);
    do
    {
      DEBUG_printString((Uint8 *)"Writing errored image data to block ");
      DEBUG_printHexInt(blockNum);
      DEBUG_printString((Uint8 *)", page ");
      DEBUG_printHexInt(pageNum);
      DEBUG_printString((Uint8 *)"\r\n");
    
      // Write data with the intentional errors in it
      if (NAND_writePageWithSpareBytes(hNandInfo, blockNum, pageNum, dataPtr2, spareBytesPtr) != E_PASS)
      {
        DEBUG_printString("Raw Writing with spare bytes data failed\n");
        NAND_reset(hNandInfo);
        return E_FAIL;
      }
      
      UTIL_waitLoop(200);
      
      // Verify the page just written (see if correction happens as expected)
      if (NAND_verifyPage(hNandInfo, blockNum, pageNum, dataPtr, gNandRx) != E_PASS)
      {
        DEBUG_printString("Verify of errored data failed.\n");
        NAND_reset(hNandInfo);
        return E_FAIL;
      }
      
      /*for(i=0; i<hNandInfo->dataBytesPerPage; i++)
      {
        if (dataPtr2[i] != gNandRx[i])
        {
          Uint32 j = 0;
          Uint8 testVal;
          // Assume a correction took place
          DEBUG_printString("Correction at Byte: ");
          DEBUG_printHexInt(i+(Uint32)(dataPtr - srcBuf));
          testVal = dataPtr2[i] ^ gNandRx[i];
          while (((testVal >> j) & 0x1) != 0x1)
          {
            j++;
          }
                
          if ((testVal >> j) != 0x1)
          {
            DEBUG_printString("Correction Failed.\r\n");
            return E_FAIL;
          }
          else
          {
            DEBUG_printString(", Bit: ");
            DEBUG_printHexInt(j);
            DEBUG_printString(".\r\n");
          }
        }
      }*/
      
      pageNum++;
      pageCnt++;
      dataPtr +=  hNandInfo->dataBytesPerPage;
      dataPtr2 +=  hNandInfo->dataBytesPerPage;
      spareBytesPtr += hNandInfo->spareBytesPerPage;
      
      if (pageNum == hNandInfo->pagesPerBlock)
      {
        // A block transition needs to take place; go to next good block
        blockNum++;
        
        // Erase the current block
        NAND_eraseBlocks(hNandInfo,blockNum,1);

        pageNum = 0;
		  }
    } while (pageCnt < totalPageCnt);
    break;
  }
  
  NAND_protectBlocks(hNandInfo);

  return E_PASS;
}


/***********************************************************
* End file                                                 
************************************************************/

