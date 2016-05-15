################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C++_DEPS)),)
-include $(C++_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
ifneq ($(strip $(CC_DEPS)),)
-include $(CC_DEPS)
endif
ifneq ($(strip $(CPP_DEPS)),)
-include $(CPP_DEPS)
endif
ifneq ($(strip $(CXX_DEPS)),)
-include $(CXX_DEPS)
endif
ifneq ($(strip $(C_UPPER_DEPS)),)
-include $(C_UPPER_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 

# All Target
all: TestServer

dependents:
	-cd /home/dengxiaobo/workspace/FxLib/LuaLib/Release && $(MAKE) all
	-cd /home/dengxiaobo/workspace/FxLib/Net/Release && $(MAKE) all
	-cd /home/dengxiaobo/workspace/FxLib/Database/Release && $(MAKE) all

# Tool invocations
TestServer: $(OBJS) $(USER_OBJS) /home/dengxiaobo/workspace/FxLib/LuaLib/Release/libLuaLib.a /home/dengxiaobo/workspace/FxLib/Net/Release/libNet.a /home/dengxiaobo/workspace/FxLib/Database/Release/libDatabase.a
	@echo 'Building target: $@'
	@echo 'Invoking: GNU C++ 链接器'
	g++ -L"/home/dengxiaobo/workspace/FxLib/Database/Release" -L"/home/dengxiaobo/workspace/FxLib/Net/Release" -L"/home/dengxiaobo/workspace/FxLib/LuaLib/Release" -L../../Database/Debug -L../../LuaLib/Debug -L../../Net/Debug -L../../mysql/lib/linux -Lcrypto -o "TestServer" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(OBJS)$(C++_DEPS)$(C_DEPS)$(CC_DEPS)$(CPP_DEPS)$(EXECUTABLES)$(CXX_DEPS)$(C_UPPER_DEPS) TestServer
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY:
/home/dengxiaobo/workspace/FxLib/LuaLib/Release/libLuaLib.a:
/home/dengxiaobo/workspace/FxLib/Net/Release/libNet.a:
/home/dengxiaobo/workspace/FxLib/Database/Release/libDatabase.a:

-include ../makefile.targets
