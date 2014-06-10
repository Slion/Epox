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
push_target(centralrepository)
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
set (sourcepath ./)
add_source(cenrepcli_uid.cpp)

set (sourcepath ../cenrepcli/)
add_source(centralrepository.cpp clirep.cpp)

#-------------------------------------------------------------------
set (sourcepath ../bwins/)
add_source(CENTRALREPOSITORYU.def)
#-------------------------------------------------------------------

#-------------------------------------------------------------------
#Define our source and target
#-------------------------------------------------------------------
get_source(source)
add_library(centralrepository SHARED ${source})
target_link_libraries(centralrepository edll euser) 
#-------------------------------------------------------------------


#-------------------------------------------------------------------
#Fix up include directories
#-------------------------------------------------------------------
user_include(../include)
user_include(../cenrepsrv)
system_include(/epoc32/include)
system_include(/epoc32/include/platform)
#-------------------------------------------------------------------


#-------------------------------------------------------------------
#Fix up compiler defines
#-------------------------------------------------------------------
#add_define(SYMBIAN_PKCS12)
#add_define(SYMBIAN_ENABLE_SPLIT_HEADERS)
#add_define(SYMBIAN_ENABLE_PUBLIC_PLATFORM_HEADER_SPLIT)
#add_define(__MEMMODEL_EMUL_SINGLE_HOST_PROCESS__)				
#add_define(__KERNEL_MODE__)
#-------------------------------------------------------------------





####################
pop_target(centralrepository)
####################



