################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include src/subdir.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 

# All Target
all: DemoLibLucasBoard.axf

# Tool invocations
DemoLibLucasBoard.axf: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: MCU Linker'
	arm-none-eabi-gcc -nostdlib -L"C:\Users\Luco\repoTest\lib_lucas_board\Debug" -L"C:\Users\Luco\repoTest\lpc_board_nxp_lpcxpresso_1769\Debug" -L"C:\Users\Luco\repoTest\lpc_chip_175x_6x\Debug" -Xlinker -Map="DemoLibLucasBoard.map" -Xlinker --gc-sections -Xlinker -print-memory-usage -mcpu=cortex-m3 -mthumb -T "DemoLibLucasBoard_Debug.ld" -o "DemoLibLucasBoard.axf" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '
	$(MAKE) --no-print-directory post-build

# Other Targets
clean:
	-$(RM) $(EXECUTABLES)$(OBJS)$(C_DEPS) DemoLibLucasBoard.axf
	-@echo ' '

post-build:
	-@echo 'Performing post-build steps'
	-arm-none-eabi-size "DemoLibLucasBoard.axf"; # arm-none-eabi-objcopy -v -O binary "DemoLibLucasBoard.axf" "DemoLibLucasBoard.bin" ; # checksum -p LPC1769 -d "DemoLibLucasBoard.bin";
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY: post-build

-include ../makefile.targets
