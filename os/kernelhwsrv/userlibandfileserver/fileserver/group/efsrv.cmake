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
push_target(efsrv)
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
#Add our sources
#-------------------------------------------------------------------
set (sourcepath ../sfsrv/)
add_source(
cl_cdir.cpp cl_cli.cpp cl_debug.cpp cl_dir.cpp
cl_drive.cpp cl_entry.cpp cl_file.cpp cl_find.cpp
cl_fman.cpp cl_fmt.cpp cl_fraw.cpp cl_ftext.cpp
cl_main.cpp cl_parse.cpp cl_scan.cpp cl_insecure.cpp
cl_plugin.cpp cl_blockmap.cpp cl_clamp.cpp
cl_shim.cpp
cl_notification.cpp)

set (sourcepath ../bwins/)
add_source(efsrvu.def)

#DLL entry point
set (sourcepath ../../../../kernelhwsrv/kernel/eka/euser/epoc/win32/)
add_source(uc_dll.cpp) #uc_atx.cpp

#-------------------------------------------------------------------

#-------------------------------------------------------------------
#Define our source and target
#-------------------------------------------------------------------
get_source(source)
add_library(efsrv SHARED ${source} efsrv_uid.cpp)

target_link_libraries(efsrv hal) 
#-------------------------------------------------------------------


#-------------------------------------------------------------------
#Fix up include directories
#-------------------------------------------------------------------
system_include(/epoc32/include)
system_include(/epoc32/include/platform)
user_include(../ftrace)
user_include(../inc)
user_include(../../../../kernelhwsrv/kernel/eka/include)
#-------------------------------------------------------------------


#-------------------------------------------------------------------
#Fix up compiler defines
#-------------------------------------------------------------------
add_define(SYMBIAN_TRACE_SYSTEM_INCLUDE)
#add_define(__EXPORT_MEMCPY__)
#add_define(__MEMMODEL_EMUL_SINGLE_HOST_PROCESS__)				
#add_define(__IN_KERNEL__)
#add_define(__KERNEL_MODE__)
#-------------------------------------------------------------------





####################
pop_target(efsrv)
####################



