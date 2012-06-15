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

#Avoid warnings
cmake_minimum_required(VERSION 2.8)

include (utils.cmake)
include (symbian.cmake)

#
# Organize projects into folders
#
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set_property(TARGET btracec PROPERTY FOLDER "Symbian/Drivers")
set_property(TARGET btracex	PROPERTY FOLDER "Symbian/Drivers")
set_property(TARGET elocd		PROPERTY FOLDER "Symbian/Drivers")
set_property(TARGET domaincli	PROPERTY FOLDER "Symbian/DomainManager")
set_property(TARGET domainpolicy	PROPERTY FOLDER "Symbian/DomainManager")
set_property(TARGET domainsrv	PROPERTY FOLDER "Symbian/DomainManager")
set_property(TARGET ecomp	PROPERTY FOLDER "Symbian/FileServer")
set_property(TARGET edll	PROPERTY FOLDER "Symbian/EntryPoints")
set_property(TARGET eexe	PROPERTY FOLDER "Symbian/EntryPoints")
set_property(TARGET ekll		PROPERTY FOLDER "Symbian/EntryPoints")
set_property(TARGET estub	PROPERTY FOLDER "Symbian/EntryPoints")
set_property(TARGET efile	PROPERTY FOLDER "Symbian/FileServer")
set_property(TARGET efsrv	PROPERTY FOLDER "Symbian/FileServer")
set_property(TARGET elocal		PROPERTY FOLDER "Symbian/FileServer")
set_property(TARGET erofs	PROPERTY FOLDER "Symbian/FileServer")
set_property(TARGET emulator	PROPERTY FOLDER "Symbian/Emulator")
set_property(TARGET ecust	PROPERTY FOLDER "Symbian/Emulator")
set_property(TARGET exstart	PROPERTY FOLDER "Symbian/Boot")
set_property(TARGET e32strt	PROPERTY FOLDER "Symbian/Boot")
set_property(TARGET hal	PROPERTY FOLDER "Symbian/BasePort")
set_property(TARGET tsymc	PROPERTY FOLDER "Symbian/Tests")
set_property(TARGET ekern		PROPERTY FOLDER "Symbian/Kernel")
set_property(TARGET scppnwdl_kern	PROPERTY FOLDER "Symbian/Kernel")
set_property(TARGET euser	PROPERTY FOLDER "Symbian/User")
set_property(TARGET scppnwdl	PROPERTY FOLDER "Symbian/User")
set_property(TARGET createEmuDataFolder	PROPERTY FOLDER "Symbian")
set_property(TARGET eka		PROPERTY FOLDER "Symbian")
set_property(TARGET epoc	PROPERTY FOLDER "Symbian")
set_property(TARGET genexec	PROPERTY FOLDER "Symbian")
set_property(TARGET symbian	PROPERTY FOLDER "Symbian")

set_property(TARGET estor	PROPERTY FOLDER "Symbian/PersistentData")
set_property(TARGET centralrepository PROPERTY FOLDER "Symbian/PersistentData")
set_property(TARGET bafl PROPERTY FOLDER "Symbian/Utils")



