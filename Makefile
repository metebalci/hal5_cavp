#
# SPDX-FileCopyrightText: 2023 Mete Balci
#
# SPDX-License-Identifier: Apache-2.0
#
# Copyright (c) 2023 Mete Balci
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

CFLAGS := -std=gnu11
CFLAGS += -mcpu=cortex-m33 -mthumb
CFLAGS += -O0 -g
CFLAGS += -ffunction-sections -fdata-sections
CFLAGS += $(FLOATFLAGS)
CFLAGS += -I. -Ihal5 
CFLAGS += -Icmsis/CMSIS/Core/Include -Icmsis_device_h5/Include -DSTM32H563xx
CFLAGS += --specs=nano.specs
CFLAGS += -Wall -Werror
CFLAGS += -Wno-unused-variable -Wno-unused-function 
CFLAGS += -fmax-errors=5

LDFLAGS := -mcpu=cortex-m33 -mthumb 
LDFLAGS += $(FLOATFLAGS)
LDFLAGS += --specs=nosys.specs 
LDFLAGS += -Wl,--gc-sections
LDFLAGS += -static
LDFLAGS += --specs=nano.specs
LDFLAGS += -Wl,--start-group -lc -lm -Wl,--end-group

APP_OBJS := startup_stm32h5.o syscalls.o
APP_OBJS += main.o bsp_nucleo_h563zi.o

# compiler
CC := arm-none-eabi-gcc
RM := rm -f

STARTUP_LD := hal5_startup/startup.ld

STARTUP_OBJS := hal5_startup/startup_stm32h5.o 
STARTUP_OBJS += hal5_startup/syscalls.o
STARTUP_OBJS += hal5_startup/bsp_nucleo_h563zi.o

APP_OBJS := main.o

CAVP_HEADERS := cavp-secure-hash/SHA1ShortMsg.rsp.h
CAVP_HEADERS += cavp-secure-hash/SHA1LongMsg.rsp.h
CAVP_HEADERS += cavp-secure-hash/SHA256ShortMsg.rsp.h
CAVP_HEADERS += cavp-secure-hash/SHA256LongMsg.rsp.h
CAVP_HEADERS += cavp-secure-hash/SHA512ShortMsg.rsp.h
CAVP_HEADERS += cavp-secure-hash/SHA512LongMsg.rsp.h

all: clean hal5_cavp.elf flash

clean: 
	$(RM) hal5_cavp.elf
	$(RM) $(STARTUP_OBJS)
	$(RM) $(APP_OBJS)
	$(RM) $(CAVP_HEADERS)

hal5/hal5.a: hal5
	make -C hal5 hal5.a

cavp-secure-hash/%.rsp.h: cavp-secure-hash/%.rsp
	./cavp-secure-hash/rsp2h.py $< > $@

%.o: %.c hal5/hal5.a $(CAVP_HEADERS)
	$(CC) $(CFLAGS) -DCAVP_TEST_${CAVP_TEST} -c -o $@ $<

cmsis:
	git clone --depth 1 -b 5.9.0 https://github.com/ARM-software/CMSIS_5 cmsis

cmsis_device_h5:
	git clone --depth 1 -b v1.1.0 https://github.com/STMicroelectronics/cmsis_device_h5 cmsis_device_h5

hal5:
	git clone https://github.com/metebalci/hal5 hal5

hal5_startup:
	git clone https://github.com/metebalci/hal5_startup hal5_startup

hal5_cavp.elf: cmsis cmsis_device_h5 hal5 hal5_startup Makefile $(APP_OBJS) $(CAVP_HEADERS) $(STARTUP_OBJS) hal5/hal5.a
	$(CC) -T"$(STARTUP_LD)" $(LDFLAGS) -o $@ $(APP_OBJS) $(STARTUP_OBJS) hal5/hal5.a

# programmer
STM32PRG ?= STM32_Programmer_CLI --verbosity 1 -c port=swd mode=HOTPLUG speed=Reliable

flash: hal5_cavp.elf
	$(STM32PRG) --write $<
	$(STM32PRG) -hardRst

erase:
	$(STM32PRG) --erase all

reset:
	$(STM32PRG) -hardRst
