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

#
# This file contains some generic cmake macros we are using for building Symbian
#

#-----------------------------------------------------
#Push a target on our target stack
macro(push_target targetName)
	get_property(targetStackValue GLOBAL PROPERTY targetStack)
	list(APPEND targetStackValue ${targetName})
	set_property(GLOBAL PROPERTY targetStack ${targetStackValue})
	#message("Push target: ${targetStackValue}")
endmacro(push_target) 

#-----------------------------------------------------
#Pop the given target from our target stack
macro(pop_target targetName)
	get_property(targetStackValue GLOBAL PROPERTY targetStack)
	list(LENGTH targetStackValue count)	
	math( EXPR count "${count} - 1")
	list(GET targetStackValue ${count} poppedTargetName)
	if (NOT(${poppedTargetName} STREQUAL ${targetName}))
		message(FATAL_ERROR "pop_target expecting ${poppedTargetName} instead of ${targetName}!")
	endif (NOT(${poppedTargetName} STREQUAL ${targetName}))
	list(REMOVE_AT targetStackValue ${count})
	set_property(GLOBAL PROPERTY targetStack ${targetStackValue})	
	#message("Pop target: ${targetName} :to: ${targetStackValue}")
endmacro(pop_target) 

#-----------------------------------------------------
#Get the name of the target from the top of our stack
macro(get_target targetName)
	list(LENGTH targetStackValue count)	
	math( EXPR count "${count} - 1")
	set(${targetName} "")
	if (NOT(${count} LESS 0))
		list(GET targetStackValue ${count} ${targetName})
	endif (NOT(${count} LESS 0))
endmacro(get_target) 

#-----------------------------------------------------
#Add the given source files to our source prepending the sourcepath
macro(add_source)
	get_target(targetName)
	set(sourcesProperty "${targetName}Sources")
	get_property(sourcesPropertyValue GLOBAL PROPERTY ${sourcesProperty})
		
	foreach(mySource ${ARGV})
		string(REGEX REPLACE "(^.+)" "${sourcepath}\\1" newsource ${mySource})		
		set(sourcesPropertyValue ${sourcesPropertyValue} ${newsource})		
	endforeach(mySource) 
	
	set_property(GLOBAL PROPERTY ${sourcesProperty} ${sourcesPropertyValue})
	#message("${targetName} sources: ${sourcesPropertyValue}")
endmacro(add_source)

#-----------------------------------------------------
#Get the sources for the current target
macro(get_source sourceVar)
	get_target(targetName)
	set(sourcesProperty "${targetName}Sources")
	get_property(sourcesPropertyValue GLOBAL PROPERTY ${sourcesProperty})
	set(${sourceVar} ${sourcesPropertyValue})
	#message("${targetName} sources: ${sourcesPropertyValue}")	
endmacro(get_source)

#-----------------------------------------------------
#Add current cmake file to our source tree
macro(add_cmake_source)
	get_target(targetName)
	set(sourcesProperty "${targetName}Sources")
	get_property(sourcesPropertyValue GLOBAL PROPERTY ${sourcesProperty})
	set(sourcesPropertyValue ${sourcesPropertyValue} ${CMAKE_CURRENT_LIST_FILE})		
	set_property(GLOBAL PROPERTY ${sourcesProperty} ${sourcesPropertyValue})
	
	source_group(CMake FILES ${CMAKE_CURRENT_LIST_FILE})
endmacro(add_cmake_source)

#-----------------------------------------------------
#TODO: implement macro to generate _uid file based on a template
#Also add the generated CPP file to our ${source}
macro(uid)
	#set(source ${source} ${CMAKE_CURRENT_LIST_FILE})
	#source_group(CMake FILES ${CMAKE_CURRENT_LIST_FILE})
endmacro(uid)

#-----------------------------------------------------
#Add a pre-compiler define to the pre set target
macro(add_define define)
	get_target(targetName)
	get_target_property(value ${targetName} COMPILE_DEFINITIONS)	
	#message("Add define: ${define} to target: ${targetName}")
	set(value ${value} ${define})	
	set_target_properties(${targetName} PROPERTIES COMPILE_DEFINITIONS "${value}")
endmacro(add_define)


#-----------------------------------------------------
#Perform Symbian bld.inf export
macro(public_export source destination)
	install(FILES 
			${source}
			DESTINATION ${PROJECT_SOURCE_DIR}/epoc32/include${destination} )
endmacro(public_export)

#-----------------------------------------------------
#Perform Symbian bld.inf export
macro(platform_export source destination)
	install(FILES 
			${source}
			DESTINATION ${PROJECT_SOURCE_DIR}/epoc32/include/platform${destination} )
endmacro(platform_export)

#-----------------------------------------------------
#Add system include path to our current target
#Use those include path macro instead of cmake built-in include_directories as it allows us to set include directories per target instead of per directory.
macro(system_include path)
	get_target(targetName)
	get_target_property(value ${targetName} COMPILE_FLAGS)
	#MS CL compiler specific. I guess gcc should use -I instead of /I
	if (${value} STREQUAL "value-NOTFOUND")
		set(value "/I ${PROJECT_SOURCE_DIR}${path}")		
	else (${value} STREQUAL "value-NOTFOUND")
		set(value "${value} /I ${PROJECT_SOURCE_DIR}${path}")		
	endif (${value} STREQUAL "value-NOTFOUND")
	#message("Add system include: ${value}")	
	set_target_properties(${targetName} PROPERTIES COMPILE_FLAGS "${value}")	
endmacro(system_include)

#-----------------------------------------------------
#Add user include path to our current target
#Use those include path macro instead of cmake built-in include_directories as it allows us to set include directories per target instead of per directory.
#TODO: Is this working with releative path?
macro(user_include path)
	get_target(targetName)
	get_target_property(value ${targetName} COMPILE_FLAGS)
	#MS CL compiler specific. I guess gcc should use -I instead of /I
	if (${value} STREQUAL "value-NOTFOUND")
		set(value "/I ${CMAKE_CURRENT_LIST_DIR}/${path}")	
	else (${value} STREQUAL "value-NOTFOUND")
		set(value "${value} /I ${CMAKE_CURRENT_LIST_DIR}/${path}")	
	endif (${value} STREQUAL "value-NOTFOUND")	
	set_target_properties(${targetName} PROPERTIES COMPILE_FLAGS "${value}")
endmacro(user_include)




#Generate our configuration file
#configure_file( ../../GameEngine/inc/GikConfig.h.cmake ../../GameEngine/inc/GikConfig.h )
#Must make sure we include the generate config from the bynary tree
#include_directories("${CMAKE_BINARY_DIR}/../../GameEngine/inc")


