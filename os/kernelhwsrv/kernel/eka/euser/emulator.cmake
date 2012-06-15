# Copyright (c) 2012 St�phane Lenclud.
# All rights reserved.
# This component and the accompanying materials are made available
# under the terms of the License "Eclipse Public License v1.0"
# which accompanies this distribution, and is available
# at the URL "http://www.eclipse.org/legal/epl-v10.html".
#
# Initial Contributors:
# St�phane Lenclud.
#

####################
push_target(emulator)
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
set (sourcepath ./epoc/win32/)
add_source(emulator.cpp)
#DEF file messing things up!
#set (sourcepath ../bwins/)
#add_source(emulatoru.def)
#-------------------------------------------------------------------


#-------------------------------------------------------------------
#Define our source and target
#-------------------------------------------------------------------
get_source(source)
add_library (emulator SHARED ${source})											
target_link_libraries(emulator 
						ekll
						kernel32) #win32
#-------------------------------------------------------------------


### EMULATOR: used by euser and kernel for emulation
#add_library (emulator SHARED ../euser/epoc/win32/emulator.cpp) #Compile the KLIB target entry point which is l_entry.cpp:  ../kernel/win32/l_entry.cpp
#set_target_properties(emulator PROPERTIES COMPILE_FLAGS "/Zl") #No default library
#set_target_properties(emulator PROPERTIES LINK_FLAGS "/ENTRY:_Win32DllMain") #Change the entry point



#-------------------------------------------------------------------
#Fix up include directories
#-------------------------------------------------------------------
system_include(/epoc32/include)
system_include(/epoc32/include/platform)
user_include(../include)
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
pop_target(emulator)
####################



