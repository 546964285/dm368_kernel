-lrts64plus.lib
-stack          0x00000800 /* Stack Size */  
-heap           0x00000800 /* Heap Size */

-e start

  _STACK_SIZE          = 0x00000800;
  _INTERNAL_RAM_SIZE   = 0x00008000;
MEMORY
{
  L2RAM		    org=0x11800000 len=0x00008000 /* DSP L2 RAM */
  DRAM        org=0xC0000000 len=0x04000000 /* DDR */
  SHARED_RAM  org=0x80000000 len=0x00020000 /* Shared memory for program */
  AEMIF       org=0x60000000 len=0x02000000 /* AEMIF CS2 region */
  AEMIF_CS3   org=0x62000000 len=0x02000000 /* AEMIF CS3 region */
  STACK	      org=0x11808800 len=0x00000800 /* Stack */

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


