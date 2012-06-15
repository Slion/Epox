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

#TODO
#include (ecom.cmake)

#------------------------------------

#------------------------------------
#Exports

public_export(../Framework/inc/ECom.h						 /ecom/)
public_export(../Framework/inc/EComErrorCodes.h			 /ecom/)
platform_export(../Framework/inc/ecompanics.h				    /ecom/)
public_export(../Framework/inc/EComResolverParams.h		 /ecom/)
public_export(../Framework/inc/EComResolverParams.inl		 /ecom/)
public_export(../Framework/inc/ImplementationInformation.h /ecom/)
public_export(../Framework/inc/ImplementationInformation.inl /ecom/)
public_export(../Framework/inc/ImplementationProxy.h		 /ecom/)
public_export(../Framework/inc/PublicRegistry.h			 /ecom/)
public_export(../Framework/inc/Resolver.h					 /ecom/)
public_export(../Framework/inc/registryinfoconst.hrh					 /ecom/)
public_export(../Framework/inc/registryinfo.rh			 /ecom/)
public_export(../Framework/inc/registryinfov2.rh					 /ecom/)
public_export(../Framework/inc/registryinfov3.rh					 /ecom/)
platform_export(../Framework/inc/EComExtendedInterfaceErrorCodes.h /ecom/)
platform_export(../Framework/inc/ExtendedInterfaceImplementationProxy.h	 /ecom/)
