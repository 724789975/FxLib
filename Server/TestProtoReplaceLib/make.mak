#define macros

CC = cl
CFLAGS = /nologo /c /hotpatch /Gd /GS /W3 /Zc:wchar_t /Gm- /WX- /Zc:inline /fp:precise /Zc:forScope /FC /EHsc /D"_CONSOLE" /D"WIN32" /diagnostics:column

!IFDEF WIN32
PLATFORM_DIR = ""
!ELSE
PLATFORM_DIR = x64\\
!ENDIF

EXECUTABLE_NAME = $(TARGET).exe
STATIC_LIB_NAME = $(TARGET).lib
DYNAMIC_LIB_NAME = $(TARGET).dll
DIR_SRC = .\\
DIR_INCLUDE = \
        /I "../meta_header"\
		/I "../property"\
		/I "../protobuf-3.5.1"

TARGET = TestProtoDLL

!IF "$(DEBUG)" == "1"
DIR_OUT = ..\\$(PLATFORM_DIR)Debug\\
OBJ_OUT = .\\$(PLATFORM_DIR)Debug
CFLAGS = $(CFLAGS) /JMC /ZI /Od /sdl- /RTC1 /Oy- /Od /Ob0 /MTd /D"_DEBUG" /D "_MBCS" /RTC1 /MTd
!ELSE
DIR_OUT = ..\\$(PLATFORM_DIR)Release\\
OBJ_OUT = .\\$(PLATFORM_DIR)Release
CFLAGS = $(CFLAGS) /Zi  /O2 /sdl- /D "NDEBUG" /Oy- /Oi /MT
!ENDIF

CFLAGS = $(CFLAGS) /Fd"$(OBJ_OUT)\$(TARGET)_nmake.pdb" /Fp"$(DIR_OUT)\$(TARGET).pch"
        
LK = link
LKFLAGS = /NOLOGO
 
!IF "$(DEBUG)" == "0"
LKFLAGS = $(LKFLAGS) /OPT:REF /OPT:ICF
!ELSE
LKFLAGS = $(LKFLAGS)
!ENDIF

LKFLAGS = $(LKFLAGS) /PDB:"$(DIR_OUT)$(TARGET).dll.pdb" /ManifestFile:"$(OBJ_OUT)\$(DYNAMIC_LIB_NAME).intermediate.manifest" /OUT:"$(DIR_OUT)$(DYNAMIC_LIB_NAME)" /MANIFEST /NXCOMPAT /DYNAMICBASE /DLL /FUNCTIONPADMIN:5 /INCREMENTAL /SUBSYSTEM:WINDOWS /MANIFESTUAC:NO /TLBID:1

!IFNDEF WIN32
LKFLAGS = $(LKFLAGS) /MACHINE:X64
!ENDIF

target : $(DYNAMIC_LIB_NAME)

$(DYNAMIC_LIB_NAME) : makeobj
	@echo Linking $(DYNAMIC_LIB_NAME)...
	$(LK) $(LKFLAGS) $(OBJ_OUT)\*.obj

makeobj : $(OBJ_OUT)
	@for %%f in (*.cpp) do ( $(CC) $(CFLAGS) /Fo"$(OBJ_OUT)\%%~nf.obj" $(DIR_INCLUDE) %%f )
	@for %%f in (*.cc) do ( $(CC) $(CFLAGS) /Fo"$(OBJ_OUT)\%%~nf.obj" $(DIR_INCLUDE) %%f )


{$(DIR_SRC)}.cpp{$(DIR_BIN)}.obj ::
        @echo $< Compiling...
	$(CC) $(CFLAGS) $(DIR_INCLUDE) $<

$(OBJ_OUT):
	@if not exist $(OBJ_OUT) mkdir $(OBJ_OUT)

# delete output directories
clean:
 	@if exist $(OBJ_OUT) del $(OBJ_OUT)\*.obj
	@if exist $(OBJ_OUT) del $(OBJ_OUT)\*.pdb
	@if exist $(DIR_OUT) del $(DIR_OUT)$(TARGET)*

# create directories and build application
all: clean target
