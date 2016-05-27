################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
O_SRCS += \
/home/dengxiaobo/workspace/FxLib/common/fxmeta.o \
/home/dengxiaobo/workspace/FxLib/common/fxtimer.o \
/home/dengxiaobo/workspace/FxLib/common/log_thread.o \
/home/dengxiaobo/workspace/FxLib/common/lua_engine.o \
/home/dengxiaobo/workspace/FxLib/common/thread.o 

CPP_SRCS += \
/home/dengxiaobo/workspace/FxLib/common/fxmeta.cpp \
/home/dengxiaobo/workspace/FxLib/common/fxtimer.cpp \
/home/dengxiaobo/workspace/FxLib/common/log_thread.cpp \
/home/dengxiaobo/workspace/FxLib/common/lua_engine.cpp \
/home/dengxiaobo/workspace/FxLib/common/thread.cpp 

OBJS += \
./common/fxmeta.o \
./common/fxtimer.o \
./common/log_thread.o \
./common/lua_engine.o \
./common/thread.o 

CPP_DEPS += \
./common/fxmeta.d \
./common/fxtimer.d \
./common/log_thread.d \
./common/lua_engine.d \
./common/thread.d 


# Each subdirectory must supply rules for building sources it contributes
common/fxmeta.o: /home/dengxiaobo/workspace/FxLib/common/fxmeta.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GNU C++ 编译器'
	g++ -I../../lua -I../../tolua -I../../LuaLib -I../../meta_header -O0 -g3 -rdynamic -p -pg -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

common/fxtimer.o: /home/dengxiaobo/workspace/FxLib/common/fxtimer.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GNU C++ 编译器'
	g++ -I../../lua -I../../tolua -I../../LuaLib -I../../meta_header -O0 -g3 -rdynamic -p -pg -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

common/log_thread.o: /home/dengxiaobo/workspace/FxLib/common/log_thread.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GNU C++ 编译器'
	g++ -I../../lua -I../../tolua -I../../LuaLib -I../../meta_header -O0 -g3 -rdynamic -p -pg -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

common/lua_engine.o: /home/dengxiaobo/workspace/FxLib/common/lua_engine.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GNU C++ 编译器'
	g++ -I../../lua -I../../tolua -I../../LuaLib -I../../meta_header -O0 -g3 -rdynamic -p -pg -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

common/thread.o: /home/dengxiaobo/workspace/FxLib/common/thread.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GNU C++ 编译器'
	g++ -I../../lua -I../../tolua -I../../LuaLib -I../../meta_header -O0 -g3 -rdynamic -p -pg -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


