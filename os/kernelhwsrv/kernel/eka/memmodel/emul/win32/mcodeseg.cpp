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
// e32\memmodel\emul\win32\mcodeseg.cpp
// 
//

#define __INCLUDE_ALL_SUPPORTED_CAPABILITIES__
#include "memmodel.h"
#include <property.h>
#include <emulator.h>
#include <wchar.h>

#define __CHECKHEAP()	((RHeapK*)K::Allocator)->Check()

static TBool BootComplete;	// flag set when boot EXE is loaded as an EPOC process
static DWin32CodeSeg* KernelCodeSeg;
static HMODULE KernelModuleHandle;

GLREF_C TInt CompareCodeSegsByName(const DCodeSeg& aA, const DCodeSeg& aB);

void TruncToNarrow(TUint8* aDes, const TUint16* aSrc, int aLen)
	{
	//converts to narrow by dumping the high order byte
	if(aDes && aSrc)
		{
		if(aLen==-1)
			{
			for(int index=0;*aSrc;index++)
				*aDes++ = (TUint8)*aSrc++;
			*aDes = '\0';
			}
		else
			for(int index=0;index<aLen;index++)
				*aDes++ = (TUint8)*aSrc++;
		}
	}

void ExpandToWide(TUint16* aDes, const TUint8* aSrc, int aLen)
	{
	if(aDes && aSrc)
		{
		for(int index=0;index<aLen;index++)
			*aDes++ = (TUint16)*aSrc++;
		}
	}

#ifdef KDLL
void DumpWin32CodeSeg(DWin32CodeSeg& a)
	{
	const TUint32* uid=(const TUint32*)&a.iUids;
	Kern::Printf("DWin32CodeSeg@%08x:",&a);
	Kern::Printf("iLink = %08x %08x", a.iLink.iNext, a.iLink.iPrev);
	Kern::Printf("iTempLink = %08x %08x", a.iTempLink.iNext, a.iTempLink.iPrev);
	Kern::Printf("iGbgLink = %08x %08x", a.iGbgLink.iNext, a.iGbgLink.iPrev);
	Kern::Printf("iAccessCount = %d", a.iAccessCount);
	Kern::Printf("iEntryPtVeneer = %08x", a.iEntryPtVeneer);
	Kern::Printf("iFileEntryPoint = %08x", a.iFileEntryPoint);
	Kern::Printf("iFileName = %lS", a.iFileName);
	Kern::Printf("iRootName = %lS", &a.iRootName);
	Kern::Printf("iExtOffset = %d", a.iExtOffset);
	Kern::Printf("iModuleVersion = %08x", a.iModuleVersion);
	Kern::Printf("iUids = %08x %08x %08x", uid[0], uid[1], uid[2]);
	Kern::Printf("iDepCount=%d iDeps=%08x", a.iDepCount, a.iDeps);
	Kern::Printf("iNextDep=%d, iMark=%08x, iAttr=%08x", a.iNextDep, a.iMark, a.iAttr);
	Kern::Printf("iExeCodeSeg=%08x, iAttachProcess=%O", a.iExeCodeSeg, a.iAttachProcess);
	Kern::Printf("iWinInstance = %08x", a.iWinInstance);
	Kern::Printf("iModuleHandle = %08x", a.iModuleHandle);
	Kern::Printf("iModuleFile = %08x", a.iModuleFile);
	Kern::Printf("iAlwaysLoaded = %08x", a.iAlwaysLoaded);
	Kern::Printf("iModuleList = %08x", a.iModuleList);
	}
#endif

void ExecInEntryPoint()
	{
	// This function will be called if an Exec:: call is attempted in the entry
	// point of a DLL which didn't specify 'epocallowdlldata'
	Kern::Fault("BAD DLL STATIC",0);
	}

static const SFastExecTable DummyFastExecTable={0,{0}};
static const SSlowExecTable DummySlowExecTable={0,(TLinAddr)ExecInEntryPoint,0,{{0,0}}};

// Force a call to a DLL entry point to initialise any gratuitous writeable data
// generated by the compiler.
// Change exec table to prevent any executive calls in case someone forgot to
// use the 'epocallowdlldata' option.
void ForceEntryPointCall(TLinAddr aEntryPoint)
	{
	__KTRACE_OPT(KDLL,Kern::Printf("ForceEntryPoint %08x", aEntryPoint));
	NKern::Lock();
	NThread* nthrd = NKern::CurrentThread();
	const SFastExecTable* fast = nthrd->iFastExecTable;
	const SSlowExecEntry* slow = nthrd->iSlowExecTable;
	nthrd->iFastExecTable = (SFastExecTable*)&DummyFastExecTable;
	nthrd->iSlowExecTable = DummySlowExecTable.iEntries;
	TLibraryEntry ep = (TLibraryEntry)aEntryPoint;
	(*ep)(KModuleEntryReasonProcessAttach);
	nthrd->iFastExecTable = fast;
	nthrd->iSlowExecTable = slow;
	NKern::Unlock();
	}

class TWin32Filename
	{
public:
	inline TWin32Filename()
		{iBuf[0] = '\0';}
	inline TWin32Filename(const TDesC& aFilename)
		{Set(aFilename);}
	inline TWin32Filename& operator=(const TDesC& aFilename)
		{Set(aFilename);return *this;}
	inline operator LPCWSTR() const
		{return iBuf;}
private:
	void Set(const TDesC& aFilename);
private:
	WCHAR iBuf[KMaxFileName + 1];
	};

void TWin32Filename::Set(const TDesC& aFilename)
	{
	TUint16 fileName[KMaxFileName];
	ExpandToWide(fileName, aFilename.Ptr(), aFilename.Length());
	TPtr p((TUint8*)iBuf,KMaxFileName<<1);
	Property::MapFilename(p, TPtrC((TUint8*)fileName,aFilename.Length()<<1));
	iBuf[p.Length()>>1] = '\0';
	}

class DModuleList : public DBase
	{
public:
	enum TFlags
		{
		EFlagData=0x01,
		EFlagDataPresent=0x02,
		EFlagAlreadyLoaded=0x04,
		EFlagMark=0x08,
		EFlagExe=0x10,
		EFlagAlwaysLoaded=0x20,
		EFlagExeInTree=0x40,
		};

	struct SModule
		{
		TLinAddr iModuleHandle;
		DWin32CodeSeg* iCodeSeg;
		TUint32 iFlags;
		SSecurityInfo iS;
		TUint32 iModuleVersion;
		TInt iDepCount;
		TInt* iDepList;
		};
public:
	DModuleList(DWin32CodeSeg* aMainSeg, DProcess* aProcess);
	virtual ~DModuleList();
	TInt Find(TInt aModule);
	TInt GatherDependencies(HMODULE aModule);
	TInt CompileDepLists();
	void SetFlags();
	TUint32 Flags(SModule* aM);
	TInt LoadDependencies();
	void LinkDependencies();
	TInt Loaded();
public:
	DWin32CodeSeg* iMain;
	DProcess* iAttachProcess;
	RArray<SModule> iModules;
	};

DModuleList::DModuleList(DWin32CodeSeg* aMainSeg, DProcess* aProcess)
	:	iMain(aMainSeg), iAttachProcess(aProcess),
		iModules(8, _FOFF(SModule,iModuleHandle), 2*256)
	{
	}

DModuleList::~DModuleList()
	{
	TInt c = iModules.Count();
	if (c)
		{
		SModule* pM = (SModule*)&iModules[0];
		SModule* pE = pM + c;
		for (; pM<pE; ++pM)
			{
			Kern::Free(pM->iDepList);
			}
		}
	iModules.Close();
	}

TInt DModuleList::Find(TInt aModule)
	{
	SModule mod;
	mod.iModuleHandle=(TLinAddr)aModule;
	return iModules.FindInUnsignedKeyOrder(mod);
	}
/**
Populate iModules with SModule entries corresponding to supplied HModule and all of its dependencies,
direct and indirect.  

@param aModule A Windows module handle
@return KErrNone if successful
*/
TInt DModuleList::GatherDependencies(HMODULE aModule)
	{
	__KTRACE_OPT(KDLL, Kern::Printf(">DModuleList::GatherDependencies %08x %08x %C", aModule, iMain, iMain));
	Emulator::TModule etm(aModule);
	SModule mod;
	mod.iModuleHandle=(TLinAddr)aModule;
	mod.iCodeSeg=NULL;
	mod.iDepList=NULL;

	TUint32 uid[3];
	TUidType& uidref=*(TUidType*)uid;
	const TEmulatorImageHeader* hdr = (const TEmulatorImageHeader*)etm.Section(KWin32SectionName_Symbian);
	if (!hdr)
		return KErrNotSupported;
	__KTRACE_OPT(KDLL,Kern::Printf("Module flags %08x, cap %08x %08x, sid %08x", hdr->iFlags, hdr->iS.iCaps[1], hdr->iS.iCaps[0], hdr->iS.iSecureId));
	mod.iS = hdr->iS;
	// Overide capabilities in image
	const SCapabilitySet& disabledCaps = *(SCapabilitySet*)&TheSuperPage().iDisabledCapabilities;
	for(TInt i=0; i<SCapabilitySet::ENCapW; i++)
		{
		mod.iS.iCaps[i] |= disabledCaps[i];
		mod.iS.iCaps[i] &= AllSupportedCapabilities[i];
		}
	mod.iModuleVersion = hdr->iModuleVersion;
	mod.iFlags = (hdr->iFlags & KEmulatorImageFlagAllowDllData) ? EFlagData|EFlagDataPresent : 0;
	uidref = *(const TUidType*)&hdr->iUids[0];
	TBool isExe = (uid[0]==KExecutableImageUidValue);
	TBool data_section_present = (etm.Section(KWin32SectionName_EpocData) != NULL);
	if (data_section_present && isExe)
		mod.iFlags = EFlagData|EFlagDataPresent;
	__KTRACE_OPT(KDLL,Kern::Printf("DataSectionPresent = %x", data_section_present));

	// Can't do this for CodeWarrior since it sticks everything in .data, even constants
	// On VC6 this check works for Base code, but not for some other code.
//	if (data_section_present && !isExe && !(hdr->iFlags & KEmulatorImageFlagAllowDllData))
//		return KErrNotSupported;

	if (!data_section_present)
		mod.iFlags = 0;
	if (isExe)
		mod.iFlags |= EFlagExe|EFlagExeInTree;

	SWin32Module w32m;
	w32m.iWin32ModuleHandle=(TLinAddr)aModule;
	w32m.iCodeSeg=NULL;
	TInt ix=MM::Win32Modules.FindInUnsignedKeyOrder(w32m);
	__KTRACE_OPT(KDLL, Kern::Printf("GlobalIx=%d", ix));
	if ((ix>=0 && !MM::Win32Modules[ix].iCodeSeg) || aModule==KernelModuleHandle)
		mod.iFlags|=EFlagAlwaysLoaded;
	TInt r=iModules.InsertInUnsignedKeyOrder(mod);
	__KTRACE_OPT(KDLL, Kern::Printf("Flags=%08x, InsertRes=%d", mod.iFlags, r));
	if (r==KErrAlreadyExists)
		return KErrNone;
	if (r!=KErrNone)
		return r;
	if (mod.iFlags & EFlagAlwaysLoaded)
		return KErrNone;	// stop if we reach an extension or variant
	if (data_section_present && (mod.iFlags & EFlagData)==0)
		{
		// DLL, .data present but 'epocallowdlldata' not specified
		// - either the compiler forgetting that something's constant or
		// developer forgetting to specify 'epocallowdlldata'.
		// call entry point from here to keep the compiler happy
		const char* entry_name = "_E32Dll" ;
		TLinAddr ep = (TLinAddr)Emulator::GetProcAddress(aModule, entry_name);
		__NK_ASSERT_ALWAYS(ep!=0);
		ForceEntryPointCall(ep);
		}

	const IMAGE_IMPORT_DESCRIPTOR* imports = etm.Imports();
	if (imports)
		{
	    for (; imports->Characteristics; ++imports )
			{
			__KTRACE_OPT(KDLL, { TPtrC8 n((const TText8*)etm.Translate(imports->Name)); Kern::Printf("Importing from %S", &n);} );
			Emulator::TModule imp_etm((PCSTR)etm.Translate(imports->Name));

			if (!imp_etm.IsValid())
				{
				__KTRACE_OPT(KDLL, Kern::Printf("Spurious import"));
				continue;
				}

			if (imp_etm.Section(KWin32SectionName_Symbian) == 0)
				{
				__KTRACE_OPT(KDLL, Kern::Printf("Not EPOC Module"));
				continue;		// not an epoc DLL
				}

			r=GatherDependencies(imp_etm.iModule);
			if (r!=KErrNone)
				return r;
			}
		}
	__KTRACE_OPT(KDLL, Kern::Printf("<DModuleList::GatherDependencies %08x %08x %C", aModule, iMain, iMain));
	return KErrNone;
	}

#ifdef __VC32__
#pragma warning( disable : 4701 )   // disable warning C4701: local variable 'missingCaps' may be used without having been initialized
#endif

/**
Iterate through imports (if any) of each SModule in list. Populate iDepList with the indicies (in iModules)
of directly required modules. Checks that dependencies are legal under platsec.
@return KErrNone on success.
*/
TInt DModuleList::CompileDepLists()
	{
	__KTRACE_OPT(KDLL, Kern::Printf(">DModuleList::CompileDepLists"));
	TInt result=KErrNone;
	TInt c=iModules.Count();
	SModule* pM=(SModule*)&iModules[0];
	SModule* pE=pM+c;
	for (; pM<pE; ++pM)
		{
		pM->iDepList=(TInt*)Kern::Alloc(c*sizeof(TInt));
		if (!pM->iDepList)
			return KErrNoMemory;
		memset(pM->iDepList, 0xff, c*sizeof(TInt));
		Emulator::TModule etm((HINSTANCE)pM->iModuleHandle);
		const IMAGE_IMPORT_DESCRIPTOR* imports = etm.Imports();
		__KTRACE_OPT(KDLL, Kern::Printf("Module %08x, imports=%08x flags=%08x", pM->iModuleHandle, imports, pM->iFlags));
		if (!imports || (pM->iFlags&EFlagAlwaysLoaded))
			continue;

		const SCapabilitySet& cap = pM->iS.iCaps;
		const SCapabilitySet& disabledCaps=*(SCapabilitySet*)&TheSuperPage().iDisabledCapabilities;

		TInt* pI=pM->iDepList;
		for (; imports->Characteristics; ++imports )
			{
			Emulator::TModule imp_etm((PCSTR)etm.Translate(imports->Name));

			if (!imp_etm.IsValid())
				continue;		// spurious import

			if (imp_etm.Section(KWin32SectionName_Symbian) == 0)
				continue;		// not an epoc DLL

			TInt ix=Find((TInt)imp_etm.iModule);
			__ASSERT_ALWAYS(ix>=0, MM::Panic(MM::ECompileDepLists));
			*pI++=ix;
			__KTRACE_OPT(KDLL, Kern::Printf("Depends on %08x, ix=%d", imp_etm.iModule, ix));

			const SModule& dep = iModules[ix];
			
			SCapabilitySet missingCaps;
			SCapabilitySet depCaps = dep.iS.iCaps;
			TUint32 checkFail = 0;
			for (TInt i=0; i<SCapabilitySet::ENCapW; ++i)
				{
				depCaps[i] = dep.iS.iCaps[i]|disabledCaps[i];
				TUint32 missing = cap[i] & ~depCaps[i];
				missingCaps[i] = missing;
				checkFail |= missing;
				}

			__KTRACE_OPT(KDLL, Kern::Printf("Cap=%08x %08x, DepCap=%08x %08x", cap[1], cap[0], depCaps[1], depCaps[0]));

			if (checkFail)
				{
				TUint8	mainBuf8[MAX_PATH];
				TInt	mainlen;
				{
				TUint16 mainBuf[MAX_PATH];
				mainlen=Emulator::GetModuleFileName(reinterpret_cast<HMODULE>(pM->iModuleHandle),(LPWSTR)mainBuf);
				TruncToNarrow(mainBuf8, mainBuf, mainlen);
				}
				TPtrC mainp(mainBuf8,mainlen);
				TInt ix=mainp.LocateReverse('\\');
				if (ix>=0)
					mainp.Set(mainp.Mid(ix+1));
				if (mainp.Length() && mainp[mainp.Length()-1]==')' && (ix=mainp.LocateReverse('('))>=0)
					mainp.Set(mainp.Left(ix));

				TUint8	depBuf8[MAX_PATH];
				TInt	deplen;
				{
				TUint16 depBuf[MAX_PATH];
				deplen=Emulator::GetModuleFileName(reinterpret_cast<HMODULE>(dep.iModuleHandle),(LPWSTR)depBuf);
				TruncToNarrow(depBuf8, depBuf, deplen);
				}
				TPtrC depp(depBuf8,deplen);
				if ((ix=depp.LocateReverse('\\'))>=0)
					depp.Set(depp.Mid(ix+1));
#ifndef __REMOVE_PLATSEC_DIAGNOSTICS__
				TInt r = PlatSec::LoaderCapabilityViolation(mainp, depp, missingCaps);
#else //__REMOVE_PLATSEC_DIAGNOSTICS__
				TInt r = PlatSec::EmitDiagnostic();
#endif // !__REMOVE_PLATSEC_DIAGNOSTICS__
				if(r!=KErrNone)
					result=r;
				}
			}
		}
	return result;
	}

#ifdef __VC32__
#pragma warning( default : 4701 )   // enable warning C4701: local variable 'missingCaps' may be used without having been initialized
#endif

void DModuleList::SetFlags()
	{
	__KTRACE_OPT(KDLL, Kern::Printf("DModuleList::SetFlags"));
	TInt c=iModules.Count();
	SModule* pM=(SModule*)&iModules[0];
	SModule* pE=pM+c;
	for (; pM<pE; ++pM)
		{
		SModule* pM2=(SModule*)&iModules[0];
		for (; pM2<pE; ++pM2)
			pM2->iFlags &= ~EFlagMark;
		TUint32 flags=Flags(pM);
		if (flags & EFlagData)
			pM->iFlags|=EFlagDataPresent;
		if (flags & EFlagExe)
			pM->iFlags|=EFlagExeInTree;
		__KTRACE_OPT(KDLL, Kern::Printf("Module %08x Flags=%08x", pM->iModuleHandle, pM->iFlags));
		}
	pM=(SModule*)&iModules[0];
	for (; pM<pE; ++pM)
		{	
		TInt dc=0;
		const TInt* pI=pM->iDepList;
		const TInt* pIE=pI+c;
		while (pI<pIE)
			{
			TInt ix=*pI++;
			if (ix<0)
				break;
			TUint32 mflags=iModules[ix].iFlags;
			if ( (mflags&(EFlagDataPresent|EFlagExeInTree)) && !(mflags&EFlagAlwaysLoaded))
				++dc;
			}
		pM->iDepCount=dc;
		__KTRACE_OPT(KDLL, Kern::Printf("Module %08x DepCount=%d", pM->iModuleHandle, pM->iDepCount));
		}
	}

TUint32 DModuleList::Flags(SModule* aM)
	{
	TUint32 flags=aM->iFlags;
	__KTRACE_OPT(KDLL, Kern::Printf(">DModuleList::Flags %08x", aM->iModuleHandle));
	if ((aM->iFlags & EFlagMark)==0)
		{
		aM->iFlags |= EFlagMark;
		const TInt* pI=aM->iDepList;
		const TInt* pE=pI+iModules.Count();
		while (pI<pE)
			{
			TInt ix=*pI++;
			if (ix<0)
				break;
			TUint32 mflags=Flags(&iModules[ix]);
			if (mflags&EFlagExe)
				mflags&=~EFlagData;
			flags |= mflags;
			}
		}
	__KTRACE_OPT(KDLL, Kern::Printf("<DModuleList::Flags %08x = %08x", aM->iModuleHandle, flags));
	return flags;
	}

TInt DModuleList::LoadDependencies()
	{
	__KTRACE_OPT(KDLL, Kern::Printf("DModuleList::LoadDependencies"));
	TInt c=iModules.Count();
	SModule* pM=(SModule*)&iModules[0];
	SModule* pE=pM+c;
	for (; pM<pE; ++pM)
		{
		DWin32CodeSeg* pS;
		TUint32 flags=pM->iFlags;
		__KTRACE_OPT(KDLL, Kern::Printf("ModuleHandle %08x flags %08x", pM->iModuleHandle, flags));
		if (!(flags & (EFlagDataPresent|EFlagExeInTree)) || (flags&EFlagAlwaysLoaded))
			continue;
		SWin32Module w32m;
		w32m.iWin32ModuleHandle=(TLinAddr)pM->iModuleHandle;
		w32m.iCodeSeg=NULL;
		TInt ix=MM::Win32Modules.FindInUnsignedKeyOrder(w32m);
		__KTRACE_OPT(KDLL, Kern::Printf("GlobalIndex=%d", ix));
		if (ix<0 && (flags & EFlagExe))
			return KErrNotSupported;	// we never implicitly load an EXE
		if (ix>=0)
			{
			pS=MM::Win32Modules[ix].iCodeSeg;
			__KTRACE_OPT(KDLL, Kern::Printf("Already loaded; CSEG=%08x %C", pS, pS));
			if (pS==iMain)
				{
				pS->iDepCount=pM->iDepCount;
				if (pS->iDepCount)
					{
					pS->iDeps=(DCodeSeg**)Kern::AllocZ(pS->iDepCount*sizeof(DCodeSeg*));
					if (!pS->iDeps)
						return KErrNoMemory;
					}
				}
			else
				{
				if (!(pS->iMark & DCodeSeg::EMarkLdr))
					{
					pS->iMark |= DCodeSeg::EMarkLdr;
					pS->CheckedOpen();
					}
				pM->iFlags|=EFlagAlreadyLoaded;
				}
			pM->iCodeSeg=pS;
			continue;
			}
		__KTRACE_OPT(KDLL, Kern::Printf("Create new, depcount=%d", pM->iDepCount));
		pS=new DWin32CodeSeg;
		if (!pS)
			return KErrNoMemory;
		TInt r=pS->CreateAlreadyLoaded((HMODULE)pM->iModuleHandle, pM->iDepCount);
		if (r!=KErrNone)
			{
			delete pS;
			return r;
			}
		pM->iCodeSeg=pS;
		pS->iS = pM->iS;
		pS->iModuleVersion = pM->iModuleVersion;
		if (iMain->iAttr & ECodeSegAttKernel)
			pS->iAttr |= ECodeSegAttKernel;
		if (flags & EFlagData)
			{
			pS->iMark |= DCodeSeg::EMarkData|DCodeSeg::EMarkDataPresent;
			if (!(flags&EFlagExe))
				pS->iMark |= DCodeSeg::EMarkDataInit;
			}
		if ( (flags & EFlagData) || ((flags&(EFlagExe|EFlagExeInTree))==EFlagExeInTree) )
			{
			if (iAttachProcess)
				{
				pS->iExeCodeSeg = iAttachProcess->CodeSeg();
				pS->SetAttachProcess(iAttachProcess);
				}
			}
		}
	return KErrNone;
	}

void DModuleList::LinkDependencies()
	{
	__KTRACE_OPT(KDLL, Kern::Printf("DModuleList::LinkDependencies"));
	TInt c=iModules.Count();
	SModule* pM=(SModule*)&iModules[0];
	SModule* pE=pM+c;
	for (; pM<pE; ++pM)
		{
		DWin32CodeSeg* pS=pM->iCodeSeg;
		if (!pS || (pM->iFlags&EFlagAlreadyLoaded))
			continue;
		const TInt* pI=pM->iDepList;
		const TInt* pIE=pI+c;
		while (pI<pIE)
			{
			TInt ix=*pI++;
			if (ix<0)
				break;
			DWin32CodeSeg* pD=iModules[ix].iCodeSeg;
			if (pD)
				pS->iDeps[pS->iNextDep++]=pD;
			}
		}
	pM=(SModule*)&iModules[0];
	}

TInt DModuleList::Loaded()
	{
	__KTRACE_OPT(KDLL, Kern::Printf("DModuleList::Loaded()"));
	TInt c=iModules.Count();
	SModule* pM=(SModule*)&iModules[0];
	SModule* pE=pM+c;
	TInt r=KErrNone;
	for (; pM<pE && r==KErrNone; ++pM)
		{
		DWin32CodeSeg* pS=pM->iCodeSeg;
		if (pS && pS!=iMain)
			{
			pS->FinaliseRecursiveFlags();
			pS->iMark |= DCodeSeg::EMarkLoaded;
			}
		}
	if (r==KErrNone)
		{
		pM=(SModule*)&iModules[0];
		for (; pM<pE; ++pM)
			pM->iCodeSeg=NULL;	// cleanup will occur by EMarkLdr if needed
		}
	return r;
	}

TInt MakeDll(HANDLE aFile)
	{
	IMAGE_DOS_HEADER dosh;
	DWORD bytes;
	ReadFile(aFile, &dosh, sizeof(dosh), &bytes, NULL);
	if ( IMAGE_DOS_SIGNATURE != dosh.e_magic )
		return KErrArgument;
	if (dosh.e_lfarlc < sizeof(dosh))
		return KErrArgument;
	SetFilePointer(aFile,dosh.e_lfanew,NULL,FILE_BEGIN);
	IMAGE_NT_HEADERS32 peh;
	ReadFile(aFile, &peh, sizeof(peh), &bytes, NULL);
    if ( peh.Signature != IMAGE_NT_SIGNATURE )
		return KErrArgument;

	peh.FileHeader.Characteristics |= IMAGE_FILE_DLL;
	peh.OptionalHeader.AddressOfEntryPoint = 0;
	SetFilePointer(aFile,dosh.e_lfanew,NULL,FILE_BEGIN);
	WriteFile(aFile, &peh, sizeof(peh), &bytes, NULL);
	return KErrNone;
	}

wchar_t* TempModuleName(const wchar_t* aExe)
	{
	static TInt ModuleGeneration = 0;
	static TInt Time = 0;

	WCHAR ex_[KMaxFileName + 1];

	if (!Time)
		{
		SYSTEMTIME st;
		GetSystemTime(&st);
		Time = st.wSecond + st.wMinute*60 + st.wHour*3600;
		}

	const char* imagePath = Property::GetString("EmulatorImagePath");
	WCHAR* p = ex_;
	while ((*p++ = *imagePath++) != 0)
		;
	wcscpy(p-1, wcsrchr(aExe, '\\') + 1);
	p = ex_ + wcslen(ex_);
	*p++ = '(';
	_itow(Time, p, 36);
	p += wcslen(p);
	*p++ = '-';
	_itow(ModuleGeneration++, p, 10);
	p += wcslen(p);
	*p++ = ')';
	*p++ = '\0';
	wchar_t* file = (wchar_t*)Kern::Alloc((p-ex_)*sizeof(WCHAR));
	if (file)
		wcscpy(file, ex_);
	return file;
	}

DCodeSeg* M::NewCodeSeg(TCodeSegCreateInfo&)
//
// Create a new instance of this class.
//
	{

	__KTRACE_OPT(KDLL,Kern::Printf("M::NewCodeSeg"));
	return new DWin32CodeSeg;
	}

DWin32CodeSeg::DWin32CodeSeg()
//
// Constructor
//
	{
	}

DWin32CodeSeg::~DWin32CodeSeg()
//
// Destructor
//
	{
	__KTRACE_OPT(KDLL,Kern::Printf("DWin32CodeSeg::Destruct %C", this));
	DBase::Delete(iModuleList);
	if (iWinInstance)
		{
		DCodeSeg::Wait();
		SWin32Module w32m;
		w32m.iWin32ModuleHandle=(TLinAddr)iModuleHandle;
		w32m.iCodeSeg=NULL;
		TInt ix=MM::Win32Modules.FindInUnsignedKeyOrder(w32m);
		if (ix>=0)
			{
			if (iAlwaysLoaded)
				MM::Win32Modules[ix].iCodeSeg=NULL;
			else
				MM::Win32Modules.Remove(ix);
			}
		// Kernel process has to be told to unmap code seg to work round
		// a problem where failed driver code seg is not unmapped before delete
		K::TheKernelProcess->UnmapCodeSeg(this);
		Emulator::FreeLibrary(iWinInstance);
		DCodeSeg::Signal();
		}
	if (iModuleFile)
		{
		Emulator::DeleteFile(iModuleFile);
		Kern::Free(iModuleFile);
		}
	Kern::Free(iDataCopy);
	DCodeSeg::Destruct();
	}

TInt DWin32CodeSeg::DoCreate(TCodeSegCreateInfo& aInfo, DProcess* aProcess)
	{
	__KTRACE_OPT(KDLL,Kern::Printf("DWin32CodeSeg::DoCreate %C proc %O", this, aProcess));

	TBool exe=(iExeCodeSeg==this);
	if (exe && (iAttr&ECodeSegAttKernel))
		{
		// loading EKERN.EXE
		__KTRACE_OPT(KDLL,Kern::Printf("W32CodeSeg : EKERN"));
		TUint16 fileNameW[KMaxFileName];
		ExpandToWide(fileNameW, iFileName->Ptr(), iFileName->Length());
		fileNameW[iFileName->Length()] = '\0';
		iWinInstance = Emulator::LoadLibrary((LPCWSTR)fileNameW);
		__KTRACE_OPT(KDLL,Kern::Printf("W32CodeSeg : EKERN %08x", iWinInstance));
		if (!iWinInstance)
			return KErrGeneral;
		if (aInfo.iTotalDataSize)
			iMark|=EMarkData|EMarkDataPresent;
		KernelCodeSeg=this;
		KernelModuleHandle=Emulator::GetModuleHandle((LPCWSTR)fileNameW);
		return RegisterCodeSeg(KernelModuleHandle);
		}
	TWin32Filename w32fn(*iFileName);
	__KTRACE_OPT(KDLL,Kern::Printf("W32CodeSeg : w32fn=%S",iFileName))
	DWin32Process* p=(DWin32Process*)aProcess;
	TInt r=KErrNone;
	DCodeSeg::Wait();
	HMODULE h = Emulator::GetModuleHandle(w32fn);
	__KTRACE_OPT(KDLL,Kern::Printf("W32CodeSeg : h=%08x exe=%08x",h,exe));
	if (h && exe)
		{
		if (h == GetModuleHandleA(NULL))
			{
			// This was the executable that was used to run the emulator
			// so all relocations will be valid and we can use it
			__KTRACE_OPT(KDLL,Kern::Printf("W32CodeSeg : initial exe"));
			if (BootComplete)
				goto makecopy;	// we can only do this once however
			iWinInstance = Emulator::LoadLibrary(w32fn);
			BootComplete = ETrue;
			}
		else
			r=KErrAlreadyExists;		// can't load same EXEXP twice
		}
	else if (h)
		{
		// Already loaded DLL
		// Code segment doesn't exist so create it
		__KTRACE_OPT(KDLL,Kern::Printf("Already loaded, module %08x",h));
		iWinInstance = Emulator::LoadLibrary(w32fn);
		}
	else if (iAttr & ECodeSegAttHDll)
		{
		// EXE with exports (EXEXP target) - can only have one instance
		// or DLL not already loaded
		__KTRACE_OPT(KDLL,Kern::Printf("W32CodeSeg : EXEXP or new DLL"));
		iWinInstance = Emulator::LoadLibrary(w32fn);
		if (iWinInstance)
			h=Emulator::GetModuleHandle(w32fn);
		}
	else
		{
		// EXE with no exports. In order that multiple instances of the EXE may be run,
		// we make a copy of the EXE file to a name which is unique across instances,
		// set the DLL flag and load it as a DLL.
makecopy:
		__KTRACE_OPT(KDLL,Kern::Printf("W32CodeSeg : MakeCopy"));
		h=0;
		iModuleFile = TempModuleName(w32fn);
		if (!iModuleFile)
			r=KErrNoMemory;
		else if (Emulator::CopyFile(w32fn,iModuleFile,FALSE))
			{
			DWORD fattr = Emulator::GetFileAttributes(iModuleFile);
			if (fattr != 0xffffffff && Emulator::SetFileAttributes(iModuleFile, fattr & ~FILE_ATTRIBUTE_READONLY))
				{
				HANDLE f = Emulator::CreateFile(iModuleFile,GENERIC_READ|GENERIC_WRITE,0,NULL,OPEN_EXISTING,FILE_FLAG_RANDOM_ACCESS,NULL);
				if (INVALID_HANDLE_VALUE != f)
					{
					r = MakeDll(f);
					CloseHandle(f);
					if (r==KErrNone)
						{
						iWinInstance = Emulator::LoadLibrary(iModuleFile);
						if (iWinInstance)
							h=Emulator::GetModuleHandle(iModuleFile);
						}
					}
				}
			}
		}
	__KTRACE_OPT(KDLL,Kern::Printf("W32CodeSeg : line %d r=%d iWinInstance=%08x",__LINE__,r,iWinInstance));
	if (r==KErrNone && !iWinInstance)
		r=Emulator::LastError();
	__KTRACE_OPT(KDLL,Kern::Printf("W32CodeSeg : line %d r=%d",__LINE__,r));
	if (r==KErrNone)
		{
		const char* entry_name = exe ? "_E32Startup" : "_E32Dll" ;
		aInfo.iFileEntryPoint = (TLinAddr)Emulator::GetProcAddress(iWinInstance, entry_name);
		if (!aInfo.iFileEntryPoint)
         {
			r=KErrNotSupported;
         }
		else
         {
			r=RegisterCodeSeg(h);
         }

		if (r == KErrNone)
         {
			r = CopyDataBss();
         }
		}
	if (r==KErrNone && aInfo.iTotalDataSize && !iAlwaysLoaded)
		{
		iMark|=EMarkData|EMarkDataPresent;
		if (!exe)
			iMark|=EMarkDataInit;
		}

	if (r==KErrNone)
		r=ProcessImports(p);
	__KTRACE_OPT(KDLL, DumpWin32CodeSeg(*this));
	DCodeSeg::Signal();
	return r;
	}

TInt DWin32CodeSeg::RegisterCodeSeg(HMODULE aModule)
	{
	__KTRACE_OPT(KDLL, Kern::Printf("DWin32CodeSeg %C RegisterCodeSeg(%08x)", this, aModule));
	Emulator::TModule etm(aModule);
	const IMAGE_NT_HEADERS32* nth=etm.NtHeader();
	iRunAddress = (TLinAddr)aModule;
	iSize = nth->OptionalHeader.SizeOfImage;
	SWin32Module w32m;
	w32m.iWin32ModuleHandle=(TLinAddr)aModule;
	w32m.iCodeSeg=this;
	TInt r=KErrNone;
	TInt ix=MM::Win32Modules.FindInUnsignedKeyOrder(w32m);
	if (ix>=0)
		{
		SWin32Module& wmr=MM::Win32Modules[ix];
		__ASSERT_ALWAYS(!wmr.iCodeSeg, MM::Panic(MM::EInvalidSharedModule));
		wmr.iCodeSeg=this;
		iAlwaysLoaded=ETrue;
		}
	else
		r=MM::Win32Modules.InsertInUnsignedKeyOrder(w32m);
	if (r==KErrNone)
		{
		iModuleHandle=aModule;
		iCodeSegId=++MM::NextCodeSegId;
		}
	return r;
	}

TInt DWin32CodeSeg::CreateAlreadyLoaded(HMODULE aModule, TInt aDepCount)
	{
	__KTRACE_OPT(KDLL, Kern::Printf("DWin32CodeSeg %08x CreateAlreadyLoaded(%08x,%d)", this, aModule, aDepCount));
	Emulator::TModule etm(aModule);
	etm.GetUids(iUids);
	TInt32 uid1=iUids.iUid[0].iUid;
	if (uid1!=KDynamicLibraryUidValue)	// only implicitly load DLLs
		return KErrNotSupported;
	
	WCHAR name[MAX_PATH+1];
	TInt r=Emulator::GetModuleFileName(aModule, name);
	if (!r)
		return Emulator::LastError();
	TUint8 name8[MAX_PATH+1];
	TruncToNarrow(name8,(const TUint16*)name,-1);

   //SL:
   __KTRACE_OPT(KDLL, Kern::Printf("Loading library %s", name8));

	TPtrC fpptr((const TText*)name8);
	TInt slash=fpptr.LocateReverse('\\');
	TPtrC nptr=fpptr.Mid(slash+1);
	iFileName=HBuf::New(nptr);
	if (!iFileName)
		return KErrNoMemory;
	iRootName.Set(*iFileName);
	iExtOffset = iFileName->LocateReverse('.');
	if (iExtOffset==KErrNotFound)
		iExtOffset = iFileName->Length();
	iModuleVersion = 0x00010000u;
	iDepCount=aDepCount;
	if (iDepCount)
		{
		iDeps=(DCodeSeg**)Kern::AllocZ(iDepCount*sizeof(DCodeSeg*));
		if (!iDeps)
			return KErrNoMemory;
		}
	iWinInstance = Emulator::LoadLibrary(name);
	if (!iWinInstance)
		return Emulator::LastError();
	iFileEntryPoint = (TLinAddr)Emulator::GetProcAddress(iWinInstance, "_E32Dll");
	if (!iFileEntryPoint)
		return KErrNotSupported;
	iEntryPtVeneer = iFileEntryPoint;
	r=RegisterCodeSeg(aModule);
	if (r == KErrNone)
		r = CopyDataBss();
	if (r == KErrNone)
		r = CodeSegsByName.InsertInOrderAllowRepeats(this, &CompareCodeSegsByName);
	if (r == KErrNone)
		r = CodeSegsByAddress.Add(this);
	if (r==KErrNone)
		{
		GlobalList.Add(&iLink);
		iAsyncDeleteNext=this;
		}
	__KTRACE_OPT(KDLL, DumpWin32CodeSeg(*this));
	return r;
	}

TInt DWin32CodeSeg::CopyDataBss()
	{
	__KTRACE_OPT(KDLL, Kern::Printf("DWin32CodeSeg::CopyDataBss()"));

	Emulator::TModule etm(iModuleHandle);
	const TEmulatorImageHeader* hdr = (const TEmulatorImageHeader*)etm.Section(KWin32SectionName_Symbian);
	if (!(hdr->iFlags & KEmulatorImageFlagAllowDllData))
		return KErrNone;
	const IMAGE_SECTION_HEADER* data=etm.SectionHeader(KWin32SectionName_EpocData);
	const IMAGE_SECTION_HEADER* bss=etm.SectionHeader(KWin32SectionName_EpocBss);
	if (data)
		{
		iRealDataSize = data->Misc.VirtualSize;
		iDataDest = (TLinAddr)etm.Translate(data->VirtualAddress);
		iDataCopy = Kern::Alloc(iRealDataSize);
		if (!iDataCopy)
			return KErrNoMemory;
		memcpy(iDataCopy, (const TAny*)iDataDest, iRealDataSize);
		}
	if (bss)
		{
		iRealBssSize = bss->Misc.VirtualSize;
		iBssDest = (TLinAddr)etm.Translate(bss->VirtualAddress);
		}
	return KErrNone;
	}

TInt DWin32CodeSeg::ProcessImports(DProcess* aProcess)
	{
	__KTRACE_OPT(KDLL, Kern::Printf("DWin32CodeSeg::ProcessImports(%O)", aProcess));
	iModuleList=new DModuleList(this, aProcess);
	if (!iModuleList)
		return KErrNoMemory;
	TInt r=iModuleList->GatherDependencies(iModuleHandle);
	if (r!=KErrNone)
		return r;
	TInt ix=iModuleList->Find((TInt)iModuleHandle);
	DModuleList::SModule& mm=iModuleList->iModules[ix];
	r=iModuleList->CompileDepLists();
	if (r==KErrNone)
		{
		iModuleList->SetFlags();
		r=iModuleList->LoadDependencies();
		if (r==KErrNone)
			{
			if ((mm.iFlags&(DModuleList::EFlagDataPresent|DModuleList::EFlagExeInTree)) && !(mm.iFlags&DModuleList::EFlagAlwaysLoaded))
				{
				if (aProcess)
					{
					iExeCodeSeg=aProcess->CodeSeg();
					SetAttachProcess(aProcess);
					}
				}
			iModuleList->LinkDependencies();
			}
		}
	return r;
	}

TInt DWin32CodeSeg::Loaded(TCodeSegCreateInfo& aInfo)
	{
	__KTRACE_OPT(KDLL, Kern::Printf("DWin32CodeSeg::Loaded()"));
	TInt r=KErrNone;
	if (iModuleList)
		{
		r=iModuleList->Loaded();
		if (r==KErrNone)
			{
			DBase::Delete(iModuleList);
			iModuleList=NULL;
			}
		}
	if (r==KErrNone)
		r=DCodeSeg::Loaded(aInfo);
	__KTRACE_OPT(KDLL, Kern::Printf("DWin32CodeSeg::Loaded() - %d", r));
	return r;
	}

void DWin32CodeSeg::ReadExportDir(TUint32*)
	{
	// not needed
	}

TBool DWin32CodeSeg::FindCheck(DProcess* /*aProcess*/)
	{
	// We never create multiple copies of the same code segment,
	// so if code segment exists, allow it to be found.
	// CodeSegOpen will fail if it is incompatible.
	return ETrue;
	}

TBool DWin32CodeSeg::OpenCheck(DProcess* aProcess)
	{
	__KTRACE_OPT(KDLL,Kern::Printf("CSEG:%08x Compat? proc=%O",this,aProcess));
	if (!aProcess)
		return EFalse;	// never reuse EXE code segments
	return ETrue;
	}

TLibraryFunction DWin32CodeSeg::Lookup(TInt aOrdinal)
//
// Find an exported function by ordinal
//
	{
	TLibraryFunction f = NULL;
	if (aOrdinal != 0)
		{
			
		f = (TLibraryFunction)Emulator::GetProcAddress(iWinInstance, (const char *)aOrdinal);
		if ((TLinAddr)f == iFileEntryPoint)
			f = NULL;
		}
	else
		{
		// Find 0th Ordinal, which should be named symbol export data, from named data segment
		Emulator::TModule etm(iModuleHandle);
		f  = (TLibraryFunction)etm.Section(KWin32SectionName_NmdExpData);
		}
 	__KTRACE_OPT(KDLL,Kern::Printf("DWin32CodeSeg::Lookup(%d)->%08x",aOrdinal,f));
	return f;
	}

DCodeSeg* DCodeSeg::FindRomCode(const TAny*)
	{
	// never used
	return NULL;
	}

void DWin32CodeSeg::InitData()
	{
	}

TInt DWin32CodeSeg::GetMemoryInfo(TModuleMemoryInfo&, DProcess*)
	{
	return KErrNotSupported;
	}

void DWin32CodeSeg::Info(TCodeSegCreateInfo& aInfo)
	{
	Emulator::TModule etm(iModuleHandle);
	const IMAGE_NT_HEADERS32* nth=etm.NtHeader();
	const IMAGE_EXPORT_DIRECTORY* exd=
		(const IMAGE_EXPORT_DIRECTORY*)etm.Translate(nth->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress);
	const TEmulatorImageHeader* hdr = (const TEmulatorImageHeader*)etm.Section(KWin32SectionName_Symbian);
	TBool isExe = hdr && hdr->iUids[0].iUid==KExecutableImageUidValue;
	const IMAGE_SECTION_HEADER* text=etm.SectionHeader(KWin32SectionName_Text);
	const IMAGE_SECTION_HEADER* rdata=etm.SectionHeader(KWin32SectionName_RData);
	const IMAGE_SECTION_HEADER* data=etm.SectionHeader(KWin32SectionName_EpocData);
	if (!isExe && !(hdr->iFlags & KEmulatorImageFlagAllowDllData))
		data = NULL;
	aInfo.iTextSize=text->Misc.VirtualSize;
	aInfo.iCodeSize=text->Misc.VirtualSize;
	if (rdata)
		aInfo.iCodeSize+=rdata->Misc.VirtualSize;
	aInfo.iDataSize=data ? (data->SizeOfRawData) : 0;
	aInfo.iBssSize=data ? (data->Misc.VirtualSize-data->SizeOfRawData) : 0;
	aInfo.iTotalDataSize=aInfo.iDataSize+aInfo.iBssSize;
	aInfo.iExportDir=(TLinAddr)exd;
	aInfo.iExportDirCount=exd ? exd->NumberOfFunctions : 0;
	aInfo.iCodeLoadAddress=(TLinAddr)etm.Translate(text->VirtualAddress);
	aInfo.iCodeRunAddress=aInfo.iCodeLoadAddress;
	aInfo.iDataLoadAddress=data ? (TLinAddr)etm.Translate(data->VirtualAddress) : 0;
	aInfo.iDataRunAddress=aInfo.iDataLoadAddress;
	DCodeSeg::Info(aInfo);
	}

TLinAddr DCodeSeg::ExceptionDescriptor()
	{
	return 0;	// not used on emulator
	}

TInt MM::RegisterModule(HMODULE aModule)
	{
	__KTRACE_OPT(KBOOT, Kern::Printf("MM::RegisterModule %08x",aModule));
	SWin32Module w32m;
	w32m.iWin32ModuleHandle=(TLinAddr)aModule;
	w32m.iCodeSeg=NULL;
	TInt r=MM::Win32Modules.InsertInUnsignedKeyOrder(w32m);
	__KTRACE_OPT(KBOOT, Kern::Printf("MM::RegisterModule ret %d",r));
	return r;
	}

_LIT(KRomSysBin, "Z:\\sys\\bin");

// Massage file names returned for stuff in EPOCROOT\epoc32\release\wins\xxxx\
// in order to make them appear to be in z:\system\libs
void P::NormalizeExecutableFileName(TDes& a)
	{
	__KTRACE_OPT(KDLL, Kern::Printf("NormalizeExecutableFileName %lS", &a));
	if (a.Length()<3 || a[1]!=':' || a[2]!='\\')
		{
		// don't have a drive so assume in 'ROM'
		TFileName initial(a);
		a=KRomSysBin();
		if (initial.Length()==0 || initial[0]!='\\')
			a.Append('\\');
		a+=initial;
		__KTRACE_OPT(KDLL, Kern::Printf("NormalizeExecutableFileName -> %lS", &a));
		}
	}

TInt LookupDllUid(TInt aModule)
	{
	TInt r = KDllUid_Default;
	Kern::AccessCode();
	SWin32Module w32m;
	w32m.iWin32ModuleHandle=(TLinAddr)aModule;
	TInt ix;
	MM::Win32Modules.SpecificFindInUnsignedKeyOrder(w32m, ix, EArrayFindMode_Last);
	if (ix>0)
		{
		DWin32CodeSeg* pS = MM::Win32Modules[ix-1].iCodeSeg;
		if (pS && pS->iFileEntryPoint==(TLinAddr)aModule)
			r = pS->iCodeSegId;
		}
	Kern::EndAccessCode();
	return r;
	}

