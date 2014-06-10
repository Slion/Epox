// ./os/boardsupport/emulator/emulatorbsp/hal/config.cpp
//
// Copyright (c) 1999-2014 Nokia Corporation and/or its subsidiary(-ies).// All rights reserved.
//
// GENERATED FILE - DO NOT EDIT
//

#include <kernel/hal_int.h>

GLREF_C TInt ProcessCPUSpeed(TInt, TInt, TBool, TAny*);	// ECPUSpeed
GLREF_C TInt GetRAMSize(TInt, TInt, TBool, TAny*);	// EMemoryRAM
GLREF_C TInt GetFreeRAM(TInt, TInt, TBool, TAny*);	// EMemoryRAMFree
GLREF_C TInt GetROMSize(TInt, TInt, TBool, TAny*);	// EMemoryROM
GLREF_C TInt ProcessKeyboardClickState(TInt, TInt, TBool, TAny*);	// EKeyboardClickState
GLREF_C TInt ProcessKeyboardClickVolume(TInt, TInt, TBool, TAny*);	// EKeyboardClickVolume
GLREF_C TInt ProcessDisplayCurrentModeInfo(TInt, TInt, TBool, TAny*);	// EDisplayXPixels
GLREF_C TInt ProcessDisplayCurrentModeInfo(TInt, TInt, TBool, TAny*);	// EDisplayYPixels
GLREF_C TInt ProcessDisplayCurrentModeInfo(TInt, TInt, TBool, TAny*);	// EDisplayXTwips
GLREF_C TInt ProcessDisplayCurrentModeInfo(TInt, TInt, TBool, TAny*);	// EDisplayYTwips
GLREF_C TInt ProcessDisplayColors(TInt, TInt, TBool, TAny*);	// EDisplayColors
GLREF_C TInt ProcessDisplayState(TInt, TInt, TBool, TAny*);	// EDisplayState
GLREF_C TInt ProcessDisplayContrast(TInt, TInt, TBool, TAny*);	// EDisplayContrast
GLREF_C TInt ProcessDisplayMaxContrast(TInt, TInt, TBool, TAny*);	// EDisplayContrastMax
GLREF_C TInt ProcessMouseState(TInt, TInt, TBool, TAny*);	// EMouseState
GLREF_C TInt ProcessMouseSpeed(TInt, TInt, TBool, TAny*);	// EMouseSpeed
GLREF_C TInt ProcessMouseAcceleration(TInt, TInt, TBool, TAny*);	// EMouseAcceleration
GLREF_C TInt GetMouseButtonState(TInt, TInt, TBool, TAny*);	// EMouseButtonState
GLREF_C TInt ProcessLEDMask(TInt, TInt, TBool, TAny*);	// ELEDmask
GLREF_C TInt ProcessDisplaySpecifiedModeInfo(TInt, TInt, TBool, TAny*);	// EDisplayIsMono
GLREF_C TInt ProcessDisplaySpecifiedModeInfo(TInt, TInt, TBool, TAny*);	// EDisplayIsPalettized
GLREF_C TInt ProcessDisplaySpecifiedModeInfo(TInt, TInt, TBool, TAny*);	// EDisplayBitsPerPixel
GLREF_C TInt ProcessDisplayNumModes(TInt, TInt, TBool, TAny*);	// EDisplayNumModes
GLREF_C TInt ProcessDisplayCurrentModeInfo(TInt, TInt, TBool, TAny*);	// EDisplayMemoryAddress
GLREF_C TInt ProcessDisplaySpecifiedModeInfo(TInt, TInt, TBool, TAny*);	// EDisplayOffsetToFirstPixel
GLREF_C TInt ProcessDisplaySpecifiedModeInfo(TInt, TInt, TBool, TAny*);	// EDisplayOffsetBetweenLines
GLREF_C TInt ProcessDisplayCurrentModeInfo(TInt, TInt, TBool, TAny*);	// EDisplayIsPixelOrderRGB
GLREF_C TInt ProcessDisplayCurrentModeInfo(TInt, TInt, TBool, TAny*);	// EDisplayIsPixelOrderLandscape
GLREF_C TInt ProcessDisplayMode(TInt, TInt, TBool, TAny*);	// EDisplayMode
GLREF_C TInt ProcessCustomRestart(TInt, TInt, TBool, TAny*);	// ECustomRestart
GLREF_C TInt ProcessCustomRestartReason(TInt, TInt, TBool, TAny*);	// ECustomRestartReason
GLREF_C TInt GetDisplayNumberOfScreens(TInt, TInt, TBool, TAny*);	// EDisplayNumberOfScreens
GLREF_C TInt ProcessNanoTickPeriod(TInt, TInt, TBool, TAny*);	// ENanoTickPeriod
GLREF_C TInt ProcessFastCounterFrequency(TInt, TInt, TBool, TAny*);	// EFastCounterFrequency
GLREF_C TInt NonsecureClockOffset(TInt, TInt, TBool, TAny*);	// ETimeNonSecureOffset
GLREF_C TInt GetDisplayMemoryHandle(TInt, TInt, TBool, TAny*);	// EDisplayMemoryHandle
GLREF_C TInt GetNumCpus(TInt, TInt, TBool, TAny*);	// ENumCpus

const TUint8 HalInternal::Properties[]=
	{
	HAL::EValid,		// EManufacturer
	HAL::EValid,		// EManufacturerHardwareRev
	HAL::EValid,		// EManufacturerSoftwareRev
	HAL::EValid,		// EManufacturerSoftwareBuild
	HAL::EValid,		// EModel
	HAL::EValid,		// EMachineUid
	HAL::EValid,		// EDeviceFamily
	HAL::EValid,		// EDeviceFamilyRev
	HAL::EValid,		// ECPU
	HAL::EValid,		// ECPUArch
	HAL::EValid,		// ECPUABI
	HAL::EValid|HAL::ESettable,		// ECPUSpeed
	0,		// ESystemStartupReason
	0,		// ESystemException
	HAL::EValid,		// ESystemTickPeriod
	HAL::EValid,		// EMemoryRAM
	HAL::EValid,		// EMemoryRAMFree
	HAL::EValid,		// EMemoryROM
	HAL::EValid,		// EMemoryPageSize
	HAL::EValid,		// EPowerGood
	0,		// EPowerBatteryStatus
	HAL::EValid,		// EPowerBackup
	0,		// EPowerBackupStatus
	HAL::EValid,		// EPowerExternal
	HAL::EValid,		// EKeyboard
	HAL::EValid,		// EKeyboardDeviceKeys
	HAL::EValid,		// EKeyboardAppKeys
	HAL::EValid,		// EKeyboardClick
	HAL::EValid|HAL::ESettable,		// EKeyboardClickState
	HAL::EValid|HAL::ESettable,		// EKeyboardClickVolume
	HAL::EValid,		// EKeyboardClickVolumeMax
	HAL::EValid,		// EDisplayXPixels
	HAL::EValid,		// EDisplayYPixels
	HAL::EValid,		// EDisplayXTwips
	HAL::EValid,		// EDisplayYTwips
	HAL::EValid,		// EDisplayColors
	HAL::EValid|HAL::ESettable,		// EDisplayState
	HAL::EValid|HAL::ESettable,		// EDisplayContrast
	HAL::EValid,		// EDisplayContrastMax
	HAL::EValid,		// EBacklight
	0,		// EBacklightState
	HAL::EValid,		// EPen
	0,		// EPenX
	0,		// EPenY
	0,		// EPenDisplayOn
	0,		// EPenClick
	0,		// EPenClickState
	0,		// EPenClickVolume
	0,		// EPenClickVolumeMax
	HAL::EValid,		// EMouse
	0,		// EMouseX
	0,		// EMouseY
	HAL::EValid|HAL::ESettable,		// EMouseState
	HAL::EValid|HAL::ESettable,		// EMouseSpeed
	HAL::EValid|HAL::ESettable,		// EMouseAcceleration
	HAL::EValid,		// EMouseButtons
	HAL::EValid,		// EMouseButtonState
	0,		// ECaseState
	HAL::EValid,		// ECaseSwitch
	0,		// ECaseSwitchDisplayOn
	0,		// ECaseSwitchDisplayOff
	HAL::EValid,		// ELEDs
	HAL::EValid|HAL::ESettable,		// ELEDmask
	HAL::EValid,		// EIntegratedPhone
	0,		// EDisplayBrightness
	0,		// EDisplayBrightnessMax
	0,		// EKeyboardBacklightState
	0,		// EAccessoryPower
	HAL::EValid|HAL::ESettable,		// ELanguageIndex
	0,		// EKeyboardIndex
	HAL::EValid,		// EMaxRAMDriveSize
	0,		// EKeyboardState
	HAL::EValid|HAL::ESettable,		// ESystemDrive
	0,		// EPenState
	HAL::EValid,		// EDisplayIsMono
	HAL::EValid,		// EDisplayIsPalettized
	HAL::EValid,		// EDisplayBitsPerPixel
	HAL::EValid,		// EDisplayNumModes
	HAL::EValid,		// EDisplayMemoryAddress
	HAL::EValid,		// EDisplayOffsetToFirstPixel
	HAL::EValid,		// EDisplayOffsetBetweenLines
	0,		// EDisplayPaletteEntry
	HAL::EValid,		// EDisplayIsPixelOrderRGB
	HAL::EValid,		// EDisplayIsPixelOrderLandscape
	HAL::EValid|HAL::ESettable,		// EDisplayMode
	0,		// ESwitches
	0,		// EDebugPort
	0,		// ELocaleLoaded
	HAL::EValid,		// EClipboardDrive
	HAL::EValid|HAL::ESettable,		// ECustomRestart
	HAL::EValid,		// ECustomRestartReason
	HAL::EValid,		// EDisplayNumberOfScreens
	HAL::EValid,		// ENanoTickPeriod
	HAL::EValid,		// EFastCounterFrequency
	HAL::EValid,		// EFastCounterCountsUp
	HAL::EValid,		// EPointer3D
	HAL::EValid|HAL::ESettable,		// EPointer3DMaxProximity
	HAL::EValid,		// EPointer3DThetaSupported
	HAL::EValid,		// EPointer3DPhiSupported
	HAL::EValid,		// EPointer3DRotationSupported
	HAL::EValid,		// EPointer3DPressureSupported
	0,		// EHardwareFloatingPoint
	HAL::EValid|HAL::ESettable,		// ETimeNonSecureOffset
	0,		// EPersistStartupModeKernel
	0,		// EMaximumCustomRestartReasons
	0,		// EMaximumRestartStartupModes
	HAL::EValid|HAL::ESettable,		// ECustomResourceDrive
	HAL::EValid,		// EPointer3DProximityStep
	HAL::EValid,		// EPointerMaxPointers
	HAL::EValid|HAL::ESettable,		// EPointerNumberOfPointers
	HAL::EValid,		// EPointer3DMaxPressure
	HAL::EValid,		// EPointer3DPressureStep
	HAL::EValid|HAL::ESettable,		// EPointer3DEnterHighPressureThreshold
	HAL::EValid|HAL::ESettable,		// EPointer3DExitHighPressureThreshold
	HAL::EValid|HAL::ESettable,		// EPointer3DEnterCloseProximityThreshold
	HAL::EValid|HAL::ESettable,		// EPointer3DExitCloseProximityThreshold
	HAL::EValid,		// EDisplayMemoryHandle
	0,		// ESerialNumber
	0,		// ECpuProfilingDefaultInterruptBase
	HAL::EValid		// ENumCpus
	};

#if 0
const TInt HalInternal::Offset[]=
	{
	0,		// EManufacturer
	4,		// EManufacturerHardwareRev
	8,		// EManufacturerSoftwareRev
	12,		// EManufacturerSoftwareBuild
	16,		// EModel
	20,		// EMachineUid
	24,		// EDeviceFamily
	28,		// EDeviceFamilyRev
	32,		// ECPU
	36,		// ECPUArch
	40,		// ECPUABI
	-1,		// ECPUSpeed
	-1,		// ESystemStartupReason
	-1,		// ESystemException
	44,		// ESystemTickPeriod
	-1,		// EMemoryRAM
	-1,		// EMemoryRAMFree
	-1,		// EMemoryROM
	48,		// EMemoryPageSize
	52,		// EPowerGood
	-1,		// EPowerBatteryStatus
	56,		// EPowerBackup
	-1,		// EPowerBackupStatus
	60,		// EPowerExternal
	64,		// EKeyboard
	68,		// EKeyboardDeviceKeys
	72,		// EKeyboardAppKeys
	76,		// EKeyboardClick
	-1,		// EKeyboardClickState
	-1,		// EKeyboardClickVolume
	80,		// EKeyboardClickVolumeMax
	-1,		// EDisplayXPixels
	-1,		// EDisplayYPixels
	-1,		// EDisplayXTwips
	-1,		// EDisplayYTwips
	-1,		// EDisplayColors
	-1,		// EDisplayState
	-1,		// EDisplayContrast
	-1,		// EDisplayContrastMax
	84,		// EBacklight
	-1,		// EBacklightState
	88,		// EPen
	-1,		// EPenX
	-1,		// EPenY
	-1,		// EPenDisplayOn
	-1,		// EPenClick
	-1,		// EPenClickState
	-1,		// EPenClickVolume
	-1,		// EPenClickVolumeMax
	92,		// EMouse
	-1,		// EMouseX
	-1,		// EMouseY
	-1,		// EMouseState
	-1,		// EMouseSpeed
	-1,		// EMouseAcceleration
	96,		// EMouseButtons
	-1,		// EMouseButtonState
	-1,		// ECaseState
	100,		// ECaseSwitch
	-1,		// ECaseSwitchDisplayOn
	-1,		// ECaseSwitchDisplayOff
	104,		// ELEDs
	-1,		// ELEDmask
	108,		// EIntegratedPhone
	-1,		// EDisplayBrightness
	-1,		// EDisplayBrightnessMax
	-1,		// EKeyboardBacklightState
	-1,		// EAccessoryPower
	112,		// ELanguageIndex
	-1,		// EKeyboardIndex
	116,		// EMaxRAMDriveSize
	-1,		// EKeyboardState
	120,		// ESystemDrive
	-1,		// EPenState
	-1,		// EDisplayIsMono
	-1,		// EDisplayIsPalettized
	-1,		// EDisplayBitsPerPixel
	-1,		// EDisplayNumModes
	-1,		// EDisplayMemoryAddress
	-1,		// EDisplayOffsetToFirstPixel
	-1,		// EDisplayOffsetBetweenLines
	-1,		// EDisplayPaletteEntry
	-1,		// EDisplayIsPixelOrderRGB
	-1,		// EDisplayIsPixelOrderLandscape
	-1,		// EDisplayMode
	-1,		// ESwitches
	-1,		// EDebugPort
	-1,		// ELocaleLoaded
	124,		// EClipboardDrive
	-1,		// ECustomRestart
	-1,		// ECustomRestartReason
	-1,		// EDisplayNumberOfScreens
	-1,		// ENanoTickPeriod
	-1,		// EFastCounterFrequency
	128,		// EFastCounterCountsUp
	132,		// EPointer3D
	136,		// EPointer3DMaxProximity
	140,		// EPointer3DThetaSupported
	144,		// EPointer3DPhiSupported
	148,		// EPointer3DRotationSupported
	152,		// EPointer3DPressureSupported
	-1,		// EHardwareFloatingPoint
	-1,		// ETimeNonSecureOffset
	-1,		// EPersistStartupModeKernel
	-1,		// EMaximumCustomRestartReasons
	-1,		// EMaximumRestartStartupModes
	156,		// ECustomResourceDrive
	160,		// EPointer3DProximityStep
	164,		// EPointerMaxPointers
	168,		// EPointerNumberOfPointers
	172,		// EPointer3DMaxPressure
	176,		// EPointer3DPressureStep
	180,		// EPointer3DEnterHighPressureThreshold
	184,		// EPointer3DExitHighPressureThreshold
	188,		// EPointer3DEnterCloseProximityThreshold
	192,		// EPointer3DExitCloseProximityThreshold
	-1,		// EDisplayMemoryHandle
	-1,		// ESerialNumber
	-1,		// ECpuProfilingDefaultInterruptBase
	-1		// ENumCpus
	};

#endif
const TInt HalInternal::HalDataSize=196;

const THalImplementation HalInternal::Implementation[]=
	{
	NULL,		// EManufacturer
	NULL,		// EManufacturerHardwareRev
	NULL,		// EManufacturerSoftwareRev
	NULL,		// EManufacturerSoftwareBuild
	NULL,		// EModel
	NULL,		// EMachineUid
	NULL,		// EDeviceFamily
	NULL,		// EDeviceFamilyRev
	NULL,		// ECPU
	NULL,		// ECPUArch
	NULL,		// ECPUABI
	ProcessCPUSpeed,		// ECPUSpeed
	NULL,		// ESystemStartupReason
	NULL,		// ESystemException
	NULL,		// ESystemTickPeriod
	GetRAMSize,		// EMemoryRAM
	GetFreeRAM,		// EMemoryRAMFree
	GetROMSize,		// EMemoryROM
	NULL,		// EMemoryPageSize
	NULL,		// EPowerGood
	NULL,		// EPowerBatteryStatus
	NULL,		// EPowerBackup
	NULL,		// EPowerBackupStatus
	NULL,		// EPowerExternal
	NULL,		// EKeyboard
	NULL,		// EKeyboardDeviceKeys
	NULL,		// EKeyboardAppKeys
	NULL,		// EKeyboardClick
	ProcessKeyboardClickState,		// EKeyboardClickState
	ProcessKeyboardClickVolume,		// EKeyboardClickVolume
	NULL,		// EKeyboardClickVolumeMax
	ProcessDisplayCurrentModeInfo,		// EDisplayXPixels
	ProcessDisplayCurrentModeInfo,		// EDisplayYPixels
	ProcessDisplayCurrentModeInfo,		// EDisplayXTwips
	ProcessDisplayCurrentModeInfo,		// EDisplayYTwips
	ProcessDisplayColors,		// EDisplayColors
	ProcessDisplayState,		// EDisplayState
	ProcessDisplayContrast,		// EDisplayContrast
	ProcessDisplayMaxContrast,		// EDisplayContrastMax
	NULL,		// EBacklight
	NULL,		// EBacklightState
	NULL,		// EPen
	NULL,		// EPenX
	NULL,		// EPenY
	NULL,		// EPenDisplayOn
	NULL,		// EPenClick
	NULL,		// EPenClickState
	NULL,		// EPenClickVolume
	NULL,		// EPenClickVolumeMax
	NULL,		// EMouse
	NULL,		// EMouseX
	NULL,		// EMouseY
	ProcessMouseState,		// EMouseState
	ProcessMouseSpeed,		// EMouseSpeed
	ProcessMouseAcceleration,		// EMouseAcceleration
	NULL,		// EMouseButtons
	GetMouseButtonState,		// EMouseButtonState
	NULL,		// ECaseState
	NULL,		// ECaseSwitch
	NULL,		// ECaseSwitchDisplayOn
	NULL,		// ECaseSwitchDisplayOff
	NULL,		// ELEDs
	ProcessLEDMask,		// ELEDmask
	NULL,		// EIntegratedPhone
	NULL,		// EDisplayBrightness
	NULL,		// EDisplayBrightnessMax
	NULL,		// EKeyboardBacklightState
	NULL,		// EAccessoryPower
	NULL,		// ELanguageIndex
	NULL,		// EKeyboardIndex
	NULL,		// EMaxRAMDriveSize
	NULL,		// EKeyboardState
	NULL,		// ESystemDrive
	NULL,		// EPenState
	ProcessDisplaySpecifiedModeInfo,		// EDisplayIsMono
	ProcessDisplaySpecifiedModeInfo,		// EDisplayIsPalettized
	ProcessDisplaySpecifiedModeInfo,		// EDisplayBitsPerPixel
	ProcessDisplayNumModes,		// EDisplayNumModes
	ProcessDisplayCurrentModeInfo,		// EDisplayMemoryAddress
	ProcessDisplaySpecifiedModeInfo,		// EDisplayOffsetToFirstPixel
	ProcessDisplaySpecifiedModeInfo,		// EDisplayOffsetBetweenLines
	NULL,		// EDisplayPaletteEntry
	ProcessDisplayCurrentModeInfo,		// EDisplayIsPixelOrderRGB
	ProcessDisplayCurrentModeInfo,		// EDisplayIsPixelOrderLandscape
	ProcessDisplayMode,		// EDisplayMode
	NULL,		// ESwitches
	NULL,		// EDebugPort
	NULL,		// ELocaleLoaded
	NULL,		// EClipboardDrive
	ProcessCustomRestart,		// ECustomRestart
	ProcessCustomRestartReason,		// ECustomRestartReason
	GetDisplayNumberOfScreens,		// EDisplayNumberOfScreens
	ProcessNanoTickPeriod,		// ENanoTickPeriod
	ProcessFastCounterFrequency,		// EFastCounterFrequency
	NULL,		// EFastCounterCountsUp
	NULL,		// EPointer3D
	NULL,		// EPointer3DMaxProximity
	NULL,		// EPointer3DThetaSupported
	NULL,		// EPointer3DPhiSupported
	NULL,		// EPointer3DRotationSupported
	NULL,		// EPointer3DPressureSupported
	NULL,		// EHardwareFloatingPoint
	NonsecureClockOffset,		// ETimeNonSecureOffset
	NULL,		// EPersistStartupModeKernel
	NULL,		// EMaximumCustomRestartReasons
	NULL,		// EMaximumRestartStartupModes
	NULL,		// ECustomResourceDrive
	NULL,		// EPointer3DProximityStep
	NULL,		// EPointerMaxPointers
	NULL,		// EPointerNumberOfPointers
	NULL,		// EPointer3DMaxPressure
	NULL,		// EPointer3DPressureStep
	NULL,		// EPointer3DEnterHighPressureThreshold
	NULL,		// EPointer3DExitHighPressureThreshold
	NULL,		// EPointer3DEnterCloseProximityThreshold
	NULL,		// EPointer3DExitCloseProximityThreshold
	GetDisplayMemoryHandle,		// EDisplayMemoryHandle
	NULL,		// ESerialNumber
	NULL,		// ECpuProfilingDefaultInterruptBase
	GetNumCpus		// ENumCpus
	};

