  .asg    A15, FP
  .asg    B14, DP
  .asg    B15, SP

  .global __stack
__stack:        .usect  ".stack",8,8
  
  .global $bss
  .sect   ".text:.start"

  .global __STACK_SIZE
  .global _main
  .global start

start:
    DINT           ; Disable Interrupts, FP1, EP1, cycle 0
    NOP 4          ; NOP 4 cycles, FP1, EP2, cycle 1-5
    B atomic       ; Branch to atomic, FP1, EP3, cycle 6
    ZERO B0        ; Clear B0, FP1, EP4, cycle 7
    MVC B0,IER     ; Clear all interrupt enables, FP1, EP5, cycle 8
    MVC B0,AMR     ; Disable circular addressing, FP1, EP6, cycle 9
    MVC B0,CSR     ; FP1, EP7, cycle 10
    NOP 1          ; FP1, EP8  cycle 11

atomic:
    MVKL            __stack + __STACK_SIZE - 4, SP ; FP2, EP1, cycle 12
    MVKH            __stack + __STACK_SIZE - 4, SP ; FP2, EP2, cycle 13
    AND             ~7,SP,SP ; Align stack to 8 byte address ; FP2, EP3, cycle 14
    MVKL            $bss,DP  ; FP2, EP4, cycle 15
    MVKH            $bss,DP  ; FP2, EP5, cycle 16
    CALLP   .S2     _main,B3 ; FP2, EP6, cycle 17
    NOP 4          ; FP2, EP7, cycle 18-22
    NOP            ; FP2, EP8, cycle 23

abort:
    B abort       ; FP3, EP1, cycle 24
    NOP           ; FP3, EP2, cycle 25
    NOP           ; FP3, EP3, cycle 26
    NOP           ; FP3, EP4, cycle 27
    NOP           ; FP3, EP5, cycle 28
    NOP           ; FP3, EP6, cycle 29
    NOP           ; FP3, EP7, never executes
    NOP           ; FP3, EP8, never executes