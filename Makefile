CROSS_COMPILE := arm-none-eabi-
CC := $(CROSS_COMPILE)gcc
LD := $(CROSS_COMPILE)ld
CP := $(CROSS_COMPILE)objcopy
DP := $(CROSS_COMPILE)objdump

obj-c := $(shell find . -name "*.c")
obj-s := $(shell find . -name "*.s")
obj-o += $(patsubst %.s,%.o,$(obj-s))
obj-o += $(patsubst %.c,%.o,$(obj-c))

target := start

INCPW := config core lib/inc src bsp
INCPW := $(patsubst %,-I%,$(INCPW))
CFLAG := -DSTM32F10X_MD -DUSE_STDPERIPH_DRIVER $(INCPW) -mcpu=cortex-m3 
THUMB_LIB := /usr/lib/arm-none-eabi/lib/thumb/v7-m/nofp
GCC_LIB   := /usr/lib/gcc/arm-none-eabi/12.2.1/thumb/v7-m/nofp

%.o: %.s
	$(CC) $(CFLAG) -c $< -o $@
%.o: %.c
	$(CC) $(CFLAG) -c $< -o $@

compile:$(obj-o)
	@echo compile finished

link: compile
	$(LD) -L$(THUMB_LIB) -L$(GCC_LIB) -Tlink.ld $(obj-o) -lc_nano -lgcc -lnosys -o $(target).elf
	$(DP) -D $(target).elf > $(target).dump

all: link
	./load.sh

.PHONY: debug clean

clean:
	rm -f $(obj-o) *.elf *.dump

debug:
	echo $(obj-o)