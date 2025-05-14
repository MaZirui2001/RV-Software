
# make ar
CROSS_COMPILE := 
AR = $(CROSS_COMPILE)llvm-ar
CC = $(CROSS_COMPILE)clang
AS = $(CROSS_COMPILE)clang

COMMON_FLAGS = -march=rv32im_zicsr_zifencei -mabi=ilp32 -Os --target=riscv32 -g

CFLAGS = -MMD $(COMMON_FLAGS) $(INC_PATH)
CFLAGS += -fno-asynchronous-unwind-tables -fno-builtin -fno-stack-protector 
AFLAGS = $(COMMON_FLAGS)
ARFLAGS = rcs

BUILD_DIR = $(abspath ./build)
TAR_DIR = $(BUILD_DIR)/riscv32
OBJS = $(addprefix $(TAR_DIR)/, $(addsuffix .o, $(basename $(KER_SRCS))))
LIBKER = $(BUILD_DIR)/lib$(LIBNAME).a

# Compile C to object file
$(TAR_DIR)/%.o: %.c
	@mkdir -p $(dir $@) && echo "\033[33m[CC]\033[0m $<"
	@$(CC) $(CFLAGS) -c -o $@ $(realpath $<)

# Compile assembly to object file
$(TAR_DIR)/%.o: %.S
	@mkdir -p $(dir $@) && echo "\033[33m[AS]\033[0m $<"
	@$(AS) $(AFLAGS) -c -o $@ $(realpath $<)

libkernel: $(LIBKER)
$(LIBKER): $(OBJS)
	@echo "\033[33m[AR]\033[0m build/$(notdir $@)"
	@$(AR) $(ARFLAGS) $@ $(realpath $^)

clean:
	rm -rf $(BUILD_DIR)
