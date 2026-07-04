# Sources: 
# https://pip-assets.raspberrypi.com/categories/1214-rp2350/documents/RP-008373-DS-2-rp2350-datasheet.pdf
# https://blog.wolfman.com/articles/2025/5/19/bare-metal-gpio-twiddling-for-risc-v-on-rpi-pico2
# https://www.weigu.lu/microcontroller/pico2_risc-v_ass/index.html
# https://github.com/wolfmanjm/RISC-V-RP2350-baremetal/blob/master/libsrc/gpio.s

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

.equ SYSCTL_BASE,   0x40000000 
.equ CLK_EN_REG,    SYSCTL_BASE + 0x100 

# this is a pad isolation control register: ? 
.equ PAD_ISO_REG,   0x40038000  + 0x40  
.equ IOMUX_BASE,    0x40028000 
.equ IOMUX_GPIO15,  IOMUX_BASE  + 0x7c 

.equ SIO_BASE, 0xD0000000 

# maps our specific wiring to GPIO pins on the Pico 2 
.equ GPIO_OUT_SET,  SIO_BASE + 0x18 
.equ GPIO_OUT_REG,  SIO_BASE + 0x10 
.equ GPIO_OUT_CLR,  SIO_BASE + 0x20   
.equ GPIO_OUT_XOR,  SIO_BASE + 0x28 
.equ GPIO_OUT_DIR,  SIO_BASE + 0x30 
.equ GPIO_R1,   0x0
.equ GPIO_G1,   0x1 
.equ GPIO_B1,   0x3 
.equ GPIO_GND1, 0x2 
.equ GPIO_R2,   0x4 
.equ GPIO_G2,   0x5 
.equ GPIO_B2,   0x6 
.equ GPIO_A,    0x7
.equ GPIO_B,    0x9 
.equ GPIO_C,    0x10 
.equ GPIO_D,    0x11 
.equ GPIO_GND2, 0x2

# Masks a GPIO pin on the Pico 2; the toggled bit specifies 
# the pin that we're interested in 
# Input: a0 = a GPIO register to mask 
# Output: (1 << GPIO_PIN) 
gpio_mask: 
    slli a0, a0, 1 
    ret 
# Sets all the pins we need, mapped above, as output 
set_pins: 
    addi sp, sp, -16 
    sd ra, 0(sp) 

    li t0, GPIO_CTRL 
    lw t1, 0(t0)    # reload the value in the direction register  
    call gpio_mask 
    lw t2, 0(a0) 
    or t1, t1, t2 
    sw t1, 0(t0) 

    ld ra, 0(sp) 
    addi sp, sp, 16 
    ret 

# Returns the exact identifier of the GPIO pin 
# at the offset 
# Input:  a0 = a GPIO register to identify 
# Output: a0 = GPIO_PIN*8 + 4 
gpio_offset: 
    mul a0, a0, 0x8 
    add a0, a0, 0x4 
    ret

_entry_point:   
        la sp, _stack_top 
        call main 
        wfi 
        j _start 

        .section .text 
        .global main

main: 
    # need to tell the multiplexer to set our pins to GPIO 
    li t0, GPIO_CTRL 
    lw t1, 0(t0) 
    andi t1, t1, ~0x1F 
    ori t1, t1, 5 # this sets the function select to 5 for GPIO mode 
    sw t1, 0(t0)  # GPIO mode is now selected in GPIO_CTRL  

    # every pin we're using needs to be set as an output 
    li t0, GPIO_DIR # this pin specifies whether a pin is input or output 
    lw t1, 0(t0)    # the current value in the direction register   
    lw a0, GPIO_R1 
    call gpio_mask 
    lw t2, 0(a0)    # load the masked GPIO pin into register t1 
    or t1, t1, t2  
    sw t1, 0(t0)    # stores the updated value back to the GPIO direction register

    lw t1, 0(t0)    # reload the value in the direction register 
    lw a0, GPIO_G1 
    call gpio_mask 
    lw t2, 0(a0) 
    or t1, t1, t2 
    sw t1, 0(t0) 

    lw t1, 0(t0)    # reload the value in the direction register 
    lw a0, GPIO_G1 
    call gpio_mask 
    lw t2, 0(a0) 
    or t1, t1, t2 
    sw t1, 0(t0) 
    



    j main 
    
