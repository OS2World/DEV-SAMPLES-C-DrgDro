#include "dropinfo.h"

/*****************************************************************************/
/* Put message info in listbox item                                          */
/*****************************************************************************/
void PutMsg(HWND hwnd, char *type, MPARAM mp1, MPARAM mp2)
{
  char buf[100];                                 /* Message buffer           */

  sprintf(buf,                                   /* Copy info to buffer      */
          "%s 0x%p, 0x%p",
          type,
          mp1,
          mp2);
  WinSendMsg(hwnd,                               /* Set listbox top index    */
             LM_SETTOPINDEX,
             (MPARAM)WinSendMsg(hwnd,            /* Position of new item     */
                                LM_INSERTITEM,
                                (MPARAM)LIT_END,
                                MPFROMP(buf)),
             NULL);
  return;
}

/*****************************************************************************/
/* Put DRAGINFO structure contents in listbox item                           */
/*****************************************************************************/
void PutDInfo(HWND hwnd, PDRAGINFO pDInfo)
{
  char buf[500];                                 /* Buffer                   */
  sprintf(buf,                                   /* Copy info to buffer      */
          " DRAGINFO: size: %d, %d, ops: %d, hwndSource: %08X, drop coords: (%d, %d), cdItem cnt: %d",
          pDInfo->cbDraginfo,
          pDInfo->cbDragitem,
          pDInfo->usOperation,
          pDInfo->hwndSource,
          pDInfo->xDrop,
          pDInfo->yDrop,
          pDInfo->cditem);

  WinSendMsg(hwnd,                               /* Set listbox top index    */
             LM_SETTOPINDEX,
             (MPARAM)WinSendMsg(hwnd,            /* Position of new item     */
                                LM_INSERTITEM,
                                (MPARAM)LIT_END,
                                MPFROMP(buf)),
             NULL);
  return;
}

/*****************************************************************************/
/* Put DRAGITEM structure contents in listbox item                           */
/*****************************************************************************/
void PutDItem(HWND hwnd, PDRAGITEM pDItem)
{
  char buf[500],                                 /* Buffers                  */
       srceType[100],
       srceContainer[100],
       srceName[100],
       srceRMF[100],
       targetName[100];

  DrgQueryStrName(pDItem->hstrSourceName,        /* Get info from DRAGITEM   */
                  100,
                  srceName);
  DrgQueryStrName(pDItem->hstrContainerName,
                  100,
                  srceContainer);
  DrgQueryStrName(pDItem->hstrType,
                  100,
                  srceType);
  DrgQueryStrName(pDItem->hstrRMF,
                  100,
                  srceRMF);
  DrgQueryStrName(pDItem->hstrTargetName,
                  100,
                  targetName);

  sprintf(buf,                                   /* Copy info to buffer      */
          "   DRAGITEM: ulItemID: %d",
          pDItem->ulItemID);
  WinSendMsg(hwnd,                               /* Set listbox top index    */
             LM_SETTOPINDEX,
             (MPARAM)WinSendMsg(hwnd,            /* Position of new item     */
                                LM_INSERTITEM,
                                (MPARAM)LIT_END,
                                MPFROMP(buf)),
             NULL);

  sprintf(buf,
          "   --Type:      \"%s\"",
          srceType);
  WinSendMsg(hwnd,
             LM_SETTOPINDEX,
             (MPARAM)WinSendMsg(hwnd,
                                LM_INSERTITEM,
                                (MPARAM)LIT_END,
                                MPFROMP(buf)),
             NULL);

  sprintf(buf,
          "   --RMF:       \"%s\"",
          srceRMF);
  WinSendMsg(hwnd,
             LM_SETTOPINDEX,
             (MPARAM)WinSendMsg(hwnd,
                                LM_INSERTITEM,
                                (MPARAM)LIT_END,
                                MPFROMP(buf)),
             NULL);

  sprintf(buf,
          "   --Container: \"%s\"",
          srceContainer);
  WinSendMsg(hwnd,
             LM_SETTOPINDEX,
             (MPARAM)WinSendMsg(hwnd,
                                LM_INSERTITEM,
                                (MPARAM)LIT_END,
                                MPFROMP(buf)),
             NULL);

  sprintf(buf,
          "   --SourceName:      \"%s\"",
          srceName);
  WinSendMsg(hwnd,
             LM_SETTOPINDEX,
             (MPARAM)WinSendMsg(hwnd,
                                LM_INSERTITEM,
                                (MPARAM)LIT_END,
                                MPFROMP(buf)),
             NULL);

  sprintf(buf,
          "   --TargetName:      \"%s\"",
          targetName);
  WinSendMsg(hwnd,
             LM_SETTOPINDEX,
             (MPARAM)WinSendMsg(hwnd,
                                LM_INSERTITEM,
                                (MPARAM)LIT_END,
                                MPFROMP(buf)),
             NULL);

  sprintf(buf,
          "   --Offset: (%d, %d), cntl: %d, ops: %d",
          pDItem->cxOffset,
          pDItem->cyOffset,
          pDItem->fsControl,
          pDItem->fsSupportedOps);
  WinSendMsg(hwnd,
             LM_SETTOPINDEX,
             (MPARAM)WinSendMsg(hwnd,
                                LM_INSERTITEM,
                                (MPARAM)LIT_END,
                                MPFROMP(buf)),
             NULL);
  return;
}
