-e boot

IRAMStart  = 0x80000000;
IRAMSize   = 0x00008000;
DRAMStart  = 0x80000000;
DRAMSize   = 0x00008000;

ASYNC_MEM_START    = 0x62000000;

INTERNAL_RAM_START = 0x80000000;
INTERNAL_RAM_SIZE  = 0x00008000;

STACK_START = INTERNAL_RAM_START + INTERNAL_RAM_SIZE;

MEMORY
{
  ARM_I_IVT       (RX)  : origin = 0xFFFFD000   length = 0x00000020

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
    *(.boot) 
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
  } run = UBL_DRAM, type=DSECT, RUN_START(EXTERNAL_RAM_START), RUN_END(EXTERNAL_RAM_END), SIZE(EXTERNAL_RAM_SIZE)
  
  .stack :
  {
    .+=0x0400;
  } run = UBL_STACK, type=DSECT, SIZE(STACK_SIZE)
   
}

