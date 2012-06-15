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
push_target(hal)
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
#Add custom commands to generate config.cpp and values.cpp
#-------------------------------------------------------------------
#perl halcfg.pl hal_data.h values.hda values.cpp
add_custom_command (
	OUTPUT ${PROJECT_SOURCE_DIR}/os/boardsupport/emulator/emulatorbsp/hal/values.cpp	
   	COMMAND perl ./os/kernelhwsrv/halservices/hal/halcfg.pl ./os/kernelhwsrv/halservices/hal/inc/hal_data.h ./os/boardsupport/emulator/emulatorbsp/hal/values.hda ./os/boardsupport/emulator/emulatorbsp/hal/values.cpp
	WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}	
   	COMMENT "Generating HAL values..."
   	DEPENDS ./os/kernelhwsrv/halservices/hal/halcfg.pl ./os/kernelhwsrv/halservices/hal/inc/hal_data.h ./os/boardsupport/emulator/emulatorbsp/hal/values.hda
   	VERBATIM
 	)

#perl halcfg.pl -x hal_data.h config.hcf config.cpp
add_custom_command (
	OUTPUT ${PROJECT_SOURCE_DIR}/os/boardsupport/emulator/emulatorbsp/hal/config.cpp	
   	COMMAND perl ./os/kernelhwsrv/halservices/hal/halcfg.pl -x ./os/kernelhwsrv/halservices/hal/inc/hal_data.h ./os/boardsupport/emulator/emulatorbsp/hal/config.hcf ./os/boardsupport/emulator/emulatorbsp/hal/config.cpp
	WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}	
   	COMMENT "Generating HAL config..."
   	DEPENDS ./os/kernelhwsrv/halservices/hal/halcfg.pl ./os/kernelhwsrv/halservices/hal/inc/hal_data.h ./os/boardsupport/emulator/emulatorbsp/hal/config.hcf
   	VERBATIM
 	)	
	
#Tell cmake those files won't be available until build time
set_source_files_properties(${PROJECT_SOURCE_DIR}/os/boardsupport/emulator/emulatorbsp/hal/values.cpp PROPERTIES GENERATED 1)
set_source_files_properties(${PROJECT_SOURCE_DIR}/os/boardsupport/emulator/emulatorbsp/hal/config.cpp  PROPERTIES GENERATED 1)
#-------------------------------------------------------------------	



#-------------------------------------------------------------------
#Define our source and target
#-------------------------------------------------------------------

set (sourcepath ../../../../kernelhwsrv/halservices/hal/src/)
add_source(hal_main.cpp userhal.cpp)

#Hoping we include the right entry point
set (sourcepath ../../../../kernelhwsrv/kernel/eka/euser/epoc/win32/)
#add_source(uc_dll.cpp) #uc_atx.cpp



#TODO: add the generated CPPs 

get_source(source)
add_library (hal SHARED ${source}
						${PROJECT_SOURCE_DIR}/os/boardsupport/emulator/emulatorbsp/hal/config.cpp 
						${PROJECT_SOURCE_DIR}/os/boardsupport/emulator/emulatorbsp/hal/values.cpp
						hal.cpp hal_uid.cpp)
#set_target_properties(hal PROPERTIES LINK_FLAGS "/ENTRY:_Win32DllMain") #Change the entry point						
#set_target_properties(hal PROPERTIES COMPILE_FLAGS "/Zl") #No default library
set_target_properties(hal PROPERTIES LINK_FLAGS "/INCLUDE:__E32Dll")
#set_target_properties(hal PROPERTIES LINK_FLAGS "/INCLUDE:_DllMain")
target_link_libraries(hal edll euser)
#-------------------------------------------------------------------

#-------------------------------------------------------------------
#Fix up include directories
#-------------------------------------------------------------------
system_include(/epoc32/include)
system_include(/epoc32/include/platform)
user_include(../../../../kernelhwsrv/kernel/eka/include)
#-------------------------------------------------------------------

#-------------------------------------------------------------------
#Fix up compiler defines
#-------------------------------------------------------------------
add_define(__DLL__)
#-------------------------------------------------------------------


#-------------------------------------------------------------------
#Export includes
#-------------------------------------------------------------------

public_export(${PROJECT_SOURCE_DIR}/os/kernelhwsrv/halservices/hal/inc/hal_int.h /kernel)
public_export(${PROJECT_SOURCE_DIR}/os/kernelhwsrv/halservices/hal/inc/hal_data.h /)
public_export(${PROJECT_SOURCE_DIR}/os/kernelhwsrv/halservices/hal/inc/hal.h /)

#-------------------------------------------------------------------


####################
pop_target(hal)
####################



