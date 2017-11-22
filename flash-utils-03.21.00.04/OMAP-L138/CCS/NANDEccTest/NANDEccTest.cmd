-lrts64plus.lib
-stack          0x00000800 /* Stack Size */  
-heap           0x00000800 /* Heap Size */

MEMORY
{
  DRAM        org=0xC0000000 len=0x03000000 /* External DRAM */
  TEST_DRAM   org=0xC3000000 len=0x01000000 /* External DRAM for test flash image*/
  SHARED_RAM  org=0x80000000 len=0x00020000 
  AEMIF       org=0x60000000 len=0x02000000 /* AEMIF CS2 region */
  AEMIF_CS3   org=0x62000000 len=0x02000000 /* AEMIF CS3 region */
}

SECTIONS
{
  .text :
  {
  } > SHARED_RAM
  .const :
  {
  } > SHARED_RAM
  .bss :
  {
  } > SHARED_RAM
  .far :
  {
  } > SHARED_RAM
  .stack :
  {
  } > SHARED_RAM
  .data :
  {
  } > SHARED_RAM
  .cinit :
  {
  } > SHARED_RAM
  .sysmem :
  {
  } > SHARED_RAM
  .cio :
  {
  } > SHARED_RAM
  .switch :
  {
  } > SHARED_RAM
  .aemif_mem :
  {
  } > AEMIF_CS3, RUN_START(_NANDStart)
  .ddrram	 :
  {
    . += 0x04000000;
  } > DRAM, type=DSECT, START(_EXTERNAL_RAM_START), END(_EXTERNAL_RAM_END)
  
  .testImage :
  {
  } > TEST_DRAM
}