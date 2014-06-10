/**
Define PE file .SYMBIAN section.
Notably provides UIDs.
@file econsnogui_uid.cpp
*/

#include <e32cmn.h>
#include <e32cmn_private.h>
#include <e32uid.h>

//const TInt KDynamicLibraryUidValue=0x10000079;
//const TInt KExecutableImageUidValue=0x1000007a;

// VENDORID 0x70000001

#pragma data_seg(push,".SYMBIAN")
__EMULATOR_IMAGE_HEADER2(KDynamicLibraryUidValue,0x1000008d,0x100039e7,EPrioritySupervisor,0x00000000u,0x00000000u,0x00000000,0x70000001,0x00010000,0)
#pragma data_seg(pop)

	