ENTRY(_start)

PHDRS {
  image PT_LOAD ;
}

SECTIONS {
  .image 0x100000 : AT(0) {
    *(entry)
    *(.text*)
    *(.rodata*)
    *(.rdata*)
    *(.data*)
  } :image

  /* The .bss section will follow .image in memory */
  .bss : {
    *(.bss*)
    *(COMMON)
  }

  etext = .;
  _etext = .;
  edata = .;
  _edata = .;
  end = .;
  _end = .;

  _heap_start = ALIGN(4096);
  _stack_pointer = 0x7c00;
  _heap_end = 0x8000000;

  /DISCARD/ : {
    *(.debug*)
    *(.comment)
    *(.note*)
  }
}