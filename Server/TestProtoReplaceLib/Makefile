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
C_FLAGS			= -g -O0 -rdynamic -Wall -D_DEBUG -DLINUX
CXX_FLAGS 		= -g -O0 -rdynamic -Wall -Woverloaded-virtual -D_DEBUG -DLINUX
else
D = 
C_FLAGS			= -g -rdynamic -Wall -DNDEBUG -DLINUX
CXX_FLAGS 		= -g -rdynamic -Wall -Woverloaded-virtual -DNDEBUG -DLINUX
endif

ARFLAGS			= -rc
#==========================================================

#==========================================================
#  Commands
CODE_DIR = ./
INCLUDE_DIR =./ ../gflags ../meta_header/ ../protobuf-3.5.1 ../property
OUTPUT_DIR =../Debug
OUTPUT = TestProto.so
#==========================================================

OBJS = $(foreach i, $(CODE_DIR), $(shell find $(i) -name "*.cpp"))
COBJS = $(foreach i, $(CODE_DIR), $(shell find $(i) -name "*.cc"))
INCLUDE_FLAG = $(foreach i, $(INCLUDE_DIR), -I$(i))

all:$(OUTPUT)

$(OUTPUT):$(COBJS:.cc=.o) $(OBJS:.cpp=.o)
	@echo Build...$@
	$(CPP) $(CXX_FLAGS) -shared -fpic -o $(OUTPUT_DIR)/$(OUTPUT) $(COBJS:.cc=.o) $(OBJS:.cpp=.o)

%.o: %.cpp
	@echo Compile...$@
	$(CPP) $(CXX_FLAGS) $(INCLUDE_FLAG) -fpic -std=c++11 -c $< -o $@ -MMD -MP -MF$(@:%.o=%.d) 

%.o: %.cc
	@echo Compile...$@
	$(CC) $(C_FLAGS) $(INCLUDE_FLAG) -fpic -std=c++11 -c $< -o $@ -MMD -MP -MF$(@:%.o=%.d)
	
-include $(OBJS:.cpp=.d)
-include $(COBJS:.cc=.d)

clean:
	rm -f $(OUTPUT_DIR)/$(OUTPUT)
	rm -f $(OBJS:.cpp=.o)
	rm -f $(OBJS:.cpp=.d)
	rm -f $(COBJS:.cc=.o)
	rm -f $(COBJS:.cc=.d)
	
	@echo Clean ...done!
