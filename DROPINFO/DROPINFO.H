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

#define INCL_WINSTDDRAG
#include <os2.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define ID_WINDOW   256
#define ID_EXITPROG 257
#define ID_LISTBOX  300

#define ID_OPTIONS     258
#define IDM_RESPONSE   259
#define IDM_CLEARLIST  260
#define IDM_EXIT       261

void PutMsg(HWND hwnd, char *type, MPARAM mp1, MPARAM mp2);
void PutDInfo(HWND hwnd, PDRAGINFO pdinfo);
void PutDItem(HWND hwnd, PDRAGITEM pditem);
