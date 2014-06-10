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

push_target(soundsc)

#Add current file to our source tree
add_cmake_source()
#Make sure all the output from all projects will go in one place
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)


include_directories(./inc)
include_directories(../../../kernelhwsrv/kernel/eka/include/kernel/win32)
include_directories(../../../kernelhwsrv/kernel/eka/include/memmodel/emul)
include_directories(../../../kernelhwsrv/kernel/eka/include)
include_directories(../../../kernelhwsrv/kernel/eka/include/nkern)
include_directories(../../../kernelhwsrv/kernel/eka/include/nkern/win32)


#Hoping we include the right entry point
set (sourcepath ../../../kernelhwsrv/kernel/eka/kernel/win32/)
add_source(v_entry.cpp)

set (sourcepath ../../../kernelhwsrv/kernel/eka/drivers/power/binary/)
add_source(
bpower.cpp
)

set (sourcepath ./specific/)
add_source(
variant.cpp timer.cpp property.cpp
)

#Hoping we include the right entry point
set (sourcepath ./bwins/)
add_source(vwinsu.def)

get_source(source)
add_library (ecust SHARED ${source})
target_link_libraries(ecust emulator ekern)
#WIN32 libraries
target_link_libraries(ecust kernel32 user32 winmm)
#
add_define(__EXPORT_MEMCPY__)				
add_define(__IN_KERNEL__)
add_define(__KERNEL_MODE__)
add_define(__MEMMODEL_EMUL_SINGLE_HOST_PROCESS__)


#Create our data directory for the emulator
#See 
#file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/\${BUILD_TYPE}/data)				
#execute_process(COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/${BUILD_TYPE}/data)

#add_custom_command (
# 	TARGET ecust
#   	PRE_BUILD
#   	COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}${CMAKE_CFG_INTDIR}/data
# 	)


add_custom_target (
	createEmuDataFolder
   	COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/${CMAKE_CFG_INTDIR}/data
 	)

pop_target(ecust)