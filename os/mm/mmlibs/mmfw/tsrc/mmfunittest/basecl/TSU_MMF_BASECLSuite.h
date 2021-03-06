// Copyright (c) 2002-2009 Nokia Corporation and/or its subsidiary(-ies).
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
// This contains CTestSuite_MMF_BASECL 
// 
//

#ifndef __TSU_MMF_BASECL_SUITE_H_
#define __TSU_MMF_BASECL_SUITE_H_

#include <testframework.h>

class CTestStep_MMF_BASECL; //forward reference

/**
 *
 * CTestSuite_MMF_BASECL, which is the base class for 
 * the baseclasses MMF_BASECL suite.
 *
 * @lib "TSU_MMF_BASECL.lib"
 *
 * @xxxx
 *
 */
class  CTestSuite_MMF_BASECL : public CTestSuite
	{
public:
	
	void InitialiseL( void );
	virtual ~CTestSuite_MMF_BASECL();
	virtual TPtrC GetVersion( void ) const;

private:

	};


#endif /* TSU_MMF_BASECL_SUITE_H_ */
