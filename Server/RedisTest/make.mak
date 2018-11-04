#define macros

CC = cl
CFLAGS = /nologo /c /W4 /Zc:forScope /Zc:wchar_t /EHsc /D"_CONSOLE" /D"WIN32"

!IF "$(DEBUG)" == "1"
CFLAGS = $(CFLAGS) /Od /Ob0 /MTd /ZI /D"_DEBUG"
!ELSE
CFLAGS = $(CFLAGS) /O2 /Ob1 /MT /Zi /D"NDEBUG"
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
DIR_OUT = ..\\DEBUG\\

CFLAGS = $(CFLAGS) /Fo"$(DIR_BIN)\\"  /Fd"$(DIR_OUT)\$(TARGET).pdb"

LK = link
LKFLAGS = /NOLOGO /MANIFEST:NO
 
!IF "$(DEBUG)" == "0"
LKFLAGS = $(LKFLAGS) /OPT:REF /OPT:ICF
!ENDIF
 
LKFLAGS = $(LKFLAGS) /DEBUG /PDB:"$(DIR_OUT)\$(TARGET).pdb" /OUT:"$(DIR_OUT)\$(EXECUTABLE_NAME)"

LIBDIRS = ..\\DEBUG\\
LIBS = kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ws2_32.lib dbghelp.lib hiredis.lib gflags_static.lib Win32_Interop.lib RedisCon.lib common.lib
LINKLIBS = $(LIBDIRS) $(LIBS)

LKFLAGS = $(LKFLAGS) /LIBPATH:$(LINKLIBS)

###{$(DIR_COMMON)}.cpp{$(DIR_BIN)}.obj ::
###        @echo $< Compiling...
###	$(CC) $(CFLAGS) $(DIR_INCLUDE) $<

{$(DIR_SRC)}.cpp{$(DIR_BIN)}.obj ::
        @echo $< Compiling...
	$(CC) $(CFLAGS) $(DIR_INCLUDE) $<

$(EXECUTABLE_NAME) : $(DIR_BIN)\*.obj
	@echo Linking $(EXECUTABLE_NAME)...
	$(LK) $(LKFLAGS)  $(DIR_BIN)\*.obj 

# build application
target: $(EXECUTABLE_NAME)

# delete output directories
clean:
 @if exist $(DIR_OUT) del $(DIR_BIN)*.obj
 @if exist $(DIR_BIN) del $(DIR_OUT)*.exe
 @if exist $(DIR_BIN) del $(DIR_OUT)$(TARGET).pdb

# create directories and build application
all: clean target
