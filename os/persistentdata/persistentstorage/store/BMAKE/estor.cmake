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
push_target(estor)
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
#-------------------------------------------------------------------

#-------------------------------------------------------------------
#Add our sources
#-------------------------------------------------------------------
set (sourcepath ../BMAKE/)
add_source(estor_uid.cpp)

set (sourcepath	../UBTREE/)
add_source(UB_FIX.CPP UB_INL.CPP UB_KEY.CPP UB_NODE.CPP UB_TREE.CPP UB_UTL.CPP)
 
set (sourcepath ../UCONT/)
add_source(UC_SET.CPP UC_UTL.CPP)
 
set (sourcepath ../UCRYPT/)
add_source(UE_FLTR.CPP UE_STOR.CPP UE_STRM.CPP UE_UTL.CPP)
 
set (sourcepath ../UFILE/)
add_source(UF_BUF.CPP UF_DICT.CPP UF_DIR.CPP UF_PERM.CPP UF_STOR.CPP)
add_source(UF_STRM.CPP UF_UTL.CPP)
 
set (sourcepath ../ULIB/)
add_source(UL_UTL.CPP)
 
set (sourcepath ../UMEM/)
add_source(UM_BUF.CPP UM_STOR.CPP UM_STRM.CPP UM_UTL.CPP)
 
set (sourcepath ../UPAGE/)
add_source(UP_CACHE.CPP UP_FILE.CPP UP_PAGE.CPP UP_STOR.CPP UP_UTL.CPP)
 
set (sourcepath ../USTOR/)
add_source(UT_COLL.CPP UT_DICT.CPP UT_EMBED.CPP UT_MAP.CPP UT_PERM.CPP)
add_source(UT_STOR.CPP UT_STRM.CPP UT_SWZ.CPP UT_UTL.CPP UT_Iter.cpp)
 
set (sourcepath ../USTRM/)
add_source(US_BUF.CPP US_FLTR.CPP US_FRAME.CPP US_FUNC.CPP US_HOST.CPP)
add_source(US_IO.CPP US_SHARE.CPP US_STRM.CPP US_UTL.CPP US_UCMP.CPP)

#-------------------------------------------------------------------
set (sourcepath ../bwins/)
add_source(estor3.def)
#-------------------------------------------------------------------

#-------------------------------------------------------------------
#Define our source and target
#-------------------------------------------------------------------
get_source(source)
add_library(estor SHARED ${source})
target_link_libraries(estor edll euser efsrv)
set_target_properties(estor PROPERTIES LINK_FLAGS "/INCLUDE:__E32Dll")
#-------------------------------------------------------------------


#-------------------------------------------------------------------
#Fix up include directories
#-------------------------------------------------------------------
user_include(../INC)
user_include(../../../../security/crypto/weakcrypto/inc)
system_include(/epoc32/include)
system_include(/epoc32/include/platform)
#-------------------------------------------------------------------


#-------------------------------------------------------------------
#Fix up compiler defines
#-------------------------------------------------------------------
add_define(SYMBIAN_PKCS12)
#add_define(SYMBIAN_ENABLE_SPLIT_HEADERS)
#add_define(SYMBIAN_ENABLE_PUBLIC_PLATFORM_HEADER_SPLIT)
#add_define(__MEMMODEL_EMUL_SINGLE_HOST_PROCESS__)				
#add_define(__KERNEL_MODE__)
#-------------------------------------------------------------------





####################
pop_target(estor)
####################



