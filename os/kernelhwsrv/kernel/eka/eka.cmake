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

#Add current file to our source tree
add_cmake_source()
#Make sure all the output from all projects will go in one place
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_definitions(-D__SYMC__)


#Generate the exec enum using CMAKE custom command
#perl genexec.pl -i execs.txt -e ../include/exec_enum.h -u ../include/exec_user.h -k ../include/exec_kernel.h
add_custom_command (
	OUTPUT ./include/exec_enum.h ./include/exec_user.h ./include/kernel/exec_kernel.h	
   	COMMAND perl ./kernel/genexec.pl -i ./kernel/execs.txt -e ./include/exec_enum.h -u ./include/exec_user.h -k ./include/kernel/exec_kernel.h
	WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}	
   	COMMENT "Generating exec headers..."
   	DEPENDS ./kernel/genexec.pl ./kernel/execs.txt
   	VERBATIM
 	)
	
#Tell cmake those file won't be available until build time
set_source_files_properties(./include/exec_enum.h PROPERTIES GENERATED 1)
set_source_files_properties(./include/exec_user.h PROPERTIES GENERATED 1)
set_source_files_properties(./include/kernel/exec_kernel.h PROPERTIES GENERATED 1)

#----------------------------------------------------
# EKA: Custom target that will generate our exec headers 
#----------------------------------------------------
get_source(source)
add_custom_target	(	eka 
						DEPENDS ./include/exec_enum.h ./include/exec_user.h ./include/kernel/exec_kernel.h
						SOURCES ${source}
					)
#TODO: shall we get ride of that genexec?
add_custom_target(genexec)

add_subdirectory(./euser)
add_subdirectory(./kernel)
#----------------------------------------------------



#--------------------------------------------------
# Symbian exports
#--------------------------------------------------

public_export(include/gcc.h /)

## Kernel interface
platform_export(include/collate.h /)
platform_export(include/k32keys.h /)
platform_export(include/k32keys.inl /)
platform_export(include/u32std.h /)
platform_export(include/u32hal.h /)
platform_export(include/u32property.h /)
platform_export(include/u32exec.h /)
platform_export(include/cpudefs.h /)
platform_export(include/e32cia.h /)
platform_export(include/arm_vfp.h /)
platform_export(include/kernperflogger.h /)



platform_export(include/nkern/dfcs.h /nkern)
platform_export(include/nkern/nkern.h /nkern)
platform_export(include/nkern/nklib.h /nkern)
platform_export(include/nkern/nk_priv.h /nkern)
platform_export(include/nkern/nk_trace.h /nkern)
platform_export(include/nkern/nk_cpu.h /nkern)

platform_export(include/nkern/arm/nk_plat.h /nkern/arm)
platform_export(include/nkern/win32/nk_plat.h /nkern/win32)
platform_export(include/nkern/x86/nk_plat.h /nkern/x86)

public_export(nkern/nkern_ext.mmh /nkern)


platform_export(include/nkernsmp/dfcs.h /nkernsmp)
platform_export(include/nkernsmp/nkern.h /nkernsmp)
platform_export(include/nkern/nklib.h /nkernsmp)
platform_export(include/nkernsmp/nk_event.h /nkernsmp)
platform_export(include/nkernsmp/nk_priv.h /nkernsmp)
platform_export(include/nkern/nk_trace.h /nkernsmp)
platform_export(include/nkern/nk_cpu.h /nkernsmp)
platform_export(include/nkernsmp/nk_irq.h /nkernsmp)

platform_export(include/nkernsmp/arm/ncern.h /nkernsmp/arm)
platform_export(include/nkernsmp/arm/nk_plat.h /nkernsmp/arm)
platform_export(include/nkernsmp/arm/arm_gic.h /nkernsmp/arm)
platform_export(include/nkernsmp/arm/arm_scu.h /nkernsmp/arm)
platform_export(include/nkernsmp/arm/arm_tmr.h /nkernsmp/arm)
platform_export(include/nkernsmp/x86/ncern.h /nkernsmp/x86)
platform_export(include/nkernsmp/x86/nk_plat.h /nkernsmp/x86)
platform_export(include/nkernsmp/x86/apic.h /nkernsmp/x86)

public_export(nkernsmp/nkern_ext.mmh /nkernsmp)


platform_export(include/kernel/debug.h /kernel)
platform_export(include/kernel/klib.h /kernel)
platform_export(include/kernel/kdebug.h /kernel)
platform_export(include/kernel/kernel.h /kernel)
platform_export(include/kernel/kern_priv.h /kernel)
platform_export(include/kernel/localise.h /kernel)
platform_export(include/kernel/kpower.h /kernel)
platform_export(include/kernel/sproperty.h /kernel)
platform_export(include/kernel/kernboot.h /kernel)
platform_export(include/kernel/kbma.h /kernel)

platform_export(include/kernel/ls_std.h /kernel)

platform_export(include/kernel/monitor.h /kernel)
platform_export(include/kernel/cache.h /kernel)
platform_export(include/kernel/emi.h /kernel)
platform_export(include/nkern/emievents.h /nkern)


platform_export(include/nkernsmp/emievents.h /nkernsmp)

platform_export(include/memmodel/epoc/platform.h /memmodel/epoc)
platform_export(include/memmodel/epoc/plat_priv.h /memmodel/epoc)
platform_export(include/memmodel/epoc/mmubase/mmubase.h /memmodel/epoc/mmubase)
platform_export(include/memmodel/epoc/mmubase/kblockmap.h /memmodel/epoc/mmubase)
platform_export(include/memmodel/epoc/multiple/memmodel.h /memmodel/epoc/multiple)
platform_export(include/memmodel/epoc/moving/memmodel.h /memmodel/epoc/moving)
platform_export(include/memmodel/epoc/direct/memmodel.h /memmodel/epoc/direct)

platform_export(include/memmodel/emul/platform.h /memmodel/emul)
platform_export(include/memmodel/emul/plat_priv.h /memmodel/emul)
platform_export(include/memmodel/emul/win32/memmodel.h /memmodel/emul/win32)

platform_export(include/kernel/arm/arm.h /kernel/arm)
platform_export(include/kernel/arm/arm_types.h /kernel/arm)
platform_export(include/kernel/arm/assp.h /kernel/arm)
platform_export(include/kernel/arm/bootcpu.inc /kernel/arm)
platform_export(include/kernel/arm/bootdefs.h /kernel/arm)
platform_export(include/kernel/arm/bootmacro.inc /kernel/arm)
platform_export(include/kernel/arm/bootstrap.lnk /kernel/arm)
platform_export(include/kernel/arm/vfpsupport.h /kernel/arm)

platform_export(include/kernel/x86/x86.h /kernel/x86)
platform_export(include/kernel/x86/x86boot.h /kernel/x86)
platform_export(include/kernel/x86/assp.h /kernel/x86)
platform_export(include/kernel/x86/bootdefs.h /kernel/x86)
platform_export(include/kernel/x86/bootcpu.inc /kernel/x86)
platform_export(include/kernel/x86/bootmacr.inc /kernel/x86)

platform_export(include/kernel/win32/win32.h /kernel/win32)
platform_export(include/kernel/win32/assp.h /kernel/win32)
platform_export(include/kernel/win32/property.h /kernel/win32)

platform_export(include/kernel/sshbuf.h /kernel)
platform_export(include/kernel/smap.h /kernel)

platform_export(include/memmodel/epoc/moving/arm/arm_mem.h /memmodel/epoc/moving/arm)
platform_export(include/memmodel/epoc/moving/arm/mmboot.h /memmodel/epoc/moving/arm)
platform_export(include/memmodel/epoc/direct/arm/arm_mem.h /memmodel/epoc/direct/arm)
platform_export(include/memmodel/epoc/multiple/arm/arm_mem.h /memmodel/epoc/multiple/arm)
platform_export(include/memmodel/epoc/multiple/arm/mmboot.h /memmodel/epoc/multiple/arm)
platform_export(include/memmodel/epoc/direct/x86/x86_mem.h /memmodel/epoc/direct/x86)
platform_export(include/memmodel/epoc/multiple/x86/x86_mem.h /memmodel/epoc/multiple/x86)
platform_export(include/memmodel/epoc/multiple/x86/mmboot.h /memmodel/epoc/multiple/x86)
platform_export(include/memmodel/epoc/flexible/arm/mmboot.h /memmodel/epoc/flexible/arm)
platform_export(include/memmodel/epoc/flexible/x86/mmboot.h /memmodel/epoc/flexible/x86)


public_export(kernel/kern_ext.mmh /kernel)

platform_export(include/partitions.h /)


platform_export(include/drivers/comm.h /drivers)
platform_export(include/drivers/comm.inl /drivers)
platform_export(include/drivers/locmedia.h /drivers)
platform_export(include/drivers/flash_media.h /drivers)
platform_export(include/drivers/nanddevice.h /drivers)
platform_export(include/drivers/pbus.h /drivers)
platform_export(include/drivers/pbus.inl /drivers)
platform_export(include/drivers/pbusmedia.h /drivers)
platform_export(include/drivers/pccard.h /drivers)
platform_export(include/drivers/pccard.inl /drivers)
platform_export(include/drivers/soundsc.h /drivers)
platform_export(include/drivers/soundsc.inl /drivers)
platform_export(include/drivers/xyin.h /drivers)
platform_export(include/drivers/binpower.h /drivers)
platform_export(include/drivers/pccd_chunk.h /drivers)
platform_export(include/drivers/pccd_ifc.h /drivers)
platform_export(include/drivers/pccd_socket.h /drivers)
platform_export(include/drivers/pccd_medchg.h /drivers)
platform_export(include/drivers/pccd_vcc.h /drivers)
platform_export(include/drivers/ethernet.h /drivers)
platform_export(include/drivers/ethernet.inl /drivers)
platform_export(include/drivers/dma.h /drivers)
platform_export(include/drivers/dma_v1.h /drivers)
platform_export(include/drivers/dma_v1.inl /drivers)
platform_export(include/drivers/dma_v2.h /drivers)
platform_export(include/drivers/dma_v2.inl /drivers)
platform_export(include/drivers/dmadefs.h /drivers)
platform_export(include/drivers/dma_compat.inl /drivers)
platform_export(include/drivers/dma_hai.h /drivers)
platform_export(include/drivers/dma_hai.inl /drivers)
platform_export(include/drivers/iic.h /drivers)
platform_export(include/drivers/iic.inl /drivers)
platform_export(include/drivers/iic_channel.h /drivers)
platform_export(include/drivers/iic_channel.inl /drivers)
platform_export(include/drivers/iic_trace.h /drivers)




platform_export(include/drivers/gpio.h /drivers)
platform_export(include/drivers/mmccd_ifc.h /drivers)
platform_export(include/drivers/mmc.h /drivers)
platform_export(include/drivers/mmc.inl /drivers)
platform_export(include/drivers/sdcard.h /drivers)
platform_export(include/drivers/sdcard.inl /drivers)
platform_export(include/drivers/emmcptn.h /drivers)

platform_export(include/drivers/crashflash.h /drivers)
platform_export(include/drivers/crashflashnor.h /drivers)
platform_export(include/drivers/crashflashnand.h /drivers)

platform_export(include/rm_debug_api.h /)
platform_export(include/sm_debug_api.h /)

platform_export(include/drivers/resourceman.h /drivers)
platform_export(include/drivers/resourceman.inl /drivers)
platform_export(include/drivers/resourcecontrol.h /drivers)
platform_export(include/drivers/resourcecontrol.inl /drivers)
platform_export(include/drivers/resource.h /drivers)
platform_export(include/drivers/resourcecontrol_trace.h /drivers)
platform_export(include/drivers/resource_category.h /drivers)
platform_export(include/drivers/resource_extend.h /drivers)

## Power Resource Manager User-Side API
platform_export(include/drivers/resmanus_trace.h /drivers)

platform_export(include/drivers/i2s.h /drivers)

platform_export(include/drivers/display.h /drivers)

## Example RTOS personality layer
platform_export(personality/example/personality.h /personality/example)
platform_export(personality/example/personality_int.h /personality/example)


## User interface
public_export(include/e32atomics.h /)
public_export(include/e32base.h /)
platform_export(include/e32base_private.h /)
public_export(include/e32base.inl /)
public_export(include/e32capability.h /)
public_export(include/e32cons.h /)
public_export(include/e32const.h /)
platform_export(include/e32const_private.h /)
public_export(include/e32cmn.h /)
platform_export(include/e32cmn_private.h /)
public_export(include/e32cmn.inl /)
public_export(include/e32def.h /)
platform_export(include/e32def_private.h /)
public_export(include/e32des16.h /)
platform_export(include/e32des16_private.h /)
public_export(include/e32des8.h /)
platform_export(include/e32des8_private.h /)
public_export(include/e32err.h /)
platform_export(include/e32hal.h /)
public_export(include/e32hashtab.h /)
public_export(include/e32keys.h /)
public_export(include/e32lang.h /)
public_export(include/e32reg.h /)
public_export(include/e32math.h /)
public_export(include/e32math.inl /)
public_export(include/e32panic.h /)
platform_export(include/e32power.h /)
public_export(include/e32property.h /)
public_export(include/e32kpan.h /)
platform_export(include/e32rom.h /)
public_export(include/e32std.h /)
platform_export(include/e32std_private.h /)
public_export(include/e32std.inl /)
platform_export(include/e32svr.h /)
platform_export(include/e32notif.h /)
platform_export(include/e32ktran.h /)
public_export(include/e32debug.h /)
platform_export(include/e32lmsg.h /)
public_export(include/e32event.h /)
platform_export(include/e32event_private.h /)
public_export(include/e32ldr.h /)
platform_export(include/e32ldr_private.h /)
public_export(include/e32test.h /)
platform_export(include/e32uid.h /)
platform_export(include/e32ver.h /)
platform_export(include/e32wins.h /)

platform_export(include/winsdef.h /)
platform_export(include/emulator.h /)
platform_export(include/d32comm.h /)
platform_export(include/d32comm.inl /)
platform_export(include/d32fir.h /)
platform_export(include/d32locd.h /)
platform_export(include/d32locd.inl /)
platform_export(include/unicode.h /)
platform_export(include/e32twin.h /)
platform_export(include/twintnotifier.h /)
platform_export(include/videodriver.h /)
platform_export(include/d32ethernet.h /)
platform_export(include/d32ethernet.inl /)
platform_export(include/d32soundsc.h /)
platform_export(include/d32soundsc.inl /)
public_export(include/e32msgqueue.h /)
public_export(include/e32msgqueue.inl /)
public_export(include/d32public.h /)
platform_export(include/d32usbcshared.h /)
platform_export(include/d32usbcshared.inl /)
platform_export(include/d32usbcsc.h /)
platform_export(include/d32usbcsc.inl /)
platform_export(include/d32usbc.h /)
platform_export(include/d32usbc.inl /)
platform_export(include/usb.h /)
platform_export(include/d32usbdi.h /)
platform_export(include/d32usbdi.inl /)
platform_export(include/d32usbdi_errors.h /)
platform_export(include/d32usbdi_hubdriver.h /)
platform_export(include/d32usbdi_hubdriver.inl /)
platform_export(include/d32usbtransfers.h /)
platform_export(include/d32usbdescriptors.h /)
platform_export(include/d32camerasc.h /)
platform_export(include/d32camerasc.inl /)
platform_export(include/d32otgdi.h /)
platform_export(include/d32otgdi.inl /)
platform_export(include/d32otgdi_errors.h /)
platform_export(include/d32btrace.h /)
platform_export(include/e32btrace.h /)
platform_export(include/rpipe.h /)
public_export(include/e32modes.h /)
platform_export(include/pixelformats.h /)
platform_export(include/dispchannel.h /)
platform_export(include/dispchannel.inl /)

platform_export(include/d32resmanus.h /)
platform_export(include/d32resmanus.inl /)

## Unified Trace Framework API
platform_export(include/e32utf.h /)
platform_export(include/e32utrace.inl /)
platform_export(include/e32utrace_basic_types.h /)
public_export(include/e32utrace.mmh /)


## Byte pair compressor
platform_export(include/byte_pair_compress.h /)

## shareable data buffers
platform_export(include/e32shbuf.h /)
platform_export(include/e32shbufcmn.h /)
platform_export(include/e32shbuf_priv.h /)


public_export(include/drivers/iic_transaction.h /drivers)						
public_export(include/drivers/iic_transaction.inl /drivers)



public_export(include/drivers/usbcshared.h /drivers)						
public_export(include/drivers/usbcshared.inl /drivers)						

public_export(include/drivers/usbc.h /drivers)						
public_export(include/drivers/usbc.inl /drivers)						

public_export(include/drivers/usbcsc.h /drivers)						
public_export(include/drivers/usbcsc.inl /drivers)						


public_export(include/drivers/usbcque.h /drivers)						
public_export(include/drivers/usbcdesc.h /drivers)						

public_export(include/drivers/usbdi_hub.h /drivers)						
public_export(include/drivers/usbdi_channel.h /drivers)						

public_export(include/drivers/camerasc.h /drivers)						
public_export(include/drivers/camerasc.inl /drivers)						

public_export(include/drivers/otgdi.h /drivers)


## Open System Trace API
platform_export(include/opensystemtrace.h /)
platform_export(include/opensystemtrace.inl /)
platform_export(include/opensystemtrace_types.h /)
platform_export(include/opensystemtrace.mmh /)


#SL:
platform_export(include/exec_enum.h /)
platform_export(include/exec_user.h /)
platform_export(include/win32crt.h /)
platform_export(include/x86hlp.inl /)



#--------------------------------------------------

