/* --------------------------------------------------------------------------
  FILE      : async_mem.c 				                             	 	        
  PROJECT   : TI Boot and Flashing Utilities
  AUTHOR    : Daniel Allred
  DESC	    : Generic Asynchronous Memory driver
-------------------------------------------------------------------------- */

// General type include
#include "tistdtypes.h"

// Device specific CSL
#include "device.h"

// Misc. utility function include
#include "util.h"

// This module's header file  
#include "async_mem.h"


/************************************************************
* Explicit External Declarations                            *
************************************************************/

// The device specific async memory info structure
extern ASYNC_MEM_DEVICE_InfoObj     DEVICE_ASYNC_MEM_info;


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

#ifdef USE_IN_ROM
ASYNC_MEM_InfoObj gAsyncMemInfo;
#endif


/************************************************************
* Global Function Definitions                               *
************************************************************/

ASYNC_MEM_InfoHandle ASYNC_MEM_Open(ASYNC_MEM_Type memType, Uint32 baseAddress, Uint8 busWidth)
{
  Uint32 i,j;
  ASYNC_MEM_InfoHandle hAsyncMemInfo;
  
  // Create structure
#ifdef USE_IN_ROM
  hAsyncMemInfo = (ASYNC_MEM_InfoHandle) &gAsyncMemInfo;
#else
  hAsyncMemInfo = (ASYNC_MEM_InfoHandle) UTIL_callocMem(sizeof(ASYNC_MEM_InfoObj));
#endif

  // Fill in structure
  hAsyncMemInfo->hDeviceInfo = &DEVICE_ASYNC_MEM_info;
  hAsyncMemInfo->memType = memType;
  hAsyncMemInfo->busWidth = busWidth;
  
  hAsyncMemInfo->interfaceNum = 0xFF;
  for (i = 0; ((i<hAsyncMemInfo->hDeviceInfo->interfaceCnt) && (hAsyncMemInfo->interfaceNum == 0xFF)); i++)
  {
    for (j = 0; j<hAsyncMemInfo->hDeviceInfo->interfaces[i].regionCnt; j++)
    {
      Uint32 start, end;
      
      start = hAsyncMemInfo->hDeviceInfo->interfaces[i].regionStarts[j];
      end = start + hAsyncMemInfo->hDeviceInfo->interfaces[i].regionSizes[j];
      if ((baseAddress>=start) && (baseAddress <end))
      {
        hAsyncMemInfo->interfaceNum = i;
        hAsyncMemInfo->chipSelectNum = j;
        hAsyncMemInfo->regs = (void *) hAsyncMemInfo->hDeviceInfo->interfaces[i].regs;
        break;
      }
    }
  }
  
  // Do device level init (pinmux, power domain, etc.)
  if (DEVICE_AsyncMemInit(hAsyncMemInfo->interfaceNum) != E_PASS)
  {
    return NULL;
  }
  
  // Do device specific init for the specified memory type, memory width, etc.
  (*hAsyncMemInfo->hDeviceInfo->fxnInit)(hAsyncMemInfo);
  
  return hAsyncMemInfo;    
}


/************************************************************
* Local Function Definitions                                *
************************************************************/


/***********************************************************
* End file                                                 *
***********************************************************/

