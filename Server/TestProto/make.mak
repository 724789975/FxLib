#define macros

CC = cl
CFLAGS = /nologo /c /W4 /Zc:forScope /Zc:wchar_t /EHsc /D"_CONSOLE" /D"WIN32"

!IF "$(DEBUG)" == "1"
CFLAGS = $(CFLAGS) /Od /Ob0 /MTd /ZI /D"_DEBUG"
!ELSE
CFLAGS = $(CFLAGS) /O2 /Ob1 /MT /Zi /D"NDEBUG"
!ENDIF

TARGET = TestProto
EXECUTABLE_NAME = $(TARGET).exe
DIR_SRC = .\\
DIR_COMMON = ..\\common
DIR_INCLUDE = \
        /I "../meta_header" \
		/I "../protobuf-3.5.1" \
		/I "../property"
        
DIR_BIN = .\\
DIR_OUT = ..\\DEBUG\\

CFLAGS = $(CFLAGS) /Fo"$(DIR_BIN)\\"  /Fd"$(DIR_OUT)\$(TARGET).pdb"

LK = link
LKFLAGS = /NOLOGO /MANIFEST:NO
 
!IF "$(DEBUG)" == "0"
LKFLAGS = $(LKFLAGS) /OPT:REF /OPT:ICF
!ENDIF
 
LKFLAGS = $(LKFLAGS) /DEBUG /PDB:"$(DIR_OUT)\$(TARGET).pdb" /OUT:"$(DIR_OUT)\$(EXECUTABLE_NAME)"

LIBDIRS = ..\\DEBUG\\
LIBS = kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ws2_32.lib dbghelp.lib Win32_Interop.lib libprotobufd.lib common.lib
LINKLIBS = $(LIBDIRS) $(LIBS)

LKFLAGS = $(LKFLAGS) /LIBPATH:$(LINKLIBS)

# build application
target: $(EXECUTABLE_NAME)

###$(EXECUTABLE_NAME) : test.pb.obj proto_dispatcher.obj main.obj
$(EXECUTABLE_NAME) : makeobj
	@echo Linking $(EXECUTABLE_NAME)...
	$(LK) $(LKFLAGS) $(DIR_BIN)\*.obj

makeobj:
	@for %%f in (*.cpp) do ( $(CC) $(CFLAGS) $(DIR_INCLUDE) %%f )
	@for %%f in (*.cc) do ( $(CC) $(CFLAGS) $(DIR_INCLUDE) %%f )

# delete output directories
clean:
	@if exist $(DIR_OUT) del $(DIR_BIN)*.obj
	@if exist $(DIR_BIN) del $(DIR_OUT)*.exe
	@if exist $(DIR_BIN) del $(DIR_OUT)$(TARGET).pdb

# create directories and build application
all: clean target

