#define macros

CC = cl
CFLAGS = /nologo /c /GS /JMC /Gd /W3 /Zc:forScope /Zc:wchar_t /EHsc /D"_CONSOLE" /D"WIN32"

!IF "$(DEBUG)" == "1"
CFLAGS = $(CFLAGS) /Od /Ob0 /MTd /ZI /D"_DEBUG"
!ELSE
CFLAGS = $(CFLAGS) /O2 /Ob1 /MT /Zi /D"NDEBUG"
!ENDIF

TARGET = Net
EXECUTABLE_NAME = $(TARGET).exe
STATIC_LIB_NAME = $(TARGET).lib
DIR_SRC = .\\
DIR_INCLUDE = \
        /I "../meta_header"
        
DIR_BIN = .\\
DIR_OUT = ..\\DEBUG\\

CFLAGS = $(CFLAGS) /Fo"$(DIR_BIN)\\"  /Fd"$(DIR_OUT)\$(TARGET).pdb"

LK = link
LKFLAGS = /NOLOGO
 
!IF "$(DEBUG)" == "0"
LKFLAGS = $(LKFLAGS) /OPT:REF /OPT:ICF
!ELSE
LKFLAGS = $(LKFLAGS)
!ENDIF

LKFLAGS = $(LKFLAGS) /OUT:"$(DIR_OUT)\$(STATIC_LIB_NAME)"

LIBDIRS = .\libs
LIBS = *.lib
LINKLIBS = $(LIBDIRS) $(LIBS)

LKFLAGS = $(LKFLAGS) /LIBPATH:$(LINKLIBS)

{$(DIR_SRC)}.cpp{$(DIR_BIN)}.obj ::
        @echo $< Compiling...
	$(CC) $(CFLAGS) $(DIR_INCLUDE) $<

$(STATIC_LIB_NAME) : $(DIR_BIN)\*.obj
	@echo Linking $(STATIC_LIB_NAME)...
	$(LK) /lib $(LKFLAGS) $(DIR_BIN)\*.obj 

# build application
target: $(STATIC_LIB_NAME)

# delete output directories
clean:
 @if exist $(DIR_OUT) del $(DIR_BIN)*.obj
 @if exist $(DIR_BIN) del $(DIR_OUT)$(STATIC_LIB_NAME)
 @if exist $(DIR_BIN) del $(DIR_OUT)$(TARGET).pdb

# create directories and build application
all: clean target
