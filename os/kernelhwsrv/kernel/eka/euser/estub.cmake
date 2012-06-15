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
push_target(estub)
####################

#-------------------------------------------------------------------
#Add this *.cmake file to our source tree
#-------------------------------------------------------------------
add_cmake_source()
#Make sure all the output from all projects will go in one place
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/../bin)
#Visual Studio IDE folders
source_group(Sources REGULAR_EXPRESSION ".+\\.cpp$")
source_group(Includes REGULAR_EXPRESSION ".+\\.h$") 	
source_group(Def REGULAR_EXPRESSION ".+\\.def$") 	
#-------------------------------------------------------------------


#-------------------------------------------------------------------
#Define our source and target
#-------------------------------------------------------------------
get_source(source)
add_library (estub STATIC ${source} ../euser/epoc/symc/uc_stub.cpp)
#set_target_properties(estub PROPERTIES LINK_FLAGS /ENTRY:E32Bootstrap)
#add_define(__DLL__)
#-------------------------------------------------------------------


#-------------------------------------------------------------------
#Fix up include directories
#-------------------------------------------------------------------
#include_directories(../../include/nkern)
#include_directories(../../include/nkern/win32)
#include_directories(../../include/memmodel/emul)
#include_directories(../../include/kernel/win32)
system_include(/epoc32/include)
system_include(/epoc32/include/platform)
#user_include(../sfsrv)
#user_include(../ftrace)
#user_include(.)
#user_include(../inc)
#user_include(../../../kernel/eka/include)
#-------------------------------------------------------------------


####################
pop_target(estub)
####################