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


#Make sure all the output from all projects will go in one place
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/../bin)
#------------------------------------------------------------------

project (symbian)

#Global defines will apply to all our binaries
add_definitions(-DUNICODE)
add_definitions(-D_UNICODE)
add_definitions(-D__VC32__)
add_definitions(-D__WINS__)
add_definitions(-D__SYMC__)
add_definitions(-D__LEAVE_EQUALS_THROW__)
add_definitions(-D__SUPPORT_CPP_EXCEPTIONS__)
add_definitions(-D__PLATSEC_UNLOCKED__)
# TODO: I guess public stuff should also define SYMBIAN_ENABLE_SPLIT_HEADERS 



#-------------------------------------------------------------
add_subdirectory(./os/kernelhwsrv/kernel/eka)
## Add ecust DLL
add_subdirectory(./os/boardsupport/emulator/emulatorbsp)
## Add btracex LDD
add_subdirectory(./os/kernelhwsrv/kernel/eka/drivers/trace)
## Add elocd.ldd
add_subdirectory(./os/kernelhwsrv/kernel/eka/drivers/locmedia)
## Add efile.exe
add_subdirectory(./os/kernelhwsrv/userlibandfileserver/fileserver/group)
## Add domain manager
add_subdirectory(./os/kernelhwsrv/userlibandfileserver/domainmgr/group)
## Add hal.dll
add_subdirectory(./os/boardsupport/emulator/emulatorbsp/hal)
## Add estor, centralrepository...
add_subdirectory(./os/persistentdata)
## Add bafl
add_subdirectory(./os/ossrv/lowlevellibsandfws/apputils/group)
## Add bsul
add_subdirectory(./os/ossrv/lowlevellibsandfws/apputils/bsul/group)
## Add charconv
add_subdirectory(./os/textandloc/charconvfw/charconv_fw/group)
## Add Multimedia
add_subdirectory(./os/mm/mmlibs/mmfw/group)
## Add window server
add_subdirectory(./os/graphics/windowing/windowserver/group)
## Add font store
add_subdirectory(./os/textandloc/fontservices/fontstore/group)
## Add gdi
add_subdirectory(./os/graphics/graphicsdeviceinterface/gdi/group)
add_subdirectory(./os/graphics/graphicsdeviceinterface/bitgdi/group)
add_subdirectory(./os/graphics/graphicsdeviceinterface/screendriver/group)
add_subdirectory(./os/graphics/fbs/fontandbitmapserver/group)
## Add ecom
add_subdirectory(./os/ossrv/lowlevellibsandfws/pluginfw/Group)
## Add tests
add_subdirectory(./os/kernelhwsrv/kerneltest)
#-------------------------------------------------------------

#Add a custom target just to group our cmake files together
file(GLOB_RECURSE DotCMakeFiles	"./*.cmake")
add_custom_target(symbian SOURCES ${source}	${DotCMakeFiles} ./symc/epoc.ini )					
source_group(CMake FILES ${DotCMakeFiles} ${CMAKE_CURRENT_LIST_FILE} )	
source_group(Data FILES ./symc/epoc.ini)	

#Copy epoc.ini to binary directory
install(	FILES 
			${PROJECT_SOURCE_DIR}/symc/epoc.ini
			DESTINATION ${CMAKE_BINARY_DIR}/\${BUILD_TYPE}/data )
			
