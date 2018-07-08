# IBM Developer's Workframe/2 Make File Creation run at 00:54:40 on 02/22/93

# Make File Creation run in directory:
#   D:\QUELLEN\C2\DROPINFO;

.SUFFIXES:

.SUFFIXES: .c .rc

ALL: DROPINFO.EXE \
     DROPINFO.RES

DROPINFO.EXE:  \
  DROPINFO.OBJ \
  DROPINFO.RES \
  PUTMSG.OBJ \
  DROPINFO.DEF \
  DROPINFO.MAK
   @REM @<<DROPINFO.@0
     /BAS:0x10000 /PM:PM +
     DROPINFO.OBJ +
     PUTMSG.OBJ
     DROPINFO.EXE
     
     
     DROPINFO.DEF;
<<
   LINK386.EXE @DROPINFO.@0
   RC DROPINFO.RES DROPINFO.EXE

{.}.rc.res:
   RC -r .\$*.RC

{.}.c.obj:
   ICC.EXE /Kbapr /Ti /W2 /C .\$*.c

!include DROPINFO.DEP
