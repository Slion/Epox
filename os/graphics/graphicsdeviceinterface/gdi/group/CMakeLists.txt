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
#include (gdi.cmake)

#------------------------------------

#------------------------------------
#Exports

public_export(../inc/GDI.H /)
public_export(../inc/GDI.INL /)
public_export(../inc/BIDI.H /)
public_export(../inc/BidiText.h /)
public_export(../inc/LineBreak.h /)
public_export(../inc/BidiVisual.h /)
public_export(../inc/ShapeInfo.h /)
platform_export(../inc/ShapeImpl.h /graphics/)
platform_export(../lookuptable/lookuptable.h /graphics/)
platform_export(../inc/blendingalgorithms.h /graphics/)
platform_export(../inc/blendingalgorithms.inl /graphics/)
public_export(../inc/displaymode.h  /)

platform_export(../inc/glyphsample.h  /graphics/gdi/)
platform_export(../inc/gdistructs.h /graphics/gdi/)
platform_export(../inc/gdiconsts.h /graphics/gdi/)
platform_export(../inc/gdiinline.inl /graphics/gdi/)
platform_export(../inc/gdiplatapi.h /graphics/gdi/)

platform_export(../inc/hextree.h /)
platform_export(../inc/hextree.inl /)