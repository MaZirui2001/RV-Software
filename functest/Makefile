BASE_PORT = $(abspath ../base-port)
SIM_PATH = $(abspath ../../)
# Colors
COLOR_RED   		= \033[1;31m
COLOR_GREEN 		= \033[1;32m
COLOR_NONE  		= \033[0m

RESULT = .result
$(shell > $(RESULT))

NAMES ?= $(sort $(basename $(notdir $(shell find src/. -name "*.c"))))

all: $(NAMES)

%: src/%.c
	@if BASE_PORT=$(BASE_PORT) SIM_PATH=$(SIM_PATH) NAMES=$* SRCS=$<  ARGS=-b $(MAKE) -s -f $(BASE_PORT)/Makefile $(MAKECMDGOALS) ; then\
		printf "%14s: [$(COLOR_GREEN)ACCEPT$(COLOR_NONE)]\n" $(notdir $*) >> $(RESULT);\
	else\
		printf "%14s: [$(COLOR_RED)FAILED$(COLOR_NONE)]\n" $(notdir $*) >> $(RESULT);\
	fi

run: all
	@cat $(RESULT)
	@rm -rf $(RESULT)

gdb: all 
	@cat $(RESULT)
	@rm -rf $(RESULT)

clean:
	rm -rf build

clean-all:
	rm -rf build
	rm -rf $(BUILD_DIR) $(BASE_PORT)/*/build