/* --------------------------------------------------------------------------
  FILE        : uart.h
  PROJECT     : TI Booting and Flashing Utilities
  AUTHOR      : Daniel Allred
  DESC        : UART driver header file
 ----------------------------------------------------------------------------- */

#ifndef _UART_H_
#define _UART_H_

#include "tistdtypes.h"

// Prevent C++ name mangling
#ifdef __cplusplus
extern far "c" {
#endif

/************************************************************
* Global Macro Declarations                                 *
************************************************************/

#define MAXSTRLEN 256


/***********************************************************
* Global Typedef declarations                              *
***********************************************************/

// UART Oversampling Mode
typedef enum _UART_OSM_MODE_
{
  UART_OSM_X16    = 0x01,
  UART_OSM_X13    = 0x02
}
UART_OsmMode;

// UART Parity Modes
typedef enum _UART_PARITY_MODE_
{
  UART_PARITY_NONE    = 0x01,
  UART_PARITY_EVEN    = 0x02,
  UART_PARITY_ODD     = 0x04
}
UART_ParityMode;

typedef enum _UART_STOP_BITS_
{
  UART_STOP_BITS_1    = 0x01,
  UART_STOP_BITS_1_5  = 0x02,
  UART_STOP_BITS_2    = 0x04
}
UART_StopBits;

typedef struct _UART_CONFIG_
{
  UART_OsmMode    osm;
  UART_ParityMode parity;
  UART_StopBits   stopBits;
  Uint8           charLen;
  Uint16          divider;
}
UART_ConfigObj, *UART_ConfigHandle;

// UART driver structure
typedef struct _UART_INFO_
{
  Uint32            peripheralNum;
  void              *regs;
  UART_ConfigHandle config;
}
UART_InfoObj, *UART_InfoHandle;


/***********************************************************
* Global Variable declarations                             *
***********************************************************/

extern __FAR__ UART_InfoHandle hUartInfo;


/************************************************************
* Global Function Declarations                              *
************************************************************/

extern __FAR__ UART_InfoHandle UART_open(Uint32 peripheralNum, UART_ConfigHandle config);
extern __FAR__ void UART_reset(UART_InfoHandle hUartInfo);
extern __FAR__ Uint32 UART_waitForTxEmpty(UART_InfoHandle hUartInfo, Uint32 timeout);

// Simple send/recv functions
extern __FAR__ Uint32 UART_sendString(UART_InfoHandle hUartInfo, String seq, Bool includeNull);
extern __FAR__ Uint32 UART_sendHexInt(UART_InfoHandle hUartInfo, Uint32 value);
extern __FAR__ Uint32 UART_recvString(UART_InfoHandle hUartInfo, String seq);
extern __FAR__ Uint32 UART_recvStringN(UART_InfoHandle hUartInfo, String seq, Uint32* len, Bool stopAtNull);

extern __FAR__ Uint32 UART_checkSequence(UART_InfoHandle hUartInfo, String seq, Bool includeNull);
extern __FAR__ Uint32 UART_recvHexData(UART_InfoHandle hUartInfo, Uint32 numBytes, Uint32* data);



/***********************************************************
* End file                                                 *
***********************************************************/

#ifdef __cplusplus
extern far "c" }
#endif

#endif // End _UART_H_

