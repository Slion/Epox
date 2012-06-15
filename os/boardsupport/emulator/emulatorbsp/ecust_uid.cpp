/**
Define PE file .SYMBIAN section.
Notably provides UIDs.
*/

#include <e32cmn.h>
#include <e32cmn_private.h>

//SL: Currently not used
#pragma data_seg(push,".SYMBIAN")   
__EMULATOR_IMAGE_HEADER2(0x1000007a,0x00000000,0x00000000,EPrioritySupervisor,0x00000000u,0x00000000u,0x00000000,0,0x00010000,0)
#pragma data_seg(pop) 