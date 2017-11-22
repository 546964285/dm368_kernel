/* --------------------------------------------------------------------------
  FILE        : slt.c                                                   
  PROJECT     : TI Booting and Flashing Utilities
  AUTHOR      : Daniel Allred
  DESC        : The main project file for the serial loader target
 ----------------------------------------------------------------------------- */

// General type include
#include "tistdtypes.h"

// This module's header file 
#include "slt.h"

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

static Uint32 LOCAL_boot(void);
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
  // Call to real boot function code
  if (LOCAL_boot() != E_PASS)
  {
    return;
  }
  else
  {
    // Jump to entry point
    APPEntry = (void (*)(void)) gEntryPoint;
    (*APPEntry)();
  }
}


/************************************************************
* Local Function Definitions                                *
************************************************************/

static Uint32 LOCAL_boot(void)
{
  Uint32 status;
  DEVICE_BootPeripheral bootPeriph;
  
  // Set RAM pointer to beginning of RAM space
  UTIL_setCurrMemPtr(0);

  // Init device PLLs, PSCs, external memory, etc.
  status = DEVICE_init();
  
  // Open UART peripheral for sending out status
  if (status == E_PASS)
  {
    DEVICE_UARTInit(DEVICE_UART_PERIPHNUM);
    hUartInfo = UART_open(DEVICE_UART_PERIPHNUM, hDEVICE_UART_config);
    DEBUG_printString((String) devString);
    DEBUG_printString(" initialization passed!\r\n");
  }
  else
  {
    return E_FAIL;
  }
  // Send some information to host
  DEBUG_printString("TI SLT Version: ");
  DEBUG_printString(SLT_VERSION_STRING);
  DEBUG_printString("\r\nBooting Catalog Serial Loader\r\n");
  
  // Read boot peripheral type
  bootPeriph = DEVICE_bootPeripheral();
  
  // If boot peripheral is not UART, we should not be running this
  if (bootPeriph != DEVICE_BOOTPERIPHERAL_UART)
  {
    DEBUG_printString("Not booting in UART mode...aborting!\r\n");
    return E_FAIL;
  }

  
  // Perform UART boot (always assume UART boot since this is only used for serial flashing)
  UARTBOOT_copy();
    
  DEBUG_printString("   DONE");
  
  UTIL_waitLoop(10000);

  DEVICE_TIMER0Stop();

  return E_PASS;    
}


/************************************************************
* End file                                                  *
************************************************************/
