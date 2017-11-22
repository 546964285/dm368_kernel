/* --------------------------------------------------------------------------
    FILE        : device_uart.c 				                             	 	        
    PROJECT     : TI Booting and Flashing Utilities
    AUTHOR      : Daniel Allred
    DESC        : This file descibes and implements various device-specific UART components
-------------------------------------------------------------------------- */ 

// General type include
#include "tistdtypes.h"

// Device specific CSL
#include "device.h"

// Device specific UART details
#include "device_uart.h"

// Generic UART header file
#include "uart.h"


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

const Uint32 DEVICE_UART_baseAddr[UART_PERIPHERAL_CNT] =
{
  (Uint32) UART0,
  (Uint32) UART1,
  (Uint32) UART2,
};

const UART_ConfigObj DEVICE_UART_config = 
{
  UART_OSM_X16,         // osm
  UART_PARITY_NONE,     // parity
  UART_STOP_BITS_1,     // stopBits
  8,                    // charLen
  13                    // divider
};

// Set UART config to NULL to use UART driver defaults
//UART_ConfigHandle const hDEVICE_UART_config = NULL;
UART_ConfigHandle const hDEVICE_UART_config = (UART_ConfigHandle) &DEVICE_UART_config;


/************************************************************
* Global Function Definitions                               *
************************************************************/


/************************************************************
* Local Function Definitions                                *
************************************************************/


/************************************************************
* End file                                                  *
************************************************************/


