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
push_target(bafl)
####################

#-------------------------------------------------------------------
#Add this *.cmake file to our source tree
#-------------------------------------------------------------------
add_cmake_source()
#Make sure all the output from all projects will go in one place
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/../bin)
#Visual Studio IDE folders
source_group(Sources REGULAR_EXPRESSION ".+\\.cpp$")
source_group(Sources REGULAR_EXPRESSION ".+\\.CPP$")
source_group(Includes REGULAR_EXPRESSION ".+\\.h$") 	
source_group(Includes REGULAR_EXPRESSION ".+\\.H$") 	
source_group(Def REGULAR_EXPRESSION ".+\\.def$") 	
source_group(Def REGULAR_EXPRESSION ".+\\.DEF$") 	
#-------------------------------------------------------------------

#-------------------------------------------------------------------
#Add our sources
#-------------------------------------------------------------------
set (sourcepath ./)
add_source(bafl_uid.cpp)

set (sourcepath ../src/)
add_source(babackup.cpp  BACELL.CPP   BACLINE.CPP  BACLIPB.CPP   BACNTF.CPP
BADESCA.CPP   BAFINDF.CPP  BAMATCH.CPP  BARSREAD.CPP  BARSC.CPP   
BASCHED.CPP   BASSND.CPP   BAUTILS.CPP  BANAMEDPLUGINS.CPP
BADICTIONARYCOMPRESSION.CPP
StringPoolAPI.cpp StringPoolImplementation.cpp stringlogger.cpp
BaRscImpl.cpp BaAssert.cpp BaRsc2.cpp BaRsRead2.cpp
BaRsReadImpl.cpp
BaSPI.cpp BaArchiveImpl.cpp
BaSsndStore.cpp)

#-------------------------------------------------------------------
set (sourcepath ../bwins/)
add_source(BAFLU.DEF)
#-------------------------------------------------------------------

#-------------------------------------------------------------------
#Define our source and target
#-------------------------------------------------------------------
get_source(source)
add_library(bafl SHARED ${source})
target_link_libraries(bafl edll euser hal efsrv  estor centralrepository) 
#-------------------------------------------------------------------


#-------------------------------------------------------------------
#Fix up include directories
#-------------------------------------------------------------------
user_include(../inc)
user_include(../src/inc)
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
pop_target(bafl)
####################



