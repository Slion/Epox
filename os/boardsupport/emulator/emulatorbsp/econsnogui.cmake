# Copyright (c) 2009-2012 Stéphane Lenclud.
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
push_target(econsnogui)
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
#Add project source files
#-------------------------------------------------------------------
set (sourcepath ./)
add_source(econsnogui_uid.cpp)

set (sourcepath ./specific/)
add_source(econsnogui.cpp)
#Def
set (sourcepath ./bwins/)
add_source(econsnoguiU.def)
#-------------------------------------------------------------------

#-------------------------------------------------------------------
#Define our source and target
#-------------------------------------------------------------------
get_source(source)
add_library (econsnogui SHARED ${source})
target_link_libraries(econsnogui edll euser)
#Following is needed to be able to load our DLL dynamically
set_target_properties(econsnogui PROPERTIES LINK_FLAGS "/INCLUDE:__E32Dll")
#WIN32 libraries
#target_link_libraries(econsnogui kernel32 user32 winmm)
#-------------------------------------------------------------------

#-------------------------------------------------------------------
#Fix up include directories
#-------------------------------------------------------------------
system_include(/epoc32/include)
system_include(/epoc32/include/platform)
#-------------------------------------------------------------------

#-------------------------------------------------------------------
#Fix up compiler defines
#-------------------------------------------------------------------
#add_define(__DLL__)
#-------------------------------------------------------------------


####################
pop_target(econsnogui)
####################