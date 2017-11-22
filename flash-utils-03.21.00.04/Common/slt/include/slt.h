/* --------------------------------------------------------------------------
  FILE        : slt.h
  PROJECT     : TI Booting and Flashing Utilities
  AUTHOR      : Daniel Allred
  DESC        : User boot loader header file for main components and intial
                entry point functions
 ----------------------------------------------------------------------------- */

#ifndef _SLT_H_
#define _SLT_H_

#include "tistdtypes.h"

// Prevent C++ name mangling
#ifdef __cplusplus
extern far "c" {
#endif

/***********************************************************
* Global Macro Declarations                                *
***********************************************************/

// UBL version number
#define SLT_VERSION_STRING  ("1.65")

// Define MagicNumber constants
#define SLT_MAGIC_LOADIMAGE         (0x534654EE)		// Download via image UART

// Define max UBL image size (DRAM size - 2048)
#define UBL_IMAGE_SIZE              (((Uint32)&INTERNAL_RAM_SIZE) - ((Uint32)&STACK_SIZE))

// Define max app image size (1/8th of RAM size)
#define APP_IMAGE_SIZE              (((Uint32)&EXTERNAL_RAM_SIZE) >> 3)


/***********************************************************
* Global Typedef declarations                              *
***********************************************************/


/***********************************************************
* Global Function Declarations                             *
***********************************************************/

extern void main( void );


/***********************************************************
* End file                                                 *
***********************************************************/

#ifdef __cplusplus
}
#endif

#endif //_SLT_H_

