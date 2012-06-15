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
#include (windowserver.cmake)

#------------------------------------
#Exports

public_export(../inc/W32STD.H /)
public_export(../inc/sizemode.h /)
public_export(../inc/pointerevent.h /)
public_export(../inc/advancedpointerevent.h /)
public_export(../inc/advancedpointerevent.inl /)
public_export(../inc/textcursor.h /)
public_export(../inc/wspublishandsubscribedata.h  /)
public_export(../inc/W32ADLL.H /)
public_export(../inc/W32CLICK.H /)
public_export(../inc/W32STDGRAPHIC.H /)
public_export(../inc/graphics/windowserverconstants.h /graphics/)

platform_export(../inc/graphics/pointereventdata.h /graphics/)
platform_export(../inc/openwfc/W32Debug.H /)

#Graphic Drawer exports for partners
platform_export(../inc/graphics/WSGRAPHICDRAWER.H /graphics/)
platform_export(../inc/graphics/openwfc/WSGRAPHICDRAWERINTERFACE.H /graphics/)
platform_export(../inc/graphics/WSGRAPHICMSGBUF.H /graphics/)
platform_export(../inc/graphics/W32STDGRAPHICTEST.H /graphics/)

#Plugin exports for partners
platform_export(../inc/graphics/WSPLUGIN.H /graphics/)
platform_export(../inc/graphics/WsRenderStage.h /graphics/)
platform_export(../inc/graphics/WsRenderStageFactory.h /graphics/)
platform_export(../inc/graphics/wscursor.h /graphics/)
platform_export(../inc/graphics/wsgraphicscontext.h /graphics/)
platform_export(../inc/graphics/wsscreendevice.h /graphics/)
platform_export(../inc/graphics/wsuibuffer.h /graphics/)
platform_export(../inc/graphics/wsdrawablesourceprovider.h /graphics/)
platform_export(../inc/graphics/wsdisplaycontrol.h /graphics/)
platform_export(../inc/graphics/openwfc/wsdisplaymapping.h /graphics/)
platform_export(../inc/graphics/wsdisplaypolicy.h /graphics/)
platform_export(../inc/graphics/openwfc/wselement.h /graphics/)
platform_export(../inc/graphics/openwfc/wsscene.h /graphics/)
platform_export(../inc/graphics/wskeyrouter.h /graphics/)
platform_export(../inc/graphics/wscontentreadyforcomposition.h /graphics/)

#RemoteGc exports for partners
platform_export(../inc/RemoteGc.h	 /graphics/)
platform_export(../inc/CommandBuffer.h /graphics/)
platform_export(../inc/RemoteGcUtils.h /graphics/)

#Test Screen Capture
platform_export(../inc/graphics/testscreencapture.h /graphics/)
platform_export(../inc/graphics/wsscreendeviceutils.h /graphics/)
platform_export(../inc/graphics/wstestscreencapture.h /graphics/)

platform_export(../inc/graphics/wsdrawresource.h /graphics/)

# Workaround for Bug 3283
platform_export(../inc/Graphics/wstfxconst.h /graphics/)


#Surface support
platform_export(../inc/graphics/surfaceconfiguration.h /graphics/)
platform_export(../inc/graphics/openwfc/surfaceconfiguration.inl /graphics/)
#Display Control
platform_export(../inc/graphics/displaycontrol.h /graphics/)

#Display Mapping
platform_export(../inc/graphics/displaymapping.h /graphics/)
