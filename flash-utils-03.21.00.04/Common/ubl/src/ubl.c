/* --------------------------------------------------------------------------
  FILE        : ubl.c                                                   
  PROJECT     : TI Booting and Flashing Utilities
  AUTHOR      : Daniel Allred
  DESC        : The main project file for the user boot loader
 ----------------------------------------------------------------------------- */

// General type include
#include "tistdtypes.h"

// This module's header file 
#include "ubl.h"

// Device specific CSL
#include "device.h"

// Misc. utility function include
#include "util.h"

// Project specific debug functionality
#include "debug.h"
#include "uartboot.h"

// Uart driver includes
#include "device_uart.h"
#include "uart.h"

#if defined(UBL_NOR)
// NOR driver include
#include "nor.h"
#include "norboot.h"

#elif defined(UBL_NAND)
// NAND driver include
#include "nand.h"
#include "nandboot.h"

#elif defined(UBL_ONENAND)
// OneNAND driver include
#include "onenand.h"
#include "onenandboot.h"

#elif defined(UBL_SDMMC)
// SD/MMC driver include
#include "sdmmc.h"
#include "sdmmcboot.h"

#elif defined(UBL_SPI_MEM)
// SPI MEM driver include
#include "spi_mem.h"
#include "spi_memboot.h"

#elif defined(UBL_I2C_MEM)
// I2C MEM driver include
#include "i2c_mem.h"
#include "i2c_memboot.h"
#endif

/************************************************************
* Explicit External Declarations                            *
************************************************************/

#ifdef OPERATING_POINT
  extern DEVICE_OperatingPoint gDeviceOpPoint;
#endif

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
static void LOCAL_bootAbort(void);
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
    LOCAL_bootAbort();
  }
  else
  {
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

  // Set RAM pointer to beginning of RAM space
  UTIL_setCurrMemPtr(0);

  // Init device PLLs, PSCs, external memory, etc.
  status = DEVICE_init();
  
  // Open UART peripheral for sending out status
  if (status == E_PASS)
  {
    hUartInfo = UART_open(DEVICE_UART_PERIPHNUM, hDEVICE_UART_config);
    DEVICE_UARTInit(DEVICE_UART_PERIPHNUM);
    DEBUG_printString((String)devString);
    DEBUG_printString(" initialization passed!\r\n");
  }
  else
  {
    return E_FAIL;
  }
  // Send some information to host
  DEBUG_printString("Booting TI User Boot Loader\r\n");
  DEBUG_printString("\tUBL Version: ");
  DEBUG_printString(UBL_VERSION_STRING);

  #ifdef OPERATING_POINT
  switch(gDeviceOpPoint.opp)
  {
    case DEVICE_OPP_1P2V_300MHZ:
      DEBUG_printString("\r\nDevice OPP (300MHz, 1.2V)");
      break;
    case DEVICE_OPP_1P2V_372MHZ:
      DEBUG_printString("\r\nDevice OPP (372MHz, 1.2V)");
      break;
    case DEVICE_OPP_1P3V_408MHZ:
      DEBUG_printString("\r\nDevice OPP (408MHz, 1.3V)");
      break;
    case DEVICE_OPP_1P3V_456MHZ:
      DEBUG_printString ("\r\n Device OPP ( 456MHz, 1.3V)" );
      break;
    default:
      DEBUG_printString("\r\nInvalid Operating Point");
      return E_FAIL;
  } 
  #endif

  DEBUG_printString("\r\n\tUBL Flashtype: ");
  // Select Boot Mode
#if defined(UBL_NAND)
  {
    //Report Bootmode to host
    DEBUG_printString("NAND\r\n");

    // Copy binary image application from NAND to RAM
    if (NANDBOOT_copy() != E_PASS)
    {
      DEBUG_printString("NAND Boot failed.\r\n");
      return E_FAIL;
    }
  }
#elif defined(UBL_ONENAND)  
  {
    //Report Bootmode to host
    DEBUG_printString("OneNAND\r\n");

    // Copy binary image application from OneNAND to RAM
    if (ONENANDBOOT_copy() != E_PASS)
    {
      DEBUG_printString("OneNAND Boot failed.\r\n");
      return E_FAIL;
    }
  }
#elif defined(UBL_NOR)
  {
    //Report Bootmode to host
    DEBUG_printString("NOR \r\n");

    // Copy binary application image from NOR to RAM
    if (NORBOOT_copy() != E_PASS)
    {
      DEBUG_printString("NOR Boot failed.\r\n");
      return E_FAIL;
    }
  }
#elif defined(UBL_SDMMC)
  {
    //Report Bootmode to host
    DEBUG_printString("SD/MMC \r\n");

    // Copy binary of application image from SD/MMC card to RAM
    if (SDMMCBOOT_copy() != E_PASS)
    {
      DEBUG_printString("SD/MMC Boot failed.\r\n");
      return E_FAIL;
    }
  }
#elif defined(UBL_SPI_MEM)
  {
    //Report Bootmode to host
    DEBUG_printString("SPI \r\n");

    // Copy binary of application image from SPI memory
    if (SPI_MEM_BOOT_copy() != E_PASS)
    {
      DEBUG_printString("SPI Memory Boot failed.\r\n");
      return E_FAIL;
    }
  }
#elif defined(UBL_I2C_MEM)
  {
    //Report Bootmode to host
    DEBUG_printString("I2C \r\n");

    // Copy binary of application image from I2C Memory
    if (I2C_MEM_BOOT_copy() != E_PASS)
    {
      DEBUG_printString("I2C Memory Boot failed.\r\n");
      return E_FAIL;
    }
  }  
#else
  {
    //Report Bootmode to host
    DEBUG_printString("NONE. Trying UART...\r\n");
    UARTBOOT_copy();      
  }
#endif
    
  DEBUG_printString("   DONE");
  
  // Jump to entry point
  DEBUG_printString("\r\nJumping to entry point at ");
  DEBUG_printHexInt(gEntryPoint);
  DEBUG_printString(".\r\n");
    
  UTIL_waitLoop(10000);
    
  // reset the UART peripheral in use
 #ifndef NUartReset 
  UART_reset(hUartInfo);
 #endif 

  return E_PASS;    
}

static void LOCAL_bootAbort(void)
{
  DEBUG_printString("Aborting...\r\n");
  while (TRUE);
}

/************************************************************
* End file                                                  *
************************************************************/
