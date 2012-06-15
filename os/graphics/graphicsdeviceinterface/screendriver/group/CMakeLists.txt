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
#include (screendriver.cmake)

#------------------------------------

#------------------------------------
#Exports

platform_export(../inc/BITDRAW.H /)
platform_export(../inc/BitDrawScaling.h /)
platform_export(../inc/BitDrawOrigin.h /)
platform_export(../inc/BmAlphaBlend.h /)
platform_export(../inc/BitDrawInterfaceId.h /)
public_export(../inc/GraphicsAccelerator.h /)
#ifdef SYMBIAN_BUILD_GCE
platform_export(../inc/bitdrawsurface.h /)
#endif // SYMBIAN_BUILD_GCE
platform_export(../inc/cdsb.h /)
