.section .embedded_block 

.word 0xffffded3   # header 

.word 0x11010142   
.word 0x00000344 
.word _entry_point
.word _stack_top 
.word 0x000004ff 
.word 0x00000000 
.word 0xab123579

.section .text

.global _entry_point # defines a symbol so the linker script can find 
                     # an entry point 

# maps our specific wiring to GPIO pins on the Pico 2 

.equ SIO_BASE 0xD0000000
.equ GPIO_OUT_SET SIO_BASE + 0x18 
.equ GPIO_OUT_CLR SIO_BASE + 0x20   
.equ GPIO_OUT_XOR SIO_BASE + 0x28 
.equ GPIO_OUT_DIR SIO_BASE + 0x30 
.equ GPIO_R1 0x0
.equ GPIO_G1 0x1 
.equ GPIO_B1 0x3 
.equ GPIO_GND1 0x2 
.equ GPIO_R2 0x4 
.equ GPIO_G2 0x5 
.equ GPIO_B2 0x6 
.equ GPIO_A  0x7
.equ GPIO_B  0x9 
.equ GPIO_C  0x10 
.equ GPIO_D    0x11 
.equ GPIO_GND2 0x12

# Masks a GPIO pin on the Pico 2; the toggled bit specifies 
# the pin that we're interested in 
# Input: a0 = a GPIO register to mask 
# Output: (1 << GPIO_PIN) 
gpio_mask: 
    slli a0, a0, 1 

# Returns the exact identifier of the GPIO pin 
# at the offset 
# Input:  a0 = a GPIO register to identify 
# Output: a0 = GPIO_PIN*8 + 4 
gpio_offset: 
    mul a0, a0, 0x8 
    add a0, a0, 0x4 

_entry_point:   
        la sp, _stack_top 
        call main 
        wfi 
        j _start 

        .section .text 
        .global main

main: 
    
