ENTRY(_start)

PHDRS {
  image PT_LOAD ;
}

SECTIONS {
  .image 0x4000000 : AT(0) {
    *(entry)
    *(.text*)
    *(.rodata*)
    *(.rdata*)
    *(.data*)
  } :image

  .bss : {
    *(.bss*)
    *(COMMON)
  }

  _end = .;

  /DISCARD/ : {
    *(.comment)
    *(.note*)
  }
}
