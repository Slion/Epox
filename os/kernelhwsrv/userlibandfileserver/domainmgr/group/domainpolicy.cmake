# Copyright (c) 2012 Stéphane Lenclud.
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
push_target(domainpolicy)
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
#Add our sources
#-------------------------------------------------------------------
set (sourcepath ../src/)
add_source(
domainpolicy.cpp
)

set (sourcepath ../bwins/)
add_source(domainpolicyu.def)

#DLL entry point
#set (sourcepath ../../../../kernelhwsrv/kernel/eka/euser/epoc/win32/)
#add_source(uc_dll.cpp) #uc_atx.cpp

#-------------------------------------------------------------------

#-------------------------------------------------------------------
#Define our source and target
#-------------------------------------------------------------------
get_source(source)
add_library(domainpolicy SHARED ${source} domainpolicy_uid.cpp)
set_target_properties(domainpolicy PROPERTIES LINK_FLAGS "/INCLUDE:__E32Dll")
#set_target_properties(domainpolicy PROPERTIES LINK_FLAGS "/INCLUDE:_DllMain")
target_link_libraries(domainpolicy edll euser)
#-------------------------------------------------------------------


#-------------------------------------------------------------------
#Fix up include directories
#-------------------------------------------------------------------
system_include(/epoc32/include)
system_include(/epoc32/include/platform)
user_include(../inc)
user_include(../src)
#-------------------------------------------------------------------


#-------------------------------------------------------------------
#Fix up compiler defines
#-------------------------------------------------------------------
#add_define(SYMBIAN_TRACE_SYSTEM_INCLUDE)
#add_define(__EXPORT_MEMCPY__)
#add_define(__MEMMODEL_EMUL_SINGLE_HOST_PROCESS__)				
#add_define(__IN_KERNEL__)
#add_define(__KERNEL_MODE__)
#-------------------------------------------------------------------





####################
pop_target(domainpolicy)
####################



