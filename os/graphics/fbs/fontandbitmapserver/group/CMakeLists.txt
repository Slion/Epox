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
#include (fbs.cmake)

#------------------------------------

#------------------------------------
#Exports

public_export(../inc/BITMAP.H /)
platform_export(../inc/bitmap.inl /graphics/)
public_export(../inc/FBS.H /)
platform_export(../inc/fbsrasterizer.h /graphics/)
platform_export(../inc/fbsrasterizerclearcache.h /graphics/)
platform_export(../inc/bitmapuid.h /graphics/)
public_export(../inc/fbsdefs.h    /graphics/)
platform_export(../sfbs/fbsglyphdataiterator.h /graphics/)
platform_export(../sfbs/fbsglyphmetricsarray.h /graphics/)
platform_export(../sfbs/fbsoogmmessage.h /graphics/)
platform_export(../sfbs/fbsoogmmessage.inl /graphics/)
 
