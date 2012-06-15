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
//

#ifndef __MMF_BT_IMAADPCM_TO_PCM16_HW_DEVICE_H__
#define __MMF_BT_IMAADPCM_TO_PCM16_HW_DEVICE_H__
#include <mmfbtswcodecwrapper.h>
#include "MMFAudioCodecBase.h"
#include "MMFAudioImaAdpcmToS16PcmCodec.h"

const TInt KImaToPCM16SrcBufferSize  = 1024;
const TInt KImaToPCM16SinkBufferSize = 4096;

/*
*
* class CMMFImaAdpcmToPcm16CodecHwDevice 
*
*/
class CMMFImaAdpcmToPcm16CodecHwDevice : public CMMFSwCodecWrapper
	{
public:
	static  CMMFImaAdpcmToPcm16CodecHwDevice* NewL();
	virtual CMMFSwCodec& Codec();
	~CMMFImaAdpcmToPcm16CodecHwDevice(); 
	virtual TInt Start(TDeviceFunc aFuncCmd, TDeviceFlow aFlowCmd);
private:
	void ConstructL();
	};

/*
*
* class CMMFMulawToPcm16Codec coverts Imaad to PCM16
*  
*/
class CMMFImaAdpcmToPcm16Codec : public CMMFSwCodec
	{
public:
	CMMFImaAdpcmToPcm16Codec();
	virtual TCodecProcessResult ProcessL(const CMMFBuffer& aSource, CMMFBuffer& aDest);
	virtual TUint SourceBufferSize() {return KImaToPCM16SrcBufferSize;}
	virtual TUint SinkBufferSize() {return KImaToPCM16SinkBufferSize;}
	TInt Configure(TUint aChannels, TUint aSampleRate);
private:
	TBool CheckPreconditions( const CMMFDataBuffer* aSrcBuffer, CMMFDataBuffer* aDestBuffer );
private:
	TMMFAudioImaAdpcmToS16PcmCodec iImaAdpcmTo16Pcm;
	
private:
	TUint iChannels;
	TUint iSampleRate;
	TUint iBlockAlign;
	TUint iSamplesPerBlock;
	};

#endif //__MMF_BT_IMAADPCM_TO_PCM16_HW_DEVICE_H__
