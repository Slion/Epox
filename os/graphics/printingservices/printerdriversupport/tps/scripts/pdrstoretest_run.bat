rem Copyright (c) 2005-2009 Nokia Corporation and/or its subsidiary(-ies).
rem All rights reserved.
rem This component and the accompanying materials are made available
rem under the terms of "Eclipse Public License v1.0"
rem which accompanies this distribution, and is available
rem at the URL "http://www.eclipse.org/legal/epl-v10.html".
rem
rem Initial Contributors:
rem Nokia Corporation - initial contribution.
rem
rem Contributors:
rem
rem Description:
rem PDRSTORETEST_RUN.BAT
rem 
rem


@echo on
testexecute.exe  z:\pdrstoretest\pdrstoretest_t_pdr.script
testexecute.exe  z:\pdrstoretest\pdrstoretest_t_pdrlst.script
testexecute.exe  z:\pdrstoretest\pdrstoretest_t_pdrmem.script

@echo off
md e:\logs
md e:\logs\testexecute

copy c:\logs\testexecute\pdrstoretest_t_*.htm  e:\logs\testexecute\


