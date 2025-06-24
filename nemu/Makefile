NAME = nemu
INC_DIR += ./include
BUILD_DIR ?= ./build
OBJ_DIR ?= $(BUILD_DIR)/obj
BINARY ?= $(BUILD_DIR)/$(NAME)

include Makefile.git

.DEFAULT_GOAL = app

# Compilation flags
CC = gcc
LD = gcc
INCLUDES  = $(addprefix -I, $(INC_DIR))
CFLAGS   += -O2 -MMD -Wall -Werror -ggdb $(INCLUDES)
LDFLAGS ?=

# For Windows MinGW
ifeq ($(OS),Windows_NT)
	VCPKG_ROOT ?= $(NEMU_HOME)/../vcpkg
	VCPKG_TRIPLET ?= x64-mingw-static
	VCPKG_INSTALLED_DIR = $(VCPKG_ROOT)/installed/$(VCPKG_TRIPLET)

	CFLAGS += -I$(VCPKG_INSTALLED_DIR)/include
	LDFLAGS += -L$(VCPKG_INSTALLED_DIR)/lib -lSDL2main -lSDL2 -lreadline -L./lib -l:libgnurx.a

	# Windows system libraries required by SDL2
	LDFLAGS += -lmingw32 -lgdi32 -lwinmm -limm32 -lole32 -loleaut32 -luuid -lversion -lsetupapi -lhid -lws2_32

	# OpenGL libraries
	LDFLAGS += -lopengl32

	# Additional Windows libraries for COM and other SDL2 dependencies
	LDFLAGS += -lcomdlg32 -lcomctl32 -lshell32 -ladvapi32 -luser32 -lkernel32
# For Linux
else
	LDFLAGS += -lSDL2 -lreadline
endif

# Files to be compiled
SRCS = $(shell find src/ -name "*.c")
OBJS = $(SRCS:src/%.c=$(OBJ_DIR)/%.o)

# Compilation patterns
$(OBJ_DIR)/%.o: src/%.c
	@echo + CC $<
	@mkdir -p $(dir $@)
	@$(CC) $(CFLAGS) -c -o $@ $<

# Depencies
-include $(OBJS:.o=.d)

# Some convinient rules

.PHONY: app run submit clean
app: $(BINARY)

ARGS ?= -l $(BUILD_DIR)/nemu-log.txt

# Command to execute NEMU
NEMU_EXEC := $(BINARY) $(ARGS)

$(BINARY): $(OBJS)

	@echo + LD $@
	@$(LD) -O2 -o $@ $^ $(LDFLAGS)

run: $(BINARY)

	$(NEMU_EXEC)

gdb: $(BINARY)

	gdb -s $(BINARY) --args $(NEMU_EXEC)

clean: 
	rm -rf $(BUILD_DIR)
