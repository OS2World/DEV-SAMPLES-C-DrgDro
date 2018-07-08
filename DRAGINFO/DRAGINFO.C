/*****************************************************************************/
/*                                                                           */
/*  PROGRAM NAME:  DRAGINFO                                                  */
/*                                                                           */
/*  PURPOSE:       This program provides information for testing and         */
/*                 debugging drag-and-drop applications.  When an object is  */
/*                 dragged from this program's window, the window shows the  */
/*                 messages received from the target.                        */
/*                 Additionally, the Draginfo and Dragitem-structures that   */
/*                 are used to specify the requested operation can be        */
/*                 configured on screen.It is also possible to select the    */
/*                 desired Drag-API (DrgDrag or DrgDragFiles).               */
/*                 This program only simulates the dragging; no explicit     */
/*                 rendering is performed (yet).                             */
/*                                                                           */
/*                 This program was created using code from the DROPINFO     */
/*                 sample program that comes with the Redbook Vol.4 sample   */
/*                 code. Both programs complement eachother.                 */
/*                                                                           */
/*                 As far as I'm concerned you may use this code in whatever */
/*                 way you choose, provided I'm in no way responsible for    */
/*                 the outcome. Good luck.                                   */
/*                                                                           */
/*                 Christian Sell, CIS 100021,3151                   */
/*****************************************************************************/

#define INCL_DOSFILEMGR   
#include "draginfo.h"                            /* Application header file  */
#include "pmassert.h"

/*****************************************************************************/
/* Window procedure prototypes                                               */
/*****************************************************************************/
MRESULT EXPENTRY MyWindowProc( HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2 );
MRESULT EXPENTRY wpFileList( HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2 );

/*****************************************************************************/
/* Function prototypes                                                       */
/*****************************************************************************/
void PutMsg( USHORT unSrc, ULONG msg, MPARAM mp1, MPARAM mp2);
HWND DoDrgDrag( HWND hwndSrc );
void DoDragFiles( HWND hwnd );

/*****************************************************************************/
/* Global data                                                               */
/*****************************************************************************/
HAB  hAB;                       /* Anchor block handle      */
PFNWP pwpFList;                 /* Listbox winproc address  */
HWND  hwndTarget;
HWND  hFileList;                /* Listbox window handle    */
HWND  hMsgList;                 /* Listbox window handle    */
HPOINTER  hptrDrag;             /* Drag Pointer handle */
HEV   hSemThread;               /* Semaphore to signal thread is running */
PDRAGINFO     pDragInfo;
USHORT usItemsDragged;
BOOL   bDragActive;


/*****************************************************************************/
/* Application main routine                                                  */
/*****************************************************************************/
INT main (VOID)
{
  HMQ  hMsgQ;                                    /* Message queue handle     */
  QMSG qMsg;                                     /* Message structure        */
  HWND hFrame;                                   /* Frame window handle      */

  ULONG flCreate = FCF_STANDARD;     /* Frame creation flags     */

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
                              "DragInfo",        /* Window title text        */
                              0,                 /* No special class style   */
                              (HMODULE)0L,       /* Resources in EXE file    */
                              ID_WINDOW,         /* Frame window identifier  */
                              NULL);             /* No pres params           */

  /* we need to insure that the second thread is terminated before the
   * main thread. Create a semaphore to handle that.
   */
  rc = DosCreateEventSem( NULL, &hSemThread, 0L, 0L );
  pmassert( hAB, !rc );
  
  while (WinGetMsg(hAB, &qMsg, 0L, 0, 0))        /* Process messages until   */
        WinDispatchMsg(hAB, &qMsg);              /* WM_QUIT received         */

  WinDestroyWindow(hFrame);                      /* Destroy window           */
  WinDestroyMsgQueue(hMsgQ);                     /* Destroy message queue    */
  WinTerminate(hAB);                             /* Deregister application   */
  
  return (0);
}

/*****************************************************************************/
/* Main window procedure                                                     */
/*****************************************************************************/
MRESULT EXPENTRY MyWindowProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
  SWP   swp;
  RECTL Rect;
  ULONG rc;
  LONG  dx;
  HWND  hFrame, hwndMenu;

  hFrame = WinQueryWindow(hwnd, QW_PARENT);
  switch (msg)
  {
  case WM_CREATE:
    /* create & subclass 'Files' listbox */
    hFileList = WinCreateWindow(hwnd, WC_LISTBOX, NULL,
                               WS_VISIBLE | LS_NOADJUSTPOS | LS_HORZSCROLL |
                               LS_MULTIPLESEL,
                               0, 0, 0, 0,
                               hwnd, HWND_TOP, ID_LISTBOX, 0, 0);

    pwpFList = WinSubclassWindow(hFileList,
                 wpFileList);

    /* create & subclass 'Messages' listbox */
    hMsgList = WinCreateWindow(hwnd, WC_LISTBOX, NULL,
                               WS_VISIBLE | LS_NOADJUSTPOS | LS_HORZSCROLL,
                               0, 0, 0, 0,
                               hwnd, HWND_TOP, ID_LISTBOX, 0, 0);


    /* set window position */
    rc = WinSetWindowPos(hFrame, HWND_TOP, 50, 250, 400, 200,
                         SWP_MOVE | SWP_SIZE | SWP_ACTIVATE | SWP_SHOW);
    hptrDrag = WinLoadPointer( HWND_DESKTOP, NULLHANDLE, ID_DRAGPTR );

    WinPostMsg( hFileList, DRI_CREATE_THREAD, 0L, 0L );
    break;
       
  case WM_SIZE:
    WinQueryWindowRect( hwnd, &Rect );
    WinQueryWindowPos(hwnd, &swp);
    dx = swp.cx*2/3;
    WinSetWindowPos(hFileList, HWND_TOP,
                    0, 0, 
                    swp.cx-dx, swp.cy,
                    SWP_SIZE | SWP_SHOW);
    WinSetWindowPos(hMsgList, HWND_TOP,
                    Rect.xLeft+(swp.cx-dx), Rect.yBottom,
                    dx, swp.cy,
                    SWP_MOVE | SWP_SIZE | SWP_SHOW );
    break;
       
  /* just record drag messages sent to client window */
  case DM_DROP:
  case DM_DRAGOVER:
  case DM_DRAGLEAVE:
  case DM_DROPHELP:
  case DM_ENDCONVERSATION:
  case DM_PRINT:
  case DM_RENDER:
  case DM_RENDERCOMPLETE:
  case DM_RENDERPREPARE:
  case DM_DRAGFILECOMPLETE:
  case DM_EMPHASIZETARGET:
  case DM_DRAGERROR:
  case DM_FILERENDERED:
  case DM_RENDERFILE:
  case DM_DRAGOVERNOTIFY:
  case DM_PRINTOBJECT:
  case DM_DISCARDOBJECT:
    PutMsg( 0, msg, mp1, mp2 );
    return (MRESULT)FALSE;

  case WM_MENUSELECT:
    switch( SHORT1FROMMP(mp1) )
    {
    case ID_OPTIONS:
      WinCheckMenuItem(HWNDFROMMP(mp2), IDM_DRGDRAG, bDrgDrag);
      WinCheckMenuItem(HWNDFROMMP(mp2), IDM_DRGDRAGFILES, !bDrgDrag);
      return (MRESULT)FALSE;

    default:
      return(WinDefWindowProc(hwnd, msg, mp1, mp2));
    }
  case WM_COMMAND:
    if( SHORT1FROMMP(mp2) == CMDSRC_MENU )
    {
      hwndMenu = WinWindowFromID(hFrame, FID_MENU);
      switch( SHORT1FROMMP(mp1) )
      {
      case IDM_CHANGEDIR:
        WinDlgBox(HWND_DESKTOP, hwnd, wpCDDlg, NULLHANDLE, IDD_CDIR, NULL);
        return (MRESULT)FALSE;

      case IDM_DRGDRAG:
        bDrgDrag = TRUE;
        WinCheckMenuItem(hwndMenu, IDM_DRGDRAG, bDrgDrag);
        WinCheckMenuItem(hwndMenu, IDM_DRGDRAGFILES, !bDrgDrag);
        return (MRESULT)FALSE;
  
      case IDM_DRGDRAGFILES:
        bDrgDrag = FALSE;
        WinMessageBox(HWND_DESKTOP, hwnd, "Use DrgDragFiles at your own risk!!",
                       "Warning", 0, MB_OK );
        WinCheckMenuItem(hwndMenu, IDM_DRGDRAG, bDrgDrag);
        WinCheckMenuItem(hwndMenu, IDM_DRGDRAGFILES, !bDrgDrag);
        return (MRESULT)FALSE;
  
      case IDM_CONFIGDLG:
        if( bDrgDrag == TRUE )
          WinDlgBox(HWND_DESKTOP, hwnd, wpConfDrgDrag, NULLHANDLE,
                    IDD_CONFIG1, NULL);
        else
          WinDlgBox(HWND_DESKTOP, hwnd, wpConfDrgDragFiles, NULLHANDLE,
                    IDD_CONFIG2, NULL);
        return (MRESULT)TRUE;
    
      case IDM_CLEARLIST:
        WinSendMsg( hMsgList, LM_DELETEALL, (MPARAM)0, (MPARAM)0 );
        return((MRESULT)TRUE);
    
      case IDM_DOSOMETHING:
        if( WinMessageBox(HWND_DESKTOP, hwnd, "OK, so do something else!",
                       "Message", 0, MB_OKCANCEL ) == MBID_OK )
          WinPostMsg( hwnd, WM_QUIT, (MPARAM)0, (MPARAM)0 );
        return((MRESULT)TRUE);
      }
    }
    break;

  default:
    return(WinDefWindowProc(hwnd, msg, mp1, mp2));
  }
  return((MRESULT)FALSE);
}

/*****************************************************************************/
/* Listbox subclass window procedure                                         */
/*****************************************************************************/
MRESULT EXPENTRY wpFileList(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
  HWND    hObject;
  BOOL    bOK;
  APIRET  rc;

  /* get object object window data from owner's window words */
  hObject = (HWND) WinQueryWindowULong( WinQueryWindow(hwnd, QW_OWNER), 
                                        QWL_USER );
                                      
  switch (msg)
  {
  case DRI_INIT_LIST:
    if( hObject )
      WinPostMsg( hObject, DRI_INIT_LIST, 0L, 0L );
    return (MRESULT)FALSE;

  case DRI_CREATE_THREAD:
    if( !_beginthread( ThreadMain, NULL, THREAD_STACK, (PVOID)hwnd ) )
      WinMessageBox(HWND_DESKTOP, hwnd, "error creating thread",
                         "Main", 0, MB_OK|MB_ERROR );
    break;
  case DRI_ERROR_THREAD:
    WinMessageBox(HWND_DESKTOP, hwnd, "error in thread",
                       "Main", 0, MB_OK|MB_ERROR );
    break;
  case DRI_THREAD_OK:
    /* store object window handle in owners window words */
    bOK = WinSetWindowULong( WinQueryWindow(hwnd, QW_OWNER), QWL_USER, 
                             (ULONG) mp1 );                  
    pmassert( hAB, bOK );
    break;
  case WM_DESTROY:
    pmassert( hAB, hObject );
    if( hObject )
    {
      /* tell thread 2 to close and wait until closed */
      WinPostMsg( hObject, DRI_CLOSE, 0L, 0L ); 
      rc = DosWaitEventSem( hSemThread, 3000L );
      pmassert( hAB, !rc );
    }
    break;
  /***********************/
  /*Drag-Initialisierung */
  /***********************/
  case WM_BEGINDRAG:
    if( bDrgDrag == TRUE )
    {
      hwndTarget = DoDrgDrag( hwnd );
      return (MRESULT)(hwndTarget ? TRUE : FALSE);
    }
    else
    {
      DoDragFiles( hwnd );
      return (MRESULT)TRUE;
    }
  /****************************************/
  /* messages to drag source window       */
  /****************************************/
  case DM_DRAGOVERNOTIFY:   /* Objekt ber anderem window */
                            /* mp1: Draginfo, mp2: Target indicators */
    /*PutMsg( 1, msg, mp1, mp2 );*/
    return (MRESULT)FALSE;

  case DM_RENDER:           /* Render-Anforderung */
    /* mp1: DragTransfer, return: success/error */
    if( !(--usItemsDragged) )
      DrgFreeDraginfo( pDragInfo );
    PutMsg( 1, msg, mp1, mp2 );
    return (MRESULT)FALSE;

  case DM_PRINT:           /* old print msg - what parameters?? */
    if( usItemsDragged )
    {
      DrgFreeDraginfo( pDragInfo );
      usItemsDragged = 0;
    }
    PutMsg( 1, msg, mp1, mp2 );
    return (MRESULT)FALSE;

  case DM_PRINTOBJECT:      /* Print-Anforderung */
    /* mp1: Draginfo, mp2: PrintDest, returns: action */
    PutMsg( 1, msg, mp1, mp2 );
    if( usDrgReturn == DRR_SOURCE && hObject )
      WinPostMsg( hObject, DRI_PRINT, mp1, mp2 );
    return (MRESULT)usDrgReturn;      

  case DM_DISCARDOBJECT:    /* Discard-Anforderung */
    /* mp1: Draginfo, mp2: reserved, returns: action */
    PutMsg( 1, msg, mp1, mp2 );
    if( usDrgReturn == DRR_SOURCE && hObject )
      WinPostMsg( hObject, DRI_DISCARD, mp1, mp2 );
    return (MRESULT)usDrgReturn;  

  case DM_ENDCONVERSATION:  /* Ende-Meldung */
    /* mp1: ItemId, mp2: success/fail */ 
    PutMsg( 1, msg, mp1, mp2 );
    return (MRESULT)FALSE;

  case DM_RENDERFILE: /* DrgDragFiles Render message */
    PutMsg( 1, msg, mp1, mp2 );
    DrgSendTransferMsg( ((PRENDERFILE)mp1)->hwndDragFiles, DM_FILERENDERED, mp1, 
                        (MPARAM)TRUE );
    return (MRESULT)TRUE;

  /*****************************/
  /* messages to target window */
  /*****************************/
  case DM_DRAGOVER:
    /* mp1: Draginfo, mp2: Koordinaten, return: indicator */
    PutMsg( 1, msg, mp1, mp2 );
    return(MPFROM2SHORT(DOR_NEVERDROP, DO_UNKNOWN));
  case DM_EMPHASIZETARGET:
    /* mp1: Koordinaten, mp2: Flag */
  case DM_DRAGLEAVE:
    /* mp1: Draginfo */
  case DM_DROP:
    /* mp1: Draginfo, return: indicator */

  case DM_DRAGERROR:        /* messages generated by DrgDragFiles protocol */
  case DM_FILERENDERED:
    /* mp1: Renderfile, mp2: success/fail */
  case DM_DRAGFILECOMPLETE:
    PutMsg( 1, msg, mp1, mp2 );
    return (MRESULT)FALSE;
  } /* endswitch */

  return((MRESULT)pwpFList(hwnd, msg, mp1, mp2));
}


/*************************************************************************/
/* fhis function performs the drag ...                                   */
/*************************************************************************/
HWND DoDrgDrag( HWND hwndSrc )
{
  CHAR          achFile[50];
  DRAGITEM      DItem;
  DRAGIMAGE     DImage;
  HWND          hDrop=NULLHANDLE;
  APIRET        rc;
  SHORT         sRet, asSel[50] = {LIT_NONE};
  USHORT        unCount;

  if( usUseSelected )
  {
    WinPostMsg( hObject, DRI_GET_CURDIR, 0L, 0L );
    for( unCount=0, sRet=LIT_FIRST; unCount<49; unCount++ )
    {
      sRet = (USHORT)WinSendMsg( hFileList, LM_QUERYSELECTION,
                                 MPFROMSHORT(sRet), 0L );
      asSel[unCount] = sRet;
      if( sRet == LIT_NONE )
        break;
    }
    if( unCount )
    {
      usItemsDragged = unCount;
      pDragInfo = DrgAllocDraginfo(unCount);  /* Allocate DRAGINFO     */
      pDragInfo->usOperation = usDrgOperation;
    }
    for( unCount=0; asSel[unCount] != LIT_NONE; unCount++ )
    {
      WinQueryLboxItemText( hFileList, asSel[unCount], achFile, 50 );
      DItem.hwndItem            = hwndSrc;         
      DItem.ulItemID            = (ULONG)asSel[unCount];
      DItem.hstrType            = DrgAddStrHandle( szDrgType );
      DItem.hstrRMF             = DrgAddStrHandle( szDrgRMF );
      DItem.hstrContainerName   = DrgAddStrHandle( szSelDir );
      DItem.hstrSourceName      = DrgAddStrHandle( achFile );
      DItem.hstrTargetName      = DrgAddStrHandle( achFile );
      DItem.fsControl           = usDrgControl;
      DItem.cxOffset            = 2;
      DItem.cyOffset            = 2;
      DItem.fsSupportedOps      = DO_COPYABLE|DO_MOVEABLE|DO_LINKABLE;
     
      rc = DrgSetDragitem(pDragInfo, &DItem, sizeof(DItem), unCount);
    }
  }
  else
  {
    usItemsDragged = 1;
    pDragInfo = DrgAllocDraginfo(1);  /* Allocate DRAGINFO     */
    pDragInfo->usOperation = usDrgOperation;
  
    DItem.hwndItem            = hwndSrc;         
    DItem.ulItemID            = (ULONG)55;       /* nothing special */
    DItem.hstrType            = DrgAddStrHandle( szDrgType );
    DItem.hstrRMF             = DrgAddStrHandle( szDrgRMF );
    DItem.hstrContainerName   = DrgAddStrHandle( szPath );
    DItem.hstrSourceName      = DrgAddStrHandle( szSource );
    DItem.hstrTargetName      = DrgAddStrHandle( szTarget );
    DItem.fsControl           = usDrgControl;
    DItem.cxOffset            = 2;
    DItem.cyOffset            = 2;
    DItem.fsSupportedOps      = DO_COPYABLE|DO_MOVEABLE|DO_LINKABLE;
   
    rc = DrgSetDragitem(pDragInfo, &DItem, sizeof(DItem), 0);
  }

  DImage.cb = sizeof(DRAGIMAGE); /* Initialize DRAGIMAGE  */
  DImage.cptl = 0;               /* Not a polygon         */
  DImage.hImage = hptrDrag;      /* Icon handle for drag  */
  DImage.fl = DRG_ICON | DRG_TRANSPARENT;
  DImage.cxOffset = 0;           /* No hotspot            */
  DImage.cyOffset = 0;
 
  /*****************************/
  /* now do the drag           */
  /*****************************/
  hDrop = DrgDrag(hwndSrc,          /* Initiate drag         */
             pDragInfo,             /* DRAGINFO structure    */
             &DImage,            /* DRAGIMAGE structure  */
             1,                  /* Only one DRAGIMAGE    */
             VK_ENDDRAG,         /* End of drag indicator */
             NULL);              /* Reserved              */
  if( hDrop )
    bDragActive = TRUE;

  return hDrop;
}

/**********************************************************/
/*this performs the drag via the DrgDragFiles API         */
/**********************************************************/
void DoDragFiles( HWND hwnd )
{
  CHAR      achFile[CCHMAXPATHCOMP];
  PCHAR     pcName;
  PSZ       apFiles[50];   /* no more than 50 files */
  USHORT    usCount, usLen;
  SHORT     sRet;

  if( usUseSelected )
  {
    strcpy( achFile, szSelDir );
    usLen = strlen( achFile );
    pcName = achFile + usLen;
    usLen = sizeof( achFile ) - usLen;
    for( usCount=0, sRet=LIT_FIRST; usCount<50; )
    {
      sRet = (USHORT)WinSendMsg( hFileList, LM_QUERYSELECTION, MPFROMSHORT(sRet), 0L );
      if( sRet != LIT_END )
      {
        WinQueryLboxItemText( hFileList, sRet, pcName, usLen );
        apFiles[usCount] = strdup( achFile );
        usCount++;
      }
      else
        break;
    }
    apFiles[usCount] = NULL;
  }
  else
  {
    usCount = 1;
    apFiles[0] = szFilePath;
  }

  if( !DrgDragFiles(hwnd, apFiles, NULL, NULL, usCount,
                   hptrDrag, VK_ENDDRAG, bSourceRender, 0L) )
    PutMsg( 2, (ULONG)"DrgDragFiles failed", NULL, NULL );
  else
    PutMsg( 2, (ULONG)"DrgDragFiles OK", NULL, NULL );

  if( usUseSelected )
    for( usCount=0; apFiles[usCount]; usCount++ )
      free( apFiles[usCount] );
}


/*****************************************************************************/
/* Put message info in listbox item                                          */
/*****************************************************************************/
void PutMsg( USHORT unSrc, ULONG msg, MPARAM mp1, MPARAM mp2)
{
  char   buf[120];                 /* Message buffer */
  char  *pc;

#define MAKE_MSG(c)\
  sprintf( buf+strlen(buf), "%s, %p, %p", c, mp1, mp2 )

  switch( unSrc )
  {
  case 0:
    strcpy( buf, "msg (client win): " );
    break;
  case 1:
    strcpy( buf, "msg (listbox): " );
    break;
  default:
    strcpy( buf, "general: " );
    strcat( buf, (char *)msg );
  }

  switch( msg )
  {
  case DM_DROP:
    MAKE_MSG( "DM_DROP" );
    break;
  case DM_DRAGOVER:
    MAKE_MSG( "DM_DRAGOVER" );
    break;
  case DM_DRAGLEAVE:
    MAKE_MSG( "DM_DRAGLEAVE" );
    break;
  case DM_DROPHELP:
    MAKE_MSG( "DM_DROPHELP" );
    break;
  case DM_ENDCONVERSATION:
    switch((ULONG)mp2)
    {
    case DMFL_TARGETSUCCESSFUL:
      pc = "DMFL_TARGETSUCCESSFUL"; break;
    case DMFL_TARGETFAIL:
      pc = "DMFL_TARGETFAIL"; break;
    default:
      pc = "unknown"; break;
    }
    sprintf( buf+strlen(buf), "DM_ENDCONVERSATION, %lu, %s", (ULONG)mp1,pc);
    break;
  case DM_PRINT:
    MAKE_MSG( "DM_PRINT" );
    break;
  case DM_RENDER:
    MAKE_MSG( "DM_RENDER" );
    break;
  case DM_RENDERCOMPLETE:
    MAKE_MSG( "DM_RENDERCOMPLETE" );
    break;
  case DM_RENDERPREPARE:
    MAKE_MSG( "DM_RENDERPREPARE" );
    break;
  case DM_DRAGFILECOMPLETE:
    MAKE_MSG( "DM_DRAGFILECOMPLETE" );
    break;
  case DM_EMPHASIZETARGET:
    MAKE_MSG( "DM_EMPHASIZETARGET" );
    break;
  case DM_DRAGERROR:
    MAKE_MSG( "DM_DRAGERROR" );
    break;
  case DM_FILERENDERED:
    MAKE_MSG( "DM_FILERENDERED" );
    break;
  case DM_RENDERFILE:
    MAKE_MSG( "DM_RENDERFILE" );
    break;
  case DM_DRAGOVERNOTIFY:
    MAKE_MSG( "DM_DRAGOVERNOTIFY" );
    break;
  case DM_PRINTOBJECT:
    MAKE_MSG( "DM_PRINTOBJECT" );
    break;
  case DM_DISCARDOBJECT:
    MAKE_MSG( "DM_DISCARDOBJECT" );
    break;
  }
  WinSendMsg( hMsgList, LM_INSERTITEM, (MPARAM)LIT_END, MPFROMP(buf));

  return;
}


