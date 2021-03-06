OUTPUT_FORMAT("elf32-littlearm")
OUTPUT_ARCH(arm)
TARGET(binary)
ENTRY(__start)

#include <arch/arm/rtsm-config.h>

INPUT(../guestimages/guest0.bin)
GUEST0_STACK     = CFG_MEMMAP_GUEST0_OFFSET + GUEST_SIZE_MAX;
INPUT(../guestimages/guest1.bin)
GUEST1_STACK    = CFG_MEMMAP_GUEST1_OFFSET + GUEST_SIZE_MAX;

HEAP_SIZE   = 0x00100000;

SECTIONS
{
 . = CFG_MEMMAP_PHYS_START;

 /* Guest 0 */
 . = CFG_MEMMAP_GUEST0_OFFSET;
 _guest0_bin_start = .;
 .guest0 : { ../guestimages/guest0.bin}
 _guest0_bin_end = .;

 . = GUEST0_STACK;
 guest0_stacktop = .;
 . = GUEST0_STACK + 0x01000000;
 guest0_stacklimit = .;

 . = CFG_MEMMAP_GUEST1_OFFSET;
 _guest1_bin_start = .;
 .guest1 : { ../guestimages/guest1.bin }
 _guest1_bin_end = .;

 . = GUEST1_STACK;
 guest1_stacktop = .;
 . = GUEST1_STACK + 0x01000000;
 guest1_stacklimit = .;

 .text : {
    *(.text);
    __vdev_module_high_start = .;
    *(.vdev_module0.init);
    __vdev_module_high_end = .;
    *(.vdev_module1.init);
    __vdev_module_middle_end = .;
    *(.vdev_module2.init);
    __vdev_module_low_end = .;
 }

 .= ALIGN(4);
 .rodata : {
    *(.rodata)
 }

 .= ALIGN(4);
 .data : {
    *(.data)
 }

 .= ALIGN(4);
 PROVIDE(__begin_bss = .);
 .bss : {
    *(.bss)
 }
 PROVIDE(__end_bss = .);

 . = ALIGN(4);
 PROVIDE(__begin_heap = .);
 .heap : {
    *(.heap)
     . = . + HEAP_SIZE;
 }
 PROVIDE(__end_heap = .);

 .stack : {
     __start_stack__ = .;
     . += 0x00100000;
     . = ALIGN(4);
     __end_stack = .;
 }
}
