# Microsoft Developer Studio Generated NMAKE File, Based on ManejadorPantalla.dsp
!IF "$(CFG)" == ""
CFG=ManejadorPantalla - Win32 Debug
!MESSAGE No configuration specified. Defaulting to ManejadorPantalla - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "ManejadorPantalla - Win32 Release" && "$(CFG)" != "ManejadorPantalla - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "ManejadorPantalla.mak" CFG="ManejadorPantalla - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "ManejadorPantalla - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "ManejadorPantalla - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

!IF  "$(CFG)" == "ManejadorPantalla - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

ALL : "$(OUTDIR)\ManejadorPantalla.dll"


CLEAN :
	-@erase "$(INTDIR)\mp01.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\ManejadorPantalla.dll"
	-@erase "$(OUTDIR)\ManejadorPantalla.exp"
	-@erase "$(OUTDIR)\ManejadorPantalla.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "MANEJADORPANTALLA_EXPORTS" /Fp"$(INTDIR)\ManejadorPantalla.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

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

MTL=midl.exe
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /win32 
RSC=rc.exe
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\ManejadorPantalla.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /incremental:no /pdb:"$(OUTDIR)\ManejadorPantalla.pdb" /machine:I386 /out:"$(OUTDIR)\ManejadorPantalla.dll" /implib:"$(OUTDIR)\ManejadorPantalla.lib" 
LINK32_OBJS= \
	"$(INTDIR)\mp01.obj"

"$(OUTDIR)\ManejadorPantalla.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "ManejadorPantalla - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

ALL : "$(OUTDIR)\OWDCmp.dll"


CLEAN :
	-@erase "$(INTDIR)\mp01.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\OWDCmp.dll"
	-@erase "$(OUTDIR)\OWDCmp.exp"
	-@erase "$(OUTDIR)\OWDCmp.ilk"
	-@erase "$(OUTDIR)\OWDCmp.lib"
	-@erase "$(OUTDIR)\OWDCmp.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /Gz /Zp1 /MLd /W3 /Gm /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "KERNEL" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

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

MTL=midl.exe
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /win32 
RSC=rc.exe
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\ManejadorPantalla.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib c:\e810\OWDC\lib\pdcurses.lib /nologo /dll /incremental:yes /pdb:"$(OUTDIR)\OWDCmp.pdb" /debug /machine:I386 /out:"$(OUTDIR)\OWDCmp.dll" /implib:"$(OUTDIR)\OWDCmp.lib" /pdbtype:sept 
LINK32_OBJS= \
	"$(INTDIR)\mp01.obj"

"$(OUTDIR)\OWDCmp.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 


!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("ManejadorPantalla.dep")
!INCLUDE "ManejadorPantalla.dep"
!ELSE 
!MESSAGE Warning: cannot find "ManejadorPantalla.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "ManejadorPantalla - Win32 Release" || "$(CFG)" == "ManejadorPantalla - Win32 Debug"
SOURCE=.\mp01.cpp

"$(INTDIR)\mp01.obj" : $(SOURCE) "$(INTDIR)"



!ENDIF 

