################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../dbclient.cpp \
../dbconnection.cpp \
../dbmodule.cpp \
../dbreader.cpp \
../dbstmt.cpp \
../fxdb.cpp 

OBJS += \
./dbclient.o \
./dbconnection.o \
./dbmodule.o \
./dbreader.o \
./dbstmt.o \
./fxdb.o 

CPP_DEPS += \
./dbclient.d \
./dbconnection.d \
./dbmodule.d \
./dbreader.d \
./dbstmt.d \
./fxdb.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -I../../mysql/include -I../../meta_header -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


