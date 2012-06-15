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
push_target(efile)
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
cl_file.cpp
cl_cli.cpp
cl_dir.cpp
)

set (sourcepath ../sfile/)
add_source(
sf_amh.cpp sf_dat.cpp sf_debug.cpp sf_dir.cpp
sf_drv.cpp sf_file.cpp sf_fmt.cpp
sf_ldr.cpp sf_main.cpp sf_nbs.cpp sf_ses.cpp
sf_svr.cpp sf_sys.cpp sf_utl.cpp sf_mnt.cpp
sf_raw.cpp sf_prel.cpp sf_disk.cpp
sf_request.cpp sf_ops.cpp sf_notify.cpp
sf_thread.cpp sf_local.cpp sf_obj.cpp
sf_ext.cpp sf_hash.cpp
sf_plugin.cpp sf_plugin_ops.cpp sf_plugin_man.cpp
Sf_plugin_util.cpp
sf_cache_man.cpp sf_cache_client.cpp
sf_file_cache.cpp
sf_memory_man.cpp sf_memory_client.cpp
sf_plugin_shim.cpp
sf_notifier_handlers.cpp
#ifdef SYMBIAN_F32_ENHANCED_CHANGE_NOTIFICATION
#sf_pool.cpp sf_notifier.cpp
#endif
sf_lwins.cpp
)


#Guessing our entry point
set (sourcepath ../../../kernel/eka/euser/epoc/win32/)
add_source(
uc_exe.cpp
)


set (sourcepath ../bwins/)
add_source(efileu.def)
#-------------------------------------------------------------------


#-------------------------------------------------------------------
#Define our source and target
#-------------------------------------------------------------------
get_source(source)
add_library(efile SHARED ${source} efile_uid.cpp)
set_target_properties(efile PROPERTIES SUFFIX ".exe")
set_target_properties(efile PROPERTIES LINK_FLAGS /ENTRY:_Win32DllMain)
#add_executable(efile WIN32 ${source} efile_uid.cpp)
target_link_libraries(	efile
						efsrv euser hal btracec emulator
						#scppnwdl
						kernel32 ) #win32												
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
user_include(../sfsrv)
user_include(../ftrace)
user_include(.)
user_include(../inc)
user_include(../../../kernel/eka/include)
#-------------------------------------------------------------------


#-------------------------------------------------------------------
#Fix up compiler defines
#-------------------------------------------------------------------
add_define(SYMBIAN_TRACE_SYSTEM_INCLUDE)
#add_define(SYMBIAN_F32_ENHANCED_CHANGE_NOTIFICATION)
#add_define(__EXPORT_MEMCPY__)
#add_define(__MEMMODEL_EMUL_SINGLE_HOST_PROCESS__)				
#add_define(__IN_KERNEL__)
#add_define(__KERNEL_MODE__)
#-------------------------------------------------------------------





####################
pop_target(efile)
####################



