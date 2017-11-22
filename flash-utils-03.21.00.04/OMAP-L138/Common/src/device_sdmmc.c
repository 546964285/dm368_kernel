/* --------------------------------------------------------------------------
    FILE        : device_sdmmc.c 				                             	 	        
    PROJECT     : TI Booting and Flashing Utilities
    AUTHOR      : Daniel Allred
    DESC        : This file descibes and implements various device-specific NAND
                  functionality.
-------------------------------------------------------------------------- */ 

// General type include
#include "tistdtypes.h"

// Device specific CSL
#include "device.h"

// Device specific SDMMC details
#include "device_sdmmc.h"

// Generic SDMMC header file
#include "sdmmc.h"


/************************************************************
* Explicit External Declarations                            *
************************************************************/


/************************************************************
* Local Macro Declarations                                  *
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

const Uint32 DEVICE_SDMMC_baseAddr[SDMMC_PERIPHERAL_CNT] =
{
  (Uint32) SDMMC0,
  (Uint32) SDMMC1
};

const SDMMC_ConfigObj DEVICE_SDMMC_config = 
{
  1,        // polarity
  0,        // phase
  10,       // prescalar
  8         // charLen
};

// Set SDMMC config to NULL to use SDMMC driver defaults
//SDMMC_ConfigHandle const hDEVICE_SDMMC_config = NULL;
SDMMC_ConfigHandle const hDEVICE_SDMMC_config = (SDMMC_ConfigHandle) &DEVICE_SDMMC_config;


/************************************************************
* Global Function Definitions                               *
************************************************************/


/************************************************************
* Local Function Definitions                                *
************************************************************/


/************************************************************
* End file                                                  *
************************************************************/


