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
push_target(elocal)
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
set (sourcepath ../swins/)
add_source(elocal.cpp)

set (sourcepath ../sfat/)
#add_source(fat_config.cpp sl_cache.cpp sl_dir.cpp)
#add_source(sl_disk.cpp sl_drv.cpp sl_file.cpp)
#add_source(sl_fmt.cpp sl_fsy.cpp sl_main.cpp sl_mnt.cpp)
#add_source(sl_utl.cpp sl_vfat.cpp sl_bpb.cpp)
#add_source(sl_scan.cpp sl_check.cpp)
#add_source(sl_fat16.cpp sl_mnt16.cpp sl_fatcache.cpp fat_table.cpp ram_fat_table.cpp)
#add_source(sl_leafdir_cache.cpp)
#add_source(sl_dir_cache.cpp)
#
add_source(elocal_uid.cpp)

##-- Entry point
#../../../kernel/eka/kernel/win32/d_entry.cpp
set (sourcepath ../../../kernel/eka/euser/epoc/win32/)
add_source(uc_dll.cpp) #uc_atx.cpp
#-------------------------------------------------------------------





#-------------------------------------------------------------------
#Define our source and target
#-------------------------------------------------------------------
get_source(source)
add_library (elocal SHARED ${source})
target_link_libraries(	elocal 
						emulator hal efsrv euser efile
						kernel32) #win32
set_target_properties(elocal PROPERTIES SUFFIX ".fsy")
#-------------------------------------------------------------------


#-------------------------------------------------------------------
#Fix up include directories
#-------------------------------------------------------------------
system_include(/epoc32/include)
system_include(/epoc32/include/platform)
#system_include(/epoc32/include/platform/memmodel/emul)
#system_include(/epoc32/include/platform/nkern)
#system_include(/epoc32/include/platform/nkern/win32)
#system_include(/epoc32/include/platform/kernel)
#user_include(../sfat)
#user_include(../sfat/inc)
#user_include(../sfile)
user_include(../inc)
#-------------------------------------------------------------------



#-------------------------------------------------------------------
#Fix up compiler defines
#-------------------------------------------------------------------
add_define(SYMBIAN_TRACE_SYSTEM_INCLUDE)
#-------------------------------------------------------------------





####################
pop_target(elocal)
####################



