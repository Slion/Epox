// Copyright (c) 2007-2009 Nokia Corporation and/or its subsidiary(-ies).
// All rights reserved.
// This component and the accompanying materials are made available
// under the terms of the License "Eclipse Public License v1.0"
// which accompanies this distribution, and is available
// at the URL "http://www.eclipse.org/legal/epl-v10.html".
//
// Initial Contributors:
// Nokia Corporation - initial contribution.
//
// Contributors:
//
// Description:
// e32test\resourceman\resourceman_psl\rescontrol_extended_psl.cpp
// 
//

#include "rescontrol_psl.h"
#include "rescontrol_extended_psl.h"

/** 
	This function creates all static resources which support dependencies and establishes
	the dependencies between them. This is called by PIL.
	Following is the dependency tree input
	ResourceA <----------------> ResourceD <------------->ResourceE <--------------> ResourceC
									|						  |
									|						  |
									|						  |	
									|						  |	
									|						  |
									|						  |
								ResourceF				   ResourceG	
	*/
TInt DSimulatedPowerResourceController::DoRegisterStaticResourcesDependency(DStaticPowerResourceD**& aStaticResourceDArray, TUint16& aStaticResourceDCount)
	{
	Kern::Printf(">DSimulatedPowerResourceController::DoRegisterStaticResourcesDependency");
	aStaticResourceDArray = (DStaticPowerResourceD**)new(DStaticPowerResourceD*[MAX_DEPENDENT_RESOURCE_COUNT]);
	if(!aStaticResourceDArray)
		return KErrNoMemory;
	DStaticPowerResourceD* pR = NULL;
	pR = new DMLSLGLSPDependResource();
	if(!pR)
		CLEAN_AND_RETURN(iStaticResDependencyCount, aStaticResourceDArray, KErrNoMemory)
	aStaticResourceDArray[iStaticResDependencyCount++] = pR;

	pR = new DMLSIGLSNDependResource();
	if(!pR)
		CLEAN_AND_RETURN(iStaticResDependencyCount, aStaticResourceDArray, KErrNoMemory)
	aStaticResourceDArray[iStaticResDependencyCount++] = pR;
	
	pR = new DBSIGLSPDependResource();
	if(!pR)
		CLEAN_AND_RETURN(iStaticResDependencyCount, aStaticResourceDArray, KErrNoMemory)
	aStaticResourceDArray[iStaticResDependencyCount++] = pR;

	pR = new DMLSHIGLSPDependResource();
	if(!pR)
		CLEAN_AND_RETURN(iStaticResDependencyCount, aStaticResourceDArray, KErrNoMemory)
	aStaticResourceDArray[iStaticResDependencyCount++] = pR;

	pR = new DBSHLGLSNDependResource();
	if(!pR)
		CLEAN_AND_RETURN(iStaticResDependencyCount, aStaticResourceDArray, KErrNoMemory)
	aStaticResourceDArray[iStaticResDependencyCount++] = pR;

	pR = new DMLSHLGLSNDependResource();
	if(!pR)
		CLEAN_AND_RETURN(iStaticResDependencyCount, aStaticResourceDArray, KErrNoMemory)
	aStaticResourceDArray[iStaticResDependencyCount++] = pR;

	//Establish resource dependencies
	if(CreateResourceDependency(aStaticResourceDArray))
		CLEAN_AND_RETURN(iStaticResDependencyCount, aStaticResourceDArray, KErrNoMemory)

	iDependencyResources = aStaticResourceDArray;

	aStaticResourceDCount = iStaticResDependencyCount;
	return KErrNone;
	}


// This function establishes above dependency between static dependent resource
TInt DSimulatedPowerResourceController::CreateResourceDependency(DStaticPowerResourceD** pResArray)
	{
	iNodeArray = new SNode[10];
	SNode* pN1;
	SNode* pN2;
	iNodeCount = 0;

	if(!iNodeArray)
		return KErrNoMemory;
	//Create Dependency between Resource A and Resource D 
	pN1 = &iNodeArray[iNodeCount++];
	pN2 = &iNodeArray[iNodeCount++];
	CREATE_DEPENDENCY_BETWEEN_NODES(pN1, pN2, pResArray[0], pResArray[1], 1, 1)

	//Create Dependency between Resource D and Resource F
	//Trying to add with the same priority and check whether KErrAlreadyExists is returned.
	pN1 = &iNodeArray[iNodeCount++];
	pN2 = &iNodeArray[iNodeCount++];
	pN1->iPriority = 1;																
	pN1->iResource = pResArray[0];															
	pN1->iPropagatedLevel = 0;																
	pN1->iVisited = EFalse;															
	
	pN2->iPriority = 1;																
	pN2->iResource = pResArray[2];																
	pN2->iPropagatedLevel = 0;																
	pN2->iVisited = EFalse;																	
	pN2->iResource->AddNode(pN1);

	TInt r = pN1->iResource->AddNode(pN2);
	if(r != KErrAlreadyExists)
		Kern::Fault("Power Resource Controller", __LINE__);

	pN2->iPriority = 3;
	r = pN1->iResource->AddNode(pN2);
	if(r != KErrNone)
		Kern::Fault("Power Resource Controller", __LINE__);

	//Create Dependency between Resource D and Resource E
	pN1 = &iNodeArray[iNodeCount++];
	pN2 = &iNodeArray[iNodeCount++];
	CREATE_DEPENDENCY_BETWEEN_NODES(pN1, pN2, pResArray[0], pResArray[3], 3, 2)

	//Create Dependency between Resource E and Resource C
	pN1 = &iNodeArray[iNodeCount++];
	pN2 = &iNodeArray[iNodeCount++];
	CREATE_DEPENDENCY_BETWEEN_NODES(pN1, pN2, pResArray[3], pResArray[4], 1, 1)

	//Create Dependency between Resource E and Resource G
	pN1 = &iNodeArray[iNodeCount++];
	pN2 = &iNodeArray[iNodeCount++];
	CREATE_DEPENDENCY_BETWEEN_NODES(pN1, pN2, pResArray[3], pResArray[5], 1, 2)

	return KErrNone;
	}

//Constructors of all resources
_LIT(KDMLSLGLSPDependResource, "StaticDependResourceD");
DMLSLGLSPDependResource::DMLSLGLSPDependResource() : DStaticPowerResourceD(KDMLSLGLSPDependResource, -100), iMinLevel(-100), iMaxLevel(100), iCurrentLevel(-100)
	{
	iFlags = KMultiLevel | KLongLatencyGet | KLongLatencySet;
	NKern::LockSystem();
	iBlockTime = MIN_BLOCK_TIME + Kern::Random() % MAX_BLOCK_TIME;
	NKern::UnlockSystem();
	}

_LIT(KDMLSIGLSNDependResource, "StaticDependResourceA");
DMLSIGLSNDependResource::DMLSIGLSNDependResource() : DStaticPowerResourceD(KDMLSIGLSNDependResource, -10), iMinLevel(-10), iMaxLevel(-20), iCurrentLevel(-10)
	{
	iFlags = KMultiLevel | KLongLatencySet | KSenseNegative;
	NKern::LockSystem();
	iBlockTime = MIN_BLOCK_TIME + Kern::Random() % MAX_BLOCK_TIME;
	NKern::UnlockSystem();
	}

_LIT(KDBSIGLSPDependResource, "StaticDependResourceF");
DBSIGLSPDependResource::DBSIGLSPDependResource() : DStaticPowerResourceD(KDBSIGLSPDependResource, 0), iMinLevel(0), iMaxLevel(1), iCurrentLevel(0)
	{
	iFlags = KBinary | KLongLatencySet;
	NKern::LockSystem();
	iBlockTime = MIN_BLOCK_TIME  + Kern::Random() % MAX_BLOCK_TIME;
	NKern::UnlockSystem();
	}

_LIT(KDMLSHIGLSPDependResource, "StaticDependResourceE");
DMLSHIGLSPDependResource::DMLSHIGLSPDependResource() : DStaticPowerResourceD(KDMLSHIGLSPDependResource, 10), iMinLevel(10), iMaxLevel(75), iCurrentLevel(10)
	{
	// Make it a Instantaneous Resource.
	iFlags = KMultiLevel | KShared;
	NKern::LockSystem();
	iBlockTime = MIN_BLOCK_TIME + Kern::Random() % MAX_BLOCK_TIME;
	NKern::UnlockSystem();
	}

_LIT(KDBSHLGLSNDependResource, "StaticDependResourceC");
DBSHLGLSNDependResource::DBSHLGLSNDependResource() : DStaticPowerResourceD(KDBSHLGLSNDependResource, 1), iMinLevel(1), iMaxLevel(0), iCurrentLevel(1)
	{
	iFlags = KBinary | KLongLatencyGet | KLongLatencySet | KShared | KSenseNegative;
	NKern::LockSystem();
	iBlockTime = MIN_BLOCK_TIME  + Kern::Random() % MAX_BLOCK_TIME;
	NKern::UnlockSystem();
	}

_LIT(KDMLSHLGLSNDependResource, "StaticDependResourceG");
DMLSHLGLSNDependResource::DMLSHLGLSNDependResource() : DStaticPowerResourceD(KDMLSHLGLSNDependResource, 75), iMinLevel(75), iMaxLevel(30), iCurrentLevel(75)
	{
	iFlags = KMultiLevel | KLongLatencyGet | KLongLatencySet | KShared | KSenseNegative;
	NKern::LockSystem();
	iBlockTime = MIN_BLOCK_TIME  + Kern::Random() % MAX_BLOCK_TIME;
	NKern::UnlockSystem();
	}

_LIT(KDMLSHLGLSCDependResource, "DMLSHLGLSCDependResource");
DMLSHLGLSCDependResource::DMLSHLGLSCDependResource() : DStaticPowerResourceD(KDMLSHLGLSCDependResource, -100), iMinLevel(-100), iMaxLevel(100), iCurrentLevel(-100)
	{
	iFlags = KMultiLevel;
	NKern::LockSystem();
	iBlockTime = MIN_BLOCK_TIME + Kern::Random() % MAX_BLOCK_TIME;
	NKern::UnlockSystem();
	}

//Implementation of handle dependent state for all resources
TChangePropagationStatus DMLSLGLSPDependResource::TranslateDependentState(TInt /*aDepId*/, TInt /*aDepState*/, TInt& /*aResState*/)
	{
	//This resources changes only when the client asks to change
	return ENoChange;
	}

TChangePropagationStatus DMLSIGLSNDependResource::TranslateDependentState(TInt /*aDepId*/, TInt aDepState, TInt& aResState)
	{
	if(aDepState == -100)
		{
		aResState = iMinLevel;
		return EChange;
		}
	if(aDepState < 0)
		{
		aResState = iCurrentLevel > -15 ? iCurrentLevel-1 : -15;
		if(aResState == iCurrentLevel)
			return ENoChange;
		return EChange;
		}
	aResState = iCurrentLevel > iMaxLevel ? iCurrentLevel-1 : iMaxLevel;
	if(aResState == iCurrentLevel)
		return ENoChange;
	return EChange;
	}

TChangePropagationStatus DBSIGLSPDependResource::TranslateDependentState(TInt /*aDepId*/, TInt aDepState, TInt& aResState)
	{
	if(aDepState == -100)
		{
		aResState = iMinLevel;
		return EChange;
		}
	if(iCurrentLevel == E_ON)
		return ENoChange;
	aResState = iMaxLevel;
	return EChange;
	}

TChangePropagationStatus DMLSHIGLSPDependResource::TranslateDependentState(TInt aDepId, TInt aDepState, TInt& aResState)
	{
	if((aDepId & ID_INDEX_BIT_MASK) != 0x1)
		return ENoChange;
	if(aDepState == -100)
		{
		aResState = iMinLevel;
		return EChange;
		}
	aResState = iCurrentLevel + 3;
	if(aResState == iMaxLevel)
		return ENoChange;
	return EChange;
	}

TChangePropagationStatus DBSHLGLSNDependResource::TranslateDependentState(TInt /*aDepId*/, TInt aDepState, TInt& aResState)
	{
	if(aDepState == 10)
		{
		aResState = iMinLevel;
		return EChange;
		}
	if(iCurrentLevel == iMaxLevel)
		return ENoChange;
	aResState = iMaxLevel;
	return EChange;
	}

TChangePropagationStatus DMLSHLGLSNDependResource::TranslateDependentState(TInt /*aDepId*/, TInt aDepState, TInt& aResState)
	{
	if(aDepState == 10)
		{
		aResState = iMinLevel;
		return EChange;
		}
	aResState = iCurrentLevel - 2;
	if(aResState == iMaxLevel)
		return ENoChange;
	return EChange;
	}

TChangePropagationStatus DMLSHLGLSCDependResource::TranslateDependentState(TInt /*aDepId*/, TInt aDepState, TInt& aResState)
	{
	if((aDepState == 10) && (iCurrentLevel == -100))
		return ENoChange;
	if(aDepState == 10)
		{
		aResState = -100;
		return EChange;
		}
	if(iCurrentLevel)
		aResState = iCurrentLevel + 20;
	else
		aResState = iCurrentLevel -20;
	if(aResState > iMaxLevel)
		aResState = iMaxLevel;
	return EChange;
	}

	
//Implementation of DoRequest functionality for all resource
TInt DMLSLGLSPDependResource::DoRequest(TPowerRequest& req)
	{
	return GetControllerPtr()->ProcessEventResources(req, iCurrentLevel, iMaxLevel, iMinLevel, iDefaultLevel, iBlockTime);
	}

TInt DMLSIGLSNDependResource::DoRequest(TPowerRequest& req)
	{
	return GetControllerPtr()->ProcessEventResources(req, iCurrentLevel, iMaxLevel, iMinLevel, iDefaultLevel, iBlockTime);
	}

TInt DBSIGLSPDependResource::DoRequest(TPowerRequest& req)
	{
	return GetControllerPtr()->ProcessEventResources(req, iCurrentLevel, iMaxLevel, iMinLevel, iDefaultLevel, iBlockTime);
	}

TInt DMLSHIGLSPDependResource::DoRequest(TPowerRequest& req)
	{
	return GetControllerPtr()->ProcessEventResources(req, iCurrentLevel, iMaxLevel, iMinLevel, iDefaultLevel, iBlockTime);
	}

TInt DBSHLGLSNDependResource::DoRequest(TPowerRequest& req)
	{
	return GetControllerPtr()->ProcessEventResources(req, iCurrentLevel, iMaxLevel, iMinLevel, iDefaultLevel, iBlockTime);
	}

TInt DMLSHLGLSNDependResource::DoRequest(TPowerRequest& req)
	{
	return GetControllerPtr()->ProcessEventResources(req, iCurrentLevel, iMaxLevel, iMinLevel, iDefaultLevel, iBlockTime);
	}

TInt DMLSHLGLSCDependResource::DoRequest(TPowerRequest& req)
	{
	return GetControllerPtr()->ProcessEventResources(req, iCurrentLevel, iMaxLevel, iMinLevel, iDefaultLevel, iBlockTime);
	}

//Get info implementation of all resources.
TInt DMLSLGLSPDependResource::GetInfo(TDes8* info) const
	{
	DStaticPowerResource::GetInfo((TDes8*)info);
	TPowerResourceInfoV01 *buf1 = (TPowerResourceInfoV01*)info;
	buf1->iMinLevel = iMinLevel;
	buf1->iMaxLevel = iMaxLevel;
	return KErrNone;
	}

TInt DMLSIGLSNDependResource::GetInfo(TDes8* info) const
	{
	DStaticPowerResource::GetInfo((TDes8*)info);
	TPowerResourceInfoV01 *buf1 = (TPowerResourceInfoV01*)info;
	buf1->iMinLevel = iMinLevel;
	buf1->iMaxLevel = iMaxLevel;
	return KErrNone;
	}

TInt DBSIGLSPDependResource::GetInfo(TDes8* info) const
	{
	DStaticPowerResource::GetInfo((TDes8*)info);
	TPowerResourceInfoV01 *buf1 = (TPowerResourceInfoV01*)info;
	buf1->iMinLevel = iMinLevel;
	buf1->iMaxLevel = iMaxLevel;
	return KErrNone;
	}

TInt DMLSHIGLSPDependResource::GetInfo(TDes8* info) const
	{
	DStaticPowerResource::GetInfo((TDes8*)info);
	TPowerResourceInfoV01 *buf1 = (TPowerResourceInfoV01*)info;
	buf1->iMinLevel = iMinLevel;
	buf1->iMaxLevel = iMaxLevel;
	return KErrNone;
	}

TInt DBSHLGLSNDependResource::GetInfo(TDes8* info) const
	{
	DStaticPowerResource::GetInfo((TDes8*)info);
	TPowerResourceInfoV01 *buf1 = (TPowerResourceInfoV01*)info;
	buf1->iMinLevel = iMinLevel;
	buf1->iMaxLevel = iMaxLevel;
	return KErrNone;
	}

TInt DMLSHLGLSNDependResource::GetInfo(TDes8* info) const
	{
	DStaticPowerResource::GetInfo((TDes8*)info);
	TPowerResourceInfoV01 *buf1 = (TPowerResourceInfoV01*)info;
	buf1->iMinLevel = iMinLevel;
	buf1->iMaxLevel = iMaxLevel;
	return KErrNone;
	}

TInt DMLSHLGLSCDependResource::GetInfo(TDes8* info) const
	{
	DStaticPowerResource::GetInfo((TDes8*)info);
	TPowerResourceInfoV01 *buf1 = (TPowerResourceInfoV01*)info;
	buf1->iMinLevel = iMinLevel;
	buf1->iMaxLevel = iMaxLevel;
	return KErrNone;
	}
