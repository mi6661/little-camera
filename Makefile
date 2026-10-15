CROSS_COMPILE := arm-none-eabi-
CC := $(CROSS_COMPILE)gcc
LD := $(CROSS_COMPILE)ld
DP := $(CROSS_COMPILE)objdump
CP := $(CROSS_COMPILE)objcopy

TAR := start
INC := -Icore/inc -Ilib/inc -Ibsp
DEF := -DUSE_STDPERIPH_DRIVER -DSTM32F40_41xxx -DHSE_VALUE=8000000
CFLAG := $(INC) $(DEF) -mcpu=cortex-m4 -mthumb -g3 -O0

# FMC 外设只存在于 STM32F42x/43x/446/469 上，F40/41 用的是 FSMC，
# 所以 stm32f4xx_fmc.c 对 STM32F40_41xxx 目标无法编译，需要排除。
OBJ_C := $(filter-out %stm32f4xx_fmc.c,$(shell find . -name "*.c"))
OBJ_S := $(shell find . -name "*.s")
OBJ_O += $(patsubst %.c,%.o,$(OBJ_C))
OBJ_O += $(patsubst %.s,%.o,$(OBJ_S))


%.o: %.c
	$(CC) $(CFLAG) -c $< -o $@

%.o: %.s
	$(CC) $(CFLAG) -c $< -o $@

compile: $(OBJ_O)

link: $(OBJ_O)
	$(LD) -Tscript/linker.ld $^ -o $(TAR).elf
	$(DP) -D $(TAR).elf > $(TAR).dump

load: link
	sudo openocd -f interface/stlink-v2.cfg -f target/stm32f4x.cfg -c "program $(TAR).elf exit reset"

.PHONY: debug clean

clean:
	rm -rf $(OBJ_O) *.elf *.dump

debug:
	@echo $(OBJ_O)
