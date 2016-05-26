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
C_LFLAGS		= -g -Wall -D_DEBUG -DLINUX
CXX_FLAGS 		= -g -Wall -Woverloaded-virtual -D_DEBUG -DLINUX 
else
D = 
C_LFLAGS		= -g -Wall -DNDEBUG -DLINUX
CXX_FLAGS 		= -g -Wall -Woverloaded-virtual -DNDEBUG -DLINUX
endif

ARFLAGS			= -rc
#==========================================================

#==========================================================
#  Commands
CODE_DIR = ./
INCLUDE_DIR = ./ ../meta_header/ ../mysql/include/
LIB_FILE = -lpthread
OUTPUT_DIR =../Debug
OUTPUT = libDatabase.a
#==========================================================

OBJS = $(foreach i, $(CODE_DIR), $(shell find $(i) -name "*.cpp"))
INCLUDE_FLAG = $(foreach i, $(INCLUDE_DIR), -I$(i))

all:$(OUTPUT) $(OBJS:.cpp=.d)

$(OUTPUT):$(OBJS:.cpp=.o)
	@echo Build...$@
	@$(AR) $(ARFLAGS) $(OUTPUT_DIR)/$(OUTPUT) $(OBJS:.cpp=.o)
	@$(RANLIB) $(OUTPUT_DIR)/$(OUTPUT)

%.o: %.cpp
	@echo Compile...$@
	@echo File: $(OBJS)
	@echo File: $(INCLUDE_FLAG)
	@$(CPP) $(CXX_FLAGS) $(INCLUDE_FLAG) -c $< -o $@ -MMD -MP -MF$(@:%.o=%.d) 
	
clean:
	rm -f $(OUTPUT_DIR)/$(OUTPUT)
	rm -f $(OBJS:.cpp=.o)
	rm -f $(OBJS:.cpp=.d)
	
	@echo Clean ...done!
