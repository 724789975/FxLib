################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../connection.cpp \
../connectionmgr.cpp \
../ifnet.cpp \
../iothread.cpp \
../loopbuff.cpp \
../mysock.cpp \
../net.cpp \
../sockmgr.cpp 

OBJS += \
./connection.o \
./connectionmgr.o \
./ifnet.o \
./iothread.o \
./loopbuff.o \
./mysock.o \
./net.o \
./sockmgr.o 

CPP_DEPS += \
./connection.d \
./connectionmgr.d \
./ifnet.d \
./iothread.d \
./loopbuff.d \
./mysock.d \
./net.d \
./sockmgr.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GNU C++ 编译器'
	g++ -I../../meta_header -O0 -g3 -rdynamic -p -pg -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


