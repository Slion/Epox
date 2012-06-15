/**
Define PE file .SYMBIAN section.
Notably provides UIDs.
@file cenrepcli_uid.cpp
*/

#include <e32cmn.h>
#include <e32cmn_private.h>
#include <e32uid.h>

//const TInt KDynamicLibraryUidValue=0x10000079;
//const TInt KExecutableImageUidValue=0x1000007a;

// VENDORID 0x70000001

#pragma data_seg(".SYMBIAN")
__EMULATOR_IMAGE_HEADER2(KDynamicLibraryUidValue,0x1000008d,0x101FBC70,EPrioritySupervisor,0x00000000u,0x00000000u,0x00000000,0x70000001,0x00010000,KEmulatorImageFlagAllowDllData)
#pragma data_seg()
