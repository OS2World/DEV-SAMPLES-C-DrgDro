/*****************************************************************************/
/*                                                                           */
/*  PROGRAM NAME:  DROPINFO                                                  */
/*                                                                           */
/*  PURPOSE:       This program provides information for testing and         */
/*                 debugging drag-and-drop applications.  When an object is  */
/*                 dropped over this program's window, the window shows the  */
/*                 information received from the source.                     */
/*                                                                           */
/*****************************************************************************/

#include "dropinfo.h"                            /* Application header file  */

/*****************************************************************************/
/* Window procedure prototypes                                               */
/*****************************************************************************/
MRESULT EXPENTRY MyWindowProc( HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2 );
MRESULT EXPENTRY wpSubList( HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2 );

/*****************************************************************************/
/* Function prototypes                                                       */
/*****************************************************************************/
VOID SendDiscardMsg( HWND hwnd, PDRAGITEM pDItem );
VOID ProcessDrop( HWND hwnd, PDRAGINFO pDInfo );
MRESULT ProcessRendercomplete( MPARAM mp1, MPARAM mp2 );

/*****************************************************************************/
/* Global data                                                               */
/*****************************************************************************/
HAB  hAB;                                        /* Anchor block handle      */
HWND  hListBox;                                  /* Listbox window handle    */
PFNWP pwpList;                                   /* Listbox winproc address  */

/*****************************************************************************/
/* Application main routine                                                  */
/*****************************************************************************/
INT main (VOID)
{
  HMQ  hMsgQ;                                    /* Message queue handle     */
  QMSG qMsg;                                     /* Message structure        */
  HWND hFrame;                                   /* Frame window handle      */

  ULONG flCreate = FCF_STANDARD;

  ULONG rc;                                      /* Return code              */

  hAB = WinInitialize(0);                        /* Register application     */

  hMsgQ = WinCreateMsgQueue(hAB, 0);             /* Create message queue     */

  rc = WinRegisterClass(hAB,                     /* Register window class    */
                        (PSZ)"MyWindow",         /* Class name               */
                        (PFNWP)MyWindowProc,     /* Window procedure address */
                        CS_SIZEREDRAW,           /* Class style              */
                        sizeof(PVOID));          /* Window words             */

  hFrame = WinCreateStdWindow(HWND_DESKTOP,      /* Desktop is parent        */
                              0,                 /* Standard window style    */
                              &flCreate,         /* Frame control flags      */
                              "MyWindow",        /* Window class name        */
                              "DropInfo Sample", /* Window title text        */
                              0,                 /* No special class style   */
                              (HMODULE)0L,       /* Resources in EXE file    */
                              ID_WINDOW,         /* Frame window identifier  */
                              NULL);             /* No pres params           */

  while (WinGetMsg(hAB, &qMsg, 0L, 0, 0))        /* Process messages until   */
        WinDispatchMsg(hAB, &qMsg);              /* WM_QUIT received         */

  WinDestroyWindow(hFrame);                      /* Destroy window           */
  WinDestroyMsgQueue(hMsgQ);                     /* Destroy message queue    */
  WinTerminate(hAB);                             /* Deregister application   */
}

/*****************************************************************************/
/* Main window procedure                                                     */
/*****************************************************************************/
MRESULT EXPENTRY MyWindowProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
  SWP   swp;
  ULONG rc;

  HWND  hFrame;

  switch (msg)
  {
  case WM_CREATE:
    hListBox = WinCreateWindow(hwnd, WC_LISTBOX, NULL,
                               WS_VISIBLE | LS_NOADJUSTPOS | LS_HORZSCROLL,
                               0, 0, 0, 0,
                               hwnd, HWND_TOP, ID_LISTBOX, 0, 0);

    pwpList = WinSubclassWindow(hListBox,
                 wpSubList);

    hFrame = WinQueryWindow(hwnd,
                            QW_PARENT);
    rc = WinSetWindowPos(hFrame, HWND_TOP, 300, 30, 350, 220,
                         SWP_SIZE | SWP_ACTIVATE | SWP_SHOW);
    break;

  case WM_SIZE:
    WinQueryWindowPos(hwnd, &swp);
    hListBox=WinWindowFromID(hwnd,
                             ID_LISTBOX);
    WinSetWindowPos(hListBox, HWND_TOP, swp.x, swp.y, swp.cx, swp.cy,
                    SWP_SIZE | SWP_SHOW);
    break;

  case WM_COMMAND:
    if( SHORT1FROMMP(mp2) == CMDSRC_MENU )
    {
      switch( SHORT1FROMMP(mp1) )
      {
      case IDM_RESPONSE:
        /*WinDlgBox(HWND_DESKTOP, hwnd, wpConfDrgDragFiles, NULLHANDLE,
                  IDD_RESPONSE, NULL);*/
        return (MRESULT)TRUE;
    
      case IDM_CLEARLIST:
        WinSendMsg( hListBox, LM_DELETEALL, (MPARAM)0, (MPARAM)0 );
        return((MRESULT)TRUE);
    
      case IDM_EXIT:
        WinPostMsg( hwnd, WM_QUIT, (MPARAM)0, (MPARAM)0 );
        return((MRESULT)TRUE);
      }
    }
  default:
       return(WinDefWindowProc(hwnd, msg, mp1, mp2));
  }
  return ((MRESULT)FALSE);
}

/*****************************************************************************/
/* Listbox subclass window procedure                                         */
/*****************************************************************************/
MRESULT EXPENTRY wpSubList(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
  PDRAGINFO pDInfo;                              /* Pointer to DRAGINFO      */
  PDRAGITEM pDItem;                              /* Pointer to DRAGITEM      */
  ULONG rc;                                      /* Return code              */
  int i;                                         /* Loop counter             */

  switch (msg)
  {
  case DM_DRAGOVER:
    pDInfo = (PDRAGINFO)mp1;
    rc = DrgAccessDraginfo(pDInfo);
    pDItem = DrgQueryDragitemPtr(pDInfo, 0);
    return(MPFROM2SHORT(DOR_DROP,      /* Drop Allowed             */
                        DO_UNKNOWN));

  case DM_DROP:                           /* Object being dropped     */
    PutMsg(hwnd,"DM_DROP",mp1,mp2);    /* Put message in listbox   */
    pDInfo = (PDRAGINFO)mp1;           /* Extract DRAGINFO pointer */
    rc =DrgAccessDraginfo(pDInfo);     /* Access DRAGINFO data     */
    PutDInfo(hwnd, pDInfo);            /* Put DRAGINFO in listbox  */

    pDItem = DrgQueryDragitemPtr(pDInfo, 0); /* Get DRAGITEMs      */

    for (i=0; i < pDInfo->cditem; i++) /* For each DRAGITEM        */
    {
      /*SendRenderMsg( pDInfo->hwndSource, pDItem );*/
      PutDItem(hwnd, pDItem++);      /* Put DRAGITEM in listbox  */
    }
    /*ProcessDrop( hwnd, pDInfo );*/
    DrgFreeDraginfo( pDInfo );
    break;

  case DM_RENDERCOMPLETE:
    /*PutMsg(hwnd,"DM_RENDERCOMPLETE",mp1,mp2);*/
    return ProcessRendercomplete( mp1, mp2 );

  default:                                
    return((MRESULT)pwpList(hwnd, msg, mp1, mp2));
  }
  return (MRESULT)NULL;
}

/********************************************/
/* send a fake message to source for test purposes */
/********************************************/
VOID SendRenderMsg( HWND hwndSource, PDRAGITEM pDItem )
{
  PRINTDEST pDest;

  WinSendMsg( hwndSource, DM_PRINTOBJECT, pDItem, (MPARAM)&pDest );
}

/******************************************************************/
/*  do the DM_RENDER conversation, possibly with DM_RENDERPREPARE */
/******************************************************************/
VOID ProcessDrop( HWND hwnd, PDRAGINFO pDInfo )
{
  PDRAGITEM pDItem;  
  PDRAGTRANSFER pdxfer;
  MRESULT rc;                        

  pDItem = DrgQueryDragitemPtr(pDInfo, 0);           

  pdxfer = DrgAllocDragtransfer(1);          
  pdxfer->cb = sizeof(DRAGTRANSFER);         
  pdxfer->hwndClient = hwnd;
  pdxfer->pditem = pDItem;
  pdxfer->hstrSelectedRMF =
          DrgAddStrHandle("<DRM_TEST,DRF_TEXT>");
  pdxfer->hstrRenderToName =
          DrgAddStrHandle("TESTEST");
  pdxfer->ulTargetInfo = 0;
  pdxfer->usOperation = DO_COPY;

  rc=DrgSendTransferMsg(pDInfo->hwndSource, DM_RENDER, 
                        (MPARAM)pdxfer, NULL);
  /*if( rc == TRUE ) ... */

  rc=DrgSendTransferMsg(pDInfo->hwndSource, DM_ENDCONVERSATION,
                        MPFROMLONG(pDItem->ulItemID), 
                        MPFROMSHORT(DMFL_TARGETFAIL));

  DrgFreeDragtransfer(pdxfer);  
  return;            
}

MRESULT ProcessRendercomplete( MPARAM mp1, MPARAM mp2 )
{
  return (MRESULT)0;
}

