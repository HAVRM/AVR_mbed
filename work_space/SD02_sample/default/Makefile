################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL := cmd.exe
RM := rm -rf

USER_OBJS :=

LIBS := 
PROJ := 

O_SRCS := 
C_SRCS := 
S_SRCS := 
S_UPPER_SRCS := 
OBJ_SRCS := 
ASM_SRCS := 
PREPROCESSING_SRCS := 
OBJS := 
OBJS_AS_ARGS := 
C_DEPS := 
C_DEPS_AS_ARGS := 
EXECUTABLES := 
OUTPUT_FILE_PATH :=
OUTPUT_FILE_PATH_AS_ARGS :=
AVR_APP_PATH :=$$$AVR_APP_PATH$$$
QUOTE := "
ADDITIONAL_DEPENDENCIES:=
OUTPUT_FILE_DEP:=

# Every subdirectory with source files must be described here
SUBDIRS := 


# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS +=  \
../SDcm02_sample.c \
../sd_L3.c \
../sd_common.c \
../sd_L1.c \
../sd_L2.c \
../delay.c


PREPROCESSING_SRCS += 


ASM_SRCS += 


OBJS +=  \
SDcm02_sample.o \
sd_L3.o \
sd_common.o \
sd_L1.o \
sd_L2.o \
delay.o


OBJS_AS_ARGS +=  \
SDcm02_sample.o \
sd_L3.o \
sd_common.o \
sd_L1.o \
sd_L2.o \
delay.o


C_DEPS +=  \
SDcm02_sample.d \
sd_L3.d \
sd_common.d \
sd_L1.d \
sd_L2.d \
delay.d


C_DEPS_AS_ARGS +=  \
SDcm02_sample.d \
sd_L3.d \
sd_common.d \
sd_L1.d \
sd_L2.d \
delay.d


OUTPUT_FILE_PATH +=SDcm02_sample.elf

OUTPUT_FILE_PATH_AS_ARGS +=SDcm02_sample.elf

ADDITIONAL_DEPENDENCIES:=

OUTPUT_FILE_DEP:= ./makedep.mk

# AVR32/GNU C Compiler













./%.o: .././%.c
	@echo Building file: $<
	@echo Invoking: AVR/GNU C Compiler : (AVR_8_bit_GNU_Toolchain_3.4.1_830) 4.6.2
	$(QUOTE)D:\Atmel Studio\extensions\Atmel\AVRGCC\3.4.1.95\AVRToolchain\bin\avr-gcc.exe$(QUOTE) -funsigned-char -funsigned-bitfields -Os -fpack-struct -fshort-enums -Wall -c -gdwarf-2 -std=gnu99  -MD -MP -MF "$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)"  -mmcu=atmega328p   -o"$@" "$<" 
	@echo Finished building: $<



# AVR32/GNU Preprocessing Assembler



# AVR32/GNU Assembler




ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

# Add inputs and outputs from these tool invocations to the build variables 

# All Target
all: $(OUTPUT_FILE_PATH) $(ADDITIONAL_DEPENDENCIES)

$(OUTPUT_FILE_PATH): $(OBJS) $(USER_OBJS) $(OUTPUT_FILE_DEP)
	@echo Building target: $@
	@echo Invoking: AVR/GNU Linker : (AVR_8_bit_GNU_Toolchain_3.4.1_830) 4.6.2
	$(QUOTE)D:\Atmel Studio\extensions\Atmel\AVRGCC\3.4.1.95\AVRToolchain\bin\avr-gcc.exe$(QUOTE) -o$(OUTPUT_FILE_PATH_AS_ARGS) $(OBJS_AS_ARGS) $(USER_OBJS) $(LIBS) -Wl,-Map="SDcm02_sample.map" -Wl,--start-group  -Wl,--end-group  -mmcu=atmega328p  
	@echo Finished building target: $@
	"D:\Atmel Studio\extensions\Atmel\AVRGCC\3.4.1.95\AVRToolchain\bin\avr-objcopy.exe" -O ihex -R .eeprom -R .fuse -R .lock -R .signature  "SDcm02_sample.elf" "SDcm02_sample.hex"
	"D:\Atmel Studio\extensions\Atmel\AVRGCC\3.4.1.95\AVRToolchain\bin\avr-objcopy.exe" -j .eeprom  --set-section-flags=.eeprom=alloc,load --change-section-lma .eeprom=0  --no-change-warnings -O ihex "SDcm02_sample.elf" "SDcm02_sample.eep" || exit 0
	"D:\Atmel Studio\extensions\Atmel\AVRGCC\3.4.1.95\AVRToolchain\bin\avr-objdump.exe" -h -S "SDcm02_sample.elf" > "SDcm02_sample.lss"
	"D:\Atmel Studio\extensions\Atmel\AVRGCC\3.4.1.95\AVRToolchain\bin\avr-objcopy.exe" -O srec -R .eeprom -R .fuse -R .lock -R .signature  "SDcm02_sample.elf" "SDcm02_sample.srec"
	"D:\Atmel Studio\extensions\Atmel\AVRGCC\3.4.1.95\AVRToolchain\bin\avr-size.exe" "SDcm02_sample.elf"
	





# Other Targets
clean:
	-$(RM) $(OBJS_AS_ARGS)$(C_DEPS_AS_ARGS) $(EXECUTABLES) 
	rm -rf "SDcm02_sample.elf" "SDcm02_sample.a" "SDcm02_sample.hex" "SDcm02_sample.lss" "SDcm02_sample.eep" "SDcm02_sample.map" "SDcm02_sample.srec"
	