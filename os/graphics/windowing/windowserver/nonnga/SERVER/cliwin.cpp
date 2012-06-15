// Copyright (c) 1995-2009 Nokia Corporation and/or its subsidiary(-ies).
// All rights reserved.
// This component and the accompanying materials are made available
// under the terms of "Eclipse Public License v1.0"
// which accompanies this distribution, and is available
// at the URL "http://www.eclipse.org/legal/epl-v10.html".
//
// Initial Contributors:
// Nokia Corporation - initial contribution.
//
// Contributors:
//
// Description:
// Client window functions
// 
//

#include "W32CLICK.H"
#include "server.h"
#include "cliwin.h"
#include "gc.h"
#include "rootwin.h"
#include "windowgroup.h"
#include "walkwindowtree.h"
#include "ScrDev.H"
#include "wstop.h"
#include "EVQUEUE.H"
#include "KEYCLICK.H"
#include "panics.h"
#include "password.h"
#include "pointer.h"
#include "EVENT.H"
#include "backedupwindow.h"
#include "redrawmsgwindow.h"

TBool CWsClientWindow::iAbsoluteFading = EFalse;

const TPoint corner1[1]={TPoint(1,1)};
const TPoint corner2[2]={TPoint(2,1),TPoint(1,1)};
const TPoint corner3[2]={TPoint(3,1),TPoint(1,2)};
const TPoint corner5[4]={TPoint(5,1),TPoint(3,1),TPoint(2,1),TPoint(1,2)};

/*CWsClientWindow*/

CWsClientWindow::CWsClientWindow(CWsClient* aOwner, CScreen* aScreen) : CWsWindow(aOwner,WS_HANDLE_WINDOW,aScreen)
	{
	iWinType=EWinTypeClient;
	}

void CWsClientWindow::ConstructL(const TWsClCmdCreateWindow &aCmd, CWsWindowBase *aParent, TBool aScreenDeviceIsInvalid)
	{
	CWsWindow::Construct();
	NewObjL();
	if (aCmd.clientHandle==NULL)
		OwnerPanic(EWservPanicNullHandle);
#if defined(_DEBUG)
	if (IsClientHandleInUse(aCmd.clientHandle))
		OwnerPanic(EWservPanicDuplicateHandle);
#endif
	iPointerFilter=EPointerFilterEnterExit|EPointerFilterMove|EPointerFilterDrag;
	iClientHandle=aCmd.clientHandle;
	CWsWindow* inherit=static_cast<CWsWindow *>(aParent);
	if (aParent->WinType()==EWinTypeGroup)
		inherit=RootWindow();
	SetPointerCursor(aParent->PointerCursor());
	iAbs=inherit->Abs();
	iOrigin=aParent->Origin();
	iRel.iBr.iX=inherit->Rel().iBr.iX-inherit->Rel().iTl.iX;
	iRel.iBr.iY=inherit->Rel().iBr.iY-inherit->Rel().iTl.iY;
	switch(aCmd.type)
		{
		case EWinRedraw:
			iRedraw=new(ELeave) CWsRedrawMsgWindow(this);
			break;
		case EWinBackedUp:
			iRedraw=new(ELeave) CWsBackedUpWindow(this, aCmd.displayMode);
			iAbs.iBr=iAbs.iTl;
			iRel.iBr=iRel.iTl;
			break;
		case EWinBlank:
			iRedraw=new(ELeave) CWsBlankWindow(this);
			break;
		default:
			OwnerPanic(EWservPanicRedrawType);
		}
	ResetHiddenFlag();
	CWsWindowBase::ConstructL(aParent);
	if (aScreenDeviceIsInvalid)
		{
		iFlags|=EFlagScreenDeviceInvalid;
		ResetHiddenFlag();
		}
	SetCornerTypeL(EWindowCornerSquare,0);
	iRedraw->ConstructL();
	}

void CWsClientWindow::SetClippedBaseArea(RWsRegion &aRegion) const
	{
	if(iBaseArea)
		{
		aRegion.Copy(*iBaseArea);
		}
	aRegion.ClipRect(iAbs);
	}

void CWsClientWindow::SetOpaqueClippedBaseArea(RWsRegion &aRegion) const
	{
	if (IsTranslucent())
		{
		if (iUserDefinedOpaqueRegion)
			{
			aRegion.Copy(*iUserDefinedOpaqueRegion);
			aRegion.ClipRect(iAbs);
			}
		else
			{
			aRegion.Clear();
			}
		}
	else
		{
		SetClippedBaseArea(aRegion);
		}
	}

void CWsClientWindow::ResetHiddenFlag()
//
// Reset the status of the hidden flag based on the current states of the active and invisible flags
//
	{
	CWsClientWindow *parent=static_cast<CWsClientWindow*>(iParent);

	TBool wasHidden = iFlags&EFlagHidden;
	TBool nowHidden = (parent==NULL || 
		              (parent->WinType()==EWinTypeClient && !parent->IsVisible()) ||
	                  !(iFlags&EFlagActive) ||
	                  (iFlags&EFlagInvisible) ||
			          (iFlags&EFlagScreenDeviceInvalid));

	if (nowHidden)
		{
		iFlags|=EFlagHidden;
		iFlags&=~EFlagDrawnToScreen;
		}
	else
		{
		iFlags&=~EFlagHidden;
		}
	if ((!nowHidden) != (!wasHidden))
		{
		// intentionally call the screen directly
		iScreen->ScheduleRegionUpdate(&iVisibleRegion);
		}
	}
	
void CWsClientWindow::ResetHiddenFlags()
	{
	CWsClientWindow *win=this;
	FOREVER
		{
		TUint oldHiddenFlag=win->iFlags&EFlagHidden;
		win->ResetHiddenFlag();
		if ((win->iFlags&EFlagHidden)!=oldHiddenFlag)	// If hidden status hasn't changed nothing to do
			{
			if (win->Child())
				{
				win=win->Child();
				continue;
				}
			}
		if (win==this)
			return;
		while(!win->NextSibling())
			{
			win=(CWsClientWindow *)win->BaseParent();
			if (win==this)
				return;
			}
		win=win->NextSibling();
		}
	}

void CWsClientWindow::OffsetBaseArea(const TPoint &aOffset)
	{
	iBaseArea->Offset(aOffset);
	}

void CWsClientWindow::CalcBaseArea()
//
// The windows basic area before any clipping is done
//
	{
	TInt cornerType=iCornerData&ECornerTypeMask;
	if (cornerType==EWindowCornerRegion)
		iBaseArea->ClipRect(FullRect());
	else
		{
		TSize size=Size();
		iBaseArea->Clear();
		const TPoint *corners=NULL;
		TInt count=0;
		switch(cornerType)
			{
			case EWindowCorner1:
				count=sizeof(corner1)/sizeof(TPoint);
				corners=corner1;
				break;
			case EWindowCorner2:
				count=sizeof(corner2)/sizeof(TPoint);
				corners=corner2;
				break;
			case EWindowCorner3:
				count=sizeof(corner3)/sizeof(TPoint);
				corners=corner3;
				break;
			case EWindowCorner5:
				count=sizeof(corner5)/sizeof(TPoint);
				corners=corner5;
				break;
			default:
				break;
			}
		TInt top=0;
		TInt bot=size.iHeight;
		for(TInt index=0;index<count;index++)
			{
			TInt xadjust=corners[index].iX;
			TInt yadjust=corners[index].iY;
			if ((iCornerData&(EWindowCornerNotTL|EWindowCornerNotTR))!=(EWindowCornerNotTL|EWindowCornerNotTR))
				{
				iBaseArea->AddRect(TRect(iCornerData&EWindowCornerNotTL?0:xadjust,top,
										 size.iWidth-(iCornerData&EWindowCornerNotTR?0:xadjust),top+yadjust));
				top+=yadjust;
				}
			if ((iCornerData&(EWindowCornerNotBL|EWindowCornerNotBR))!=(EWindowCornerNotBL|EWindowCornerNotBR))
				{
				iBaseArea->AddRect(TRect(iCornerData&EWindowCornerNotBL?0:xadjust,bot-yadjust,
										 size.iWidth-(iCornerData&EWindowCornerNotBR?0:xadjust),bot));
				bot-=yadjust;
				}
			}
		iBaseArea->AddRect(TRect(0,top,size.iWidth,bot));
		iBaseArea->Offset(Origin());
		iBaseArea->ClipRect(FullRect());
		iBaseArea->Sort();
		}
	}

void CWsClientWindow::GenerateArea(RWsRegion &aArea, TBool aClipTranslucent) const
//
// Create the window area list.
//
	{
	aArea.Clear();
	if (IsVisible())
		{
		aArea.Copy(*iBaseArea);
		aArea.ClipRect(iAbs);
		const CWsClientWindow *win=this;
		FOREVER
			{
			if (win->IsTopClientWindow())
				break;
			ClipWindows(aArea,(CWsClientWindow *)win->BaseParent()->BaseChild(),win,aClipTranslucent);
			win=(CWsClientWindow *)win->iParent;
			}
		TInt tidyCount=0;
		for(const CWsClientWindow *cwin=RootWindow()->FirstTopClientWindow();aArea.Count() && cwin!=win;cwin=cwin->NextSiblingMultiParent())
			{
			if (!tidyCount--)
				{
				aArea.Tidy();
				tidyCount=ETidyCountSetting;	// Tidy every ETidyCountSetting times around
				}
			if (cwin->IsVisible())
				{
				if (cwin->IsTranslucent() && !aClipTranslucent)
					{
					if (cwin->iUserDefinedOpaqueRegion)
						{
						aArea.SubRegion(*cwin->iUserDefinedOpaqueRegion);
						}
					}
				else
					{
					aArea.SubRegion(*cwin->iBaseArea);
					}
				}
			}
		aArea.Tidy();
		}
	}

void CWsClientWindow::ClipWindows(TRegion &region,const CWsClientWindow *start, const CWsClientWindow *end, TBool aClipTranslucent)
//
// Remove out of the region the opaque part of the abs rect of all the windows starting from 'start'
// along the sibling list to (and not including) the end window.
//
	{
	for(const CWsClientWindow *win=start;region.Count() && win!=end;win=win->NextSibling())
		{
		if (win->IsVisible())
			{
			if (win->IsTranslucent() && !aClipTranslucent)
				{
				if (win->iUserDefinedOpaqueRegion)
					{
					region.SubRegion(*win->iUserDefinedOpaqueRegion);
					}
				}
			else
				{
				region.SubRegion(*win->iBaseArea);
				}
			}
		}
	}

void CWsClientWindow::GenerateTopRegion(RWsRegion& aRegion) const
	{
	GenerateArea(aRegion,ETrue);
	if (iChild)
		ClipWindows(aRegion,Child(),NULL,ETrue);
	}

void CWsClientWindow::GenerateWindowRegion(RWsRegion &aRegion) const
//
// Calculate the windows clipping region without using the usual stored iArea or iRegion fields
// this function is used by the screen backup code to calculate "what if" regions to work out
// whether something would be visible if the backed up window didn't exist, on this basis we
// don't want to modify the existing copies of iArea & iRegion.
//
	{
	GenerateArea(aRegion,EFalse);
	if (iChild)
		ClipWindows(aRegion,Child(),NULL,EFalse);
	}

void CWsClientWindow::RecalcChildAbs(const TPoint *aOffset)
	{
	CWsClientWindow *win=this;
	FOREVER
		{
		FOREVER
			{
			win->SetAbsFromRel();
			if (aOffset)
				win->OffsetBaseArea(*aOffset);
			if (win->Child()==NULL)
				break;
			win=win->Child();
			}
		FOREVER
			{
			if (win==this)
				return;
			if (win->NextSibling()!=NULL)
				{
				win=win->NextSibling();
				break;
				}
			win=(CWsClientWindow *)win->iParent;	// The cast is safe as the loop is aborted when win==this
			}
		}
	}

void CWsClientWindow::SetAbsFromRel()
	{
	iOrigin=iRel.iTl+iParent->Origin();
	iAbs=iRel;
	iAbs.Move(iParent->Origin());
	iAbs.Intersection(iParent->AbsRect());
	}

void CWsClientWindow::SetExtentL(const TPoint *aPos,const TSize *aSize)
	{
	if (iParent==NULL)
		OwnerPanic(EWservPanicParentDeleted);
	TPoint offset = TPoint(0,0);
	TSize oldSize;
	TSize newSize;
	TBool sizeChanged = EFalse;
	TBool posChanged = EFalse;
	
	if (aPos)
		{
		offset = *aPos+iParent->Origin()-iOrigin;
		if (offset.iX != 0 || offset.iY != 0)
			{
			posChanged = ETrue;
			}
		}
		
	if (posChanged)
		{
		TWalkWindowTreeScheduleRedraws wwt;
		WalkWindowTree(wwt, EWalkChildren);
		}
	
	if (aSize)
		{
		newSize=*aSize;
		if (newSize.iWidth<0)
			newSize.iWidth=0;
		if (newSize.iHeight<0)
			newSize.iHeight=0;
		// This should be the only part of resizing that can fail
		// and it can only fail for backedup windows.
		iRedraw->PrepareForResizeL(newSize,oldSize);
		sizeChanged = *aSize != iRel.Size();
		}

	if (posChanged)
		{
		iRel.Move(offset);
		RecalcChildAbs(&offset);
		TWalkWindowTreeOffsetTransparentRegions offsetTransparent(offset);
		WalkWindowTree(offsetTransparent, EWalkChildren);
		}

	if (sizeChanged)
		{
		iRel.SetSize(newSize);
		RecalcChildAbs(NULL);
		CalcBaseArea();
		iRedraw->Resize(newSize,oldSize);
		}

	if (posChanged || sizeChanged)
		{
		iRedraw->ClipInvalidRegion(TRect(iRel.Size()));
		iRedraw->Moved();
		ScheduleRegionUpdate(NULL);
		TWalkWindowTreeRecalcOpaque recalcOpaque;
		WalkWindowTree(recalcOpaque, EWalkChildren);
		}
	}

void CWsClientWindow::Scroll(const TRect &aClipRect, const TPoint &aOffset, const TRect &aRect)
	{
	if (iParent==NULL)
		OwnerPanic(EWservPanicParentDeleted);
//
	iRedraw->Scroll(aClipRect, aOffset,aRect);
//
	CWsTop::TriggerRedraws(RootWindow());
	}

void CWsClientWindow::DeleteBaseArea()
	{
	WS_ASSERT_DEBUG(iBaseArea!=&nullRegion, EWsPanicRegionNull);
 	if ((iCornerData&ECornerTypeMask)==EWindowCornerRegion)
		((RWsRegion *)iBaseArea)->Destroy();
	else
		delete iBaseArea;
	}

CWsClientWindow::~CWsClientWindow()
	{
	iFlags|=EFlagShutDownInProgress;
	if (CClick::IsHandler())
		{
		TWindowCloseData params;
		params.iClientHandle=iClientHandle;
		//if parent already shutdown (or disconnected) send 0
		params.iWindowGroupId=iParent ? WinGroup()->Identifier():0;
		CClick::OtherEvent(EEventWindowClose,&params);
		}
	RemoveAllKeyRects();
	while(iWinGcList)
		iWinGcList->Deactivate();
//
	iFlags|=EFlagInvisible;		// First make it invisble
	if (iParent)				// In case window wasn't fully constructed
		ResetHiddenFlags();
//
	CWsWindow::Shutdown();      // Two phase destruction

	DeleteBaseArea();
	CWsPointerBuffer::Disconnect(this);
	iFlags&=~EFlagShutDownInProgress;
	SetUserTransparentRegion(0);
	CWsPassword::WindowDestroyed(this);
	}

void CWsClientWindow::Activate()
	{
	if (iFlags&EFlagActive)
		OwnerPanic(EWservPanicWindowActive);
	iFlags|=EFlagActive;

	ResetHiddenFlags();
	}

void CWsClientWindow::SetCornerTypeL(TCornerType aCornerType, TInt aCornerFlags, TRegion *aNewBaseArea)
	{
	TRegion *baseArea=NULL;
	if (aCornerFlags&ECornerTypeMask)
		OwnerPanic(EWservPanicCornerParams);
 	switch (aCornerType)
		{
		case EWindowCornerSquare:
			baseArea=new(ELeave) TRegionFix<1>();
			break;
		case EWindowCorner1:
			baseArea=new(ELeave) TRegionFix<3>();
			break;
		case EWindowCorner2:
		case EWindowCorner3:
			baseArea=new(ELeave) TRegionFix<5>();
			break;
		case EWindowCorner5:
			baseArea=new(ELeave) TRegionFix<9>();
			break;
		case EWindowCornerRegion:
			User::LeaveIfNull(baseArea=aNewBaseArea);
			baseArea->Offset(Origin());
			break;
		default:
			OwnerPanic(EWservPanicCornerParams);
		}
	DeleteBaseArea();
	iCornerData=aCornerType;
	iCornerData|=aCornerFlags;
	iBaseArea=baseArea;
	CalcBaseArea();
	ScheduleRegionUpdate(NULL);
	}

void CWsClientWindow::SetVisible(TBool aState)
	{
	if (aState)
		{
		if (iParent==NULL)
			OwnerPanic(EWservPanicParentDeleted);
		if (!(iFlags&EFlagInvisible))	// Already visible
			return;
		iFlags&=~EFlagInvisible;
		ResetHiddenFlags();
		}
	else
		{
		if (iFlags&EFlagInvisible || !iParent)	// Already invisible or parent has been deleted
			return;
		TWalkWindowTreePurgeEvents wwt;
		WalkWindowTree(wwt,EWalkChildren);		// Destroy all events on this and all children
		iFlags|=EFlagInvisible;
		ResetHiddenFlags();
		}
	}

void CWsClientWindow::CommandL(TInt aOpcode, const TAny *aCmdData)
	{
#ifdef _DEBUG
	// Save root window for performing CheckTree at the end of this func.
	// When aOpcode is EWsWinOpFree, this object would've been destroyed
	// and a call to RootWindow() in that case would be impossible
	CWsRootWindow* rootWindow=RootWindow();
#endif
	TWsWinCmdUnion pData;
	pData.any=aCmdData;
	if (CWsWindowBase::CommandL(aOpcode,pData)==EFalse)
		{
		switch(aOpcode)
			{
			case EWsWinOpActivate:
				Activate();
				break;
			case EWsWinOpSetPos:
				SetExtentL(pData.pos,NULL);
				break;
			case EWsWinOpSetExtent:
			case EWsWinOpSetExtentErr:
				SetExtentL(&pData.SetEx->pos,&pData.SetEx->size);
				break;
			case EWsWinOpSetSize:
			case EWsWinOpSetSizeErr:
				SetExtentL(NULL,pData.size);
				break;
			case EWsWinOpInquireOffset:
				CWsClient::ReplyPoint(InquireOffset(*pData.UInt));
				break;
			case EWsWinOpPosition:
				CWsClient::ReplyPoint(iRel.iTl);
				break; 
			case EWsWinOpAbsPosition:
				CWsClient::ReplyPoint(iOrigin);
				break;
			case EWsWinOpSize:
				CWsClient::ReplySize(iRel.Size());
				break;
			case EWsWinOpTestInvariant:
				SetReply(EFalse);
				break;
			case EWsWinOpPointerFilter:
				{
				TUint old=iPointerFilter;
				iPointerFilter&=~pData.PointerFilter->mask;
				iPointerFilter|=pData.PointerFilter->mask&pData.PointerFilter->flags;
				if (old&EPointerFilterEnterExit)
					WsPointer::ReLogWindow(this);
				}
				break;
			case EWsWinOpSetPointerGrab:
				if (*pData.Bool==EFalse)
					iFlags&=~EFlagPointerGrab;
				else
					iFlags|=EFlagPointerGrab;
				break;
			case EWsWinOpClaimPointerGrab:
				if (!iParent)
					OwnerPanic(EWservPanicParentDeleted);
				WsPointer::ClaimGrab(this,*pData.Bool);
				break;
			case EWsWinOpSetPointerCapture:
				iFlags&=~(EFlagPointerCaptured|EFlagPointerCaptureDragDrop|EFlagPointerCaptureAllGroups);
				if ((*pData.UInt)&RWindowBase::TCaptureFlagEnabled)
					{
					iFlags|=EFlagPointerCaptured;
					if ((*pData.UInt)&RWindowBase::TCaptureFlagDragDrop)
						iFlags|=EFlagPointerCaptureDragDrop;
					if ((*pData.UInt)&RWindowBase::TCaptureFlagAllGroups)
						iFlags|=EFlagPointerCaptureAllGroups;
					
					}
				WsPointer::ReLogCurrentWindow();
				break;
			case EWsWinOpSetPointerCapturePriority:
				iPointerCapturePriority=*pData.Int;
				break;
			case EWsWinOpGetPointerCapturePriority:
				SetReply(iPointerCapturePriority);
				break;
			case EWsWinOpSetVisible:
				SetVisible(*pData.Bool);
				break;
			case EWsWinOpScroll:
				{
				TPoint origin(0,0);
				TRect src(TRect(origin,iRel.Size()));
				src.Move(-pData.ScrollRect->offset);
				Scroll(TRect(TPoint(0,0),iRel.Size()),pData.ScrollRect->offset,src);
				}
				break;
			case EWsWinOpScrollClip:
				{
				TPoint origin(0,0);
				TRect src(TRect(origin,iRel.Size()));
				src.Move(-pData.ScrollRect->offset);
				TRect clip(pData.ScrollRect->clip);
				Scroll(clip,pData.ScrollRect->offset,src);
				}
				break;
			case EWsWinOpScrollRect:
				{
				TRect src(pData.ScrollRect->rect);
				Scroll(TRect(TPoint(0,0),iRel.Size()),pData.ScrollRect->offset,src);
				}
				break;
			case EWsWinOpScrollClipRect:
				{
				TRect src(pData.ScrollRect->rect);
				TRect clip(pData.ScrollRect->clip);
				Scroll(clip, pData.ScrollRect->offset,src);
				}
				break;
			case EWsWinOpSetOrdinalPositionPri:
				iOrdinalPriority=pData.OrdinalPos->ordinalPriority;
				SetOrdinalPosition(pData.OrdinalPos->pos);
				break;
			case EWsWinOpSetShadowHeight:
				if ((*pData.Int)<0)
					OwnerPanic(EWservPanicNegativeShadowHeight);
				break;
			case EWsWinOpShadowDisabled:
				break;
			case EWsWinOpSetCornerType:
				SetCornerTypeL(pData.SetCornerType->type, pData.SetCornerType->flags);
				break;
			case EWsWinOpSetShape:
				SetCornerTypeL(EWindowCornerRegion,0,GetRegionFromClientL(iWsOwner, *pData.Int));
				break;
			case EWsWinOpRequiredDisplayMode:
				if (Backup()!=NULL)
					OwnerPanic(EWservPanicBackupDisplayMode);
				SetReply(DisplayMode());
				break;
			case EWsWinOpGetDisplayMode:
				SetReply(DisplayMode());
				break;
			case EWsWinOpRequestPointerRepeatEvent:
				if (!iParent)
					OwnerPanic(EWservPanicParentDeleted);
				WsPointer::RequestPointerRepeatEvent(this,pData.RequestPointerRepeatEvent->time,pData.RequestPointerRepeatEvent->rect);
				break;
			case EWsWinOpCancelPointerRepeatEventRequest:
				WsPointer::CancelPointerRepeatEventRequest();
				break;
			case EWsWinOpAllocPointerMoveBuffer:
				CWsPointerBuffer::ConnectL(this,pData.AllocPointerMoveBuffer->maxNumPoints,pData.AllocPointerMoveBuffer->flags);
				iFlags|=EFlagUsingPointerBuffer|EFlagHasPointerBuffer;
				break;
			case EWsWinOpFreePointerMoveBuffer:
				CWsPointerBuffer::Disconnect(this);
				iFlags&=~(EFlagUsingPointerBuffer|EFlagHasPointerBuffer);
				break;
			case EWsWinOpRetrievePointerMoveBuffer:
				CWsPointerBuffer::RetrievePointerMoveBuffer(this,*pData.Int);
				break;
			case EWsWinOpEnablePointerMoveBuffer:
				if (!(iFlags&EFlagHasPointerBuffer))
					OwnerPanic(EWservPanicNoPointerBuffer);
				iFlags|=EFlagUsingPointerBuffer;
				break;
			case EWsWinOpDisablePointerMoveBuffer:
				iFlags&=~EFlagUsingPointerBuffer; 
				/*Fall Through*/
			case EWsWinOpDiscardPointerMoveBuffer:	
				CWsPointerBuffer::DiscardPointerMoveBuffer(this);
				break;
			case EWsWinOpAddKeyRect:
				AddKeyRectL(pData.AddKeyRect->rect, pData.AddKeyRect->scanCode, pData.AddKeyRect->activatedByPointerSwitchOn);
				break;
			case EWsWinOpRemoveAllKeyRects:
				RemoveAllKeyRects();
				break;
			case EWsWinOpPasswordWindow:
				if (!iParent)
					OwnerPanic(EWservPanicParentDeleted);
				CWsPassword::SetPasswordWindowL(this, *pData.PasswordMode);
				break;
			case EWsWinOpEnableBackup:
				if (!iParent)
					OwnerPanic(EWservPanicParentDeleted);
				if (*pData.UInt==0)
					iBackupsRequested|=EWindowBackupAreaBehind;		//For backwards compatibility
				else
					iBackupsRequested|=*pData.UInt;
				break;
			case EWsWinOpFadeBehind:
				{
				if (!iParent)
					OwnerPanic(EWservPanicParentDeleted);
				TUint8 blackMap;
				TUint8 whiteMap;
				iScreen->GetFadingParams(blackMap,whiteMap);
				SetFadeBehind(*pData.Bool);
				TWalkWindowTreeSetFaded wwt(*pData.Bool,this,blackMap,whiteMap);
				WalkWindowTree(wwt,EWalkBehind);
				if (CWsTop::IsFadeEnabled()) 
					{
					Screen()->AcceptFadeRequest( this, *pData.Bool, ETrue, EFalse  );
					}
				}
				break;
			case EWsWinOpGetIsFaded:
				SetReply(iFadeCount);
				break;
			case EWsWinOpGetIsNonFading:
				SetReply(iFlags&EFlagNonFadingWindow);
				break;
			case EWsWinOpMoveToGroup:
				if (!iParent)
					OwnerPanic(EWservPanicParentDeleted);
				if (iParent->WinType()!=EWinTypeGroup)
					OwnerPanic(EWservPanicNotTopClient);
				DoMoveWindowToGroupL(*pData.Int);
				break;
			case EWsWinOpTestLowPriorityRedraw:
				{
				// This is purely for testing purposes
				// Returns the redraw priority
				TUint priority=0;
				TPckgBuf<TUint> priBuf;
				priority=WsOwner()->RedrawQueue()->RedrawPriority((CWsWindowRedraw*)this->iRedraw);
				priBuf()=priority;
				CWsClient::ReplyBuf(priBuf);
				}
				break;
			case EWsWinOpEnableVisibilityChangeEvents:
				iFlags |= EFlagGeneratesVisibilityEvents;
				if (iFlags&EFlagActive)
					{
					iScreen->DoRedrawNow();
					PossibleVisibilityChangedEvent(ETrue);
					}
				break;
			case EWsWinOpDisableVisibilityChangeEvents:
				iFlags &= ~EFlagGeneratesVisibilityEvents;
				break;
			case EWsWinOpSetTransparentRegion:
				{
				if (IsTranslucent())
					{
					TInt recs=*pData.Int;
	 				RWsRegion* reg=recs>0? GetRegionFromClientL(iWsOwner,recs) : new(ELeave) RWsRegion;
	 				SetUserTransparentRegion(reg);
					SetReply(KErrNone);
					}
				else
					{
					OwnerPanic(EWservPanicTransparencyObjNotCreated);	
					}				
				}
				break;
			case EWsWinOpSetTransparencyPolicy:
				{
				if (IsTranslucent())
					SetReply(KErrNone);
				else
					OwnerPanic(EWservPanicTransparencyObjNotCreated);
				}
				break;
			case EWsWinOpSetTransparencyAlphaChannel:
				{
				iFlags |= static_cast<TUint>(EFlagHasAlpha);
				SetReply(KErrNone);
				break;
				}
			default:
				if (iRedraw->CommandL(aOpcode,pData)==EFalse)
					{
					OwnerPanic(EWservPanicOpcode);
					}
			}
		}
#if defined(_DEBUG)
	rootWindow->CheckTree();
#endif
	}

void CWsClientWindow::GcActivated(CWsGc *aGc)
	{
	aGc->SetNextWinGc(iWinGcList);
	iWinGcList=aGc;
	}

void CWsClientWindow::GcDeactivated(CWsGc *aGc)
	{
	if (aGc==iWinGcList)
		iWinGcList=aGc->NextWinGc();
	else
		{
		CWsGc *gc=iWinGcList;
		CWsGc *next;
		FOREVER
			{
			next=gc->NextWinGc();
			WS_ASSERT_DEBUG(next!=NULL, EWsPanicBadActiveGcList);
			if (next==aGc)
				{
				gc->SetNextWinGc(next->NextWinGc());
				break;
				}
			gc=next;
			}
		}
	aGc->SetNextWinGc(NULL);
	}

void CWsClientWindow::ReactivateGcs()
	{
	for (CWsGc * gc = iWinGcList; gc; gc = gc->NextWinGc())
		{
		gc->Reactivate();
		}
	}

void CWsClientWindow::OffsetUserTransparentRegion(const TPoint& aOffset)
	{
	if (iUserDefinedTransparentRegion)
		{
		iUserDefinedTransparentRegion->Offset(aOffset);
		}	
	}

void CWsClientWindow::SetUserTransparentRegion(RWsRegion* aRegion)
	{
	if (iUserDefinedTransparentRegion)
		{
		iUserDefinedTransparentRegion->Close();
		delete iUserDefinedTransparentRegion;
		iUserDefinedTransparentRegion = 0;
		}
		
	if (aRegion)
		{		
		aRegion->Offset(iOrigin);
		iUserDefinedTransparentRegion=aRegion;
		}
		
	SetUserOpaqueRegion();
	}

void CWsClientWindow::SetUserOpaqueRegion()
	{
	if (iUserDefinedOpaqueRegion)
		{
		iUserDefinedOpaqueRegion->Close();
		delete iUserDefinedOpaqueRegion;
		iUserDefinedOpaqueRegion = 0;
		}
	if (iUserDefinedTransparentRegion)
		{
		iUserDefinedOpaqueRegion=new RWsRegion;
		if (iUserDefinedOpaqueRegion)
			{
			iUserDefinedOpaqueRegion->Copy(*iBaseArea);
			iUserDefinedOpaqueRegion->SubRegion(*iUserDefinedTransparentRegion);
			if (iUserDefinedOpaqueRegion->CheckError() || iUserDefinedOpaqueRegion->Count() == 0)
				{
				iUserDefinedOpaqueRegion->Close();
				delete iUserDefinedOpaqueRegion;
				iUserDefinedOpaqueRegion = 0;
				}
			}
		}
	}

TUint CWsClientWindow::RedrawPriority(TInt *aShift) const
	{
	if (IsTopClientWindow())
		{
		TUint ordinalPos=OrdinalPosition(EFalse);
		if (ordinalPos>KWinRedrawPriMaxOrdinal)	// Algorithm only works for upto KWinRedrawPriMaxOrdinal windows,
			ordinalPos=KWinRedrawPriMaxOrdinal;	// make all windows after this equal in priority
		if (aShift)
			*aShift=KWinRedrawPriMaxLevel;
		return(ordinalPos<<(KWinRedrawPriMaxLevel*KWinRedrawPriBitsPerLevel));
		}
	else
		{
		TUint ordinalPos=OrdinalPosition(EFalse)+1;
		if (ordinalPos>KWinRedrawPriMaxOrdinal)	// Algorithm only works upto 15 , make all windows after 15 equal in priority
			ordinalPos=KWinRedrawPriMaxOrdinal;
		TInt shift;
		TUint parent=((CWsClientWindow *)iParent)->RedrawPriority(&shift);
		if (shift>0)
			shift--;
		if (aShift)
			*aShift=shift;
		return(parent+(ordinalPos<<(shift*KWinRedrawPriBitsPerLevel)));
		}
	}

TDblQue<TPointerKeyList> *CWsClientWindow::PointerKeyList() const
	{
	return(iPointerKeyList);
	}

void CWsClientWindow::AddKeyRectL(const TRect &aRect, TInt aScanCode, TBool aActivatedByPointerSwitchOn)
	{
	if (!iPointerKeyList)
		iPointerKeyList=new(ELeave) TDblQue<TPointerKeyList>(_FOFF(TPointerKeyList,iQue));
	TPointerKeyList *pkl=new(ELeave) TPointerKeyList();
	iPointerKeyList->AddLast(*pkl);
	pkl->iRect=aRect;
	pkl->iScanCode=aScanCode;
	pkl->iActivatedByPointerSwitchOn=aActivatedByPointerSwitchOn;
	}

void CWsClientWindow::RemoveAllKeyRects()
	{
	if (iPointerKeyList)
		{
		TPointerKeyList *pkl=NULL;
		for(TDblQueIter<TPointerKeyList> iter(*iPointerKeyList);(pkl=iter++)!=NULL;)
			{
			pkl->iQue.Deque();
			delete pkl;
			}
		delete iPointerKeyList;
		iPointerKeyList=NULL;
		}
	}

TBool CWsClientWindow::IsHidden()
	{
	return (!IsVisible()) || VisibleRegion().IsEmpty();
	}

void CWsClientWindow::SetFaded(TBool aFade,TUint8 aBlackMap,TUint8 aWhiteMap)
	{
	iBlackMap=aBlackMap;
	iWhiteMap=aWhiteMap;
	if (iAbsoluteFading) 
		{ 
		if (aFade) 
			{ 
			iFadeCount = 1; 
			} 
		else 
			{ 
			iFadeCount = 0; 
			} 
		} 
	else 
		{ 
		if (aFade) 
			{ 
			++iFadeCount; 
			} 
		else if (iFadeCount > 0) 
			{ 
			--iFadeCount; 
			} 
		} 
	}

void CWsClientWindow::ResetHiddenFlagsInParentAndChildren()
	{
	ResetHiddenFlag();
	for(CWsClientWindow* child=Child();child;child=child->NextSibling())
		{
		child->ResetHiddenFlagsInParentAndChildren();
		}
	}

const TRegion& CWsClientWindow::WindowArea() const
	{
	return *iBaseArea;
	}

void CWsClientWindow::Invalidate(const TRect * aRect)
	{
	iRedraw->Invalidate(aRect);
	}

void CWsClientWindow::ScheduleRegionUpdate(const TRegion* aDefinitelyDirty)
	{
	if (IsVisible())
		{
		iScreen->ScheduleRegionUpdate(aDefinitelyDirty);
		}
	}
	
TBool CWsClientWindow::IsDSAHost() const
	{
	TBool res = CWsWindow::IsDSAHost();
	if ( !res )
		{ // check for grace period when DSA is being restarted (after aborting but before client started DSA again)
		res = Screen()->IsDSAClientWindow( this );
		}
	return res;
	}
void CWsClientWindow::SetScreenDeviceValidState(TBool aState)
	{
	if (SetScreenDeviceValidStateFlag(aState))
		ResetHiddenFlags();
	}

TBool CWsClientWindow::SetScreenDeviceValidStateFlag(TBool aState)
	{
	TBool isSet=iFlags&EFlagScreenDeviceInvalid;
	if (!isSet==!aState)
		{
		if (aState)
			iFlags&=~EFlagScreenDeviceInvalid;
		else
			iFlags|=EFlagScreenDeviceInvalid;
		return ETrue;
		}
	return EFalse;
	}

void CWsClientWindow::DoMoveWindowToGroupL(TInt aIdentifier)
	{
	CWsWindowGroup* group=CWsWindowGroup::WindowGroupFromIdentifierL(aIdentifier);
	if (group==iParent)
		return;
	if (group->WsOwner()!=WsOwner())
		User::Leave(KErrNotFound);
	ChangeWindowPosition(0, group);
	CWsTop::TriggerRedraws(RootWindow());
	}

void CWsClientWindow::SetInactive()
	{
	iFlags&=~EFlagActive;
	ResetHiddenFlags();
	}

