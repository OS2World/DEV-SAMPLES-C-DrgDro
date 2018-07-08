# IBM Developer's Workframe/2 Make File Creation run at 19:34:12 on 03/20/93

# Make File Creation run in directory:
#   D:\QUELLEN\C2\DRAGINFO;

.SUFFIXES:

.SUFFIXES: .c .rc

ALL: DRAGINFO.EXE \
     DRAGINFO.RES

draginfo.exe:  \
  DRAGDLG.OBJ \
  DRAGINFO.OBJ \
  DRAGINFO.RES \
  DRAGOBJ.OBJ \
  draginfo.DEF \
  DRAGINFO.MAK
   @REM @<<DRAGINFO.@0
     /CO /BAS:0x10000 /PM:PM +
     DRAGDLG.OBJ +
     DRAGINFO.OBJ +
     DRAGOBJ.OBJ
     draginfo.exe
     
     
     draginfo.DEF;
<<
   LINK386.EXE @DRAGINFO.@0
   RC DRAGINFO.RES draginfo.exe

{.}.rc.res:
   RC -r .\$*.RC

{.}.c.obj:
   ICC.EXE /Ti /W2 /Gm /C .\$*.c

!include DRAGINFO.DEP
