// Copyright (c) 1995-2009 Nokia Corporation and/or its subsidiary(-ies).
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
// e32test\dll\t_dll1.cpp
// 
//

#include "t_dll.h"

const TInt KHeapSize=0x2000;

class TlsData
	{
public:
	TlsData() : iData(0x12345678),iTest1(0),iTest2(0),iServer(0) {}
	TInt Data() {return(iData);}
	void SetData(TInt aValue) {iData=aValue;}
public:
	TInt iData;
	TUint iTest1;
	TUint iTest2;
	TUint iServer;
	RSemaphore iSem;
	};
inline TlsData& Tls()
	{return(*((TlsData*)Dll::Tls()));}

EXPORT_C TInt TestDll1::Data()
//
// Return the current TLS data value
//
	{

	return(Tls().Data());
	}

EXPORT_C void TestDll1::SetData(TInt aValue)
//
// Return the current TLS data value
//
	{

	Tls().SetData(aValue);
	}

EXPORT_C TUint TestDll1::Test1()
//
// Return the test1 result.
//
	{

	return(Tls().iTest1);
	}

EXPORT_C TUint TestDll1::Test2()
//
// Return the test2 result.
//
	{

	return(Tls().iTest2);
	}

EXPORT_C TUint TestDll1::Server()
//
// Return the server result.
//
	{

	return(Tls().iServer);
	}

EXPORT_C RSemaphore TestDll1::Sem()
//
// Return the semaphore.
//
	{

	return(Tls().iSem);
	}

EXPORT_C TInt TestDll1::Attach(TBool aAttach)
	{
	TInt r = KErrNone;
	TlsData* pD;
	if (aAttach)
		{
		pD = new TlsData;
		r = Dll::SetTls(pD);
		__ASSERT_ALWAYS(r==KErrNone, User::Panic(_L("T_DLL1 att"),r));
		}
	else
		{
		delete &Tls();
		r = Dll::SetTls(NULL);
		__ASSERT_ALWAYS(r==KErrNone, User::Panic(_L("T_DLL1 det"),r));
		}
	return r;
    }



EXPORT_C TInt TestDll1::GlobalAlloc(TInt )
	{
	return KErrNone;
	}
EXPORT_C TBool TestDll1::GlobalAllocated()
	{
	return KErrNone;
	}
EXPORT_C TInt TestDll1::GlobalRead(TInt ,TDes8 &)
	{
	return KErrNone;
	}
EXPORT_C TInt TestDll1::GlobalWrite(TInt ,const TDesC8 &)
	{
	return KErrNone;
	}

