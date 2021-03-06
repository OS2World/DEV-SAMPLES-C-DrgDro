/*********** DROPINFO C Sample Program Include file (.H) *************************/
/*                                                                            */
/*  The DROPINFO include file defines symbolic constants used                    */
/*  in the DROPINFO.C and .RC files.                                             */
/*  DROPINFO local procedure declarations may appear in this file to             */
/*  ensure have been declared before being used.                              */
/*                                                                            */
/******************************************************************************/

#define INCL_WIN
#define INCL_GPI
#define INCL_DOSFILEMGR
#define INCL_WINSTDDRAG
#define INCL_DOSSEMAPHORES
#include <os2.h>                /* PM header file  */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "drgconfg.h"     /* dialog header */

/* general ID's */
#define ID_WINDOW   256
#define ID_DRAGPTR  257
#define ID_EXITPROG 258
#define ID_LISTBOX  259

/* menu ID's */
#define ID_OPTIONS  300
#define IDM_CONFIGDLG    301
#define IDM_CLEARLIST    302
#define IDM_DOSOMETHING  303
#define IDM_DRGDRAG      304
#define IDM_DRGDRAGFILES 305
#define IDM_CHANGEDIR    306

/* internal window messages */
#define DRI_CLOSE          WM_USER + 1
#define DRI_PRINT          WM_USER + 2
#define DRI_THREAD_OK      WM_USER + 3
#define DRI_ERROR_THREAD   WM_USER + 4
#define DRI_CREATE_THREAD  WM_USER + 5
#define DRI_DISCARD        WM_USER + 6
#define DRI_INIT_LIST      WM_USER + 7
#define DRI_GET_CURDIR     WM_USER + 8

/* general defines */
#define THREAD_STACK             (6*4096)

extern HEV    hSemThread;
    
extern HWND   hFileList;
extern HWND   hObject;

extern USHORT usUseSelected;
extern BOOL   bDrgDrag;
extern BOOL   bSourceRender;
extern USHORT usDrgOperation;
extern USHORT usDrgReturn;
extern PSZ    szDrgType;
extern CHAR   szDrgRMF[];
extern CHAR   szPath[];
extern CHAR   szSource[];
extern CHAR   szTarget[];
extern CHAR   szFilePath[];
extern CHAR   szSelDir[];
extern USHORT usDrgControl;
extern USHORT usItemsDragged;
extern PDRAGINFO  pDragInfo;

typedef struct{
  ULONG sz;
  PDRAGINFO pd;
} DISCARD_PARAM, *PDISCARD_PARAM;


void _Optlink ThreadMain( PVOID );
MRESULT EXPENTRY wpConfDrgDrag( HWND, ULONG, MPARAM, MPARAM );
MRESULT EXPENTRY wpConfDrgDragFiles( HWND, ULONG, MPARAM, MPARAM );
MRESULT EXPENTRY delLboxProc( HWND, ULONG, MPARAM, MPARAM );
MRESULT EXPENTRY wpCDDlg( HWND, ULONG, MPARAM, MPARAM );
