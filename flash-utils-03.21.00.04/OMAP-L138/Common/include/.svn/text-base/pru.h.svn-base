/* --------------------------------------------------------------------------
  FILE        : pru.h                                                   
  PROJECT     : DA8xx/OMAP-L138/C674x PRU Development
  DESC        : PRU Load and Run API Definitions
----------------------------------------------------------------------------- */

#ifndef _PRU_H_
#define _PRU_H_


#include "tistdtypes.h"


// Prevent C++ name mangling
#ifdef __cplusplus
extern far "c" {
#endif

/***********************************************************
* Global Macro Declarations                                *
***********************************************************/

// PRU Memory Macros
#define PRU0_DATA_RAM_START  (0x01C30000)
#define PRU0_PROG_RAM_START  (0x01C38000)

#define PRU1_DATA_RAM_START  (0x01C32000)
#define PRU1_PROG_RAM_START  (0x01C3C000)

#define PRU_DATA_RAM_SIZE    (0x200)
#define PRU_PROG_RAM_SIZE    (0x1000)


/***********************************************************
* Global Typedef declarations                              *
***********************************************************/


/***********************************************************
* Global Variable Declarations                             *
***********************************************************/


/***********************************************************
* Global Function Declarations                             *
***********************************************************/

extern __FAR__ void   PRU_enable (void);
extern __FAR__ void   PRU_load (Uint32* pruCode, Uint32 codeSizeInWords);
extern __FAR__ void   PRU_run ();
extern __FAR__ Uint32 PRU_waitForHalt (Int32 timeout);
extern __FAR__ void   PRU_disable (void);


/***********************************************************
* End file                                                 *
***********************************************************/

#ifdef __cplusplus
}
#endif

#endif // End _PRU_H_


