PROJECT := tiny-monitor
BUILD := build
TOOLCHAIN_DIR ?= C:/MounRiver/MounRiver_Studio2/resources/app/resources/win32/components/WCH/Toolchain/RISC-V Embedded GCC/bin
PREFIX ?= $(TOOLCHAIN_DIR)/riscv-none-embed-
CC := $(PREFIX)gcc
OBJCOPY := $(PREFIX)objcopy
SIZE := $(PREFIX)size
HOSTCC ?= gcc
INCLUDES := -IUser -IDebug -ICore -IPeripheral/inc
ARCH := -march=rv32imacxw -mabi=ilp32 -msmall-data-limit=8 -msave-restore
CFLAGS := $(ARCH) -Os -std=gnu99 -Wall -Wextra -Werror -ffunction-sections -fdata-sections -fsigned-char -fno-common $(INCLUDES)
LDFLAGS := $(ARCH) -nostartfiles --specs=nano.specs --specs=nosys.specs -Wl,--gc-sections,-Map,$(BUILD)/$(PROJECT).map -TLd/Link.ld
C_SOURCES := $(wildcard User/*.c Debug/*.c Core/*.c Peripheral/src/*.c)
ASM_SOURCES := Startup/startup_ch32x035.S
OBJECTS := $(patsubst %.c,$(BUILD)/%.o,$(C_SOURCES)) $(patsubst %.S,$(BUILD)/%.o,$(ASM_SOURCES))
.PHONY: all clean test size
all: $(BUILD)/$(PROJECT).elf $(BUILD)/$(PROJECT).hex $(BUILD)/$(PROJECT).bin size
$(BUILD)/$(PROJECT).elf: $(OBJECTS)
	@if not exist "$(subst /,\,$(@D))" mkdir "$(subst /,\,$(@D))"
	$(CC) $(OBJECTS) $(LDFLAGS) -o $@
$(BUILD)/%.o: %.c
	@if not exist "$(subst /,\,$(@D))" mkdir "$(subst /,\,$(@D))"
	$(CC) $(CFLAGS) -MMD -MP -c $< -o $@
$(BUILD)/%.o: %.S
	@if not exist "$(subst /,\,$(@D))" mkdir "$(subst /,\,$(@D))"
	$(CC) $(ARCH) -x assembler-with-cpp -IStartup -c $< -o $@
$(BUILD)/$(PROJECT).hex: $(BUILD)/$(PROJECT).elf
	$(OBJCOPY) -O ihex $< $@
$(BUILD)/$(PROJECT).bin: $(BUILD)/$(PROJECT).elf
	$(OBJCOPY) -O binary $< $@
size: $(BUILD)/$(PROJECT).elf
	$(SIZE) $<
test:
	$(HOSTCC) -DHOST_TEST -std=c99 -Wall -Wextra -Werror -IUser tests/test_monitor_core.c User/parser.c -o $(BUILD)/test_monitor_core
	$(BUILD)/test_monitor_core
	$(HOSTCC) -DHOST_TEST -std=c99 -Wall -Wextra -Werror tests/test_line_endings.c -o $(BUILD)/test_line_endings
	$(BUILD)/test_line_endings
	$(HOSTCC) -DHOST_TEST -std=c99 -Wall -Wextra -Werror -IUser tests/test_pin_policy.c User/pin_policy.c -o $(BUILD)/test_pin_policy
	$(BUILD)/test_pin_policy
clean:
	@if exist "$(BUILD)" rmdir /s /q "$(BUILD)"
-include $(OBJECTS:.o=.d)
