/**
Define PE file .SYMBIAN section.
Notably provides UIDs.
@file elocd_uid.cpp
*/

#include <e32cmn.h>
#include <e32cmn_private.h>
#include <e32uid.h>

//const TInt KDynamicLibraryUidValue=0x10000079;
//const TInt KExecutableImageUidValue=0x1000007a;


// Hope we did put the vendor UID at the right place
// VENDORID 0x70000001

#pragma data_seg(".SYMBIAN")
//__EMULATOR_IMAGE_HEADER2(KDynamicLibraryUidValue,0,0x100000c9,EPriorityForeground,0x00000000u,0x00000000u,0x00000000,0x70000001,0x00010000,KEmulatorImageFlagAllowDllData)
//0x100000c9 //Why was this used in MMP?
__EMULATOR_IMAGE_HEADER2(KDynamicLibraryUidValue,KLogicalDeviceDriverUidValue,0,EPriorityForeground,0x00000000u,0x00000000u,0x00000000,0x70000001,0x00010000,KEmulatorImageFlagAllowDllData)
#pragma data_seg()
