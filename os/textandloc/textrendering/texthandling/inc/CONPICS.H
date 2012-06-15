/*
* Copyright (c) 1997-2009 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
* This component and the accompanying materials are made available
* under the terms of "Eclipse Public License v1.0"
* which accompanies this distribution, and is available
* at the URL "http://www.eclipse.org/legal/epl-v10.html".
*
* Initial Contributors:
* Nokia Corporation - initial contribution.
*
* Contributors:
*
* Description: 
*
*/


#ifndef __CONPICS_H__
#define __CONPICS_H__

#include <e32std.h>
#include <e32base.h>
#include <s32stor.h>
#include <gdi.h>
#include <fldbltin.h>

//
// Classes declared in this file
class CXzePicture;
class CTestPicture;
class CDummyField;

/**
@internalTechnology
*/
const TUid KUidXzePictureType={268435508};

/**
@internalComponent
*/
const TUid KUidXzeDoorType={268435509};

/**
@internalComponent
*/
const TInt KWidthInTwips=360;
const TInt KHeightInTwips=360;

class MDemPictureFactory : public MPictureFactory
/**
@internalComponent
*/
	{
	// Instantiates new CDemPictureHeaders.
public:
	IMPORT_C MDemPictureFactory();  // Force GCC to export vtable.
	IMPORT_C virtual void NewPictureL(TPictureHeader& aHeader,const CStreamStore& aDeferredPictureStore)const;
	};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CXzePicture : public CPicture
/**
@internalTechnology
*/
    {
	// A simple test picture, that is store aware, and has state.
	//
public:
	IMPORT_C static CXzePicture* NewL(TChar aLabel);
	IMPORT_C static CXzePicture* NewL(const CStreamStore& aStore,TStreamId aId);
	IMPORT_C virtual ~CXzePicture();
	//
	IMPORT_C void ExternalizeL(RWriteStream& aStream)const;
	IMPORT_C void InternalizeL(RReadStream& aStream);
	IMPORT_C void RestoreL(const CStreamStore& aStore,TStreamId aId);
	//
	IMPORT_C void GetOriginalSizeInTwips(TSize& aSize) const;
	//
	IMPORT_C virtual void Draw(CGraphicsContext& aGc,const TPoint& aTopLeft,const TRect& aClipRect,MGraphicsDeviceMap* aMap=NULL)const;
protected:
	IMPORT_C CXzePicture(TChar aLabel);
	IMPORT_C CXzePicture();
public:
	TSize iOriginalSizeInTwips;
	TChar iLabel;  // Provides object state.
	};


class CXzeDoor : public CXzePicture
/**
@internalComponent
*/
	{
	// A sample door, that has emulates 2 states of presence.  An iconic representation
	// and a deep representation.
	//
public:
	IMPORT_C static CXzeDoor* NewL(TChar aLabel,TBool aAlwaysFailToDetach=EFalse);
	IMPORT_C static CXzeDoor* NewL(const CStreamStore& aStore,TStreamId aId);
	//
	IMPORT_C virtual void DetachFromStoreL(TDetach aDegree);
	//
	IMPORT_C void ExternalizeL(RWriteStream& aStream)const;
	IMPORT_C void InternalizeL(RReadStream& aStream);
	IMPORT_C void RestoreL(const CStreamStore& aStore,TStreamId aId);
protected:
	IMPORT_C CXzeDoor(TChar aLabel,TBool aAlwaysFailToDetach);
	IMPORT_C CXzeDoor(TBool aAlwaysFailToDetach);
public:
	TBool iAlwaysFailToDetach;
	};


class CTestPicture : public CPicture
/**
@internalTechnology
*/
    {
public:
	IMPORT_C static CTestPicture* NewL();
	//
	IMPORT_C void ExternalizeL(RWriteStream& aStream)const;
	//
	IMPORT_C void GetOriginalSizeInTwips(TSize& aSize) const;
	//
	IMPORT_C virtual void Draw(CGraphicsContext& aGc,const TPoint& aTopLeft,const TRect& aClipRect,MGraphicsDeviceMap* aMap=NULL) const;
protected:
	IMPORT_C CTestPicture();
public:
	TSize iOriginalSizeInTwips;
	};

///////////////////////////////////////////////////////////////////////


class CDummyField : public CTextField
/**
@internalTechnology
*/
	{
public:
	IMPORT_C CDummyField();
	IMPORT_C virtual TInt Value(TPtr& aValueText); // value is always "XXX"
	IMPORT_C virtual void InternalizeL(RReadStream& aStream);
	IMPORT_C virtual void ExternalizeL(RWriteStream& aStream)const;
	IMPORT_C virtual TUid Type() const;
	};

#endif
