################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../SocketSession.cpp \
../main.cpp 

OBJS += \
./SocketSession.o \
./main.o 

CPP_DEPS += \
./SocketSession.d \
./main.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GNU C++ 编译器'
	g++ -I../../Net -I../../lua -I../../tolua -I../../LuaLib -I../../Net -I../../meta_header -O0 -g3 -rdynamic -p -pg -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


