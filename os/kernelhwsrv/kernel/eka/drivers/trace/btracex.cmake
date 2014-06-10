# Copyright (c) 2009 Stéphane Lenclud.
# All rights reserved.
# This component and the accompanying materials are made available
# under the terms of the License "Eclipse Public License v1.0"
# which accompanies this distribution, and is available
# at the URL "http://www.eclipse.org/legal/epl-v10.html".
#
# Initial Contributors:
# Stéphane Lenclud.
#

####################
push_target(btracex)
####################

#-------------------------------------------------------------------
#Add this *.cmake file to our source tree
#-------------------------------------------------------------------
add_cmake_source()
#Make sure all the output from all projects will go in one place
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
#Visual Studio IDE folders
source_group(Sources REGULAR_EXPRESSION ".+\\.cpp$")
source_group(Includes REGULAR_EXPRESSION ".+\\.h$") 	
source_group(Def REGULAR_EXPRESSION ".+\\.def$") 	
#-------------------------------------------------------------------


#-------------------------------------------------------------------
#Define our source and target
#-------------------------------------------------------------------
get_source(source)
add_library (btracex SHARED ${source} btracex.cpp ../../kernel/win32/d_entry.cpp)
target_link_libraries(btracex emulator ekern)
set_target_properties(btracex PROPERTIES SUFFIX ".ldd")
#-------------------------------------------------------------------


#-------------------------------------------------------------------
#Fix up include directories
#-------------------------------------------------------------------
system_include(/epoc32/include)
system_include(/epoc32/include/platform)
system_include(/epoc32/include/platform/memmodel/emul)
system_include(/epoc32/include/platform/nkern)
system_include(/epoc32/include/platform/nkern/win32)
system_include(/epoc32/include/platform/kernel)
#-------------------------------------------------------------------



#-------------------------------------------------------------------
#Fix up compiler defines
#-------------------------------------------------------------------
add_define(__EXPORT_MEMCPY__)
add_define(__MEMMODEL_EMUL_SINGLE_HOST_PROCESS__)				
add_define(__IN_KERNEL__)
add_define(__KERNEL_MODE__)
#-------------------------------------------------------------------





####################
pop_target(btracex)
####################



