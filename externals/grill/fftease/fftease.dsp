# Microsoft Developer Studio Project File - Name="fftease" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** NICHT BEARBEITEN **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=fftease - Win32 Debug
!MESSAGE Dies ist kein gültiges Makefile. Zum Erstellen dieses Projekts mit NMAKE
!MESSAGE verwenden Sie den Befehl "Makefile exportieren" und führen Sie den Befehl
!MESSAGE 
!MESSAGE NMAKE /f "fftease.mak".
!MESSAGE 
!MESSAGE Sie können beim Ausführen von NMAKE eine Konfiguration angeben
!MESSAGE durch Definieren des Makros CFG in der Befehlszeile. Zum Beispiel:
!MESSAGE 
!MESSAGE NMAKE /f "fftease.mak" CFG="fftease - Win32 Debug"
!MESSAGE 
!MESSAGE Für die Konfiguration stehen zur Auswahl:
!MESSAGE 
!MESSAGE "fftease - Win32 Release" (basierend auf  "Win32 (x86) Dynamic-Link Library")
!MESSAGE "fftease - Win32 Debug" (basierend auf  "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName "max/fftease"
# PROP Scc_LocalPath "."
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "fftease - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "pd-msvc/r"
# PROP Intermediate_Dir "pd-msvc/r"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /I "c:\programme\audio\pd\src" /I "f:\prog\max\flext\source" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D FLEXT_SYS=2 /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0xc07 /d "NDEBUG"
# ADD RSC /l 0xc07 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winsfftease.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib pd.lib ../flext_sh/pd-msvc/t/flext.lib /nologo /dll /machine:I386 /out:"pd-msvc/fftease.dll" /libpath:"c:\programme\audio\pd\bin" /libpath:"f:\prog\max\flext\pd-msvc"

!ELSEIF  "$(CFG)" == "fftease - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "pd-msvc/d"
# PROP Intermediate_Dir "pd-msvc/d"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /I "c:\programme\audio\pd\src" /I "f:\prog\max\flext\source" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D FLEXT_SYS=2 /FR /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0xc07 /d "_DEBUG"
# ADD RSC /l 0xc07 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winsfftease.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib pd.lib flext_d-pdwin.lib /nologo /dll /debug /machine:I386 /pdbtype:sept /libpath:"c:\programme\audio\pd\bin" /libpath:"f:\prog\max\flext\pd-msvc"

!ENDIF 

# Begin Target

# Name "fftease - Win32 Release"
# Name "fftease - Win32 Debug"
# Begin Group "pv-lib"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\convert.c
# End Source File
# Begin Source File

SOURCE=.\src\convert_new.c
# End Source File
# Begin Source File

SOURCE=.\src\fft.c
# End Source File
# Begin Source File

SOURCE=.\src\fft4.c
# End Source File
# Begin Source File

SOURCE=.\src\fold.c
# End Source File
# Begin Source File

SOURCE=.\src\leanconvert.c
# End Source File
# Begin Source File

SOURCE=.\src\leanunconvert.c
# End Source File
# Begin Source File

SOURCE=.\src\makewindows.c
# End Source File
# Begin Source File

SOURCE=.\src\overlapadd.c
# End Source File
# Begin Source File

SOURCE=.\src\pv.h
# End Source File
# Begin Source File

SOURCE=.\src\unconvert.c
# End Source File
# End Group
# Begin Group "objects"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\burrow~.cpp
# End Source File
# End Group
# Begin Group "doc"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\gpl.txt
# End Source File
# Begin Source File

SOURCE=.\license.txt
# End Source File
# Begin Source File

SOURCE=.\readme.txt
# End Source File
# End Group
# Begin Group "Ori"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\packs\FFTease\c\src\burrow~.c

!IF  "$(CFG)" == "fftease - Win32 Release"

!ELSEIF  "$(CFG)" == "fftease - Win32 Debug"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\packs\FFTease\c\src\cross~.c

!IF  "$(CFG)" == "fftease - Win32 Release"

!ELSEIF  "$(CFG)" == "fftease - Win32 Debug"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\packs\FFTease\c\src\dentist~.c

!IF  "$(CFG)" == "fftease - Win32 Release"

!ELSEIF  "$(CFG)" == "fftease - Win32 Debug"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\packs\FFTease\c\src\disarray~.c

!IF  "$(CFG)" == "fftease - Win32 Release"

!ELSEIF  "$(CFG)" == "fftease - Win32 Debug"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\packs\FFTease\c\src\drown~.c

!IF  "$(CFG)" == "fftease - Win32 Release"

!ELSEIF  "$(CFG)" == "fftease - Win32 Debug"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\packs\FFTease\c\src\ether~.c

!IF  "$(CFG)" == "fftease - Win32 Release"

!ELSEIF  "$(CFG)" == "fftease - Win32 Debug"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\packs\FFTease\c\src\morphine~.c

!IF  "$(CFG)" == "fftease - Win32 Release"

!ELSEIF  "$(CFG)" == "fftease - Win32 Debug"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\packs\FFTease\c\src\pvcompand~.c

!IF  "$(CFG)" == "fftease - Win32 Release"

!ELSEIF  "$(CFG)" == "fftease - Win32 Debug"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\packs\FFTease\c\src\pvoc~.c

!IF  "$(CFG)" == "fftease - Win32 Release"

!ELSEIF  "$(CFG)" == "fftease - Win32 Debug"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\packs\FFTease\c\src\scrape~.c

!IF  "$(CFG)" == "fftease - Win32 Release"

!ELSEIF  "$(CFG)" == "fftease - Win32 Debug"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\packs\FFTease\c\src\shapee~.c

!IF  "$(CFG)" == "fftease - Win32 Release"

!ELSEIF  "$(CFG)" == "fftease - Win32 Debug"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\packs\FFTease\c\src\swinger~.c

!IF  "$(CFG)" == "fftease - Win32 Release"

!ELSEIF  "$(CFG)" == "fftease - Win32 Debug"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\packs\FFTease\c\src\taint~.c

!IF  "$(CFG)" == "fftease - Win32 Release"

!ELSEIF  "$(CFG)" == "fftease - Win32 Debug"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\packs\FFTease\c\src\thresher~.c

!IF  "$(CFG)" == "fftease - Win32 Release"

!ELSEIF  "$(CFG)" == "fftease - Win32 Debug"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\packs\FFTease\c\src\unconvert.c

!IF  "$(CFG)" == "fftease - Win32 Release"

!ELSEIF  "$(CFG)" == "fftease - Win32 Debug"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\packs\FFTease\c\src\vacancy~.c

!IF  "$(CFG)" == "fftease - Win32 Release"

!ELSEIF  "$(CFG)" == "fftease - Win32 Debug"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\packs\FFTease\c\src\xsyn~.c

!IF  "$(CFG)" == "fftease - Win32 Release"

!ELSEIF  "$(CFG)" == "fftease - Win32 Debug"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# End Group
# Begin Source File

SOURCE=.\src\main.cpp
# End Source File
# Begin Source File

SOURCE=.\src\main.h
# End Source File
# End Target
# End Project
