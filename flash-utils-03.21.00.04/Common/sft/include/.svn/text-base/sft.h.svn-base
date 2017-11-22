/* --------------------------------------------------------------------------
  FILE        : sft.h
  PROJECT     : TI Booting and Flashing Utilities
  AUTHOR      : Daniel Allred
  DESC        : Serial Flasher Target header file for main function
 ----------------------------------------------------------------------------- */

#ifndef _SFT_H_
#define _SFT_H_

#include "tistdtypes.h"

// Prevent C++ name mangling
#ifdef __cplusplus
extern far "c" {
#endif

/***********************************************************
* Global Macro Declarations                                *
***********************************************************/

// UBL version number
#define SFT_VERSION_STRING  ("1.65")

// Define MagicNumber constants
#ifdef DM35X_STANDARD
#define MAGIC_NUMBER_VALID          (0xA1BCED00)
#else
#define MAGIC_NUMBER_VALID          (0xA1ACED00)
#endif

// Used by UBL when doing UART boot
#define UBL_UART_BOOT               (0xA1ACED00)    /* Safe boot mode */
#define UBL_NOR_BURN                (0xA1ACED11)    /* Download via UART & Burn NOR with UBL readable header and BIN data*/
#define UBL_NOR_ERASE               (0xA1ACED22)    /* Download via UART & Global erase the NOR Flash*/
#define UBL_NAND_BURN               (0xA1ACED33)    /* Download via UART & Burn NAND - Image is binary */
#define UBL_NAND_ERASE              (0xA1ACED44)    /* Download via UART & Global erase the NAND Flash*/

#ifdef DM35X_STANDARD
// Used by DM35x RBL when doing NAND boot
#define UBL_MAGIC_SAFE              (0xA1BCED00)    /* Safe boot mode */
#define UBL_MAGIC_DMA               (0xA1BCED11)    /* DMA boot mode */
#define UBL_MAGIC_IC                (0xA1BCED22)    /* I Cache boot mode */
#define UBL_MAGIC_FAST              (0xA1BCED33)    /* Fast EMIF boot mode */
#define UBL_MAGIC_DMA_IC            (0xA1BCED44)    /* DMA + ICache boot mode */
#define UBL_MAGIC_DMA_IC_FAST       (0xA1BCED55)    /* DMA + ICache + Fast EMIF boot mode */
#define UBL_MAGIC_BIN_IMG           (0xA1BCED66)    /* Execute in place supported*/
#else
// Used by RBL when doing NAND boot
#define UBL_MAGIC_SAFE              (0xA1ACED00)    /* Safe boot mode */
#define UBL_MAGIC_DMA               (0xA1ACED11)    /* DMA boot mode */
#define UBL_MAGIC_IC                (0xA1ACED22)    /* I Cache boot mode */
#define UBL_MAGIC_FAST              (0xA1ACED33)    /* Fast EMIF boot mode */
#define UBL_MAGIC_DMA_IC            (0xA1ACED44)    /* DMA + ICache boot mode */
#define UBL_MAGIC_DMA_IC_FAST       (0xA1ACED55)    /* DMA + ICache + Fast EMIF boot mode */
#define UBL_MAGIC_BIN_IMG           (0xA1ACED66)    /* Execute in place supported*/
#endif

// Defined commands for flashing single boot image
#define UBL_MAGIC_FLASH_NO_UBL      (0x55424C00u)

// Defined commands for flashing single secondary boot image and 
// the true boot image (loaded by secondary boot image)
#define UBL_MAGIC_FLASH             (0x55424C10u)

// Defined commands for flashing two secondary boot images and 
// the true boot image (loaded by secondary boot image)
#define UBL_MAGIC_FLASH_DSP         (0x55424C30u)

// Defined commands to do global erase of targeted memory device
#define UBL_MAGIC_ERASE             (0x55424C20u)

// Application image magic number
#define UBL_MAGIC_BINARY_BOOT       (0x55424CBBu)

// Defined command to terminate target operations
#define UBL_MAGIC_FINISHED          (0x55424CFFu)


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

#endif //_SFT_H_
