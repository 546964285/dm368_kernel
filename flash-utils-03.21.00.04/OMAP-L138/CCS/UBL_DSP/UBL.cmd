-e start

IRAMStart  = 0x80000000;
IRAMSize   = 0x00008000;
DRAMStart  = 0x80000000;
DRAMSize   = 0x00008000;

ASYNC_MEM_START    = 0x62000000;

_INTERNAL_RAM_START = 0x80000000;
_INTERNAL_RAM_SIZE  = 0x00008000;

_STACK_START = _INTERNAL_RAM_START + _INTERNAL_RAM_SIZE;

MEMORY
{
  /* ARM Internal RAM Memory*/
  RAM_ARM_VECT          : o = 0xFFFF0000, l = 0x00000020  //     32  B
  RAM_ARM_STACK         : o = 0xFFFF0020, l = 0x000006E0  //   1768  B
  RAM_ARM_BL1           : o = 0xFFFF0700, l = 0x00000100  //    256  B
  RAM_ARM_MISC          : o = 0xFFFF0800, l = 0x00000800  //      2 KB
  RAM_ARM_NANDPAGE      : o = 0xFFFF1000, l = 0x00001000  //      4 KB

  /* PRU Memories */
  PRU0_DATA_RAM         : o = 0x01C30000, l = 0x00000200  //    512  B
  PRU1_DATA_RAM         : o = 0x01C32000, l = 0x00000200  //    512  B
  
  PRU0_PROG_RAM         : o = 0x01C38000, l = 0x00001000  //      4 KB
  PRU1_PROG_RAM         : o = 0x01C3C000, l = 0x00001000  //      4 KB

  UBL_TEXT        (RWX) : origin = 0x80000000   length = 0x00006800
  UBL_DATA        (RWX) : origin = 0x80006800   length = 0x00000800

  UBL_BSS         (RW)  : origin = 0x80007000   length = 0x00000800
  UBL_STACK       (RW)  : origin = 0x80007800   length = 0x00000800
  UBL_DRAM        (RWX) : origin = 0xC0000000   length = 0x10000000
}

SECTIONS
{

  .text :
  {
    *(.text:.start)
    . = align(4);
    *(.text)
    . = align(4);
  } > UBL_TEXT

  .data :
  {
    *(.const)
  } > UBL_DATA

  .bss :
  {
    *(.bss)
    . = align(4);
  } > UBL_BSS
  
  .ddr_mem :
  {
    . += 0x10000000;
  } run = UBL_DRAM, type=DSECT, RUN_START(_EXTERNAL_RAM_START), RUN_END(_EXTERNAL_RAM_END), SIZE(_EXTERNAL_RAM_SIZE)
  
  .stack :
  {
    .+=0x0400;
  } run = UBL_STACK, type=DSECT, SIZE(_STACK_SIZE)

/* DUMMY Sections */ 
  .armVectorRAM:
  {
    . += 0x20;
  } load=RAM_ARM_VECT, type=DSECT, FILL=0x00000000, START(_ARM_VECT_START), SIZE(_ARM_VECT_SIZE)

  .PRU0InstRAM:
  {
    . += 0x1000;
  } load=PRU0_PROG_RAM, type=DSECT, FILL=0x00000000, START(_PRU0_INST_START), SIZE(_PRU0_INST_SIZE)

  .PRU1InstRAM:
  {
    . += 0x1000;
  } load=PRU1_PROG_RAM, type=DSECT, FILL=0x00000000, START(_PRU1_INST_START), SIZE(_PRU1_INST_SIZE)
    
  .PRU0DataRAM
  {
    . += 0x200;
  } load=PRU0_DATA_RAM, type=DSECT, FILL=0x00000000, START(_PRU0_DATA_START), SIZE(_PRU0_DATA_SIZE)
  
  .PRU1DataRAM
  {
    . += 0x200;
  } load=PRU1_DATA_RAM, type=DSECT, FILL=0x00000000, START(_PRU1_DATA_START), SIZE(_PRU1_DATA_SIZE)

}


