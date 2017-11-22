/*
 *  Desc: Linker command file sft application program.
 *  Auth: Daniel Allred
 */

ENTRY(boot)
SECTIONS 
{  
  INTERNAL_RAM_SIZE   = 0x00008000;
  INTERNAL_RAM_START  = 0x00010000;
  INTERNAL_RAM_END    = INTERNAL_RAM_START + INTERNAL_RAM_SIZE;
  
  EXTERNAL_RAM_SIZE   = 0x10000000;  
  EXTERNAL_RAM_START  = 0x80000000;
  EXTERNAL_RAM_END    = EXTERNAL_RAM_START + EXTERNAL_RAM_SIZE;
  
  ASYNC_MEM_START     = 0x02000000;

  IRAM_START          = 0x00000000;
  IRAM_SIZE           = INTERNAL_RAM_SIZE;
  DRAM_START          = 0x00008000;
  DRAM_SIZE           = INTERNAL_RAM_SIZE;
  
  STACK_START         = DRAM_START + DRAM_SIZE;
  STACK_SIZE          = 0x00000800;
  
  . = ASYNC_MEM_START;
  __selfcopysrc = ASYNC_MEM_START;
  __selfcopydest = DRAM_START + 0x00000020;
  
  /* Self copy code is in first 256-32=224 bytes of image */
  .selfcopy : AT (__selfcopydest)
  {
    *(.selfcopy)    
    . = 0xE0;
  }

  /* The entry point address is fixed at 0x100 */
  . = 0x100;
  .boot    : AT ( LOADADDR(.selfcopy) + SIZEOF(.selfcopy))
  {
    *(.boot)
    . = ALIGN(4);
  }
  .text    : AT ( LOADADDR(.boot) + SIZEOF(.boot) )
  {
    *(.text)
    . = ALIGN(4);
  }
  
  /* Move into DRAM for placing const and data sections */
  . += (DRAM_START - IRAM_START);
  .rodata    : AT ( LOADADDR(.text) + SIZEOF(.text) )
  {
    *(.rodata*)
    *(.rodata)
    . = ALIGN(4);
  }  
  .data    : AT ( LOADADDR(.rodata) + SIZEOF(.rodata) )
  {
    *(.data)
    . = ALIGN(4);
  }
      
  __selfcopysrcend = __selfcopysrc + SIZEOF(.selfcopy) +
            SIZEOF(.boot) + SIZEOF(.text) + 
            SIZEOF(.data) + SIZEOF(.rodata);
  
  .bss    :
  {
    *(.bss) *(COMMON)
    . = ALIGN(4);
  }
}

