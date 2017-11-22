/* ---------------------------------------------------------------------------
  FILE        : debug.c 				                             	 	        
  PROJECT     : TI Booting and Flashing Utilities
  AUTHOR      : Daniel Allred
  DESC        : Debug utility functions that are mapped to a specific I/O
                device for this particulat project.
 ----------------------------------------------------------------------------- */

// General type include
#include "tistdtypes.h"

// I/O module
#include "uart.h"

// This module's header file
#include "debug.h"


/************************************************************
* Explicit External Declarations                            *
************************************************************/

extern UART_InfoHandle hUartInfo;

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
\***********************************************************/


/************************************************************
* Global Variable Definitions
************************************************************/


/************************************************************
* Global Function Definitions                               *
************************************************************/

// Debug print function (could use stdio or maybe UART)
Uint32 DEBUG_printString(String s)
{
  return UART_sendString(hUartInfo, s, FALSE);
}

Uint32 DEBUG_printHexInt(Uint32 i)
{
  UART_sendString(hUartInfo,"0x",FALSE);
  return UART_sendHexInt(hUartInfo, i);
}

Uint32 DEBUG_printInt(Uint32 i)
{
  return UART_sendHexInt(hUartInfo, i);
}

Uint32 DEBUG_readString(String s)
{
  return UART_recvString(hUartInfo, s);
}

Uint32 DEBUG_readChar(Char *c)
{
  Uint32 len = 1;
  return UART_recvStringN(hUartInfo,c,&len,FALSE);
}


/************************************************************
* Local Function Definitions                                *
************************************************************/


/***********************************************************
* End file                                                 *
***********************************************************/



