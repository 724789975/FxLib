#define macros

CC = cl
CFLAGS = /c /analyze- /W3 /Zc:wchar_t /Gm- /Zc:inline /fp:precise /D "WIN32" /D "_CONSOLE" /errorReport:prompt /WX- /Zc:forScope /Gd /FC /EHsc /nologo /diagnostics:classic

!IF "$(DEBUG)" == "1"
CFLAGS = $(CFLAGS) /JMC /GS  /ZI /Od /sdl- /D "_DEBUG" /Fd"Debug\vc142.pdb" /D "_MBCS" /RTC1 /Oy- /MTd /Fp"Debug\TestProto.pch"
DIR_OUT = ..\\DEBUG\\
OBJ_OUT = .\\DEBUG
!ELSE
CFLAGS = $(CFLAGS) /GS /GL /Gy /Zi  /O2 /sdl /D "NDEBUG" /Fd"Release\vc142.pdb" /Oy- /Oi /MD /Fp"Release\TestProto.pch"
DIR_OUT = ..\\RELEASE\\
OBJ_OUT = .\\RERLEASE
!ENDIF

TARGET = RedisTest
EXECUTABLE_NAME = $(TARGET).exe
DIR_SRC = .\\
DIR_COMMON = ..\\common
DIR_INCLUDE = \
        /I "../meta_header" \
		/I "../mysql/include" \
		/I "../gflags_win" \
		/I "../Database"
        
DIR_BIN = .\\

LK = link
LKFLAGS = /NOLOGO /MANIFEST:NO
 
!IF "$(DEBUG)" == "0"
LKFLAGS = $(LKFLAGS) /OPT:REF /OPT:ICF
!ENDIF
 
LKFLAGS = $(LKFLAGS) /DEBUG /PDB:"$(DIR_OUT)\$(TARGET).pdb" /OUT:"$(DIR_OUT)\$(EXECUTABLE_NAME)"

LIBDIRS = ..\\DEBUG\\
LIBS = kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ws2_32.lib dbghelp.lib hiredis.lib gflags_static.lib Win32_Interop.lib RedisCon.lib
LINKLIBS = $(LIBDIRS) $(LIBS)

LKFLAGS = $(LKFLAGS) /LIBPATH:$(LINKLIBS)

# build application
target: $(EXECUTABLE_NAME)

###$(EXECUTABLE_NAME) : test.pb.obj proto_dispatcher.obj main.obj
$(EXECUTABLE_NAME) : makeobj
	@echo Linking $(EXECUTABLE_NAME)...
	$(LK) $(LKFLAGS) $(OBJ_OUT)\*.obj $(DIR_COMMON)\*.obj

makeobj:
	@for %%f in (*.cpp) do ( $(CC) $(CFLAGS) /Fo"$(OBJ_OUT)\%%~nf.obj" $(DIR_INCLUDE) %%f )
	@for %%f in ($(DIR_COMMON)\*.cpp) do ( $(CC) $(CFLAGS) /Fo"$(DIR_COMMON)\%%~nf.obj" $(DIR_INCLUDE) %%f )
	@for %%f in (*.cc) do ( $(CC) $(CFLAGS) /Fo"$(OBJ_OUT)\%%~nf.obj" $(DIR_INCLUDE) %%f )

# delete output directories
clean:
 @if exist $(OBJ_OUT) del $(OBJ_OUT)\*.obj
 @if exist $(DIR_OUT) del $(DIR_OUT)$(TARGET)*

# create directories and build application
all: clean target

