MEMORY
{
  /* ARM ROM Memory */
  ROM_ARM         : o = 0xFFFD0000  l = 0x00020000  /*     64 KB */

  /* ARM Internal RAM Memory*/
  RAM_ARM         : o = 0xFFFF0000  l = 0x00002000  /*      8 KB */
}

SECTIONS
{
  .arm_vectors :
  {
    *(.text) 
  } > RAM_ARM
  
  .rom : LOAD=ROM_ARM, TYPE=DSECT, START(ARM_ROM_START), FILL=0xFFFFFFFF
  {
    .+=0x04;
  } 
}