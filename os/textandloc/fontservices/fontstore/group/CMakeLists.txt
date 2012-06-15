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
#include (fntstore.cmake)

#------------------------------------

#------------------------------------
#Exports

public_export(../inc/FNTSTORE.H /)
public_export(../inc/OPENFONT.H /)
public_export(../inc/openfontlinkedtypefaceelementspec.h /)
public_export(../inc/linkedfonts.h /)

platform_export(../inc/openfontconstants.h /graphics/)
platform_export(../inc/openfontrasterizer.h /graphics/)

platform_export(../inc/openfontlinkedtypefacespecification.h /graphics/)
platform_export(../inc/openfontlinkedtypefaceextension.h /graphics/)
platform_export(../inc/shaperparams.h /graphics/)

