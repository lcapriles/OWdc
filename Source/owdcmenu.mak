# Microsoft Developer Studio Generated NMAKE File, Based on owdcmenu.dsp
!IF "$(CFG)" == ""
CFG=owdcmenu - Win32 Debug
!MESSAGE No configuration specified. Defaulting to owdcmenu - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "owdcmenu - Win32 Release" && "$(CFG)" != "owdcmenu - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "owdcmenu.mak" CFG="owdcmenu - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "owdcmenu - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "owdcmenu - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

!IF  "$(CFG)" == "owdcmenu - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

ALL : "$(OUTDIR)\owdcmenu.exe"


CLEAN :
	-@erase "$(INTDIR)\owdcmenu.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\owdcmenu.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /ML /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /Fp"$(INTDIR)\owdcmenu.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

.c{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

RSC=rc.exe
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\owdcmenu.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /incremental:no /pdb:"$(OUTDIR)\owdcmenu.pdb" /machine:I386 /out:"$(OUTDIR)\owdcmenu.exe" 
LINK32_OBJS= \
	"$(INTDIR)\owdcmenu.obj"

"$(OUTDIR)\owdcmenu.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "owdcmenu - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

ALL : "$(OUTDIR)\owdcmenu.exe"


CLEAN :
	-@erase "$(INTDIR)\owdcmenu.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\owdcmenu.exe"
	-@erase "$(OUTDIR)\owdcmenu.ilk"
	-@erase "$(OUTDIR)\owdcmenu.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /Gz /Zp1 /MLd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /D "KERNEL" /Fp"$(INTDIR)\owdcmenu.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

.c{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

RSC=rc.exe
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\owdcmenu.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib c:\e810\system\lib32\jdekrnl.lib c:\e810\system\lib32\jdeunicode.lib c:\e810\system\lib32\jdel.lib "C:\Documents and Settings\LuisCapriles\Mis documentos\_Proyectos\Dev\OW Data Capture\owdcmenUnicode\Debug\pdcurses.lib" "C:\Documents and Settings\LuisCapriles\Mis documentos\_Proyectos\Dev\OW Data Capture\owdcmenUnicode\Debug\OWDC.lib" /nologo /subsystem:console /incremental:yes /pdb:"$(OUTDIR)\owdcmenu.pdb" /debug /machine:I386 /out:"$(OUTDIR)\owdcmenu.exe" /pdbtype:sept 
LINK32_OBJS= \
	"$(INTDIR)\owdcmenu.obj"

"$(OUTDIR)\owdcmenu.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 


!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("owdcmenu.dep")
!INCLUDE "owdcmenu.dep"
!ELSE 
!MESSAGE Warning: cannot find "owdcmenu.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "owdcmenu - Win32 Release" || "$(CFG)" == "owdcmenu - Win32 Debug"
SOURCE=.\owdcmenu.cpp

"$(INTDIR)\owdcmenu.obj" : $(SOURCE) "$(INTDIR)"



!ENDIF 

