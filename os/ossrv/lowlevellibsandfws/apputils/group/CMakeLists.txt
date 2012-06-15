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


include (bafl.cmake)

#----------------------------------------------
#--- Export



platform_export(../inc/patchdata.h /bafl/) 
public_export(../inc/babackup.h  /) 
platform_export(../inc/backup_std.h /bafl/) 
public_export(../inc/BACELL.H  /) 
public_export(../inc/BACLINE.H  /) 
public_export(../inc/BACLIPB.H  /) 
public_export(../inc/BACNTF.H  /) 
public_export(../inc/BADEF.RH  /) 
public_export(../inc/BADESCA.H  /) 
public_export(../inc/BAERRHAN.H  /) 
platform_export(../inc/BAERROR.H  /) 
public_export(../inc/BAFINDF.H  /) 
platform_export(../inc/BAFLVER.H  /) 
public_export(../inc/BALIBA.H  /) 
public_export(../inc/BAMATCH.H  /) 
public_export(../inc/BAMDESCA.H  /) 
public_export(../inc/BARSC.H 	 /) 
public_export(../inc/BARSREAD.H  /) 
public_export(../inc/BASCHED.H  /) 
public_export(../inc/BASSND.H  /) 
public_export(../inc/BAUTILS.H  /) 
public_export(../inc/BANAMEDPLUGINS.H /) 
public_export(../inc/stringpool.h  /) 
public_export(../inc/stringpool.inl  /) 
platform_export(../inc/StringTableSupport.h /) 
platform_export(../inc/Baksrv.h  /) 
public_export(../inc/Babitflags.h  /) 
public_export(../inc/BaRsc2.h  /) 
public_export(../inc/BaRsRead2.H  /) 
public_export(../inc/BAERRRSVR.RH  /) 
public_export(../inc/BaSPI.h 	 /) 
platform_export(../inc/BAFLPAN.H  /) 
platform_export(../inc/BASSNDUID.h  /) 
public_export(../inc/uikon.rh /) 
