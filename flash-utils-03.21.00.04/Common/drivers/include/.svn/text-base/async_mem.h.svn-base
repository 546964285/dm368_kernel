/* --------------------------------------------------------------------------
  FILE      : async_mem.h
  PROJECT   : TI Booting and Flashing Utilities
  AUTHOR    : Daniel Allred
  DESC      : Header to define common interface for async memory controller
-------------------------------------------------------------------------- */

#ifndef _ASYNC_MEM_H_
#define _ASYNC_MEM_H_

#include "tistdtypes.h"

// Prevent C++ name mangling
#ifdef __cplusplus
extern far "c" {
#endif

/************************************************************
* Global Macro Declarations                                 *
************************************************************/


/***********************************************************
* Global Typedef declarations                              *
***********************************************************/

// Supported asynchronous memory type
typedef enum _ASYNC_MEM_TYPE_
{
  AYSNC_MEM_TYPE_SRAM     = 0x01,
  AYSNC_MEM_TYPE_NOR      = 0x02,
  AYSNC_MEM_TYPE_NAND     = 0x03,
  AYSNC_MEM_TYPE_ONENAND  = 0x04
}
ASYNC_MEM_Type;

// ASYNC_MEM_INFO structure - holds pertinent info for open driver instance
typedef struct _ASYNC_MEM_INFO_
{
  ASYNC_MEM_Type    memType;            // Type of memory
  Uint8             busWidth;           // Operating bus width  
  Uint8             interfaceNum;       // Async Memory Interface Number
  Uint8             chipSelectNum;      // Operating chip select
  void              *regs;              // Configuration register overlay
  struct _ASYNC_MEM_DEVICE_INFO_ *hDeviceInfo;
} 
ASYNC_MEM_InfoObj, *ASYNC_MEM_InfoHandle;

// Supported asynchronous memory interface type
typedef enum _ASYNC_MEM_DEVICE_INTERFACE_TYPE_
{
  AYSNC_MEM_INTERFACE_TYPE_EMIF2 = 0x01,
  AYSNC_MEM_INTERFACE_TYPE_GPMC  = 0x02
}
ASYNC_MEM_DEVICE_InterfaceType;

typedef struct _ASYNC_MEM_DEVICE_INTERFACE_
{
  const ASYNC_MEM_DEVICE_InterfaceType type;
  const void *regs;
  const Uint32 regionCnt;
  const Uint32 *regionStarts;
  const Uint32 *regionSizes;
}
ASYNC_MEM_DEVICE_InterfaceObj, *ASYNC_MEM_DEVICE_InterfaceHandle;

typedef void (*ASYNC_MEM_DEVICE_Init)(ASYNC_MEM_InfoHandle hAsyncMemInfo);
typedef void (*ASYNC_MEM_DEVICE_Close)(ASYNC_MEM_InfoHandle hAsyncMemInfo);
typedef Uint8 (*ASYNC_MEM_DEVICE_IsNandReadyPin)(ASYNC_MEM_InfoHandle hAsyncMemInfo);

typedef struct _ASYNC_MEM_DEVICE_INFO_
{
  const Uint8                           interfaceCnt;
  const ASYNC_MEM_DEVICE_InterfaceObj   *interfaces;
  const ASYNC_MEM_DEVICE_Init           fxnInit;
  const ASYNC_MEM_DEVICE_IsNandReadyPin fxnNandIsReadyPin;
}
ASYNC_MEM_DEVICE_InfoObj, *ASYNC_MEM_DEVICE_InfoHandle;


/************************************************************
* Global Function Declarations                              *
************************************************************/

extern __FAR__ ASYNC_MEM_InfoHandle ASYNC_MEM_Open(ASYNC_MEM_Type memType, Uint32 baseAddress, Uint8 busWidth);


/***********************************************************
* End file                                                 *
***********************************************************/

#ifdef __cplusplus
}
#endif

#endif //_ASYNC_MEM_H_

