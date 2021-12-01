#define macros

CC = cl
CFLAGS = /nologo /c /JMC /hotpatch /GS /Gd /W3 /Zc:wchar_t /ZI /Gm- /WX- /Zc:inline /fp:precise /Zc:forScope /RTC1 /Oy- /FC /EHsc /D"_CONSOLE" /D"WIN32" /diagnostics:column


#/JMC /permissive- /hotpatch /GS /analyze- /W3 /Zc:wchar_t   /sdl /Fd"Debug\vc142.pdb" /D "WIN32" /D "TESTPROTOREPLACELIB_EXPORTS" /D "_WINDOWS" /D "_USRDLL" /D "_WINDLL" /D "_UNICODE" /D "UNICODE" /errorReport:prompt /WX- /Zc:forScope /RTC1 /Gd /Oy- /MDd /FC /Fa"Debug\" /EHsc /nologo /Fo"Debug\" /Fp"Debug\TestProtoReplaceLib.pch" /diagnostics:column

#/JMC /permissive- /GS /W3 /Zc:wchar_t /ZI /Gm- /Od /sdl /Fd"x64\Debug\vc142.pdb" /Zc:inline /fp:precise /D "_DEBUG" /D "TESTPROTOREPLACELIB_EXPORTS" /D "_WINDOWS" /D "_USRDLL" /D "_WINDLL" /D "_UNICODE" /D "UNICODE" /errorReport:prompt /WX- /Zc:forScope /RTC1 /Gd /MDd /FC /Fa"x64\Debug\" /EHsc /nologo /Fo"x64\Debug\" /Fp"x64\Debug\TestProtoReplaceLib.pch" /diagnostics:column /JMC /permissive- /hotpatch /GS /analyze- /W3 /Zc:wchar_t /I"../protobuf-3.5.1" /I"../meta_header" /I"../property" /ZI /Gm- /Od /sdl /Fd"Debug\vc142.pdb" /Zc:inline /fp:precise /D "WIN32" /D "_DEBUG" /D "TESTPROTOREPLACELIB_EXPORTS" /D "_WINDOWS" /D "_USRDLL" /D "_WINDLL" /D "_UNICODE" /D "UNICODE" /errorReport:prompt /WX- /Zc:forScope /RTC1 /Gd /Oy- /MDd /FC /Fa"Debug\" /EHsc /nologo /Fo"Debug\" /Fp"Debug\TestProtoReplaceLib.pch" /diagnostics:column 

#/OUT:"C:\Users\72478\Desktop\FxLib\Server\Debug\TestProtoReplaceLib.dll" /MANIFEST /NXCOMPAT /PDB:"C:\Users\72478\Desktop\FxLib\Server\Debug\TestProtoReplaceLib.pdb" /DYNAMICBASE "kernel32.lib" "user32.lib" "gdi32.lib" "winspool.lib" "comdlg32.lib" "advapi32.lib" "shell32.lib" "ole32.lib" "oleaut32.lib" "uuid.lib" "odbc32.lib" "odbccp32.lib" /IMPLIB:"C:\Users\72478\Desktop\FxLib\Server\Debug\TestProtoReplaceLib.lib" /DEBUG /DLL /MACHINE:X86 /INCREMENTAL /PGD:"C:\Users\72478\Desktop\FxLib\Server\Debug\TestProtoReplaceLib.pgd" /SUBSYSTEM:WINDOWS /MANIFESTUAC:NO /ManifestFile:"Debug\TestProtoReplaceLib.dll.intermediate.manifest" /ERRORREPORT:PROMPT /NOLOGO /TLBID:1 

!IF "$(DEBUG)" == "1"
CFLAGS = $(CFLAGS) /Od /Ob0 /MTd /ZI /D"_DEBUG"
!ELSE
CFLAGS = $(CFLAGS) /O2 /Ob1 /MT /Zi /D"NDEBUG"
!ENDIF

TARGET = common
EXECUTABLE_NAME = $(TARGET).exe
STATIC_LIB_NAME = $(TARGET).lib
DIR_SRC = .\\
DIR_INCLUDE = \
        /I "../meta_header"\
		/I "../property"\
		/I "../protobuf-3.5.1"
        
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
