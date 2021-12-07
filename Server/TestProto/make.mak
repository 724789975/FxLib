#define macros

CC = cl
CFLAGS = /c /analyze- /W3 /Zc:wchar_t /Gm- /Zc:inline /fp:precise /D "WIN32" /D "_CONSOLE" /errorReport:prompt /WX- /Zc:forScope /Gd /FC /EHsc /nologo /diagnostics:classic /hotpatch

TARGET = TestProto
EXECUTABLE_NAME = $(TARGET).exe

!IFDEF WIN32
PLATFORM_DIR = ""
!ELSE
PLATFORM_DIR = x64\\
!ENDIF

!IF "$(DEBUG)" == "1"
DIR_OUT = ..\\$(PLATFORM_DIR)Debug\\
OBJ_OUT = .\\$(PLATFORM_DIR)Debug
COMMON_OUT = ..\\common\\$(PLATFORM_DIR)Debug
CFLAGS = $(CFLAGS) /JMC /GS /ZI /Od /sdl- /D"_DEBUG" /D"_MBCS" /RTC1 /Oy- /MTd
!ELSE
CFLAGS = $(CFLAGS) /GS /GL /Gy /Zi /O2 /sdl- /D"NDEBUG" /Oy- /Oi /MT
DIR_OUT = ..\\$(PLATFORM_DIR)Release\\
OBJ_OUT = .\\$(PLATFORM_DIR)Release
COMMON_OUT = ..\\common\\$(PLATFORM_DIR)Release
!ENDIF

CFLAGS = $(CFLAGS) /Fp"$(DIR_OUT)\$(TARGET).pch" /Fd"$(OBJ_OUT)\$(TARGET)_nmake.pdb"

DIR_SRC = .\\
DIR_COMMON = ..\\common
DIR_INCLUDE = \
        /I "../meta_header" \
		/I "../protobuf-3.5.1" \
		/I "../property"
        
LK = link
LKFLAGS = /NOLOGO /MANIFEST:NO
 
!IF "$(DEBUG)" == "0"
LKFLAGS = $(LKFLAGS) /OPT:REF /OPT:ICF
!ENDIF
 
LKFLAGS = $(LKFLAGS) /DEBUG /PDB:"$(DIR_OUT)$(TARGET).pdb" /ManifestFile:"$(OBJ_OUT)\$(DYNAMIC_LIB_NAME).intermediate.manifest" /OUT:"$(DIR_OUT)\$(EXECUTABLE_NAME)" /FUNCTIONPADMIN:5 /INCREMENTAL

LIBDIRS = ..\\$(PLATFORM_DIR)DEBUG\\
LIBS = kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ws2_32.lib dbghelp.lib Win32_Interop.lib libprotobufd.lib
LINKLIBS = $(LIBDIRS) $(LIBS)

LKFLAGS = $(LKFLAGS) /LIBPATH:$(LINKLIBS)

!IFNDEF WIN32
LKFLAGS = $(LKFLAGS) /MACHINE:X64
!ENDIF

# build application
target : $(EXECUTABLE_NAME)

$(EXECUTABLE_NAME) : makeobj
	@echo Linking $(EXECUTABLE_NAME)...
	$(LK) $(LKFLAGS) $(OBJ_OUT)\*.obj $(COMMON_OUT)\*.obj

makeobj: $(OBJ_OUT) $(COMMON_OUT)
	@for %%f in (*.cpp) do ( $(CC) $(CFLAGS) /Fo"$(OBJ_OUT)\%%~nf.obj" $(DIR_INCLUDE) %%f )
	@for %%f in ($(DIR_COMMON)\*.cpp) do ( $(CC) $(CFLAGS) /Fo"$(COMMON_OUT)\%%~nf.obj" $(DIR_INCLUDE) %%f )
	@for %%f in (*.cc) do ( $(CC) $(CFLAGS) /Fo"$(OBJ_OUT)\%%~nf.obj" $(DIR_INCLUDE) %%f )

# delete output directories
clean:
	@if exist $(OBJ_OUT) del $(OBJ_OUT)\*.obj
	@if exist $(OBJ_OUT) del $(OBJ_OUT)\*.pdb
	@if exist $(COMMON_OUT) del $(COMMON_OUT)\*.obj
	@if exist $(DIR_OUT) del $(DIR_OUT)$(TARGET)*

$(OBJ_OUT):
	@if not exist $(OBJ_OUT) mkdir $(OBJ_OUT)
$(COMMON_OUT):
	@if not exist $(COMMON_OUT) mkdir $(COMMON_OUT)

# create directories and build application
all: clean target

