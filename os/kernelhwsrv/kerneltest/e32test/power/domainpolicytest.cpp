// Copyright (c) 2004-2009 Nokia Corporation and/or its subsidiary(-ies).
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
//

#include <domainpolicy.h>
#include "domainpolicytest.h"

const TInt KDomainTimeout = 2000000; /* 2000ms */

static const TDmDomainSpec DomainHierarchy[] = 
	{
		{ KDmIdRoot,	KDmIdNone,	_INIT_SECURITY_POLICY_C1(ECapabilityWriteDeviceData),		EStartupCriticalStatic,	KDomainTimeout	},

		// row 1		
		{ KDmIdTestA,	KDmIdRoot,	_INIT_SECURITY_POLICY_PASS,									EStartupCriticalStatic,	KDomainTimeout	},
		{ KDmIdTestB,	KDmIdRoot,	_INIT_SECURITY_POLICY_PASS,									EStartupCriticalStatic,	KDomainTimeout	},
		{ KDmIdTestC,	KDmIdRoot,	_INIT_SECURITY_POLICY_PASS,									EStartupCriticalStatic,	KDomainTimeout	},

		// row 2
		{ KDmIdTestAA,	KDmIdTestA,	_INIT_SECURITY_POLICY_PASS,									EStartupCriticalStatic,	KDomainTimeout	},
		{ KDmIdTestAB,	KDmIdTestA,	_INIT_SECURITY_POLICY_PASS,									EStartupCriticalStatic,	KDomainTimeout	},
		{ KDmIdTestBA,	KDmIdTestB,	_INIT_SECURITY_POLICY_PASS,									EStartupCriticalStatic,	KDomainTimeout	},
		{ KDmIdTestCA,	KDmIdTestC,	_INIT_SECURITY_POLICY_PASS,									EStartupCriticalStatic,	KDomainTimeout	},
		
		// row 3
		{ KDmIdTestABA,	KDmIdTestAB,_INIT_SECURITY_POLICY_PASS,									EStartupCriticalStatic,	KDomainTimeout	},
		{ KDmIdTestABB,	KDmIdTestAB,_INIT_SECURITY_POLICY_PASS,									EStartupCriticalStatic,	KDomainTimeout	},
		{ KDmIdTestCAA,	KDmIdTestCA,_INIT_SECURITY_POLICY_PASS,									EStartupCriticalStatic,	KDomainTimeout	},

		// end of array marker
		{ KDmIdNone,	KDmIdNone,	_INIT_SECURITY_POLICY_PASS,	0,	0	}
	};
static const TDmHierarchyPolicy HierarchyPolicy	= 
	{ETraverseParentsFirst, ETraverseChildrenFirst, ETransitionFailureStop};
	


/**
Gets access to the test hierarchy specification.

The domain hierarchy specification is a simple array of TDmDomainSpec items.

The default implementation provided by Symbian OS just returns a pointer to
the domain hierarchy specification array.

@return A pointer to the domain hierarchy specification array.
*/
EXPORT_C const TDmDomainSpec* DmPolicy::GetDomainSpecs()
	{
	return (TDmDomainSpec*) DomainHierarchy;
	}




/**
Releases access to the specified domain hierarchy specification.

The domain hierarchy specification is a simple array of TDmDomainSpec items.

As the default Symbian OS implementation of GetDomainSpecs() just returns
a pointer to the domain hierarchy specification array, then the default
implementation of Release() is empty. The API is provided to permit
more complex implementations, if required.

@param aDomainSpec A pointer to the domain hierarchy specification array.
*/
EXPORT_C void DmPolicy::Release(const TDmDomainSpec* /*aDomainSpec*/)
	{
	}


/**
Retrieves the domain hierarchy policy 

@param	aPolicy a client-supplied policy which on exit
		will contain a copy of the policy for the requested domain hierarchy id.

  
@return	KErrNone
*/
EXPORT_C TInt DmPolicy::GetPolicy(TDmHierarchyPolicy& aPolicy)
	{
	aPolicy = HierarchyPolicy;
	return KErrNone;
	}

