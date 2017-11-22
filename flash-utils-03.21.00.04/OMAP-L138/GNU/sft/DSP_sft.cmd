-lrts64plus.lib
-stack          0x00000800 /* Stack Size */  
-heap           0x00000800 /* Heap Size */

-e start

  _STACK_SIZE          = 0x00000800;
  _INTERNAL_RAM_SIZE   = 0x00008000;
MEMORY
{
  L2RAM		  org=0x11800000 len=0x00008000 /* DSP L2 RAM */
  DRAM        org=0xC0000000 len=0x04000000 /* DDR */
  SHARED_RAM  org=0x80000000 len=0x00020000 /* Shared memory for program */
  AEMIF       org=0x60000000 len=0x02000000 /* AEMIF CS2 region */
  AEMIF_CS3   org=0x62000000 len=0x02000000 /* AEMIF CS3 region */
  STACK	      org=0x11808800 len=0x00000800 /* Stack */
}

SECTIONS
{
 
  .text :
  {
  } > L2RAM
  .const :
  {
  } > L2RAM
  .bss :
  {
  } > L2RAM
  .far :
  {
  } > L2RAM
  .stack :
  {
  } > STACK
  .data :
  {
  } > L2RAM
  .cinit :
  {
  } > L2RAM
  .sysmem :
  {
  } > L2RAM
  .cio :
  {
  } > L2RAM
  .switch :
  {
  } > L2RAM
  
  .aemif_mem
  {
    . += 0x1000;
  } load = AEMIF, FILL=0x00000000, type=DSECT, START(_NORStart)
  
  .extram 
  {
    . += 0x04000000;
  } load = DRAM, FILL=0x00000000, type=DSECT, START(_EXTERNAL_RAM_START), END(_EXTERNAL_RAM_END), SIZE(_EXTERNAL_RAM_SIZE)
}