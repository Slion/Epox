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

push_target(ekern)

#----------------------------------------------------------------
#Add current file to our source tree
add_cmake_source()
#Make sure all the output from all projects will go in one place
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
#Visual Studio IDE folders
source_group(Sources REGULAR_EXPRESSION ".+\\.cpp$")
source_group(Includes REGULAR_EXPRESSION ".+\\.h$") 	
source_group(Def REGULAR_EXPRESSION ".+\\.def$") 	
#----------------------------------------------------------------


include_directories (../euser ../euser/unicode ../include 
../include/nkern ../include/nkern/win32 ../include/kernel ../include/kernel/win32 ../include/memmodel/emul/win32 ../include/memmodel/emul)

#Define source for ekern
set (sourcepath ../nkern/)
add_source(
nkern.cpp nkerns.cpp sched.cpp dfcs.cpp nk_timer.cpp
nk_old.cpp
)

set (sourcepath ../common/win32/)
add_source(
atomics.cpp
)

set (sourcepath ../nkern/win32/)
add_source(
vectors.cpp ncsched.cpp nctimer.cpp ncutilf.cpp ncutils.cpp
ncthrd.cpp ncglob.cpp
)

set (sourcepath ../common/win32/)
add_source(
seh.cpp
)

#UID data seg. TODO: generate that?
set (sourcepath ./win32/)
add_source(
ekern_uid.cpp
)

set (sourcepath ../kernel/win32/)
add_source(
cboot.cpp
cglobals.cpp cexec.cpp cinit.cpp
ckernel.cpp csched.cpp
cutils.cpp cache.cpp
k_entry.cpp #originally from kc_exe.mmp
#TODO: add our own entry point?
)


set (sourcepath ../kernel/)
add_source(
sglobals.cpp device.cpp eventq.cpp
sexec.cpp sinit.cpp skernel.cpp
scodeseg.cpp sipc.cpp notifier.cpp 
object.cpp power.cpp server.cpp
stimer.cpp sutils.cpp random.cpp
sprocess.cpp sthread.cpp 
smqueue.cpp
sproperty.cpp
ssharedio.cpp
ssecure.cpp
emi.cpp
objectix.cpp
sbtrace.cpp
stest.cpp
sshbuf.cpp smap.cpp
)
				
set (sourcepath ../klib/)
add_source(
dbase.cpp kdes8.cpp
kheap.cpp bma.cpp
)

#
set (sourcepath ../common/)
add_source(
alloc.cpp array.cpp des8.cpp heap.cpp mem.cpp
secure.cpp
)

set (sourcepath ../memmodel/emul/)
add_source(
pglobals.cpp
nvram.cpp
)

#
set (sourcepath ../memmodel/emul/win32/)
add_source(
zrom.cpp
mchunk.cpp mglobals.cpp minit.cpp mkernel.cpp
mcodeseg.cpp mprocess.cpp mutils.cpp mshbuf.cpp
)

#def export
set (sourcepath ../blike/)
add_source(
ekern.def
)

#------------------------------------------------------------------------------------
# Define target
#------------------------------------------------------------------------------------
#It looks like exes on wins are still DLL but with a .exe extension 
get_source(source) 
add_library(ekern SHARED ${source})
set_target_properties(ekern PROPERTIES SUFFIX ".exe")
#add_executable(ekern ${source})
target_link_libraries(ekern kernel32 emulator scppnwdl_kern)
#------------------------------------------------------------------------------------

#add_dependencies(ekern genexec)
#set_target_properties(ekern PROPERTIES LINK_FLAGS "/ENTRY:KernelMain /NODEFAULTLIB")
#target_link_libraries(ekern kernel32 msvcrt)
#set_target_properties(ekern PROPERTIES COMPILE_FLAGS "/Zl")
#install(TARGETS ekern DESTINATION ./symbian/bin)

add_define(__EXPORT_MEMCPY__)				
add_define(__IN_KERNEL__)
add_define(__KERNEL_MODE__)
add_define(__MEMMODEL_EMUL_SINGLE_HOST_PROCESS__)
#__CPU_X86 is declared by __WINS__ 
#add_define(__CPU_X86)
#add_define(__DLL__)


##
##Copy ekern.dll to be next to epoc.exe
##

#get_target_property(LIB_NAME ekern LOCATION)
#get_target_property(DEST_NAME epoc LOCATION)
#set(target_suffix .dll)
#Extract path from full filename
#string(REGEX REPLACE "(^.+)/epoc.exe" "\\1" DEST_NAME ${DEST_NAME})
#set(DEST_NAME ${DEST_NAME}/ekern${target_suffix})

#message(${DEST_NAME})

#add_custom_command (
# 	TARGET ekern
#   	POST_BUILD
#   	COMMAND ${CMAKE_COMMAND} -E copy ${LIB_NAME} ${DEST_NAME}
# 	)
	
######################


pop_target(ekern)