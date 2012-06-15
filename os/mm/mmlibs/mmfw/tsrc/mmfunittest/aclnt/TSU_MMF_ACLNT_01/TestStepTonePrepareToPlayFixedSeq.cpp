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
// This file contains an example Test step implementation 
// This demonstrates the various functions provided
// by the CTestStep base class which are available within
// a test step 
// 
//

// EPOC includes
#include <e32base.h>
#include <e32test.h>
#include <e32keys.h>
#include <c32comm.h>
#include <f32file.h>
#include <etel.h>
#include <etelmm.h>
#include <testframework.h>

#include <mdaaudiotoneplayer.h>

// Test system includes
#include <testframework.h>

// Specific includes for this test suite
#include "TestStepUnitMMFAudClient.h"
#include "TestSuiteUnitMMFAudClient.h"

// Specific includes for these test steps
#include "TestStepTone.h"

#include "toneTest.h"

//#include <mmfaudiocontroller.h>
#include "mmfclienttoneplayer.h"
//#include "MmfClientAudioPlayer.h"
//#include <mmffourcc.h>
//#include <mmfpaniccodes.h>
//#include <mmfFormatImplementationUIDs.hrh>
// --------------------------------------------

/**
 *
 * Static constructor for CTestStepTonePrepareToPlayFixedSeq.
 *
 *
 * @return	"CTestStepTonePrepareToPlayFixedSeq*"
 *			The constructed CTestStepTonePrepareToPlayFixedSeq
 *
 * @xxxx
 * 
 */
CTestStepTonePrepareToPlayFixedSeq* CTestStepTonePrepareToPlayFixedSeq::NewL()
	{
	CTestStepTonePrepareToPlayFixedSeq* self = new(ELeave) CTestStepTonePrepareToPlayFixedSeq;
	return self;
	}

/**
 *
 * Test step constructor.
 * Each test step initialises its own name.
 *
 * @xxxx
 * 
 */
CTestStepTonePrepareToPlayFixedSeq::CTestStepTonePrepareToPlayFixedSeq() : iError(KErrNone)
	{
	// store the name of this test case
	// this is the name that is used by the script file
	iTestStepName = _L("MM-MMF-ACLNT-U-0457-CP");
	
	// need a bigger heap size on EKA2 HW
#if !defined __WINS__
	iHeapSize = KHeapSizeToneTestEKA2;
#endif // EKA2
	}

/**
 *
 * Test step destructor.
 *
 * @xxxx
 * 
 */
CTestStepTonePrepareToPlayFixedSeq::~CTestStepTonePrepareToPlayFixedSeq()
	{
	}

void CTestStepTonePrepareToPlayFixedSeq::MatoPrepareComplete(TInt aError)
	{
	iError = aError;
	CActiveScheduler::Stop();
	}

void CTestStepTonePrepareToPlayFixedSeq::MatoPlayComplete(TInt aError)
	{
	iError = aError;
	CActiveScheduler::Stop();
	}

/**
 *
 * Test step Preamble.
 *
 * @xxxx
 * 
 */
enum TVerdict CTestStepTonePrepareToPlayFixedSeq::DoTestStepPreambleL(void)
	{
    TInt err       = KErrNone;
	enum TVerdict verdict;
	// this installs the scheduler
	verdict = CTestStepUnitMMFAudClient::DoTestStepPreambleL();

	// Printing to the console and log file
	INFO_PRINTF1(_L("MM-MMF-ACLNT-U-0457-CP"));
	INFO_PRINTF1(_L("this is a test of CMdaAudioToneUtility::PrepareToPlayFixedSequence() "));

	// get the file with the DTMF string
	if(!GetStringFromConfig(_L("SectionOne"), _L("DTMFStringFName1"), iFileName))
		{
		//INFO_PRINTF2(_L("file name %s not found..."), fileptr);
		return EInconclusive;
		}

	// create the Tone utility
	TRAP( err, (iTone = CMMFMdaAudioToneUtility::NewL(*this)) );
	if ( err != KErrNone ||
		 iTone->State() != EMdaAudioToneUtilityNotReady	)
		verdict = EInconclusive;

	return verdict;

	}

/**
 *
 * Test step Postamble.
 *
 * @xxxx
 * 
 */
enum TVerdict CTestStepTonePrepareToPlayFixedSeq::DoTestStepPostambleL(void)
	{
	delete iTone;
	iTone = NULL;
	//[ Destroy the scheduler ]
	return CTestStepUnitMMFAudClient::DoTestStepPostambleL();
	}

	/**
 *
 * Do the test step.
 * Each test step must supply an implementation for DoTestStepL.
 *
 * @return	"TVerdict"
 *			The result of the test step
 *
 * @xxxx
 * 
 */
TVerdict CTestStepTonePrepareToPlayFixedSeq::DoTestStepL()
	{
	iTestStepResult = EPass;

	TInt fixedNum = iTone->FixedSequenceCount();

	for (TInt i=0; i< fixedNum; i++)
		{
		iTone->PrepareToPlayFixedSequence(0);
		CActiveScheduler::Start();
		TInt err1 = KErrNone;
		TInt type = 0;
		err1 = GetAudToneType(iTone, type);
		if (err1 != KErrNone ||
			type != CMMFToneConfig::EMmfToneTypeFixedSeq   ||
			iError != KErrNone ||
			iTone->State() != EMdaAudioToneUtilityPrepared)
			return EFail;

		iTone->Play(); 
		CActiveScheduler::Start();
		User::After(2000000);
		if (iError != KErrNone	)
			return EInconclusive;
		
		iTone->CancelPrepare();
		if (iTone->State() != EMdaAudioToneUtilityNotReady	)
			return EFail;
		}

	INFO_PRINTF1(_L("finished with this test step"));
	// test steps return a result
	return iTestStepResult;
	}

