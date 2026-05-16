.section .embedded_block 

.word 0xffffded3   # header 
.byte 0x42         # type 
.byte 1            # size 
.half 0x0003   # data 

.byte 0x44         # type  
.byte 2            # size 
.byte 1 

.word 0x10210142   # footer 

.section .reset 

.global _entry_point # defines a symbol so the linker script can find 
                     # an entry point 
_entry_point: 

.section .text

.section .data 

.section .bss 
