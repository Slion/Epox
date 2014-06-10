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
push_target(e32strt)
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
set (sourcepath ../../../kernelhwsrv/userlibandfileserver/fileserver/estart/)
add_source(estart.cpp)

set (sourcepath ./estart/)
add_source(estartmain.cpp)


#Guessing our entry point
#set (sourcepath ../../../kernelhwsrv/kernel/eka/euser/epoc/win32/)
#add_source(uc_exe.cpp)


#-------------------------------------------------------------------


#-------------------------------------------------------------------
#Define our source and target
#-------------------------------------------------------------------
get_source(source)
#add_executable(e32strt WIN32 ${source} estart_uid.cpp)
add_library(e32strt SHARED ${source} estart_uid.cpp)
set_target_properties(e32strt PROPERTIES SUFFIX ".exe")
set_target_properties(e32strt PROPERTIES LINK_FLAGS "/INCLUDE:__E32Startup")

target_link_libraries(	e32strt
						eexe
						efsrv euser hal domaincli emulator
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
user_include(../../../kernelhwsrv/userlibandfileserver/fileserver/estart)
#user_include(../ftrace)
#user_include(.)
#user_include(../inc)
#user_include(../../../kernel/eka/include)
#-------------------------------------------------------------------


#-------------------------------------------------------------------
#Fix up compiler defines
#-------------------------------------------------------------------
#add_define(SYMBIAN_TRACE_SYSTEM_INCLUDE)
#add_define(SYMBIAN_F32_ENHANCED_CHANGE_NOTIFICATION)
#add_define(__EXPORT_MEMCPY__)
#add_define(__MEMMODEL_EMUL_SINGLE_HOST_PROCESS__)				
#add_define(__IN_KERNEL__)
#add_define(__KERNEL_MODE__)
#-------------------------------------------------------------------





####################
pop_target(e32strt)
####################



