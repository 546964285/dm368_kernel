/* --------------------------------------------------------------------------
    FILE        : device_async_mem.c 				                             	 	        
    PROJECT     : TI Booting and Flashing Utilities
    AUTHOR      : Daniel Allred
    DESC        : This file descibes and implements various device-specific 
                  Async Memory components
-------------------------------------------------------------------------- */ 

// General type include
#include "tistdtypes.h"

// Device specific CSL
#include "device.h"

// Device specific EMIF details
#include "device_async_mem.h"

// Generic Async Mem header file
#include "async_mem.h"


/************************************************************
* Explicit External Declarations                            *
************************************************************/


/************************************************************
* Local Macro Declarations                                  *
************************************************************/


/************************************************************
* Local Function Declarations                               *
************************************************************/

static void DEVICE_ASYNC_MEM_Init(ASYNC_MEM_InfoHandle hAsyncMemInfo);
static Uint8 DEVICE_ASYNC_MEM_IsNandReadyPin(ASYNC_MEM_InfoHandle hAsyncMemInfo);


/************************************************************
* Local Variable Definitions                                *
************************************************************/


/************************************************************
* Global Variable Definitions                               *
************************************************************/

// AEMIF Definitions
const Uint32 DEVICE_ASYNC_MEM_regionStarts[DEVICE_ASYNC_MEM0_REGION_CNT] =
{
  0x02000000,
  0x04000000,
  0x06000000,
  0x08000000
};

const Uint32 DEVICE_ASYNC_MEM_regionSizes[DEVICE_ASYNC_MEM0_REGION_CNT] =
{
  0x02000000,
  0x02000000,
  0x02000000,
  0x02000000
};

const ASYNC_MEM_DEVICE_InterfaceObj DEVICE_ASYNC_MEM_interfaces[DEVICE_ASYNC_MEM_INTERFACE_CNT] =
{
  {
    AYSNC_MEM_INTERFACE_TYPE_EMIF2,
    (void *) AEMIF,
    DEVICE_ASYNC_MEM0_REGION_CNT,
    DEVICE_ASYNC_MEM_regionStarts,
    DEVICE_ASYNC_MEM_regionSizes
  }
};

const ASYNC_MEM_DEVICE_InfoObj DEVICE_ASYNC_MEM_info = 
{
  DEVICE_ASYNC_MEM_INTERFACE_CNT,       // interfaceCnt 
  DEVICE_ASYNC_MEM_interfaces,          // interfaces
  &(DEVICE_ASYNC_MEM_Init),             // fxnOpen
  &(DEVICE_ASYNC_MEM_IsNandReadyPin)    // fxnNandIsReadyPin;    
};


/************************************************************
* Global Function Definitions                               *
************************************************************/


/************************************************************
* Local Function Definitions                                *
************************************************************/

static void DEVICE_ASYNC_MEM_Init(ASYNC_MEM_InfoHandle hAsyncMemInfo)
{
  
  if (hAsyncMemInfo->interfaceNum == 0)
  {
    VUint32 *ABCR = NULL;
    DEVICE_EmifRegs *EMIF = (DEVICE_EmifRegs *) hAsyncMemInfo->regs;

    // Do width settings
    EMIF->AWCCR &= ~(DEVICE_EMIF_AWCC_WAITSTATE_MASK);
    ABCR = &(EMIF->A1CR);
    // Adjust for quicker access times   
    ABCR[hAsyncMemInfo->chipSelectNum] = 0x3FFFFFFC | ((hAsyncMemInfo->busWidth == DEVICE_BUSWIDTH_8BIT)? 0 : 1);
    
    // Do NAND enable
    if (hAsyncMemInfo->memType == AYSNC_MEM_TYPE_NAND)
    {
      // Setup  registers for NAND  
      EMIF->NANDFCR |= (0x1 << (hAsyncMemInfo->chipSelectNum));        // NAND enable for CSx
    }
    AEMIF->A2CR = 0x00a00505;
  }
}

static Uint8 DEVICE_ASYNC_MEM_IsNandReadyPin(ASYNC_MEM_InfoHandle hAsyncMemInfo)
{
  return ((Uint8) ((AEMIF->NANDFSR & DEVICE_EMIF_NANDFSR_READY_MASK)>>DEVICE_EMIF_NANDFSR_READY_SHIFT));
}


/************************************************************
* End file                                                  *
************************************************************/


