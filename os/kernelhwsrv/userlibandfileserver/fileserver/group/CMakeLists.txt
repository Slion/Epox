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

cmake_minimum_required(VERSION 2.8)
#
include(efile.cmake)
include(efsrv.cmake)
#
include(../sfat/elocal.cmake)
include(../srofs/erofs.cmake)
include(../scomp/ecomp.cmake)

#--------------------------------------------------
# Symbian exports
#--------------------------------------------------

public_export(../inc/f32file.h /)
platform_export(../inc/f32file_private.h /)
public_export(../inc/f32file.inl /)
#ifdef SYMBIAN_ENABLE_64_BIT_FILE_SERVER_API
public_export(../inc/f32file64.h /)
public_export(../inc/f32file64.inl /)
#endif
#ifdef SYMBIAN_F32_ENHANCED_CHANGE_NOTIFICATION
public_export(../inc/f32notification.h /)
#endif
platform_export(../inc/f32fsys.h /)
platform_export(../inc/f32fsys.inl /)
platform_export(../inc/f32ver.h /)
platform_export(../inc/f32dbg.h /)
platform_export(../inc/f32image.h /)
platform_export(../inc/rofs.h /)
platform_export(../inc/ftlcontrolio.h /)
platform_export(../inc/f32pluginUtils.h /)
platform_export(../inc/F32plugin.h /)
platform_export(../inc/f32plugin.inl /)

platform_export(../inc/f32tracedef.h /)
platform_export(../inc/utraceefsrv.h /)
platform_export(../inc/utraceefile.h /)

#--------------------------------------------------