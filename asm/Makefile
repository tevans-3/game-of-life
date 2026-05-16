MAKEFLAGS += -r
PREFIX := riscv64-elf
AS := $(PREFIX)-as
LD := $(PREFIX)-ld
OBJCOPY := $(PREFIX)-objcopy

ASFLAGS := -march=rv32imac -mabi=ilp32
LDFLAGS := -melf32lriscv -T link.ld
UF2_FAM := 0xe48b52a

TARGET := game-of-life
SRC := game-of-life.s

all: $(TARGET).uf2

%.o: %.s
	$(AS) $(ASFLAGS) -o $@ $<

%.elf: %.o
	$(LD) $(LDFLAGS) -o $@ $<

%.uf2: %.elf
	elf2uf2-rs $< $@

clean:
	rm -rf *.o *.elf *.bin *.uf2
