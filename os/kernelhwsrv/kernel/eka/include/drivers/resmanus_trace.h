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
// e32\include\drivers\resmanus_trace.h
// 
// WARNING: This file contains some APIs which are internal and are subject
//          to change without notice. Such APIs should therefore not be used
//          outside the Kernel and Hardware Services package.
//

/**
 @file
 @internalComponent
*/
#ifndef __RESMANUSCONTROL_TRACE_H__
#define __RESMANUSCONTROL_TRACE_H__
#ifdef BTRACE_RESMANUS

//Function to format the output.
static void UsTraceFormatPrint(TDes8& aBuf, const char* aFmt, ...)
	{
	if(!(&aBuf))
		return;
	VA_LIST list;
	VA_START(list,aFmt);
	Kern::AppendFormat(aBuf,aFmt,list);
	}

//definition of subcategories.
#define PRM_US_OPEN_CHANNEL_START				BTrace::EOpenChannelUsStart
#define PRM_US_OPEN_CHANNEL_END					BTrace::EOpenChannelUsEnd
#define PRM_US_REGISTER_CLIENT_START			BTrace::ERegisterClientUsStart
#define PRM_US_REGISTER_CLIENT_END				BTrace::ERegisterClientUsEnd
#define PRM_US_DEREGISTER_CLIENT_START			BTrace::EDeRegisterClientUsStart
#define PRM_US_DEREGISTER_CLIENT_END			BTrace::EDeRegisterClientUsEnd
#define PRM_US_GET_RESOURCE_STATE_START			BTrace::EGetResourceStateUsStart
#define PRM_US_GET_RESOURCE_STATE_END			BTrace::EGetResourceStateUsEnd
#define PRM_US_SET_RESOURCE_STATE_START			BTrace::ESetResourceStateUsStart
#define PRM_US_SET_RESOURCE_STATE_END			BTrace::ESetResourceStateUsEnd
#define PRM_US_CANCEL_GET_RESOURCE_STATE_START  BTrace::ECancelGetResourceStateUsStart
#define PRM_US_CANCEL_GET_RESOURCE_STATE_END	BTrace::ECancelGetResourceStateUsEnd
#define PRM_US_CANCEL_SET_RESOURCE_STATE_START	BTrace::ECancelSetResourceStateUsStart
#define PRM_US_CANCEL_SET_RESOURCE_STATE_END	BTrace::ECancelSetResourceStateUsEnd

// Macro to output identification information provided in a request to open a channel
#define PRM_US_OPEN_CHANNEL_START_TRACE						\
	{														\
    TBuf8<256> printBuf;									\
    printBuf.Zero();										\
    UsTraceFormatPrint(printBuf, "%S", iUserNameUsed);	\
	BTraceContextN(BTrace::EResourceManagerUs, PRM_US_OPEN_CHANNEL_START, 0, (TInt)(iClient), printBuf.Ptr(), printBuf.Length()); \
	}

// Macro to output identification information generated during a request to open a channel
#define PRM_US_OPEN_CHANNEL_END_TRACE						\
	{														\
    TBuf8<256> printBuf;									\
    printBuf.Zero();										\
    UsTraceFormatPrint(printBuf, "%S", iUserNameUsed);		\
	BTraceContextN(BTrace::EResourceManagerUs, PRM_US_OPEN_CHANNEL_END, 0, (TInt)(ClientHandle()), printBuf.Ptr(), printBuf.Length()); \
	}

// Macro to output information provided for a request to register with the Resource Controller
#define PRM_US_REGISTER_CLIENT_START_TRACE					\
	{														\
    TBuf8<256> printBuf;									\
    printBuf.Zero();										\
    UsTraceFormatPrint(printBuf, "%S 0x%x %d", iUserNameUsed, (TInt)(ClientHandle()),(TInt)(stateRes[0]));	\
	BTraceContextN(BTrace::EResourceManagerUs, PRM_US_REGISTER_CLIENT_START, (TInt)(stateRes[1]), (TInt)(stateRes[2]), printBuf.Ptr(), printBuf.Length()); \
	}

// Macro to output information after issuing a request to register with the Resource Controller
#define PRM_US_REGISTER_CLIENT_END_TRACE					\
	{														\
	BTraceContext8(BTrace::EResourceManagerUs, PRM_US_REGISTER_CLIENT_END, (TInt)(ClientHandle()), r);	\
	}

// Macro to output information provided for a request to de-register with the Resource Controller
#define PRM_US_DEREGISTER_CLIENT_START_TRACE				\
	{														\
    TBuf8<256> printBuf;									\
    printBuf.Zero();										\
    UsTraceFormatPrint(printBuf, "%S ", iUserNameUsed);		\
	BTraceContextN(BTrace::EResourceManagerUs, PRM_US_DEREGISTER_CLIENT_START, 0, (TInt)(ClientHandle()), printBuf.Ptr(), printBuf.Length()); \
	}

// Macro to output information after issuing a request to de-register with the Resource Controller
#define PRM_US_DEREGISTER_CLIENT_END_TRACE					\
	{														\
	BTraceContext4(BTrace::EResourceManagerUs, PRM_US_DEREGISTER_CLIENT_END, (TInt)(ClientHandle()));	\
	}

// Macro to output information provided for a request to get the state of a resource
#define PRM_US_GET_RESOURCE_STATE_START_TRACE				\
	{														\
    TBuf8<256> printBuf;									\
    printBuf.Zero();										\
    UsTraceFormatPrint(printBuf, "%S ", iUserNameUsed);		\
	BTraceContextN(BTrace::EResourceManagerUs, PRM_US_GET_RESOURCE_STATE_START, resourceId, (TInt)(ClientHandle()), printBuf.Ptr(), printBuf.Length()); \
	}

// Macro to output information on completion of a request to get the state of a resource
#define PRM_US_GET_RESOURCE_STATE_END_TRACE					\
	{														\
    TBuf8<256> printBuf;									\
    printBuf.Zero();										\
    UsTraceFormatPrint(printBuf, "%d %d", aClient, aResult);	\
	BTraceContextN(BTrace::EResourceManagerUs, PRM_US_GET_RESOURCE_STATE_END, aResourceId, aLevel, printBuf.Ptr(), printBuf.Length()); \
	}

// Macro to output information provided for a request to set the state of a resource
#define PRM_US_SET_RESOURCE_STATE_START_TRACE				\
	{														\
    TBuf8<256> printBuf;									\
    printBuf.Zero();										\
    UsTraceFormatPrint(printBuf, "%S %d", iUserNameUsed, (TInt)(ClientHandle()));	\
	BTraceContextN(BTrace::EResourceManagerUs, PRM_US_SET_RESOURCE_STATE_START, resourceId, newState, printBuf.Ptr(), printBuf.Length()); \
	}

// Macro to output information on completion of a request to set the state of a resource
#define PRM_US_SET_RESOURCE_STATE_END_TRACE					\
	{														\
    TBuf8<256> printBuf;									\
    printBuf.Zero();										\
    UsTraceFormatPrint(printBuf, "%d %d", aClient, aResult);	\
	BTraceContextN(BTrace::EResourceManagerUs, PRM_US_SET_RESOURCE_STATE_END, aResourceId, aLevel, printBuf.Ptr(), printBuf.Length()); \
	}

// Macro to output information provided for a request to cancel the get resource state requests for a resource
#define PRM_US_CANCEL_GET_RESOURCE_STATE_START_TRACE		\
	{														\
    TBuf8<256> printBuf;									\
    printBuf.Zero();										\
    UsTraceFormatPrint(printBuf, "%S ", iUserNameUsed);		\
	BTraceContextN(BTrace::EResourceManagerUs, PRM_US_CANCEL_GET_RESOURCE_STATE_START, aResourceId, (TInt)(ClientHandle()), printBuf.Ptr(), printBuf.Length()); \
	}

// Macro to output information on completion of a request to cancel the get resource state requests for a resource
#define PRM_US_CANCEL_GET_RESOURCE_STATE_END_TRACE			\
	{														\
    TBuf8<256> printBuf;									\
    printBuf.Zero();										\
    UsTraceFormatPrint(printBuf, "%S ", iUserNameUsed);		\
	BTraceContextN(BTrace::EResourceManagerUs, PRM_US_CANCEL_GET_RESOURCE_STATE_END, aResourceId, (TInt)(ClientHandle()), printBuf.Ptr(), printBuf.Length()); \
	}

// Macro to output information provided for a request to cancel the set resource state requests for a resource
#define PRM_US_CANCEL_SET_RESOURCE_STATE_START_TRACE		\
	{														\
    TBuf8<256> printBuf;									\
    printBuf.Zero();										\
    UsTraceFormatPrint(printBuf, "%S ", iUserNameUsed);		\
	BTraceContextN(BTrace::EResourceManagerUs, PRM_US_CANCEL_SET_RESOURCE_STATE_START, aResourceId, (TInt)(ClientHandle()), printBuf.Ptr(), printBuf.Length()); \
	}

// Macro to output information on completion of a request to cancel the get resource state requests for a resource
#define PRM_US_CANCEL_SET_RESOURCE_STATE_END_TRACE			\
	{														\
    TBuf8<256> printBuf;									\
    printBuf.Zero();										\
    UsTraceFormatPrint(printBuf, "%S ", iUserNameUsed);		\
	BTraceContextN(BTrace::EResourceManagerUs, PRM_US_CANCEL_SET_RESOURCE_STATE_END, aResourceId, (TInt)(ClientHandle()), printBuf.Ptr(), printBuf.Length()); \
	}


#else

#define PRM_US_OPEN_CHANNEL_START_TRACE
#define PRM_US_OPEN_CHANNEL_END_TRACE
#define PRM_US_REGISTER_CLIENT_START_TRACE
#define PRM_US_REGISTER_CLIENT_END_TRACE
#define PRM_US_DEREGISTER_CLIENT_START_TRACE
#define PRM_US_DEREGISTER_CLIENT_END_TRACE
#define PRM_US_GET_RESOURCE_STATE_START_TRACE
#define PRM_US_GET_RESOURCE_STATE_END_TRACE
#define PRM_US_SET_RESOURCE_STATE_START_TRACE
#define PRM_US_SET_RESOURCE_STATE_END_TRACE
#define PRM_US_CANCEL_GET_RESOURCE_STATE_START_TRACE
#define PRM_US_CANCEL_GET_RESOURCE_STATE_END_TRACE
#define PRM_US_CANCEL_SET_RESOURCE_STATE_START_TRACE
#define PRM_US_CANCEL_SET_RESOURCE_STATE_END_TRACE

#endif //BTRACE_RESMANUS

#endif //__RESMANUSCONTROL_TRACE_H__

