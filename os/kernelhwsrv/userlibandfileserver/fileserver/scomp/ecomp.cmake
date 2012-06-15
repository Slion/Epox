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
push_target(ecomp)
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
# DEF file to make sure exports ordinals are correct
#-------------------------------------------------------------------
#set (sourcepath ../../bwins/)
#add_source(elocdu.def)
#-------------------------------------------------------------------


#-------------------------------------------------------------------
# Add sources
#-------------------------------------------------------------------
set (sourcepath ../scomp/)
add_source(sc_comp.cpp)
add_source(ecomp_uid.cpp)

##-- Entry point
#../../../kernel/eka/kernel/win32/d_entry.cpp
set (sourcepath ../../../kernel/eka/euser/epoc/win32/)
#add_source(uc_dll.cpp) #uc_atx.cpp
#-------------------------------------------------------------------





#-------------------------------------------------------------------
#Define our source and target
#-------------------------------------------------------------------
get_source(source)
add_library (ecomp SHARED ${source})
target_link_libraries(	ecomp 
						edll efile efsrv euser
						) 
set_target_properties(ecomp PROPERTIES SUFFIX ".fsy")
set_target_properties(ecomp PROPERTIES LINK_FLAGS "/INCLUDE:__E32Dll")
#set_target_properties(ecomp PROPERTIES LINK_FLAGS "/INCLUDE:_DllMain")
#-------------------------------------------------------------------


#-------------------------------------------------------------------
#Fix up include directories
#-------------------------------------------------------------------
system_include(/epoc32/include)
system_include(/epoc32/include/platform)
#
user_include(../inc)
#-------------------------------------------------------------------



#-------------------------------------------------------------------
#Fix up compiler defines
#-------------------------------------------------------------------
add_define(SYMBIAN_TRACE_SYSTEM_INCLUDE)
#-------------------------------------------------------------------





####################
pop_target(ecomp)
####################



