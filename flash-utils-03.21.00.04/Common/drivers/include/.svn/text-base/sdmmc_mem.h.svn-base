/* --------------------------------------------------------------------------
  FILE      : sdmmc_mem.h
  PROJECT   : TI Booting and Flashing Utilities
  AUTHOR    : Daniel Allred
  DESC      : Header file for the generic SD/MMC memory driver
-------------------------------------------------------------------------- */

#ifndef _SDMMC_MEM_H_
#define _SDMMC_MEM_H_

#include "tistdtypes.h"
#include "sdmmc.h"

// Prevent C++ name mangling
#ifdef __cplusplus
extern far "c" {
#endif

/************************************************************
* Global Macro Declarations                                 *
************************************************************/

#define SDMMC_MEM_STUFF_BITS          (0x00000000u)
#define SDMMC_MEM_VDD_32_34           (0x00300000u)
#define SDMMC_MEM_VDD_32_33           (0x00100000u)
#define SDMMC_MEM_VDD_27_36           (0x00FF8000u)

// Commands and their responses
// Common to MMC and SD
#define SDMMC_BSYEXP		                (0x0100)
#define SDMMC_GO_IDLE_STATE             (SDMMC_CMD0 | SDMMC_RSPNONE)
#define SDMMC_ALL_SEND_CID              (SDMMC_CMD2 | SDMMC_RSP2 )
#define SDMMC_SET_DSR                   (SDMMC_CMD4 | SDMMC_RSPNONE)
#define SDMMC_SELECT_CARD               (SDMMC_CMD7 | SDMMC_RSP1)
#define SDMMC_DESELECT_CARD             (SDMMC_CMD7 )
#define SDMMC_SEND_CSD                  (SDMMC_CMD9 | SDMMC_RSP2)
#define SDMMC_SEND_CID                  (SDMMC_CMD10| SDMMC_RSP2)
#define SDMMC_SEND_STATUS               (SDMMC_CMD13 | SDMMC_RSP1)
#define SDMMC_GO_INACTIVE_STATE         (SDMMC_CMD15 | SDMMC_RSPNONE)
#define SDMMC_APP_CMD                   (SDMMC_CMD55 | SDMMC_RSP1)
#define SDMMC_STOP_TRANSMISSION         (SDMMC_CMD12 | SDMMC_RSP1 | SDMMC_BSYEXP)
#define SDMMC_READ_MULTIPLE_BLOCK       (SDMMC_CMD18 | SDMMC_RSP1)
#define SDMMC_WRITE_MULTIPLE_BLOCK      (SDMMC_CMD25 | SDMMC_RSP1 ) /*| SDMMC_BSYEXP)*/

// Common to SPI & MMC
#define SDMMC_SET_BLOCKLEN              (SDMMC_CMD16 | SDMMC_RSP1 )
#define SDMMC_PROGRAM_CSD               (SDMMC_CMD27 | SDMMC_RSP1 | SDMMC_BSYEXP) /* MMC-bsy, SPI-bsy optional */
#define SDMMC_SET_WRITE_PROT            (SDMMC_CMD28 | SDMMC_RSP1 | SDMMC_BSYEXP)
#define SDMMC_CLR_WRITE_PROT            (SDMMC_CMD29 | SDMMC_RSP1 | SDMMC_BSYEXP)
#define SDMMC_SEND_WRITE_PROT           (SDMMC_CMD30 | SDMMC_RSP1)
#define SDMMC_READ_SINGLE_BLOCK         (SDMMC_CMD17 | SDMMC_RSP1 )
#define SDMMC_WRITE_BLOCK               (SDMMC_CMD24 | SDMMC_RSP1 )/*| MMC_BSYEXP)*/
#define SDMMC_READ_OCR                  (SDMMC_CMD58 | SDMMC_RSP3 )
#define SDMMC_CRC_ON_OFF                (SDMMC_CMD59 | SDMMC_RSP1)

// SDHC-only commands
#define SDHC_SEND_IF_COND               (SDMMC_CMD8  | SDMMC_RSP3)
#define SDHC_CMD8_NO_VOLTAGE_RANGE      (0x0000)
#define SDHC_CMD8_HIGH_VOLTAGE_RANGE    (0x0100)
#define SDHC_CMD8_LOW_VOLTAGE_RANGE     (0x0200)
#define SDHC_CMD8_CHECK_PATTERN         (0x00AA)

// SD-only commands
#define SDMMC_PPLEN			       	        (0x0080)
#define SD_SEND_OP_COND                 (SDMMC_CMD41 | SDMMC_RSP3)
#define SD_ACMD41_HCS_LOW               (0x00000000)
#define SD_ACMD41_HCS_HIGH              (0x40000000)
#define SD_ACMD41_CCS_MASK              (0x40000000)
#define SD_SEND_RELATIVE_ADDR           (SDMMC_CMD3  | SDMMC_RSP6 | SDMMC_PPLEN)
#define SD_SET_BUS_WIDTH                (SDMMC_CMD6  | SDMMC_RSP1 | SDMMC_PPLEN)

// MMC-only commands
#define MMC_SEND_OP_COND                (SDMMC_CMD1 | SDMMC_RSP3 )
#define MMC_SET_RELATIVE_ADDR           (SDMMC_CMD3 | SDMMC_RSP1)

// Command parameters
#define MMCCMD_REG_INITCK               (0x4000u)
#define MMCCMD_REG_PPLEN                (0x0080u)

// Default timeout value
#define SDMMC_OP_TIMEOUT                (2048)

//  Direction of DMA Transfer
#define SDMMC_FROM_MMC          0           // MMCIF to SDRAM
#define SDMMC_TO_MMC            1           // SDRAM to MMCIF

/***********************************************************
* Global Typedef declarations                              *
***********************************************************/

typedef enum _SDMMC_MEM_TYPE_
{
  SDMMC_MEM_TYPE_SD       = 0x00,
  SDMMC_MEM_TYPE_MMC      = 0x01
}
SDMMC_MEM_Type;

typedef enum _SDMMC_MEM_CAPACITY_TYPE_
{
  SDMMC_MEM_CAPACITY_TYPE_STANDARD = 0x00,
  SDMMC_MEM_CAPACITY_TYPE_HIGH     = 0x01
}
SDMMC_MEM_CapacityType;

// Card lock modes as per MMC/SD Specifications
typedef enum _SDMMC_MEM_CARD_LOCK_MODES_
{
  SDMMC_SET_PASSWD      = 0x01, 
  SDMMC_CLR_PASSWD      = 0x02,
  SDMMC_LOCK_UNLOCK     = 0x04, 
  SDMMC_FORCED_ERASE    = 0x08
} 
SDMMC_MEM_CardLockModes;

// Card States as per MMC/SD Specifications
typedef enum _SDMMC_MEM_CARD_STATES_
{
  SDMMC_CARD_STATE_IDLE     = 0,
  SDMMC_CARD_STATE_READY    = 1,
  SDMMC_CARD_STATE_IDENT    = 2,
  SDMMC_CARD_STATE_STBY     = 3,
  SDMMC_CARD_STATE_TRAN     = 4,
  SDMMC_CARD_STATE_DATA     = 5,
  SDMMC_CARD_STATE_RCV      = 6,
  SDMMC_CARD_STATE_PRG      = 7,
  SDMMC_CARD_STATE_DIS      = 8,
  SDMMC_CARD_STATE_INA      = 9
}
SDMMC_MEM_CardStates;

// SDMMC: Card Status Register
typedef struct _SDMMC_MEM_CARD_STATUS_REG_
{
  Uint8                 appSpecific;
  Uint8                 ready;
  SDMMC_MEM_CardStates  currentState;
  Uint8                 eraseReset;
  Uint8                 eccDisabled;
  Uint8                 wpEraseSkip;
  Uint16                errorFlags;
  Uint8                 multiMediaCard;
}
SDMMC_MEM_CardStatusReg;

// Native mode Card CSD Register Information
typedef struct _SDMMC_MEM_CSD_REG_INFO_
{
  Uint8  permWriteProtect;
  Uint8  tmpWriteProtect;
  Uint8  writeBlkPartial;
  Uint16 writeBlkLenBytes;
  Uint8  wpGrpEnable;
  Uint8  wpGrpSize;             // Extracting 7 bits: For MMC - 5 bits reqd; For SD - 7 bits reqd. ( have to be taken care by user )
  Uint8  dsrImp;
  Uint8  readBlkMisalign;
  Uint8  writeBlkMisalign;
  Uint8  readBlkPartial;
  Uint16 readBlkLenBytes;
  Uint8  sysSpecVersion;        // These bits are reserved in the case of SD card
}
SDMMC_MEM_CsdRegInfo;

// _SDMMC_MEM_INFO_ structure - holds pertinent info for open driver instance
typedef struct _SDMMC_MEM_INFO_
{
  SDMMC_InfoHandle        hSDMMCInfo;
  SDMMC_MEM_Type          memType;
  SDMMC_MEM_CapacityType  capacity;
  Uint32                  relCardAddress;
  SDMMC_MEM_CsdRegInfo    csdRegInfo;
  SDMMC_MEM_CardStatusReg cardStatus; 
} 
SDMMC_MEM_InfoObj, *SDMMC_MEM_InfoHandle;


/************************************************************
* Global Function Declarations                              *
************************************************************/

extern __FAR__ SDMMC_MEM_InfoHandle SDMMC_MEM_open(Uint32 sdmmcPeripheralNum, SDMMC_ConfigHandle hSDMMCCfg);
extern __FAR__ Uint32 SDMMC_MEM_readBytes(SDMMC_MEM_InfoHandle hSDMMCMemInfo, Uint32 addr, Uint32 byteCnt, Uint8 *dest);

#ifndef USE_IN_ROM
extern __FAR__ Uint32 SDMMC_MEM_writeBytes (SDMMC_MEM_InfoHandle hSDMMCMemInfo, Uint32 addr, Uint32 byteCnt, Uint8 *src);
extern __FAR__ Uint32 SDMMC_MEM_verifyBytes(SDMMC_MEM_InfoHandle hSDMMCMemInfo, Uint32 addr, Uint32 byteCnt, Uint8 *src, Uint8* dest);
extern __FAR__ Uint32 SDMMC_MEM_globalErase(SDMMC_MEM_InfoHandle hSDMMCMemInfo);
extern __FAR__ Uint32 SDMMC_MEM_eraseBytes (SDMMC_MEM_InfoHandle hSDMMCMemInfo, Uint32 startAddr, Uint32 byteCnt);
#endif

/***********************************************************
* End file                                                 *
***********************************************************/

#ifdef __cplusplus
}
#endif

#endif //_SDMMC_MEM_H_

