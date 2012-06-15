//
// Copyright (c) 2005 Nokia Corporation and/or its subsidiary(-ies).All rights reserved.
//
// GENERATED FILE - DO NOT EDIT
//

/**
 * @file
 * @internalComponent
 */

class ExecHandler
	{
public:
	static RAllocator* Heap();
	static RAllocator* HeapSwitch(RAllocator*);
	static TTrapHandler* PushTrapFrame(TTrap*);
	static TTrap* PopTrapFrame();
	static CActiveScheduler* ActiveScheduler();
	static void SetActiveScheduler(CActiveScheduler*);
	static TTimerLockSpec LockPeriod();
	static TTrapHandler* TrapHandler();
	static TTrapHandler* SetTrapHandler(TTrapHandler*);
	static TUint32 DebugMask();
	static TUint32 DebugMaskIndex(TUint);
	static void SetDebugMask(TUint32);
	static void UnlockRamDrive();
	static TLinAddr RomHeaderAddress();
	static TLinAddr RomRootDirectoryAddress();
	static void SetReentryPoint(TLinAddr);
	static TInt UTCOffset();
	static TInt GetGlobalUserData(TInt);
	static void FastAtomicAxo64(SAtomicOpInfo64*);
	static TBool FastAtomicCas64(SAtomicOpInfo64*);
	static void FastAtomicAdd64(SAtomicOpInfo64*);
	static void FastAtomicTau64(SAtomicOpInfo64*);
	static void FastAtomicTas64(SAtomicOpInfo64*);
	static TUint32 FastAtomicAxo32(SAtomicOpInfo32*);
	static TUint32 FastAtomicAdd32(SAtomicOpInfo32*);
	static TBool FastAtomicCas32(SAtomicOpInfo32*);
	static TUint32 FastAtomicTau32(SAtomicOpInfo32*);
	static TInt32 FastAtomicTas32(SAtomicOpInfo32*);
	static TUint16 FastAtomicAxo16(SAtomicOpInfo32*);
	static TUint16 FastAtomicAdd16(SAtomicOpInfo32*);
	static TBool FastAtomicCas16(SAtomicOpInfo32*);
	static TUint16 FastAtomicTau16(SAtomicOpInfo32*);
	static TInt16 FastAtomicTas16(SAtomicOpInfo32*);
	static TUint8 FastAtomicAxo8(SAtomicOpInfo32*);
	static TUint8 FastAtomicAdd8(SAtomicOpInfo32*);
	static TBool FastAtomicCas8(SAtomicOpInfo32*);
	static TUint8 FastAtomicTau8(SAtomicOpInfo32*);
	static TInt8 FastAtomicTas8(SAtomicOpInfo32*);
	static TInt ObjectNext(TObjectType, TBuf8<KMaxFullName>&, TFindHandle&);
	static TUint8* ChunkBase(DChunk*);
	static TInt ChunkSize(DChunk*);
	static TInt ChunkMaxSize(DChunk*);
	static TUint HandleAttributes(TInt);
	static void LogicalDeviceGetCaps(DLogicalDevice*, TDes8&);
	static TBool LogicalDeviceQueryVersionSupported(DLogicalDevice*, const TVersion&);
	static TBool LogicalDeviceIsAvailable(DLogicalDevice*, TInt, const TDesC8*, const TDesC8*);
	static TInt LocaleExports(TAny*, TLibraryFunction*);
	static TInt ChannelRequest(DLogicalChannelBase*, TInt, TAny*, TAny*);
	static void IMBRange(TAny*, TUint);
	static TInt ResetMachine(TMachineStartupType);
	static TLibraryFunction LibraryLookup(TInt, TInt);
	static void LibraryFileName(DLibrary*, TDes8&);
	static TInt ExecuteInSupervisorMode(TSupervisorFunction, TAny*);
	static void MutexWait(DMutex*);
	static void MutexSignal(DMutex*);
	static TInt ProcessId(DProcess*);
	static void DllFileName(TInt, TDes8&);
	static void ProcessResume(DProcess*);
	static void ProcessFileName(DProcess*, TDes8&);
	static void ProcessCommandLine(DProcess*, TDes8&);
	static TExitType ProcessExitType(DProcess*);
	static TInt ProcessExitReason(DProcess*);
	static void ProcessExitCategory(DProcess*, TDes8&);
	static TProcessPriority ProcessPriority(DProcess*);
	static TInt ProcessSetPriority(DProcess*, TProcessPriority);
	static TUint ProcessFlags(DProcess*);
	static void ProcessSetFlags(DProcess*, TUint, TUint);
	static TInt SemaphoreWait(DSemaphore*, TInt);
	static void SemaphoreSignal1(DSemaphore*);
	static void SemaphoreSignalN(DSemaphore*, TInt);
	static void ServerReceive(DServer*, TRequestStatus&, TAny*);
	static void ServerCancel(DServer*);
	static void SetSessionPtr(RMessageK*, const TAny*);
	static TInt SessionSend(TInt, TInt, TAny*, TRequestStatus*);
	static TInt ThreadId(DThread*);
	static TInt SessionShare(TInt&, TInt);
	static void ThreadResume(DThread*);
	static void ThreadSuspend(DThread*);
	static TThreadPriority ThreadPriority(DThread*);
	static void ThreadSetPriority(DThread*, TThreadPriority);
	static TProcessPriority ThreadProcessPriority(DThread*);
	static void ThreadSetProcessPriority(DThread*, TProcessPriority);
	static TUint ThreadFlags(DThread*);
	static void ThreadSetFlags(DThread*, TUint, TUint);
	static TInt ThreadRequestCount(DThread*);
	static TExitType ThreadExitType(DThread*);
	static TInt ThreadExitReason(DThread*);
	static void ThreadExitCategory(DThread*, TDes8&);
	static void TimerCancel(DTimer*);
	static void TimerAfter(DTimer*, TRequestStatus&, TInt);
	static void TimerAt(DTimer*, TRequestStatus&, TUint32, TUint32);
	static void TimerLock(DTimer*, TRequestStatus&, TTimerLockSpec);
	static TInt ChangeNotifierLogon(DChangeNotifier*, TRequestStatus&);
	static TInt ChangeNotifierLogoff(DChangeNotifier*);
	static void RequestSignal(TInt);
	static void HandleName(TInt, TDes8&);
	static void HandleFullName(TInt, TDes8&);
	static void HandleInfo(TInt, THandleInfo*);
	static void HandleCount(DThread*, TInt&, TInt&);
	static void After(TInt, TRequestStatus&);
	static void At(const EXEC_TIME&, TRequestStatus&);
	static void MessageComplete(RMessageK*, TInt);
	static void MessageCompleteWithHandle(RMessageK*, TInt);
	static void TransferSession(RMessageK*, TInt);
	static TInt TimeNow(EXEC_TIME&, TInt&);
	static TInt TimeNowSecure(EXEC_TIME&, TInt&);
	static TInt SetUTCTimeAndOffset(const EXEC_TIME&, TInt, TUint, TUint);
	static TInt SetMachineConfiguration(const TDesC8&);
	static void CaptureEventHook();
	static void ReleaseEventHook();
	static void RequestEvent(TRawEventBuf&, TRequestStatus&);
	static void RequestEventCancel();
	static TInt AddEvent(const TRawEvent&);
	static TInt SessionSendSync(TInt, TInt, TAny*, TRequestStatus*);
	static TAny* DllTls(TInt, TInt);
	static TInt HalFunction(TInt, TInt, TAny*, TAny*);
	static void WsRegisterThread();
	static void FsRegisterThread();
	static TInt ProcessCommandLineLength(DProcess*);
	static void TimerInactivity(DTimer*, TRequestStatus&, TInt);
	static TInt UserInactivityTime();
	static void ResetInactivityTime();
	static void DebugPrint(TAny*, TInt);
	static TInt BreakPoint();
	static TInt ProfileStart(TInt);
	static TInt ProfileEnd(TInt);
	static TExceptionHandler ExceptionHandler(DThread*);
	static TInt SetExceptionHandler(DThread*, TExceptionHandler, TUint32);
	static void ModifyExceptionMask(DThread*, TUint32, TUint32);
	static TInt RaiseException(DThread*, TExcType);
	static TInt IsExceptionHandled(DThread*, TExcType, TBool);
	static TInt ProcessGetMemoryInfo(TInt, TModuleMemoryInfo&);
	static TInt LibraryGetMemoryInfo(TInt, TModuleMemoryInfo&);
	static TInt MachineConfiguration(TDes8&, TInt&);
	static TInt SetMemoryThresholds(TInt, TInt);
	static void LibraryType(DLibrary*, TUidType&);
	static void ProcessType(DProcess*, TUidType&);
	static TInt ChunkBottom(DChunk*);
	static TInt ChunkTop(DChunk*);
	static void ThreadContext(DThread*, TDes8&);
	static TInt ThreadCreate(const TDesC8&, TOwnerType, SThreadCreateInfo8&);
	static TInt FindHandleOpen(TOwnerType, const TFindHandle&);
	static TInt HandleClose(TInt);
	static TInt ChunkCreate(TOwnerType, const TDesC8*, TChunkCreate&);
	static TInt ChunkAdjust(DChunk*, TInt, TInt, TInt);
	static TInt OpenObject(TObjectType, const TDesC8&, TOwnerType);
	static TInt HandleDuplicate(TInt, TOwnerType, TInt&);
	static TInt MutexCreate(const TDesC8*, TOwnerType);
	static TInt SemaphoreCreate(const TDesC8*, TInt, TOwnerType);
	static TInt ThreadOpenById(TUint, TOwnerType);
	static TInt ProcessOpenById(TUint, TOwnerType);
	static void ThreadKill(TInt, TExitType, TInt, const TDesC8*);
	static void ThreadLogon(DThread*, TRequestStatus*, TBool);
	static TInt ThreadLogonCancel(DThread*, TRequestStatus*, TBool);
	static TInt DllSetTls(TInt, TInt, TAny*);
	static void DllFreeTls(TInt);
	static TInt ThreadRename(TInt, const TDesC8&);
	static TInt ProcessRename(TInt, const TDesC8&);
	static void ProcessKill(TInt, TExitType, TInt, const TDesC8*);
	static void ProcessLogon(DProcess*, TRequestStatus*, TBool);
	static TInt ProcessLogonCancel(DProcess*, TRequestStatus*, TBool);
	static TInt ThreadProcess(DThread*);
	static TInt ServerCreate(const TDesC8*, TInt);
	static TInt ServerCreateWithOptions(const TDesC8*, TInt, TInt, TInt);
	static TInt SessionCreate(const TDesC8&, TInt, const TSecurityPolicy*, TInt);
	static TInt SessionCreateFromHandle(TInt, TInt, const TSecurityPolicy*, TInt);
	static TInt DeviceLoad(TAny*, TInt);
	static TInt DeviceFree(const TDesC8&, TInt);
	static TInt ChannelCreate(const TDesC8&, TChannelCreateInfo8&, TInt);
	static TInt TimerCreate();
	static void TimerHighRes(DTimer*, TRequestStatus&, TInt);
	static void AfterHighRes(TInt, TRequestStatus&);
	static TInt ChangeNotifierCreate(TOwnerType);
	static TInt UndertakerCreate(TOwnerType);
	static TInt UndertakerLogon(DUndertaker*, TRequestStatus&, TInt*);
	static TInt UndertakerLogonCancel(DUndertaker*);
	static void KernelHeapDebug(TInt, TInt, TAny*);
	static TInt ThreadGetCpuTime(DThread*, EXEC_INT64&);
	static TInt CodeSegCreate(TCodeSegCreateInfo&);
	static TInt CodeSegLoaded(TCodeSegCreateInfo&);
	static TInt LibraryCreate(TLibraryCreateInfo&);
	static TInt CodeSegOpen(TAny*, TInt);
	static void CodeSegClose(TAny*);
	static void CodeSegNext(TAny*&, const TFindCodeSeg&);
	static void CodeSegInfo(TAny*, TCodeSegCreateInfo&);
	static TInt CodeSegAddDependency(TAny*, TAny*);
	static void CodeSegDeferDeletes();
	static void CodeSegEndDeferDeletes();
	static TInt ProcessCreate(TProcessCreateInfo&, const TDesC8*);
	static TInt ProcessLoaded(TProcessCreateInfo&);
	static TInt CheckLoaderClientState(DThread*);
	static TAny* ThreadProcessCodeSeg(DThread*);
	static void CodeSegReadExportDir(TAny*, TLinAddr*);
	static TInt WaitDllLock();
	static TInt ReleaseDllLock();
	static TInt LibraryAttach(TInt, TInt&, TLinAddr*);
	static TInt LibraryAttached(TInt);
	static TInt StaticCallList(TInt&, TLinAddr*);
	static TInt LibraryDetach(TInt&, TLinAddr*);
	static TInt LibraryDetached();
	static TInt LastThreadHandle();
	static void ThreadRendezvous(TInt);
	static void ProcessRendezvous(TInt);
	static TInt MessageGetDesLength(RMessageK*, TInt);
	static TInt MessageGetDesMaxLength(RMessageK*, TInt);
	static TInt MessageIpcCopy(RMessageK*, TInt, SIpcCopyInfo&, TInt);
	static TInt MessageClient(DThread*, TOwnerType);
	static TInt MessageSetProcessPriority(DThread*, TProcessPriority);
	static void MessageConstructFromPtr(RMessageK*, TAny*);
	static void MessageKill(TInt, TExitType, TInt, const TDesC8*);
	static TInt MessageOpenObject(RMessageK*, TObjectType, TInt, TOwnerType);
	static void ProcessSecurityInfo(DProcess*, SSecurityInfo&);
	static void ThreadSecurityInfo(DThread*, SSecurityInfo&);
	static void MessageSecurityInfo(DThread*, SSecurityInfo&);
	static void CreatorSecurityInfo(SSecurityInfo&);
	static void DisabledCapabilities(SCapabilitySet&);
	static TInt ChunkSetRestrictions(DChunk*, TUint);
	static TInt MsgQueueCreate(const TDesC8*, TInt, TInt, TOwnerType);
	static TInt MsgQueueSend(TInt, const TAny*, TInt);
	static TInt MsgQueueReceive(DMsgQueue*, TAny*, TInt);
	static void MsgQueueNotifySpaceAvailable(DMsgQueue*, TRequestStatus&);
	static void MsgQueueCancelSpaceAvailable(DMsgQueue*);
	static void MsgQueueNotifyDataAvailable(DMsgQueue*, TRequestStatus&);
	static void MsgQueueCancelDataAvailable(DMsgQueue*);
	static TInt MsgQueueSize(DMsgQueue*);
	static TInt PropertyDefine(TUint, TUint, TPropertyInfo*);
	static TInt PropertyDelete(TUint, TUint);
	static TInt PropertyAttach(TUint, TUint, TOwnerType);
	static void PropertySubscribe(DPropertyRef*, TRequestStatus*);
	static void PropertyCancel(DPropertyRef*);
	static TInt PropertyGetI(DPropertyRef*, TInt*);
	static TInt PropertyGetB(DPropertyRef*, TUint8*, TInt);
	static TInt PropertySetI(DPropertyRef*, TInt);
	static TInt PropertySetB(TInt, const TUint8*, TInt);
	static TInt PropertyFindGetI(TUint, TUint, TInt*);
	static TInt PropertyFindGetB(TUint, TUint, TUint8*, TInt);
	static TInt PropertyFindSetI(TUint, TUint, TInt);
	static TInt PropertyFindSetB(TUint, TUint, TUint8*, TInt);
	static TInt PowerEnableWakeupEvents(TPowerState);
	static void PowerDisableWakeupEvents();
	static void PowerRequestWakeupEventNotification(TRequestStatus*);
	static void PowerCancelWakeupEventNotification();
	static TInt PowerDown();
	static TInt ProcessSetHandleParameter(DProcess*, TInt, TInt);
	static TInt ProcessSetDataParameter(TInt, TInt, const TUint8*, TInt);
	static TInt ProcessGetHandleParameter(TInt, TObjectType, TOwnerType);
	static TInt ProcessGetDataParameter(TInt, TUint8*, TInt);
	static TInt ProcessDataParameterLength(TInt);
	static TUint ThreadProcessFlags(DThread*);
	static TInt ThreadStackInfo(DThread*, TThreadStackInfo&);
	static RAllocator* ThreadGetHeap(DThread*);
	static TInt ThreadAsProcess(DThread*, TInt);
	static TInt CondVarCreate(const TDesC8*, TOwnerType);
	static TInt CondVarWait(DCondVar*, TInt, TInt);
	static void CondVarSignal(DCondVar*);
	static void CondVarBroadcast(DCondVar*);
	static TInt PlatSecDiagnostic(TPlatSecDiagnostic*);
	static TLinAddr ExceptionDescriptor(TLinAddr);
	static void ThreadRequestSignal(DThread*);
	static TBool MutexIsHeld(DMutex*);
	static TTrapHandler* LeaveStart();
	static void LeaveEnd();
	static void SetDebugMaskIndex(TUint32, TUint);
	static TInt GetModuleNameFromAddress(TAny*, TDes8&);
	static void NotifyChanges(TUint);
	static TInt SetGlobalUserData(TInt, TInt);
	static TInt SessionSecurityInfo(TInt, SSecurityInfo&);
	static const TRequestStatus* MessageClientStatus(RMessageK*);
	static TInt SetFloatingPointMode(TFloatingPointMode, TFloatingPointRoundingMode);
	static TBool BTraceOut(TUint32, TUint32, const BTrace::SExecExtension&, TInt);
	static TBool BTraceOutBig(TUint32, TUint32, const BTrace::SExecExtension&, TInt);
	static TBool UTraceOut(TUint32, TUint32, const BTrace::SExecExtension&, TInt);
	static TAny* ProcessExeExportData();
	static TInt NotifyIfCodeSegDestroyed(TRequestStatus&);
	static TInt GetDestroyedCodeSegInfo(TCodeSegLoaderCookie&);
	static TInt SetWin32RuntimeHook(TAny*);
	static TInt GetBTraceId(DObject*);
	static void NotifyOnIdle(TRequestStatus*);
	static void CancelMiscNotifier(TRequestStatus*);
	static void NotifyObjectDestruction(TInt, TRequestStatus*);
	static void RegisterTrustedChunk(DChunk*);
	static TBool UserThreadExiting(TInt);
	static void SlowAtomicAxo64(SAtomicOpInfo64*);
	static TBool SlowAtomicCas64(SAtomicOpInfo64*);
	static void SlowAtomicAdd64(SAtomicOpInfo64*);
	static void SlowAtomicTau64(SAtomicOpInfo64*);
	static void SlowAtomicTas64(SAtomicOpInfo64*);
	static TBool ChunkIsPaged(DChunk*);
	static TBool ProcessDefaultDataPaged(DProcess*);
	static TUint MessageClientThreadFlags(DThread*);
	static TInt ShPoolCreate(const TShPoolInfo&, TUint);
	static TInt ShPoolAlloc(TInt, TUint, SShBufBaseAndSize&);
	static void ShPoolGetInfo(DShPool*, TShPoolInfo&);
	static TUint ShPoolFreeCount(DShPool*);
	static TInt ShPoolNotification(DShPool*, TShPoolNotifyType, TUint, TRequestStatus&);
	static TInt ShPoolNotificationCancel(DShPool*, TShPoolNotifyType, TRequestStatus&);
	static TInt ShPoolBufferWindow(DShPool*, TInt, TBool);
	static TInt ShBufMap(DShBuf*, TBool, SShBufBaseAndSize&);
	static TInt ShBufUnMap(DShBuf*);
	static TInt ShBufBaseAndSize(DShBuf*, SShBufBaseAndSize&);
	};

#ifdef __GEN_KERNEL_EXEC_CODE__
#define	FAST_EXEC_COUNT		44

FAST_EXEC_BEGIN
DECLARE_WORD(FAST_EXEC_COUNT)
DECLARE_FUNC(ExecHandler::Heap)
DECLARE_FUNC(ExecHandler::HeapSwitch)
DECLARE_FUNC(ExecHandler::PushTrapFrame)
DECLARE_FUNC(ExecHandler::PopTrapFrame)
DECLARE_FUNC(ExecHandler::ActiveScheduler)
DECLARE_FUNC(ExecHandler::SetActiveScheduler)
DECLARE_FUNC(ExecHandler::LockPeriod)
DECLARE_FUNC(ExecHandler::TrapHandler)
DECLARE_FUNC(ExecHandler::SetTrapHandler)
DECLARE_FUNC(ExecHandler::DebugMask)
DECLARE_FUNC(ExecHandler::DebugMaskIndex)
DECLARE_FUNC(ExecHandler::SetDebugMask)
DECLARE_FUNC(NKern::FastCounter)
DECLARE_FUNC(NKern::TickCount)
DECLARE_FUNC(TInternalRamDrive::Lock)
DECLARE_FUNC(ExecHandler::UnlockRamDrive)
#if defined(__EPOC32__)
DECLARE_FUNC(ExecHandler::RomHeaderAddress)
#else
DECLARE_FAST_EXEC_INVALID
#endif
#if defined(__EPOC32__)
DECLARE_FUNC(ExecHandler::RomRootDirectoryAddress)
#else
DECLARE_FAST_EXEC_INVALID
#endif
DECLARE_FUNC(ExecHandler::SetReentryPoint)
DECLARE_FUNC(K::KernelConfigFlags)
DECLARE_FUNC(ExecHandler::UTCOffset)
DECLARE_FUNC(ExecHandler::GetGlobalUserData)
DECLARE_FUNC(BTrace::CheckFilter)
#if defined(__ATOMIC64_USE_FAST_EXEC__)
DECLARE_FUNC(ExecHandler::FastAtomicAxo64)
#else
DECLARE_FAST_EXEC_INVALID
#endif
#if defined(__ATOMIC64_USE_FAST_EXEC__)
DECLARE_FUNC(ExecHandler::FastAtomicCas64)
#else
DECLARE_FAST_EXEC_INVALID
#endif
#if defined(__ATOMIC64_USE_FAST_EXEC__)
DECLARE_FUNC(ExecHandler::FastAtomicAdd64)
#else
DECLARE_FAST_EXEC_INVALID
#endif
#if defined(__ATOMIC64_USE_FAST_EXEC__)
DECLARE_FUNC(ExecHandler::FastAtomicTau64)
#else
DECLARE_FAST_EXEC_INVALID
#endif
#if defined(__ATOMIC64_USE_FAST_EXEC__)
DECLARE_FUNC(ExecHandler::FastAtomicTas64)
#else
DECLARE_FAST_EXEC_INVALID
#endif
#if defined(__ATOMIC_USE_FAST_EXEC__)
DECLARE_FUNC(ExecHandler::FastAtomicAxo32)
#else
DECLARE_FAST_EXEC_INVALID
#endif
#if defined(__ATOMIC_USE_FAST_EXEC__)
DECLARE_FUNC(ExecHandler::FastAtomicAdd32)
#else
DECLARE_FAST_EXEC_INVALID
#endif
#if defined(__ATOMIC_USE_FAST_EXEC__)
DECLARE_FUNC(ExecHandler::FastAtomicCas32)
#else
DECLARE_FAST_EXEC_INVALID
#endif
#if defined(__ATOMIC_USE_FAST_EXEC__)
DECLARE_FUNC(ExecHandler::FastAtomicTau32)
#else
DECLARE_FAST_EXEC_INVALID
#endif
#if defined(__ATOMIC_USE_FAST_EXEC__)
DECLARE_FUNC(ExecHandler::FastAtomicTas32)
#else
DECLARE_FAST_EXEC_INVALID
#endif
#if defined(__ATOMIC_USE_FAST_EXEC__)
DECLARE_FUNC(ExecHandler::FastAtomicAxo16)
#else
DECLARE_FAST_EXEC_INVALID
#endif
#if defined(__ATOMIC_USE_FAST_EXEC__)
DECLARE_FUNC(ExecHandler::FastAtomicAdd16)
#else
DECLARE_FAST_EXEC_INVALID
#endif
#if defined(__ATOMIC_USE_FAST_EXEC__)
DECLARE_FUNC(ExecHandler::FastAtomicCas16)
#else
DECLARE_FAST_EXEC_INVALID
#endif
#if defined(__ATOMIC_USE_FAST_EXEC__)
DECLARE_FUNC(ExecHandler::FastAtomicTau16)
#else
DECLARE_FAST_EXEC_INVALID
#endif
#if defined(__ATOMIC_USE_FAST_EXEC__)
DECLARE_FUNC(ExecHandler::FastAtomicTas16)
#else
DECLARE_FAST_EXEC_INVALID
#endif
#if defined(__ATOMIC_USE_FAST_EXEC__)
DECLARE_FUNC(ExecHandler::FastAtomicAxo8)
#else
DECLARE_FAST_EXEC_INVALID
#endif
#if defined(__ATOMIC_USE_FAST_EXEC__)
DECLARE_FUNC(ExecHandler::FastAtomicAdd8)
#else
DECLARE_FAST_EXEC_INVALID
#endif
#if defined(__ATOMIC_USE_FAST_EXEC__)
DECLARE_FUNC(ExecHandler::FastAtomicCas8)
#else
DECLARE_FAST_EXEC_INVALID
#endif
#if defined(__ATOMIC_USE_FAST_EXEC__)
DECLARE_FUNC(ExecHandler::FastAtomicTau8)
#else
DECLARE_FAST_EXEC_INVALID
#endif
#if defined(__ATOMIC_USE_FAST_EXEC__)
DECLARE_FUNC(ExecHandler::FastAtomicTas8)
#else
DECLARE_FAST_EXEC_INVALID
#endif
FAST_EXEC_END


#define	SLOW_EXEC_COUNT		268

SLOW_EXEC_BEGIN
DECLARE_WORD(SLOW_EXEC_COUNT)
DECLARE_INVALID_EXEC_HANDLER
DECLARE_EXEC_PREPROCESS_HANDLER
DECLARE_FLAGS_FUNC(0, ExecHandler::ObjectNext)
DECLARE_FLAGS_FUNC(0|EF_C|EF_R|EF_P|(EChunk+1), ExecHandler::ChunkBase)
DECLARE_FLAGS_FUNC(0|EF_C|EF_R|EF_P|(EChunk+1), ExecHandler::ChunkSize)
DECLARE_FLAGS_FUNC(0|EF_C|EF_R|EF_P|(EChunk+1), ExecHandler::ChunkMaxSize)
DECLARE_FLAGS_FUNC(0, ExecHandler::HandleAttributes)
DECLARE_FLAGS_FUNC(0, Kern::TickCount)
DECLARE_FLAGS_FUNC(0|EF_C|EF_P|(ELogicalDevice+1), ExecHandler::LogicalDeviceGetCaps)
DECLARE_FLAGS_FUNC(0|EF_C|EF_P|(ELogicalDevice+1), ExecHandler::LogicalDeviceQueryVersionSupported)
DECLARE_FLAGS_FUNC(0|EF_C|EF_P|(ELogicalDevice+1), ExecHandler::LogicalDeviceIsAvailable)
DECLARE_FLAGS_FUNC(0, ExecHandler::LocaleExports)
DECLARE_FLAGS_FUNC(0|EF_C|EF_P|(ELogicalChannel+1), ExecHandler::ChannelRequest)
DECLARE_FLAGS_FUNC(0|EF_C|EF_R, Kern::Random)
DECLARE_FLAGS_FUNC(0, ExecHandler::IMBRange)
DECLARE_FLAGS_FUNC(0, ExecHandler::ResetMachine)
DECLARE_FLAGS_FUNC(0, ExecHandler::LibraryLookup)
DECLARE_FLAGS_FUNC(0|EF_C|EF_P|(ELibrary+1), ExecHandler::LibraryFileName)
DECLARE_FLAGS_FUNC(0, ExecHandler::ExecuteInSupervisorMode)
DECLARE_FLAGS_FUNC(0|EF_C|EF_R|EF_P|(EMutex+1), ExecHandler::MutexWait)
DECLARE_FLAGS_FUNC(0|EF_C|EF_P|(EMutex+1), ExecHandler::MutexSignal)
DECLARE_FLAGS_FUNC(0|EF_C|EF_R|EF_P|(EProcess+1), ExecHandler::ProcessId)
DECLARE_FLAGS_FUNC(0, ExecHandler::DllFileName)
DECLARE_FLAGS_FUNC(0|EF_C|EF_R|EF_P|(EProcess+1), ExecHandler::ProcessResume)
DECLARE_FLAGS_FUNC(0|EF_C|EF_P|(EProcess+1), ExecHandler::ProcessFileName)
DECLARE_FLAGS_FUNC(0|EF_C|EF_P|(EProcess+1), ExecHandler::ProcessCommandLine)
DECLARE_FLAGS_FUNC(0|EF_C|EF_R|EF_P|(EProcess+1), ExecHandler::ProcessExitType)
DECLARE_FLAGS_FUNC(0|EF_C|EF_R|EF_P|(EProcess+1), ExecHandler::ProcessExitReason)
DECLARE_FLAGS_FUNC(0|EF_C|EF_P|(EProcess+1), ExecHandler::ProcessExitCategory)
DECLARE_FLAGS_FUNC(0|EF_C|EF_R|EF_P|(EProcess+1), ExecHandler::ProcessPriority)
DECLARE_FLAGS_FUNC(0|EF_C|EF_P|(EProcess+1), ExecHandler::ProcessSetPriority)
DECLARE_FLAGS_FUNC(0|EF_C|EF_R|EF_P|(EProcess+1), ExecHandler::ProcessFlags)
DECLARE_FLAGS_FUNC(0|EF_C|EF_R|EF_P|(EProcess+1), ExecHandler::ProcessSetFlags)
DECLARE_FLAGS_FUNC(0|EF_C|EF_P|(ESemaphore+1), ExecHandler::SemaphoreWait)
DECLARE_FLAGS_FUNC(0|EF_C|EF_P|(ESemaphore+1), ExecHandler::SemaphoreSignal1)
DECLARE_FLAGS_FUNC(0|EF_C|EF_P|(ESemaphore+1), ExecHandler::SemaphoreSignalN)
#ifdef __MESSAGE_MACHINE_CODED__
DECLARE_FLAGS_ASMFUNC(0|EF_C|EF_R|EF_P|(EServer+1), _asm_exec_ServerReceive, ExecHandler::ServerReceive)
#else
DECLARE_FLAGS_FUNC(0|EF_C|EF_R|EF_P|(EServer+1), ExecHandler::ServerReceive)
#endif
DECLARE_FLAGS_FUNC(0|EF_C|EF_R|EF_P|(EServer+1), ExecHandler::ServerCancel)
DECLARE_FLAGS_FUNC(0|EF_C|EF_P|(EIpcMessage), ExecHandler::SetSessionPtr)
DECLARE_FLAGS_FUNC(0|EF_A5, ExecHandler::SessionSend)
DECLARE_FLAGS_FUNC(0|EF_C|EF_R|EF_P|(EThread+1), ExecHandler::ThreadId)
DECLARE_FLAGS_FUNC(0, ExecHandler::SessionShare)
DECLARE_FLAGS_FUNC(0|EF_C|EF_R|EF_P|(EThread+1), ExecHandler::ThreadResume)
DECLARE_FLAGS_FUNC(0|EF_C|EF_R|EF_P|(EThread+1), ExecHandler::ThreadSuspend)
DECLARE_FLAGS_FUNC(0|EF_C|EF_R|EF_P|(EThread+1), ExecHandler::ThreadPriority)
DECLARE_FLAGS_FUNC(0|EF_C|EF_R|EF_P|(EThread+1), ExecHandler::ThreadSetPriority)
DECLARE_FLAGS_FUNC(0|EF_C|EF_R|EF_P|(EThread+1), ExecHandler::ThreadProcessPriority)
DECLARE_FLAGS_FUNC(0|EF_C|EF_P|(EThread+1), ExecHandler::ThreadSetProcessPriority)
DECLARE_FLAGS_FUNC(0|EF_C|EF_R|EF_P|(EThread+1), ExecHandler::ThreadFlags)
DECLARE_FLAGS_FUNC(0|EF_C|EF_R|EF_P|(EThread+1), ExecHandler::ThreadSetFlags)
DECLARE_FLAGS_FUNC(0|EF_C|EF_R|EF_P|(EThread+1), ExecHandler::ThreadRequestCount)
DECLARE_FLAGS_FUNC(0|EF_C|EF_R|EF_P|(EThread+1), ExecHandler::ThreadExitType)
DECLARE_FLAGS_FUNC(0|EF_C|EF_R|EF_P|(EThread+1), ExecHandler::ThreadExitReason)
DECLARE_FLAGS_FUNC(0|EF_C|EF_P|(EThread+1), ExecHandler::ThreadExitCategory)
DECLARE_FLAGS_FUNC(0|EF_C|EF_R|EF_P|(ETimer+1), ExecHandler::TimerCancel)
DECLARE_FLAGS_FUNC(0|EF_C|EF_R|EF_P|(ETimer+1), ExecHandler::TimerAfter)
DECLARE_FLAGS_FUNC(0|EF_C|EF_R|EF_P|(ETimer+1), ExecHandler::TimerAt)
DECLARE_FLAGS_FUNC(0|EF_C|EF_R|EF_P|(ETimer+1), ExecHandler::TimerLock)
DECLARE_FLAGS_FUNC(0|EF_C|EF_P|(EChangeNotifier+1), ExecHandler::ChangeNotifierLogon)
DECLARE_FLAGS_FUNC(0|EF_C|EF_P|(EChangeNotifier+1), ExecHandler::ChangeNotifierLogoff)
DECLARE_FLAGS_FUNC(0, ExecHandler::RequestSignal)
DECLARE_FLAGS_FUNC(0, ExecHandler::HandleName)
DECLARE_FLAGS_FUNC(0, ExecHandler::HandleFullName)
DECLARE_FLAGS_FUNC(0, ExecHandler::HandleInfo)
DECLARE_FLAGS_FUNC(0|EF_C|EF_P|(EThread+1), ExecHandler::HandleCount)
DECLARE_FLAGS_FUNC(0, ExecHandler::After)
DECLARE_FLAGS_FUNC(0, ExecHandler::At)
#ifdef __MESSAGE_MACHINE_CODED__
DECLARE_FLAGS_ASMFUNC(0|EF_C|EF_R|EF_P|(EIpcMessageD), _asm_exec_MessageComplete, ExecHandler::MessageComplete)
#else
DECLARE_FLAGS_FUNC(0|EF_C|EF_R|EF_P|(EIpcMessageD), ExecHandler::MessageComplete)
#endif
DECLARE_FLAGS_FUNC(0|EF_C|EF_R|EF_P|(EIpcMessage), ExecHandler::MessageCompleteWithHandle)
DECLARE_FLAGS_FUNC(0|EF_C|EF_R|EF_P|(EIpcMessage), ExecHandler::TransferSession)
DECLARE_FLAGS_FUNC(0, ExecHandler::TimeNow)
DECLARE_FLAGS_FUNC(0, ExecHandler::TimeNowSecure)
DECLARE_FLAGS_FUNC(0, ExecHandler::SetUTCTimeAndOffset)
DECLARE_FLAGS_FUNC(0, ExecHandler::SetMachineConfiguration)
DECLARE_FLAGS_FUNC(0, ExecHandler::CaptureEventHook)
DECLARE_FLAGS_FUNC(0, ExecHandler::ReleaseEventHook)
DECLARE_FLAGS_FUNC(0, ExecHandler::RequestEvent)
DECLARE_FLAGS_FUNC(0, ExecHandler::RequestEventCancel)
DECLARE_FLAGS_FUNC(0, ExecHandler::AddEvent)
DECLARE_FLAGS_FUNC(0|EF_A5, ExecHandler::SessionSendSync)
DECLARE_FLAGS_FUNC(0, ExecHandler::DllTls)
DECLARE_FLAGS_FUNC(0, ExecHandler::HalFunction)
DECLARE_FLAGS_FUNC(0|EF_C|EF_R, ExecHandler::WsRegisterThread)
DECLARE_FLAGS_FUNC(0|EF_C|EF_R, ExecHandler::FsRegisterThread)
DECLARE_FLAGS_FUNC(0|EF_C|EF_R|EF_P|(EProcess+1), ExecHandler::ProcessCommandLineLength)
DECLARE_FLAGS_FUNC(0|EF_C|EF_R|EF_P|(ETimer+1), ExecHandler::TimerInactivity)
DECLARE_FLAGS_FUNC(0, ExecHandler::UserInactivityTime)
DECLARE_FLAGS_FUNC(0, ExecHandler::ResetInactivityTime)
DECLARE_FLAGS_FUNC(0, ExecHandler::DebugPrint)
DECLARE_FLAGS_FUNC(0|EF_C|EF_R, ExecHandler::BreakPoint)
DECLARE_FLAGS_FUNC(0|EF_C|EF_R, ExecHandler::ProfileStart)
DECLARE_FLAGS_FUNC(0|EF_C|EF_R, ExecHandler::ProfileEnd)
DECLARE_FLAGS_FUNC(0|EF_C|EF_R|EF_P|(EThread+1), ExecHandler::ExceptionHandler)
DECLARE_FLAGS_FUNC(0|EF_C|EF_R|EF_P|(EThread+1), ExecHandler::SetExceptionHandler)
DECLARE_FLAGS_FUNC(0|EF_C|EF_R|EF_P|(EThread+1), ExecHandler::ModifyExceptionMask)
DECLARE_FLAGS_FUNC(0|EF_C|EF_R|EF_P|(EThread+1), ExecHandler::RaiseException)
DECLARE_FLAGS_FUNC(0|EF_C|EF_P|(EThread+1), ExecHandler::IsExceptionHandled)
DECLARE_FLAGS_FUNC(0, ExecHandler::ProcessGetMemoryInfo)
DECLARE_FLAGS_FUNC(0, ExecHandler::LibraryGetMemoryInfo)
DECLARE_FLAGS_FUNC(0, ExecHandler::MachineConfiguration)
DECLARE_FLAGS_FUNC(0|EF_C|EF_R, ExecHandler::SetMemoryThresholds)
DECLARE_FLAGS_FUNC(0|EF_C|EF_P|(ELibrary+1), ExecHandler::LibraryType)
DECLARE_FLAGS_FUNC(0|EF_C|EF_P|(EProcess+1), ExecHandler::ProcessType)
DECLARE_FLAGS_FUNC(0|EF_C|EF_R|EF_P|(EChunk+1), ExecHandler::ChunkBottom)
DECLARE_FLAGS_FUNC(0|EF_C|EF_R|EF_P|(EChunk+1), ExecHandler::ChunkTop)
DECLARE_FLAGS_FUNC(0|EF_C|EF_P|(EThread+1), ExecHandler::ThreadContext)
DECLARE_FLAGS_FUNC(0, ExecHandler::ThreadCreate)
DECLARE_FLAGS_FUNC(0, ExecHandler::FindHandleOpen)
DECLARE_FLAGS_FUNC(0, ExecHandler::HandleClose)
DECLARE_FLAGS_FUNC(0, ExecHandler::ChunkCreate)
DECLARE_FLAGS_FUNC(0|EF_C|EF_P|(EChunk+1), ExecHandler::ChunkAdjust)
DECLARE_FLAGS_FUNC(0, ExecHandler::OpenObject)
DECLARE_FLAGS_FUNC(0, ExecHandler::HandleDuplicate)
DECLARE_FLAGS_FUNC(0, ExecHandler::MutexCreate)
DECLARE_FLAGS_FUNC(0, ExecHandler::SemaphoreCreate)
DECLARE_FLAGS_FUNC(0, ExecHandler::ThreadOpenById)
DECLARE_FLAGS_FUNC(0, ExecHandler::ProcessOpenById)
DECLARE_FLAGS_FUNC(0, ExecHandler::ThreadKill)
DECLARE_FLAGS_FUNC(0|EF_C|EF_P|(EThread+1), ExecHandler::ThreadLogon)
DECLARE_FLAGS_FUNC(0|EF_C|EF_P|(EThread+1), ExecHandler::ThreadLogonCancel)
DECLARE_FLAGS_FUNC(0, ExecHandler::DllSetTls)
DECLARE_FLAGS_FUNC(0, ExecHandler::DllFreeTls)
DECLARE_FLAGS_FUNC(0, ExecHandler::ThreadRename)
DECLARE_FLAGS_FUNC(0, ExecHandler::ProcessRename)
DECLARE_FLAGS_FUNC(0, ExecHandler::ProcessKill)
DECLARE_FLAGS_FUNC(0|EF_C|EF_P|(EProcess+1), ExecHandler::ProcessLogon)
DECLARE_FLAGS_FUNC(0|EF_C|EF_P|(EProcess+1), ExecHandler::ProcessLogonCancel)
DECLARE_FLAGS_FUNC(0|EF_C|EF_P|(EThread+1), ExecHandler::ThreadProcess)
DECLARE_FLAGS_FUNC(0, ExecHandler::ServerCreate)
DECLARE_FLAGS_FUNC(0, ExecHandler::ServerCreateWithOptions)
DECLARE_FLAGS_FUNC(0, ExecHandler::SessionCreate)
DECLARE_FLAGS_FUNC(0, ExecHandler::SessionCreateFromHandle)
DECLARE_FLAGS_FUNC(0, ExecHandler::DeviceLoad)
DECLARE_FLAGS_FUNC(0, ExecHandler::DeviceFree)
DECLARE_FLAGS_FUNC(0, ExecHandler::ChannelCreate)
DECLARE_FLAGS_FUNC(0, ExecHandler::TimerCreate)
DECLARE_FLAGS_FUNC(0|EF_C|EF_R|EF_P|(ETimer+1), ExecHandler::TimerHighRes)
DECLARE_FLAGS_FUNC(0|EF_C|EF_R, ExecHandler::AfterHighRes)
DECLARE_FLAGS_FUNC(0, ExecHandler::ChangeNotifierCreate)
DECLARE_FLAGS_FUNC(0, ExecHandler::UndertakerCreate)
DECLARE_FLAGS_FUNC(0|EF_C|EF_R|EF_P|(EUndertaker+1), ExecHandler::UndertakerLogon)
DECLARE_FLAGS_FUNC(0|EF_C|EF_P|(EUndertaker+1), ExecHandler::UndertakerLogonCancel)
DECLARE_FLAGS_FUNC(0, ExecHandler::KernelHeapDebug)
DECLARE_FLAGS_FUNC(0|EF_C|EF_P|(EThread+1), ExecHandler::ThreadGetCpuTime)
DECLARE_FLAGS_FUNC(0, ExecHandler::CodeSegCreate)
DECLARE_FLAGS_FUNC(0, ExecHandler::CodeSegLoaded)
DECLARE_FLAGS_FUNC(0, ExecHandler::LibraryCreate)
DECLARE_FLAGS_FUNC(0, ExecHandler::CodeSegOpen)
DECLARE_FLAGS_FUNC(0, ExecHandler::CodeSegClose)
DECLARE_FLAGS_FUNC(0, ExecHandler::CodeSegNext)
DECLARE_FLAGS_FUNC(0, ExecHandler::CodeSegInfo)
DECLARE_FLAGS_FUNC(0, ExecHandler::CodeSegAddDependency)
DECLARE_FLAGS_FUNC(0, ExecHandler::CodeSegDeferDeletes)
DECLARE_FLAGS_FUNC(0, ExecHandler::CodeSegEndDeferDeletes)
DECLARE_FLAGS_FUNC(0, ExecHandler::ProcessCreate)
DECLARE_FLAGS_FUNC(0, ExecHandler::ProcessLoaded)
DECLARE_FLAGS_FUNC(0|EF_C|EF_R|EF_P|(EIpcClient), ExecHandler::CheckLoaderClientState)
DECLARE_FLAGS_FUNC(0|EF_C|EF_R|EF_P|(EThread+1), ExecHandler::ThreadProcessCodeSeg)
DECLARE_FLAGS_FUNC(0, ExecHandler::CodeSegReadExportDir)
DECLARE_FLAGS_FUNC(0, ExecHandler::WaitDllLock)
DECLARE_FLAGS_FUNC(0, ExecHandler::ReleaseDllLock)
DECLARE_FLAGS_FUNC(0, ExecHandler::LibraryAttach)
DECLARE_FLAGS_FUNC(0, ExecHandler::LibraryAttached)
DECLARE_FLAGS_FUNC(0, ExecHandler::StaticCallList)
DECLARE_FLAGS_FUNC(0, ExecHandler::LibraryDetach)
DECLARE_FLAGS_FUNC(0, ExecHandler::LibraryDetached)
DECLARE_FLAGS_FUNC(0, ExecHandler::LastThreadHandle)
DECLARE_FLAGS_FUNC(0, ExecHandler::ThreadRendezvous)
DECLARE_FLAGS_FUNC(0, ExecHandler::ProcessRendezvous)
DECLARE_FLAGS_FUNC(0|EF_C|EF_R|EF_P|(EIpcMessage), ExecHandler::MessageGetDesLength)
DECLARE_FLAGS_FUNC(0|EF_C|EF_R|EF_P|(EIpcMessage), ExecHandler::MessageGetDesMaxLength)
DECLARE_FLAGS_FUNC(0|EF_C|EF_R|EF_P|(EIpcMessage)|EF_A3, ExecHandler::MessageIpcCopy)
DECLARE_FLAGS_FUNC(0|EF_C|EF_P|(EIpcClient), ExecHandler::MessageClient)
DECLARE_FLAGS_FUNC(0|EF_C|EF_P|(EIpcClient), ExecHandler::MessageSetProcessPriority)
DECLARE_FLAGS_FUNC(0|EF_C|EF_P|(EIpcMessageD), ExecHandler::MessageConstructFromPtr)
DECLARE_FLAGS_FUNC(0, ExecHandler::MessageKill)
DECLARE_FLAGS_FUNC(0|EF_C|EF_P|(EIpcMessage), ExecHandler::MessageOpenObject)
DECLARE_FLAGS_FUNC(0|EF_C|EF_P|(EProcess+1), ExecHandler::ProcessSecurityInfo)
DECLARE_FLAGS_FUNC(0|EF_C|EF_P|(EThread+1), ExecHandler::ThreadSecurityInfo)
DECLARE_FLAGS_FUNC(0|EF_C|EF_P|(EIpcClient), ExecHandler::MessageSecurityInfo)
DECLARE_FLAGS_FUNC(0, ExecHandler::CreatorSecurityInfo)
DECLARE_FLAGS_FUNC(0, ExecHandler::DisabledCapabilities)
DECLARE_FLAGS_FUNC(0|EF_C|EF_R|EF_P|(EChunk+1), ExecHandler::ChunkSetRestrictions)
DECLARE_FLAGS_FUNC(0, ExecHandler::MsgQueueCreate)
DECLARE_FLAGS_FUNC(0, ExecHandler::MsgQueueSend)
DECLARE_FLAGS_FUNC(0|EF_C|EF_P|(EMsgQueue+1), ExecHandler::MsgQueueReceive)
DECLARE_FLAGS_FUNC(0|EF_C|EF_P|(EMsgQueue+1), ExecHandler::MsgQueueNotifySpaceAvailable)
DECLARE_FLAGS_FUNC(0|EF_C|EF_P|(EMsgQueue+1), ExecHandler::MsgQueueCancelSpaceAvailable)
DECLARE_FLAGS_FUNC(0|EF_C|EF_P|(EMsgQueue+1), ExecHandler::MsgQueueNotifyDataAvailable)
DECLARE_FLAGS_FUNC(0|EF_C|EF_P|(EMsgQueue+1), ExecHandler::MsgQueueCancelDataAvailable)
DECLARE_FLAGS_FUNC(0|EF_C|EF_R|EF_P|(EMsgQueue+1), ExecHandler::MsgQueueSize)
DECLARE_FLAGS_FUNC(0, ExecHandler::PropertyDefine)
DECLARE_FLAGS_FUNC(0, ExecHandler::PropertyDelete)
DECLARE_FLAGS_FUNC(0, ExecHandler::PropertyAttach)
DECLARE_FLAGS_FUNC(0|EF_C|EF_P|(EPropertyRef+1), ExecHandler::PropertySubscribe)
DECLARE_FLAGS_FUNC(0|EF_C|EF_P|(EPropertyRef+1), ExecHandler::PropertyCancel)
DECLARE_FLAGS_FUNC(0|EF_C|EF_P|(EPropertyRef+1), ExecHandler::PropertyGetI)
DECLARE_FLAGS_FUNC(0|EF_C|EF_P|(EPropertyRef+1), ExecHandler::PropertyGetB)
DECLARE_FLAGS_FUNC(0|EF_C|EF_P|(EPropertyRef+1), ExecHandler::PropertySetI)
DECLARE_FLAGS_FUNC(0, ExecHandler::PropertySetB)
DECLARE_FLAGS_FUNC(0, ExecHandler::PropertyFindGetI)
DECLARE_FLAGS_FUNC(0, ExecHandler::PropertyFindGetB)
DECLARE_FLAGS_FUNC(0, ExecHandler::PropertyFindSetI)
DECLARE_FLAGS_FUNC(0, ExecHandler::PropertyFindSetB)
DECLARE_FLAGS_FUNC(0, ExecHandler::PowerEnableWakeupEvents)
DECLARE_FLAGS_FUNC(0, ExecHandler::PowerDisableWakeupEvents)
DECLARE_FLAGS_FUNC(0, ExecHandler::PowerRequestWakeupEventNotification)
DECLARE_FLAGS_FUNC(0, ExecHandler::PowerCancelWakeupEventNotification)
DECLARE_FLAGS_FUNC(0, ExecHandler::PowerDown)
DECLARE_FLAGS_FUNC(0|EF_C|EF_R|EF_P|(EProcess+1), ExecHandler::ProcessSetHandleParameter)
DECLARE_FLAGS_FUNC(0, ExecHandler::ProcessSetDataParameter)
DECLARE_FLAGS_FUNC(0|EF_C|EF_R, ExecHandler::ProcessGetHandleParameter)
DECLARE_FLAGS_FUNC(0|EF_C|EF_R, ExecHandler::ProcessGetDataParameter)
DECLARE_FLAGS_FUNC(0|EF_C|EF_R, ExecHandler::ProcessDataParameterLength)
DECLARE_FLAGS_FUNC(0|EF_C|EF_R|EF_P|(EIpcClient), ExecHandler::ThreadProcessFlags)
DECLARE_FLAGS_FUNC(0|EF_C|EF_P|(EThread+1), ExecHandler::ThreadStackInfo)
DECLARE_FLAGS_FUNC(0|EF_C|EF_R|EF_P|(EThread+1), ExecHandler::ThreadGetHeap)
DECLARE_FLAGS_FUNC(0|EF_C|EF_P|(EThread+1), ExecHandler::ThreadAsProcess)
DECLARE_FLAGS_FUNC(0, ExecHandler::CondVarCreate)
DECLARE_FLAGS_FUNC(0|EF_C|EF_P|(ECondVar+1), ExecHandler::CondVarWait)
DECLARE_FLAGS_FUNC(0|EF_C|EF_P|(ECondVar+1), ExecHandler::CondVarSignal)
DECLARE_FLAGS_FUNC(0|EF_C|EF_P|(ECondVar+1), ExecHandler::CondVarBroadcast)
DECLARE_FLAGS_FUNC(0, ExecHandler::PlatSecDiagnostic)
DECLARE_FLAGS_FUNC(0, ExecHandler::ExceptionDescriptor)
DECLARE_FLAGS_ASMFUNC(0|EF_C|EF_P|(EThread+1), _asm_exec_ThreadRequestSignal, ExecHandler::ThreadRequestSignal)
DECLARE_FLAGS_FUNC(0|EF_C|EF_R|EF_P|(EMutex+1), ExecHandler::MutexIsHeld)
DECLARE_FLAGS_FUNC(0, ExecHandler::LeaveStart)
DECLARE_FLAGS_FUNC(0, ExecHandler::LeaveEnd)
DECLARE_FLAGS_FUNC(0, ExecHandler::SetDebugMaskIndex)
DECLARE_FLAGS_FUNC(0, ExecHandler::GetModuleNameFromAddress)
DECLARE_FLAGS_FUNC(0, ExecHandler::NotifyChanges)
DECLARE_FLAGS_FUNC(0, ExecHandler::SetGlobalUserData)
DECLARE_FLAGS_FUNC(0, ExecHandler::SessionSecurityInfo)
DECLARE_FLAGS_FUNC(0|EF_C|EF_R|EF_P|(EIpcMessage), ExecHandler::MessageClientStatus)
DECLARE_FLAGS_FUNC(0, ExecHandler::SetFloatingPointMode)
DECLARE_FLAGS_FUNC(0|EF_A3, ExecHandler::BTraceOut)
DECLARE_FLAGS_FUNC(0|EF_A3, ExecHandler::BTraceOutBig)
DECLARE_FLAGS_FUNC(0|EF_A3, ExecHandler::UTraceOut)
DECLARE_FLAGS_FUNC(0, BTrace::CheckFilter2)
DECLARE_FLAGS_FUNC(0, ExecHandler::ProcessExeExportData)
DECLARE_FLAGS_FUNC(0|EF_C|EF_R, ExecHandler::NotifyIfCodeSegDestroyed)
DECLARE_FLAGS_FUNC(0, ExecHandler::GetDestroyedCodeSegInfo)
#if !defined(__EPOC32__)
DECLARE_FLAGS_FUNC(0, ExecHandler::SetWin32RuntimeHook)
#else
DECLARE_SLOW_EXEC_INVALID
#endif
DECLARE_FLAGS_FUNC(0|EF_C|EF_R|EF_P|(0), ExecHandler::GetBTraceId)
DECLARE_FLAGS_FUNC(0, ExecHandler::NotifyOnIdle)
DECLARE_FLAGS_FUNC(0, ExecHandler::CancelMiscNotifier)
DECLARE_FLAGS_FUNC(0, ExecHandler::NotifyObjectDestruction)
DECLARE_FLAGS_FUNC(0|EF_C|EF_R|EF_P|(EChunk+1), ExecHandler::RegisterTrustedChunk)
DECLARE_FLAGS_FUNC(0, ExecHandler::UserThreadExiting)
#if defined(__ATOMIC64_USE_SLOW_EXEC__)
DECLARE_FLAGS_FUNC(0, ExecHandler::SlowAtomicAxo64)
#else
DECLARE_SLOW_EXEC_INVALID
#endif
#if defined(__ATOMIC64_USE_SLOW_EXEC__)
DECLARE_FLAGS_FUNC(0, ExecHandler::SlowAtomicCas64)
#else
DECLARE_SLOW_EXEC_INVALID
#endif
#if defined(__ATOMIC64_USE_SLOW_EXEC__)
DECLARE_FLAGS_FUNC(0, ExecHandler::SlowAtomicAdd64)
#else
DECLARE_SLOW_EXEC_INVALID
#endif
#if defined(__ATOMIC64_USE_SLOW_EXEC__)
DECLARE_FLAGS_FUNC(0, ExecHandler::SlowAtomicTau64)
#else
DECLARE_SLOW_EXEC_INVALID
#endif
#if defined(__ATOMIC64_USE_SLOW_EXEC__)
DECLARE_FLAGS_FUNC(0, ExecHandler::SlowAtomicTas64)
#else
DECLARE_SLOW_EXEC_INVALID
#endif
DECLARE_FLAGS_FUNC(0|EF_C|EF_R|EF_P|(EChunk+1), ExecHandler::ChunkIsPaged)
DECLARE_FLAGS_FUNC(0|EF_C|EF_R|EF_P|(EProcess+1), ExecHandler::ProcessDefaultDataPaged)
DECLARE_FLAGS_FUNC(0|EF_C|EF_R|EF_P|(EIpcClient), ExecHandler::ThreadFlags)
DECLARE_FLAGS_FUNC(0, ExecHandler::ShPoolCreate)
DECLARE_FLAGS_FUNC(0, ExecHandler::ShPoolAlloc)
DECLARE_FLAGS_FUNC(0|EF_C|EF_P|(EShPool+1), ExecHandler::ShPoolGetInfo)
DECLARE_FLAGS_FUNC(0|EF_C|EF_R|EF_P|(EShPool+1), ExecHandler::ShPoolFreeCount)
DECLARE_FLAGS_FUNC(0|EF_C|EF_P|(EShPool+1), ExecHandler::ShPoolNotification)
DECLARE_FLAGS_FUNC(0|EF_C|EF_P|(EShPool+1), ExecHandler::ShPoolNotificationCancel)
DECLARE_FLAGS_FUNC(0|EF_C|EF_P|(EShPool+1), ExecHandler::ShPoolBufferWindow)
DECLARE_FLAGS_FUNC(0|EF_C|EF_P|(EShBuf+1), ExecHandler::ShBufMap)
DECLARE_FLAGS_FUNC(0|EF_C|EF_P|(EShBuf+1), ExecHandler::ShBufUnMap)
DECLARE_FLAGS_FUNC(0|EF_C|EF_P|(EShBuf+1), ExecHandler::ShBufBaseAndSize)
#if defined(__EPOC32__)
DECLARE_FLAGS_FUNC(0, KernMsgTest)
#else
DECLARE_SLOW_EXEC_INVALID
#endif
#if defined(__EPOC32__)
DECLARE_FLAGS_FUNC(0, GetLatencyValues)
#else
DECLARE_SLOW_EXEC_INVALID
#endif
DECLARE_FLAGS_FUNC(0, CalcKernelHeapUsed)
SLOW_EXEC_END


#endif
