# Microsoft Developer Studio Project File - Name="encmid" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=encmid - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "encmid.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "encmid.mak" CFG="encmid - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "encmid - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "encmid - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "encmid - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /W3 /GX /O2 /I "." /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /FD /c
# SUBTRACT CPP /YX /Yc /Yu
# ADD BASE RSC /l 0x804 /d "NDEBUG"
# ADD RSC /l 0x804 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386

!ELSEIF  "$(CFG)" == "encmid - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /I "." /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /FD /GZ /c
# SUBTRACT CPP /YX /Yc /Yu
# ADD BASE RSC /l 0x804 /d "_DEBUG"
# ADD RSC /l 0x804 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept

!ENDIF 

# Begin Target

# Name "encmid - Win32 Release"
# Name "encmid - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Group "ucl"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\ucl\alloc.c
# End Source File
# Begin Source File

SOURCE=.\ucl\fake16.h
# End Source File
# Begin Source File

SOURCE=.\ucl\getbit.h
# End Source File
# Begin Source File

SOURCE=.\ucl\internal.h
# End Source File
# Begin Source File

SOURCE=.\ucl\io.c
# End Source File
# Begin Source File

SOURCE=.\ucl\n2_99.ch
# End Source File
# Begin Source File

SOURCE=.\ucl\n2b_99.c
# End Source File
# Begin Source File

SOURCE=.\ucl\n2b_d.c
# End Source File
# Begin Source File

SOURCE=.\ucl\n2b_ds.c
# End Source File
# Begin Source File

SOURCE=.\ucl\n2b_to.c
# End Source File
# Begin Source File

SOURCE=.\ucl\n2d_99.c
# End Source File
# Begin Source File

SOURCE=.\ucl\n2d_d.c
# End Source File
# Begin Source File

SOURCE=.\ucl\n2d_ds.c
# End Source File
# Begin Source File

SOURCE=.\ucl\n2d_to.c
# End Source File
# Begin Source File

SOURCE=.\ucl\n2e_99.c
# End Source File
# Begin Source File

SOURCE=.\ucl\n2e_d.c
# End Source File
# Begin Source File

SOURCE=.\ucl\n2e_ds.c
# End Source File
# Begin Source File

SOURCE=.\ucl\n2e_to.c
# End Source File
# Begin Source File

SOURCE=.\ucl\ucl.h
# End Source File
# Begin Source File

SOURCE=.\ucl\ucl_conf.h
# End Source File
# Begin Source File

SOURCE=.\ucl\ucl_crc.c
# End Source File
# Begin Source File

SOURCE=.\ucl\ucl_dll.c
# End Source File
# Begin Source File

SOURCE=.\ucl\ucl_init.c
# End Source File
# Begin Source File

SOURCE=.\ucl\ucl_ptr.c
# End Source File
# Begin Source File

SOURCE=.\ucl\ucl_ptr.h
# End Source File
# Begin Source File

SOURCE=.\ucl\ucl_str.c
# End Source File
# Begin Source File

SOURCE=.\ucl\ucl_util.c
# End Source File
# Begin Source File

SOURCE=.\ucl\ucl_util.h
# End Source File
# Begin Source File

SOURCE=.\ucl\uclconf.h
# End Source File
# Begin Source File

SOURCE=.\ucl\uclutil.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\encmid.cpp
# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# Begin Source File

SOURCE=.\ReadMe.txt
# End Source File
# End Target
# End Project
