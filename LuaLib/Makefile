#==========================================================#
#===============    Makefile v1.0    ======================#
#==========================================================#

#==========================================================
#  Commands
CC		= gcc
CPP		= g++
AR		= ar
RANLIB	= ranlib

#  Flags
ifeq ($(BUILD),DEBUG)
D = d
C_FLAGS			= -g3 -O2 -rdynamic -Wall -D_DEBUG -DLINUX
CXX_FLAGS 		= -g3 -O2 -rdynamic -Wall -Woverloaded-virtual -D_DEBUG -DLINUX 
else
D = 
C_FLAGS			= -g -Wall -DNDEBUG -DLINUX
CXX_FLAGS 		= -g -Wall -Woverloaded-virtual -DNDEBUG -DLINUX
endif

ARFLAGS			= -rc
#==========================================================

#==========================================================
#  Commands
CODE_DIR = ./
INCLUDE_DIR = ./ ../lua/ ../tolua/
LIB_FILE =
OUTPUT_DIR =../Debug
OUTPUT = libLuaLib.a
#==========================================================

OBJS = $(foreach i, $(CODE_DIR), $(shell find $(i) -name "*.c"))
INCLUDE_FLAG = $(foreach i, $(INCLUDE_DIR), -I$(i))

all:$(OUTPUT)

$(OUTPUT):$(OBJS:.c=.o)
	@echo Build...$@
	$(AR) $(ARFLAGS) $(OUTPUT_DIR)/$(OUTPUT) $(OBJS:.c=.o)
	@$(RANLIB) $(OUTPUT_DIR)/$(OUTPUT)

%.o: %.c
#	@echo Compile...$@
	$(CC) $(C_FLAGS) $(INCLUDE_FLAG) -c $< -o $@ -MMD -MP -MF$(@:%.o=%.d)

-include $(OBJS:.c=.d)

clean:
	rm -f $(OUTPUT_DIR)/$(OUTPUT)
	rm -f $(OBJS:.c=.o)
	rm -f $(OBJS:.c=.d)
	
	@echo Clean ...done!
