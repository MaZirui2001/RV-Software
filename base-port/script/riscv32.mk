CROSS_COMPILE := 
COMMON_FLAGS  := --target=riscv32 -march=rv32im_zicsr_zifencei -mabi=ilp32 -g -fno-pic 
CFLAGS        += $(COMMON_FLAGS) -static -fdata-sections -ffunction-sections
AFLAGS        += $(COMMON_FLAGS) 
LDFLAGS       += -melf32lriscv -static --gc-sections -e _start