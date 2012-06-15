// Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).
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
// FBRENDERFACTORY.CPP
// 
//

#include "fbrenderstagefactory.h"
#include "fbrenderstage.h"

_LIT(KFbRenderStageName, "flickerbuffer");

CFbRenderStageFactory* CFbRenderStageFactory::CreateL()
	{
	return new(ELeave) CFbRenderStageFactory;
	}
	
void CFbRenderStageFactory::ConstructL(MWsGraphicDrawerEnvironment& aEnv,const TDesC8& /*aData*/)
	{
	BaseConstructL(aEnv);
	}
	
CFbRenderStageFactory::CFbRenderStageFactory()
	{
	}
	
CFbRenderStageFactory::~CFbRenderStageFactory()
	{
	}

/**
Overidding MWsObjectProvider
*/
TAny* CFbRenderStageFactory::ResolveObjectInterface(TUint aTypeId)
	{
	switch (aTypeId)
		{
		case KMWsRenderStageFactory:
			return static_cast<MWsRenderStageFactory*>(this);
		}

	return NULL;
	}

CWsRenderStage * CFbRenderStageFactory::CreateStageL(MWsScreen * aScreen, MWsScreenRedraw * aScreenRedraw)
	{
	CFbRenderStage * stage = CFbRenderStage::NewL(&Env(), aScreen, aScreenRedraw);
	return stage;
	}

const TDesC& CFbRenderStageFactory::PluginName() const
	{
	return KFbRenderStageName;
	}
