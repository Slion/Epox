/**
Define PE file .SYMBIAN section.
Notably provides UIDs.
@file ekern_uid.cpp
*/

#include <e32cmn.h>
#include <e32cmn_private.h>
#include <e32uid.h>

//const TInt KDynamicLibraryUidValue=0x10000079;
//const TInt KExecutableImageUidValue=0x1000007a;


#pragma data_seg(".SYMBIAN")
__EMULATOR_IMAGE_HEADER2(KExecutableImageUidValue,0x00000000,0x00000000,EPrioritySupervisor,0x00000000u,0x00000000u,0x00000000,0,0x00010000,0)
#pragma data_seg()
