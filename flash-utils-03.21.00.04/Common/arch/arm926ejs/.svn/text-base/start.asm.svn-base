    .global __stack
__stack:  .section  ".stack"

    .sect   ".text:.start"
    .global __STACK_SIZE
    .global main
    .global start

start:
	  /* set the cpu to SVC32 mode */
    mrs r0,cpsr
    bic	r0,r0,#0x1f
    orr	r0,r0,#0xd3
    msr	cpsr,r0

cache_mmu_setup:
    /* Set the IVT to low memory, disable MMU & dcache, enable icache */
    mrc p15,#0,r0,c1,c0,#0
    bic r0,r0,#0x00003300
    bic r0,r0,#0x00000087
    orr r0,r0,#0x00000002
    orr r0,r0,#0x00001000
    mcr p15,#0,r0,c1,c0,#0
    
stack_setup:
    /* Set up the stack */
    ldr	r0, stackptr
    ldr r1, stacksize
    add	r0, r0, r1
    sub	sp, r0, #4
    bic sp, sp, #7 

    /* Load the Kernel entry point address */
    ldr	r0, main_entry

    /* Jump to Entry Point */
    mov pc, r0
    
abort:
    b   abort

stackptr:
    .word __stack
stacksize:
    .word 0x800
main_entry:
    .word  main

