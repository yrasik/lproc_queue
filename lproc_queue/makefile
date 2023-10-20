#/*
#* This file is part of the "Lua Extentions" distribution
#* (https://github.com/yrasik/lproc_queue).
#* Copyright (c) 2023 Yuri Stepanenko.
#*
#* This program is free software: you can redistribute it and/or modify
#* it under the terms of the GNU General Public License as published by
#* the Free Software Foundation, version 3.
#*
#* This program is distributed in the hope that it will be useful, but
#* WITHOUT ANY WARRANTY; without even the implied warranty of
#* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
#* General Public License for more details.
#*
#* You should have received a copy of the GNU General Public License
#* along with this program. If not, see <http://www.gnu.org/licenses/>.
#*/

TARGET_NAME := lproc_queue.so


PWD = $(shell pwd)

TARGET_SYSTEM ?= Ubuntu_host_Linux_FS

ifeq ($(TARGET_SYSTEM),)
	# Для сборки buildroot
	#$(MAKE) CC="$(TARGET_CC)" LD="$(TARGET_LD)" -C $(@D)
	 GCC ?= $(CC)
	 INC_LUA_DIR ?= 
	 LIB_LUA_DIR ?= $(PREFIX)/lib
	 LIB_PTHREAD_DIR ?= 
	 LIB_LUA ?= lua
endif
# Сборка под host
ifeq ($(TARGET_SYSTEM), Ubuntu_host_Linux_FS)
	GCC := gcc
	INC_LUA_DIR ?= /usr/include/lua5.3
	LIB_LUA_DIR ?= /usr/lib/x86_64-linux-gnu/
	LIB_PTHREAD_DIR ?= /usr/lib/x86_64-linux-gnu/
	LIB_LUA ?= lua5.3
endif


SRC_FOLDER   ?= ./src
BUILD_FOLDER ?= ./build


GCC_FLAGS := -std=gnu99


DEFINES := -DSTRUCT_PACKED -DPIC


INC_DIR := \
	-I$(INC_LUA_DIR) \
		\
	-I$(SRC_FOLDER)/debug \
	-I$(SRC_FOLDER)/time


LIB_DIR := \
	-L$(LIB_PTHREAD_DIR) \
	-L$(LIB_LUA_DIR) \


LIBS := -lpthread -l$(LIB_LUA)


OBJS += \
	$(BUILD_FOLDER)/lproc_queue.o \
	$(BUILD_FOLDER)/time/ul_time.o



# All Target
all: main-build

clean:
	rm -rf ${BUILD_FOLDER}
	rm -rf Release
	rm -rf Debug
	rm -rf Release_Linux
	rm -rf Release_MinGW
	rm -rf .settings
	rm -rf ./doc/_minted-top
	find ./doc -type f -name "*.aux" -exec rm -f {} \;
	find ./doc -type f -name "*.log" -exec rm -f {} \;
	find ./doc -type f -name "*.toc" -exec rm -f {} \;
	find ./doc -type f -name "*.out" -exec rm -f {} \;


# Main-build Target
main-build: prepare $(TARGET_NAME)

prepare:
	mkdir -p $(BUILD_FOLDER)

# Tool invocations
$(TARGET_NAME): $(OBJS) 
	@echo 'Building target: $@'
	@echo 'Invoking: GCC C Linker'
	$(GCC) $(FLAGS) $(DEFINES) $(LIB_DIR) -shared -o ${BUILD_FOLDER}/$(TARGET_NAME) $(OBJS)  $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '
	#cp ${BUILD_FOLDER}/$(TARGET_NAME) $(TARGET_NAME)


$(BUILD_FOLDER)/%.o: $(SRC_FOLDER)/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	#mkdir -p $(BUILD_FOLDER)/debug
	$(GCC) $(GCC_FLAGS) $(DEFINES) $(INC_DIR) -O3 -Wall -c -fmessage-length=0 -fPIC -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

$(BUILD_FOLDER)/time/%.o: $(SRC_FOLDER)/time/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	mkdir -p $(BUILD_FOLDER)/time
	$(GCC) $(GCC_FLAGS) $(DEFINES) $(INC_DIR) -O3 -Wall -c -fmessage-length=0 -fPIC -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '
