/**************************************************************
* This code creates an object window that will perform all 
* processing in response to a drag operations on thread one
**************************************************************/
#define INCL_DEV
#include "draginfo.h"
#include "pmassert.h"

CHAR *szObjectClass = "DRI_OBJWIN";

typedef struct{
  USHORT  unSz;
  HWND    hwnd;
} CREAT_PARM, *PCREAT_PARM;

#define MAKE_CREAT_PARM( pc, hw )\
   (pc) = (PCREAT_PARM)malloc( sizeof(CREAT_PARM) );\
    if( pc ) { (pc)->unSz = sizeof(CREAT_PARM); (pc)->hwnd = (hw); }


MRESULT EXPENTRY ObjectWinProc( HWND, ULONG, MPARAM, MPARAM );
MRESULT PrintObjects( PDRAGITEM, PPRINTDEST );
VOID DiscardObjects( PDRAGINFO pDragInfo );
VOID InitFileList( void );

static HAB hab;
HWND       hObject;

/*************************************************************
* create object window
**************************************************************/
VOID _Optlink ThreadMain( PVOID pvHwnd )
{
  BOOL       bOK;
  HMQ        hmq;
  QMSG       qmsg;
  PCREAT_PARM  pCParm;

  hab = WinInitialize( 0 );
  pmassert( hab, hab );
  hmq = WinCreateMsgQueue( hab, 0 );
  pmassert( hab, hmq );
  
  bOK = WinRegisterClass( hab, szObjectClass, ObjectWinProc, 
                          0L, sizeof( PCREAT_PARM ) );
  pmassert( hab, bOK );

  MAKE_CREAT_PARM( pCParm, (HWND)pvHwnd );
  pmassert( hab, pCParm );
  
  hObject = WinCreateWindow( HWND_OBJECT, szObjectClass, "",
                             0L, 0L, 0L, 0L, 0L, HWND_OBJECT,
                             HWND_BOTTOM, 0L, pCParm, NULL);
  pmassert( hab, hObject );

  while( WinGetMsg ( hab, &qmsg, (HWND)NULLHANDLE, 0L, 0L ))
     WinDispatchMsg ( hab, &qmsg );

  /* clean up */
  free( pCParm );
  bOK = WinDestroyWindow( hObject );
  bOK = WinDestroyMsgQueue( hmq );
  bOK = WinTerminate( hab );
  
  /* tell thread one we're done */
  DosPostEventSem( hSemThread );    
}  /*  threadmain  */


/************************************************/
/*this is the window proc for the object window */
/************************************************/
MRESULT EXPENTRY 
ObjectWinProc( HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2 )
{
  switch( msg )
  {
  case WM_CREATE:
    {
    /* post an acknowledgement and pass object handle */
    HWND hMain = ((PCREAT_PARM)PVOIDFROMMP(mp1))->hwnd;
    WinPostMsg( hMain, DRI_THREAD_OK, MPFROMHWND(hwnd), 0L );
    /* fall through */
    }
  case DRI_INIT_LIST:
    InitFileList();
    WinPostMsg( hwnd, DRI_GET_CURDIR, 0L, 0L );
    return (MRESULT)FALSE;

  case DRI_GET_CURDIR:
    {
    ULONG ulDrv, ulTmp;
    PCHAR pc;
    DosQueryCurrentDisk( &ulDrv, &ulTmp );
    szSelDir[0] = (CHAR)64+(CHAR)ulDrv;
    DosQueryCurrentDir( ulDrv, szSelDir+3, &ulTmp );
    pc = szSelDir + strlen( szSelDir ) - 1;
    if( *pc != '\\' ) strcat( szSelDir, "\\" );
    break;
    }
    return (MRESULT)FALSE;

  case DRI_CLOSE:
    /* post quit msg to queue */
    WinPostMsg( hwnd, WM_QUIT, 0L, 0L );
    break;

  case DRI_PRINT:
    /* perform print operation */
    PrintObjects( (PDRAGITEM)mp1, (PPRINTDEST)mp2 );
    break;

  case DRI_DISCARD:
    if( usItemsDragged )
      DiscardObjects( (PDRAGINFO)mp1 );
    break;

  default:
    return WinDefWindowProc( hwnd, msg, mp1, mp2 );
  }

  return (MRESULT)NULL;
}


/*****************************************************************/
/*this does the printing in response to a DRI_PRINT message */
/*****************************************************************/
MRESULT PrintObjects( PDRAGITEM pDragItem, PPRINTDEST ppd )
{
  CHAR    szFullFileName[ CCHMAXPATH + 1 ];
  PSZ     pszFileName;
  INT     cbFileName = sizeof( szFullFileName );
  CHAR    achMsg[120];
 
  DrgQueryStrName( pDragItem->hstrContainerName,
                   cbFileName, szFullFileName );

  pszFileName = szFullFileName + strlen( szFullFileName );

  if( *szFullFileName && *(pszFileName - 1) != '\\' )
  {
    strcpy(pszFileName, "\\");
    pszFileName++;
  }

  DrgQueryStrName( pDragItem->hstrTargetName,
                   cbFileName - (pszFileName - szFullFileName),
                   pszFileName );

  sprintf( achMsg, "File %s would be printed on %s ", szFullFileName,
           ppd->pszPrinter );
  WinMessageBox(HWND_DESKTOP, HWND_DESKTOP, achMsg,
                  "Printing", 0, MB_OK );

  if( !(--usItemsDragged) )
    DrgFreeDragInfo( pDragInfo );

  return (MRESULT) DRR_SOURCE;
}


/***************************************************/
/* this function handles DM_DISCARDOBJECT requests */
/***************************************************/
VOID DiscardObjects( PDRAGINFO pDInfo )
{
  DISCARD_PARAM param;
  
  param.sz = sizeof(DISCARD_PARAM);
  param.pd = pDInfo;
  
  /* DM_DISCARDOBJECT always sends a full DragInfo, so we can
   * cancel the operation after this one.
   */
  usItemsDragged = 0;
  WinDlgBox( HWND_DESKTOP, HWND_DESKTOP, delLboxProc, NULLHANDLE,
             IDD_DISCARD, &param );
             
  DrgFreeDraginfo( pDInfo );

  return;
}


/**********************************/
/* fill 'Files' listbox with data */
/**********************************/
VOID InitFileList( void )
{
  HDIR          hDir = HDIR_SYSTEM;
  FILEFINDBUF3  FindBuffer;
  ULONG         ulCount=1;
  APIRET        rc;         /* Return code */
  
  rc = DosFindFirst( "*.*", &hDir, 0, (PVOID) &FindBuffer,
                    sizeof(FindBuffer), &ulCount, FIL_STANDARD);
 
  WinSendMsg( hFileList, LM_DELETEALL, (MPARAM)0, (MPARAM)0 );
  while( !rc )
  {
    WinSendMsg( hFileList, LM_INSERTITEM, (MPARAM)LIT_END,
                MPFROMP(FindBuffer.achName));
    rc = DosFindNext( hDir, &FindBuffer, sizeof(FindBuffer), &ulCount );
  }
  return;
}
