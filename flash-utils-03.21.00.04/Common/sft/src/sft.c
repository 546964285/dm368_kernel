/* --------------------------------------------------------------------------
  FILE        : sft.c                                                   
  PROJECT     : TI Booting and Flashing Utilities
  AUTHOR      : Daniel Allred
  DESC        : The main project file for the serial flasher target
 ----------------------------------------------------------------------------- */

// General type include
#include "tistdtypes.h"

// This module's header file 
#include "sft.h"

// Device specific CSL
#include "device.h"

// Misc. utility function include
#include "util.h"

// Project specific debug functionality
#include "debug.h"
// UART boot functionality
#include "uartboot.h"

// Uart driver includes
#include "device_uart.h"
#include "uart.h"

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

static void (*APPEntry)(void);


/************************************************************
* Local Variable Definitions                                *
************************************************************/


/************************************************************
* Global Variable Definitions                               *
************************************************************/

Uint32 gEntryPoint;
UART_InfoHandle hUartInfo;


/************************************************************
* Global Function Definitions                               *
************************************************************/

// Main entry point
void main(void)
{
  Uint32 status;

  // Give some time for host to become ready
  UTIL_waitLoop(100000);
  
  // Set RAM pointer to beginning of RAM space
  UTIL_setCurrMemPtr(0);
  
  // Init device PLLs, PSCs, external memory, etc.
  status = DEVICE_init();
  
  // Open UART peripheral for sending out status
  if (status == E_PASS)
  {
    DEVICE_UARTInit(DEVICE_UART_PERIPHNUM);
    hUartInfo = UART_open(DEVICE_UART_PERIPHNUM, hDEVICE_UART_config);
    //DEBUG_printString((String) devString);
    //DEBUG_printString("Akshay initialization passed!\r\n");
  }
  else
  {
    return;
  }
  // Send some information to host
  //DEBUG_printString("TI SFT Version: ");
  //DEBUG_printString(SFT_VERSION_STRING);
  //DEBUG_printString("\r\n");
  
  // Perform UART boot (always assume UART boot since this is only used for serial flashing)
  UARTBOOT_copy();
    
  //DEBUG_printString("   DONE");
  
  //UTIL_waitLoop(10000);

  // FIXME: This should bre replaced with DEVICE_finalize()
  DEVICE_TIMER0Stop();

  // Jump to entry point
  APPEntry = (void (*)(void)) gEntryPoint;
  (*APPEntry)();
}


/************************************************************
* Local Function Definitions                                *
************************************************************/


/************************************************************
* End file                                                  *
************************************************************/
