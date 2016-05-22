################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../lapi.c \
../lauxlib.c \
../lbaselib.c \
../lcode.c \
../ldblib.c \
../ldebug.c \
../ldo.c \
../ldump.c \
../lfunc.c \
../lgc.c \
../linit.c \
../liolib.c \
../llex.c \
../lmathlib.c \
../lmem.c \
../loadlib.c \
../lobject.c \
../lopcodes.c \
../loslib.c \
../lparser.c \
../lstate.c \
../lstring.c \
../lstrlib.c \
../ltable.c \
../ltablib.c \
../ltm.c \
../lundump.c \
../lvm.c \
../lzio.c \
../print.c \
../tolua_event.c \
../tolua_is.c \
../tolua_map.c \
../tolua_push.c \
../tolua_to.c 

OBJS += \
./lapi.o \
./lauxlib.o \
./lbaselib.o \
./lcode.o \
./ldblib.o \
./ldebug.o \
./ldo.o \
./ldump.o \
./lfunc.o \
./lgc.o \
./linit.o \
./liolib.o \
./llex.o \
./lmathlib.o \
./lmem.o \
./loadlib.o \
./lobject.o \
./lopcodes.o \
./loslib.o \
./lparser.o \
./lstate.o \
./lstring.o \
./lstrlib.o \
./ltable.o \
./ltablib.o \
./ltm.o \
./lundump.o \
./lvm.o \
./lzio.o \
./print.o \
./tolua_event.o \
./tolua_is.o \
./tolua_map.o \
./tolua_push.o \
./tolua_to.o 

C_DEPS += \
./lapi.d \
./lauxlib.d \
./lbaselib.d \
./lcode.d \
./ldblib.d \
./ldebug.d \
./ldo.d \
./ldump.d \
./lfunc.d \
./lgc.d \
./linit.d \
./liolib.d \
./llex.d \
./lmathlib.d \
./lmem.d \
./loadlib.d \
./lobject.d \
./lopcodes.d \
./loslib.d \
./lparser.d \
./lstate.d \
./lstring.d \
./lstrlib.d \
./ltable.d \
./ltablib.d \
./ltm.d \
./lundump.d \
./lvm.d \
./lzio.d \
./print.d \
./tolua_event.d \
./tolua_is.d \
./tolua_map.d \
./tolua_push.d \
./tolua_to.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I../../tolua -I../../meta_header -I../../lua -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


