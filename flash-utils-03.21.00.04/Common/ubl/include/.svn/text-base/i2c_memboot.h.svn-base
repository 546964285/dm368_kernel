/* --------------------------------------------------------------------------
  FILE        : spiboot.h
  PROJECT     : TI Booting and Flashing Utilities
  AUTHOR      : Daniel Allred
  DESC        : This file defines all needed structures and macros for 
                booting a user application in SPI memory mode.
 ----------------------------------------------------------------------------- */

#ifndef _SPI_MEM_BOOT_H_
#define _SPI_MEM_BOOT_H_

#include "tistdtypes.h"

// Prevent C++ name mangling
#ifdef __cplusplus
extern far "c" {
#endif

/************************************************************
* Global Macro Declarations                                 *
************************************************************/


/************************************************************
* Global Typedef declarations                               *
************************************************************/

typedef struct _SPIBOOT_HEADER_
{
  Uint32 magicNum;    // Expected magic number
  Uint32 entryPoint;  // Entry point of the user application
  Uint32 appSize;     // Number of bytes of the application image
  Uint32 memAddress;  // SPI memory offset where application image is located
  Uint32 ldAddress;   // Address where image is copied to
}
SPI_MEM_BOOT_HeaderObj,*SPI_MEM_BOOT_HeaderHandle;


/******************************************************
* Global Function Declarations                        *
******************************************************/

extern __FAR__ Uint32 SPI_MEM_BOOT_copy(void);


/***********************************************************
* End file                                                 *
***********************************************************/

#ifdef __cplusplus
}
#endif

#endif //_SPI_MEM_BOOT_H_




