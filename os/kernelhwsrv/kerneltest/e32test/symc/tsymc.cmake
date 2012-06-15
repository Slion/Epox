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
push_target(tsymc)
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

### EPOC: emulator executable

#-------------------------------------------------------------------
#Define our source and target
#-------------------------------------------------------------------
get_source(source)
add_executable(tsymc WIN32 ${source} ./e32test/symc/tsymc.cpp ./e32test/symc/tsymc_uid.cpp) 
# Using /INCLUDE to obtain same effect has MMP's FIRSTLIB
set_target_properties(tsymc PROPERTIES LINK_FLAGS "/INCLUDE:__E32Startup")
target_link_libraries(tsymc estub eexe euser efsrv)
#-------------------------------------------------------------------



#-------------------------------------------------------------------
#Fix up include directories
#-------------------------------------------------------------------
system_include(/epoc32/include)
system_include(/epoc32/include/platform)
#-------------------------------------------------------------------


#-------------------------------------------------------------------
#Install
#-------------------------------------------------------------------
#Copy test files
install(	FILES 
			${PROJECT_SOURCE_DIR}/os/kernelhwsrv/kerneltest/e32test/symc/tsymc.txt
			DESTINATION ${CMAKE_BINARY_DIR}/\${BUILD_TYPE}/c/tsymc )
#-------------------------------------------------------------------


####################
pop_target(tsymc)
####################