.section entry, "ax"
.globl _start

_start:
  call main
  pushl %eax
  call exit
  jmp _start
