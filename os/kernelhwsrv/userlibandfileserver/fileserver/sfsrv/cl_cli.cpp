// Copyright (c) 1995-2009 Nokia Corporation and/or its subsidiary(-ies).
// All rights reserved.
// This component and the accompanying materials are made available
// under the terms of the License "Eclipse Public License v1.0"
// which accompanies this distribution, and is available
// at the URL "http://www.eclipse.org/legal/epl-v10.html".
//
// Initial Contributors:
// Nokia Corporation - initial contribution.
//
// Contributors:
//
// Description:
// f32\sfsrv\cl_cli.cpp
// 
//

#include "cl_std.h"
#include <f32fsys.h>







EFSRV_EXPORT_C TBool RFs::IsValidDrive(TInt aDrive)
/**
Tests whether the specified drive number is valid.

A valid drive number is any number between 0 and (KMaxDrives-1) inclusive,
or the specific value KDefaultDrive (implying the session default drive).

@param aDrive The drive number.
			
@return True if the drive is valid; false if not.				

@see TDriveNumber 				
*/
	{

	return((aDrive>=0 && aDrive<KMaxDrives) || aDrive==KDefaultDrive);
	}




EFSRV_EXPORT_C TInt RFs::CharToDrive(TChar aChar,TInt& aDrive)
/**
Maps a drive character to a drive number.

The drive character must be in the range A to Z or a to z. For example, drive A (or a)
corresponds to zero, drive B (or b) corresponds to 1 etc. For the drive number
enumeration, see TDriveNumber.

@param aChar  The drive character.
@param aDrive On return, contains the drive number.

@return KErrNone, if successful;
        KErrArgument, if the drive character is not in the range A to Z or a to z.
        
@see TDriveNumber        
*/
	{

	aChar.UpperCase();
	if (aChar>='A' && aChar<='Z')
		{
		aDrive=(TInt)aChar-'A';
		return(KErrNone);
		}
	return(KErrArgument);
	}




EFSRV_EXPORT_C TInt RFs::DriveToChar(TInt aDrive,TChar& aChar)
/**
Maps a drive number to the corresponding character.

The drive number must be in the range 0 to (KMaxDrives-1). For example, drive
number zero (EDriveA) corresponds to drive A, one (EDriveB)
corresponds to drive B. For the drive number enumeration, see TDriveNumber.

The drive number can also be KDefaultDrive, implying the default drive. In this
case the current drive is taken and converted.

@param aDrive The drive number.
@param aChar  On return, contains the drive character.

@return KErrNone, if successful;
        KErrArgument, if the drive number is invalid;
        otherwise one of the other system-wide error codes.
*/
	{

	if (aDrive==KDefaultDrive)
		{
		TRACE1(UTF::EBorder, UTraceModuleEfsrv::EFsDriveToChar, MODULEUID, aDrive);
		RFs fs;
		TFileName path;
		TInt r=fs.Connect();
		if (r!=KErrNone)
			return(r);
		r=fs.SessionPath(path);
		fs.Close();
		if (r!=KErrNone)
			return(r);
		aChar=path[0];
		TRACE2(UTF::EBorder, UTraceModuleEfsrv::EFsDriveToCharReturn, MODULEUID, KErrNone, aChar);
		return(KErrNone);
		}
	if (!IsValidDrive(aDrive))
		return(KErrArgument);
	aChar=aDrive+'A';
	return(KErrNone);
	}




EFSRV_EXPORT_C TBool RFs::IsRomAddress(TAny *aPtr)
/**
Tests whether the specified address is in ROM.

@param aPtr The address.

@return True, if the address is in ROM; false, if not.
*/
	{
	TRACE1(UTF::EBorder, UTraceModuleEfsrv::EFsIsRomAddress, MODULEUID, aPtr);

	TBool res;
	TInt r=User::IsRomAddress(res,aPtr); // Only returns error on WINS
	if (r!=KErrNone)
		res=EFalse;

	TRACERET1(UTF::EBorder, UTraceModuleEfsrv::EFsIsRomAddressReturn, MODULEUID, res);
	return(res);
	}



/** 
Obtain the system drive number.
 
The System Drive is a defined drive on the device which is:
 - Read/Writeable
 - Internal: Always available and not removable from the device
 - Non-Volatile (e.g. Flash memory, battery-backed RAM)
 - Only Accessible via Rfs (e.g. not available via USB mass storage)
     
The System drive is utilised as:
 - Storage for Persistent settings from system and application software
 - Storage for Localisation resources
 - A Default Drive for user data
 - A Target Drive for Software installations

It the system drive is not set previously (see RFs::SetSystemDrive) EDriveC is returned by default.
 
@see RFs::GetSystemDriveChar
@see RFs::SetSystemDrive   
@see TDriveNumber
@return TDriveNumber contains the drive number of the system drive.
 */
EFSRV_EXPORT_C TDriveNumber RFs::GetSystemDrive()
    {
	TRACE0(UTF::EBorder, UTraceModuleEfsrv::EFsGetSystemDrive, MODULEUID);
    TInt drive;
	TInt err = RProperty::Get(TSecureId(KFileServerUidValue), KSystemDriveKey, drive);
    if(err==KErrNone)
        {
        if((drive>=EDriveA) && (drive<=EDriveZ))
            {
			TRACE1(UTF::EBorder, UTraceModuleEfsrv::EFsGetSystemDriveReturn, MODULEUID, drive);
            return static_cast<TDriveNumber>(drive);
            }
        }

	TRACE1(UTF::EBorder, UTraceModuleEfsrv::EFsGetSystemDriveReturn, MODULEUID, EDriveC);
    return EDriveC;
	}
    


/**
This is a wrapper around GetSystemDrive() function. It returns the character corresponding to the system drive.

@parameter aDriveChar On return, contains the system drive character
@return KErrNone if successful, otherwise one of the other system-wide error codes
@see RFs::GetSystemDrive
*/
EFSRV_EXPORT_C TChar RFs::GetSystemDriveChar()
	{
	TRACE0(UTF::EBorder, UTraceModuleEfsrv::EFsGetSystemDriveChar, MODULEUID);

	TInt r = 'A' + GetSystemDrive();

	TRACE1(UTF::EBorder, UTraceModuleEfsrv::EFsGetSystemDriveCharReturn, MODULEUID, r);
	return r;
	}



/**
Set a specified drive as a "System Drive", see RFs::GetSystemDrive().
The "System Drive" can be set only once, any subsequent calls will result in the error 'KErrAlreadyExists'.

The media type for the system drive shall be one of the: EMediaHardDisk, EMediaFlash, EMediaNANDFlash, EMediaRam
Required drive attributes: KDriveAttLocal, KDriveAttInternal
Prohibited drive attributes: KDriveAttRom,KDriveAttRedirected,KDriveAttSubsted,KDriveAttRemovable

@param  aSystemDrive specifies the drive number to be set as System Drive
@return KErrNone if successful, otherwise one of the other system-wide error codes
@capability TCB
*/
EFSRV_EXPORT_C TInt RFs::SetSystemDrive(TDriveNumber aSystemDrive)
	{
	TRACE2(UTF::EBorder, UTraceModuleEfsrv::EFsSetSystemDrive, MODULEUID, Handle(), aSystemDrive);
    TInt r = SendReceive(EFsSetSystemDrive, TIpcArgs(aSystemDrive));

	TRACERET1(UTF::EBorder, UTraceModuleEfsrv::EFsSetSystemDriveReturn, MODULEUID, r);
	return r;
	}

    

EFSRV_EXPORT_C TInt RFs::Connect(TInt aMessageSlots)
/**
Connects a client to the file server.

To end the file server session, use Close().

@param aMessageSlots The number of message slots required. The default value of
				     KFileServerDefaultMessageSlots indicates that message
				     slots will be acquired dynamically from the system
				     wide pool. Override this value with a fixed number, if
				     a fixed number of slots are to be allocated to the session.
				     If overriding, note that the number of message slots
				     represents the number of operations, such as reads
				     and writes, that can be outstanding at once;
				     always remember to provide a spare slot for
				     the cancel operation.

@return KErrNone, if successful, otherwise one of the other system-wide
        error codes.
*/
	{
	TRACE1(UTF::EBorder, UTraceModuleEfsrv::EFsConnect, MODULEUID, aMessageSlots);
	_LIT(KFileServerName,"!FileServer");
	TInt r = CreateSession(KFileServerName,Version(),aMessageSlots);

	TRACERET2(UTF::EBorder, UTraceModuleEfsrv::EFsConnectReturn, MODULEUID, r, Handle());
	return r;
	}




EFSRV_EXPORT_C TInt RFs::SetSessionToPrivate(TInt aDrive)
/**
Sets the session path to point to the private path on the specified drive.

The private directory does not need to exist at this point.

The private path for a process has the form: \\Private\\13579BDF\\
where 13579BDF is the identity of the process.

@param aDrive The drive for which information is requested.
              Specify a drive in the range EDriveA to EDriveZ for drives
			  A to Z respectively.

@return KErrNone, if successful, otherwise one of the other system-wide
        error codes.
*/
	{	
	TRACE2(UTF::EBorder, UTraceModuleEfsrv::EFsSetSessionToPrivate, MODULEUID, Handle(), aDrive);
	TInt r = SendReceive(EFsSessionToPrivate,TIpcArgs(aDrive));

	TRACERET1(UTF::EBorder, UTraceModuleEfsrv::EFsSetSessionToPrivateReturn, MODULEUID, r);
	return r;
	}



EFSRV_EXPORT_C TInt RFs::PrivatePath(TDes& aPath)
/**
Creates the text defining the private path for a process.

The private path for a process has the form: \\Private\\13579BDF\\
where 13579BDF is the identity of the process.

@param aPath On successful return, contains the private path for a process.
*/
	{
	TRACE1(UTF::EBorder, UTraceModuleEfsrv::EFsPrivatePath, MODULEUID, Handle());
	TInt r = SendReceive(EFsPrivatePath,TIpcArgs(&aPath));

	TRACERETMULT2(UTF::EBorder, UTraceModuleEfsrv::EFsPrivatePathReturn, MODULEUID, r, aPath);
	return r;
	}



EFSRV_EXPORT_C TInt RFs::CreatePrivatePath(TInt aDrive)
/**
Creates the private path for a process on the specified drive.

The private path for a process has the form: \\Private\\13579BDF\\
where 13579BDF is the identity of the process.

@param aDrive The drive for which the private path is to be created.
              Specify a drive in the range EDriveA to EDriveZ for drives
			  A to Z respectively.

@return KErrNone, if successful, otherwise one of the other system-wide
        error codes.
*/
	{
	TRACE2(UTF::EBorder, UTraceModuleEfsrv::EFsCreatePrivatePath, MODULEUID, Handle(), aDrive);
	TInt r = SendReceive(EFsCreatePrivatePath,TIpcArgs(aDrive));

	TRACERET1(UTF::EBorder, UTraceModuleEfsrv::EFsCreatePrivatePathReturn, MODULEUID, r);
	return r;
	}	




EFSRV_EXPORT_C TVersion RFs::Version() const
/**
Gets the client side version number.

@return The client side version number.
*/
	{
	TRACE1(UTF::EBorder, UTraceModuleEfsrv::EFsVersion, MODULEUID, Handle());

	TVersion r = TVersion(KF32MajorVersionNumber,KF32MinorVersionNumber,KF32BuildVersionNumber);

	TRACERET3(UTF::EBorder, UTraceModuleEfsrv::EFsVersionReturn, MODULEUID, r.iMajor, r.iMinor, r.iBuild);
	return r;
	}




EFSRV_EXPORT_C TInt RFs::AddFileSystem(const TDesC& aFileName) const
/**
Adds a file system to the file server.

After calling this function, use MountFileSystem() to mount the file system
on a drive.

@param aFileName The name of the file system .FSY to install. Its full path can
				 be specified.

@return KErrNone, if successful, otherwise one of the other system-wide
        error codes.

@capability DiskAdmin
        
@see RFs::MountFileSystem        
*/
	{
	TRACEMULT2(UTF::EBorder, UTraceModuleEfsrv::EFsAddFileSystem, MODULEUID, Handle(), aFileName);
	RLoader loader;
	TInt r = loader.Connect();
	if (r==KErrNone)
		{
		r = loader.SendReceive(ELoadFileSystem, TIpcArgs(0, &aFileName, 0));
		loader.Close();
		}

	TRACERET1(UTF::EBorder, UTraceModuleEfsrv::EFsAddFileSystemReturn, MODULEUID, r);
	return r;
	}




EFSRV_EXPORT_C TInt RFs::RemoveFileSystem(const TDesC& aFileSystemName) const
/**
Removes the specified file system.

@param aFileSystemName The fullname of the file system, as returned from
                       a call to FileSystemName(), to be removed.

@return KErrNone, if successful;
        KErrNotFound, if aFileSystemName is not found;
        otrherwise one of the other system-wide error codes.

@capability DiskAdmin

*/
	{
	TRACEMULT2(UTF::EBorder, UTraceModuleEfsrv::EFsRemoveFileSystem, MODULEUID, Handle(), aFileSystemName);
	TInt r = SendReceive(EFsRemoveFileSystem,TIpcArgs(&aFileSystemName));

	TRACERET1(UTF::EBorder, UTraceModuleEfsrv::EFsRemoveFileSystemReturn, MODULEUID, r);
	return r;
	}




EFSRV_EXPORT_C TInt RFs::MountFileSystem(const TDesC& aFileSystemName,TInt aDrive) const
/**
Mounts a file system on a drive.

The file system must first have been added to the file server using AddFileSystem().
The drive is mounted as asynchronous, i.e operations on it don't block the file server and other drives;

@param aFileSystemName The fullname of the file system, as returned from  a call to FileSystemName().
@param aDrive          The drive on which the file system is to be mounted; this can be one of the values defined by TDriveNumber.

@return KErrNone if successful, otherwise one of the other system-wide error codes.

@capability DiskAdmin

@see RFs::AddFileSystem
@see RFs::FileSystemName
*/
	{
	TRACEMULT3(UTF::EBorder, UTraceModuleEfsrv::EFsMountFileSystem1, MODULEUID, Handle(), aFileSystemName, aDrive);
	TInt r = SendReceive(EFsMountFileSystem,TIpcArgs(&aFileSystemName,aDrive,NULL,EFalse));

	TRACERET1(UTF::EBorder, UTraceModuleEfsrv::EFsMountFileSystem1Return, MODULEUID, r);
	return r;
	}





EFSRV_EXPORT_C TInt RFs::MountFileSystem(const TDesC& aFileSystemName,TInt aDrive, TBool aIsSync) const
/**
Mounts a file system on a specified drive.

The file system must first have been added to the file server using AddFileSystem().
Depending on the aIsSync parameter, the drive can be mounted as synchronous or asynchronous.

Asynchronous drive has its own processing thread, i.e operations on it don't block the file server and other drives;
Synchronous drives' requests are being processed by the main file server thread and there is a possibility to block it along with
all operations on other drives. Mounting a drive as synch. makes a sense if the operations on such drive are very fast e.g. this is an
internal RAM or ROFS drive.

@param aFileSystemName The fullname of the file system, as returned from a call to FileSystemName().
@param aDrive          The drive number on which the file system is to be mounted; this can be one of the values defined by TDriveNumber.

@param aIsSync         if ETrue  the drive will be mounted as synchronous one;
                       if EFalse the drive will be mounted as Asynchronous.

@return KErrNone if successful, otherwise one of the other system-wide error codes.
@capability DiskAdmin

@see RFs::AddFileSystem
@see RFs::FileSystemName
*/
	{
	TRACEMULT4(UTF::EBorder, UTraceModuleEfsrv::EFsMountFileSystem2, MODULEUID, Handle(), aFileSystemName, aDrive, aIsSync);

	TInt r = SendReceive(EFsMountFileSystem,TIpcArgs(&aFileSystemName,aDrive,NULL,aIsSync));

	TRACERET1(UTF::EBorder, UTraceModuleEfsrv::EFsMountFileSystem2Return, MODULEUID, r);
	return r;
	}




EFSRV_EXPORT_C TInt RFs::MountFileSystem(const TDesC& aFileSystemName,const TDesC& aExtensionName,TInt aDrive)
/**
Mounts a file system on a drive, and the specified extension.

The file system must first have been added to the file server using AddFileSystem().
The drive is mounted as asynchronous, i.e operations on it don't block the file server and other drives;

@param aFileSystemName The fullname of the file system, as returned from a call to FileSystemName().
@param aExtensionName  The filename of the extension.
@param aDrive          The drive on which the file system is to be mounted; this can be one of the values defined by TDriveNumber.

@return KErrNone if successful, otherwise one of the other system-wide error codes.

@capability DiskAdmin

@see RFs::AddFileSystem
@see RFs::FileSystemName
*/
	{
	TRACEMULT4(UTF::EBorder, UTraceModuleEfsrv::EFsMountFileSystem3, MODULEUID, Handle(), aFileSystemName, aExtensionName, aDrive);

	TInt r = SendReceive(EFsMountFileSystem,TIpcArgs(&aFileSystemName,aDrive,&aExtensionName,EFalse));

	TRACERET1(UTF::EBorder, UTraceModuleEfsrv::EFsMountFileSystem3Return, MODULEUID, r);
	return r;
	}




EFSRV_EXPORT_C TInt RFs::MountFileSystem(const TDesC& aFileSystemName,const TDesC& aExtensionName,TInt aDrive, TBool aIsSync)
/**
Mounts a file system on a drive, and the specified extension.

The file system must first have been added to the file server using AddFileSystem().

Depending on the aIsSync parameter, the drive can be mounted as synchronous or asynchronous.

Asynchronous drive has its own processing thread, i.e operations on it don't block the file server and other drives;
Synchronous drives' requests are being processed by the main file server thread and there is a possibility to block it along with
all operations on other drives. Mounting a drive as synch. makes  sense if the operations on such drive are very fast e.g. this is an
internal RAM or ROFS drive.

@param aFileSystemName The fullname of the file system, as returned from a call to FileSystemName().
@param aExtensionName  The filename of the extension.
@param aDrive          The drive on which the file system is to be mounted; this can be one of the values defined by TDriveNumber.

@param aIsSync         if ETrue  the drive will be mounted as synchronous one;
                       if EFalse the drive will be mounted as Asynchronous.

@return KErrNone if successful, otherwise one of the other system-wide error codes.

@capability DiskAdmin

@see RFs::AddFileSystem
@see RFs::FileSystemName
*/
	{
	TRACEMULT5(UTF::EBorder, UTraceModuleEfsrv::EFsMountFileSystem4, MODULEUID, Handle(), aFileSystemName, aExtensionName, aDrive, aIsSync);
	TInt r = SendReceive(EFsMountFileSystem,TIpcArgs(&aFileSystemName,aDrive,&aExtensionName,aIsSync));

	TRACERET1(UTF::EBorder, UTraceModuleEfsrv::EFsMountFileSystem4Return, MODULEUID, r);
	return r;
	}




EFSRV_EXPORT_C TInt RFs::MountFileSystemAndScan(const TDesC& aFileSystemName,TInt aDrive,TBool& aIsMountSuccess) const
/**
Mounts a file system on a drive, and performs a scan on that drive.
The file system must first have been added to the file server using AddFileSystem().

Note that the scan is done only if the mount is successful.

The drive is mounted as asynchronous, i.e operations on it don't block the file server and other drives;

@param aFileSystemName The fullname of the file system, as returned from a call to FileSystemName().
@param aDrive          The drive on which the file system is to be mounted; this can be one of the values defined by TDriveNumber.
@param aIsMountSuccess On return, set to: ETrue, if the  if the mount is successful, set to EFalse otherwise.

@return KErrNone if successful, otherwise one of the other system-wide error codes, reflecting the failure of the mount operation. 

@capability DiskAdmin

@see RFs::TDriveNumber
@see RFs::AddFileSystem
@see RFs::FileSystemName
*/
	{
	TRACEMULT3(UTF::EBorder, UTraceModuleEfsrv::EFsMountFileSystemAndScan1, MODULEUID, Handle(), aFileSystemName, aDrive);
	aIsMountSuccess=EFalse;
	TPckg<TInt> pckg(aIsMountSuccess);
	TInt r = SendReceive(EFsMountFileSystemScan,TIpcArgs(&aFileSystemName,aDrive,NULL,&pckg));

	TRACERET2(UTF::EBorder, UTraceModuleEfsrv::EFsMountFileSystemAndScan1Return, MODULEUID, r, aIsMountSuccess);
	return r;
	}

EFSRV_EXPORT_C TInt RFs::MountFileSystemAndScan(const TDesC& aFileSystemName,const TDesC& aExtensionName,TInt aDrive,TBool& aIsMountSuccess) const
/**
Mounts a file system on a drive, and the specified extension and performs a scan on that drive.

The file system must first have been added to the file server,
using AddFileSystem().

Note that the scan is done only if the mount is successful.

The operation is asynchronous, i.e other concurrent file server operations can continue.

@param aFileSystemName The fullname of the file system, as returned from
                       a call to FileSystemName().
@param aExtensionName  The filename of the extension.
@param aDrive          The drive on which the file system is to be mounted;
                       this can be one of the values defined by TDriveNumber.
@param aIsMountSuccess On return, set to: ETrue, if the  if the mount
                       is successful, set to EFalse otherwise.

@return KErrNone if successful, otherwise one of the other system-wide
        error codes, reflecting the failure of the mount operation. 

@capability DiskAdmin

@see RFs::TDriveNumber
@see RFs::AddFileSystem
@see RFs::FileSystemName
*/
	{
	TRACEMULT5(UTF::EBorder, UTraceModuleEfsrv::EFsMountFileSystemAndScan2, MODULEUID, 
		Handle(), aFileSystemName, aExtensionName, aDrive, aIsMountSuccess);

	aIsMountSuccess=EFalse;
	TPckg<TInt> pckg(aIsMountSuccess);
	TInt r = SendReceive(EFsMountFileSystemScan,TIpcArgs(&aFileSystemName,aDrive,&aExtensionName,&pckg));

	TRACERET2(UTF::EBorder, UTraceModuleEfsrv::EFsMountFileSystemAndScan2Return, MODULEUID, r, aIsMountSuccess);
	return r;
	}

EFSRV_EXPORT_C TInt RFs::DismountFileSystem(const TDesC& aFileSystemName,TInt aDrive) const
/**
Dismounts the file system from the specified drive.

@param aFileSystemName The fullname of the file system, as returned from
                       a call to FileSystemName().
@param aDrive          The drive from which the file system is to be dismounted.

@return KErrNone, if successful;
        KErrNotFound, if aFileSystemName is not found;
        KErrNotReady, if the drive does not have a file	system mounted on it;
        KErrInUse, if the drive has a resource open	on it;
        KErrAccessDenied, if there is an attempt to dismount a ROM file system,
        a substituted drive, or the drive which is the default drive;
 		KErrArgument, if the specified drive value is outsdide of the valid range.
 		KErrPermissionDenied, if the client does not have the necessary capabilities 
 		to dismount the file system. 		

@capability DiskAdmin
 		
@see RFs::FileSystemName 		
*/
	{
	TRACEMULT3(UTF::EBorder, UTraceModuleEfsrv::EFsDismountFileSystem, MODULEUID, Handle(), aFileSystemName, aDrive);
	TInt r = SendReceive(EFsDismountFileSystem,TIpcArgs(&aFileSystemName,aDrive));

	TRACERET1(UTF::EBorder, UTraceModuleEfsrv::EFsDismountFileSystemReturn, MODULEUID, r);
	return r;
	}




/**
    Gets the name of the file system mounted on the specified drive.
    The function can be called before calling DismountFileSystem().
			     
    @param aName  On successful return, contains the name of the file system.
    @param aDrive The drive for which the file system name is required.

    Note that the file system name, returned in the aName descriptor shall be threated as case-insensitive string. I.e. 
    "fileSystem" and "FILESYSTEM" mean absolutely the same. Therefore, case-insensitive string methods (like TDesC::FindF(), TDesC::CompareF())
    shall be used to deal with the names.

    @return KErrNone, if successful;
            KErrNotFound if aFileSystemName is not found, or the drive does not have a file	system mounted on it;
            KErrArgument, if the drive value is outside the valid range, i.e. zero to KMaxDrives-1 inclusive.

    @see RFs::DismountFileSystem				
*/
EFSRV_EXPORT_C TInt RFs::FileSystemName(TDes& aName,TInt aDrive) const
	{
	TRACE2(UTF::EBorder, UTraceModuleEfsrv::EFsFileSystemName, MODULEUID, Handle(), aDrive);

	//-- ipc argument "-1" here is to indicate legacy FileSystemName() API
    TInt r = SendReceive(EFsFileSystemName,TIpcArgs(&aName, aDrive, -1)); 

	TRACERETMULT2(UTF::EBorder, UTraceModuleEfsrv::EFsFileSystemNameReturn, MODULEUID, r, aName);
	return r;
	}


/**
    Get one of the supported file system names on a specified drive. This API can be used for enumerating 
    file systems that can be recognised and mounted automatically, without user's interaction. 
    If the automatic recognition and mountng some known file systems is supported on the specified drive, there 
    shall be at least 2 names in the list. For example "FAT" and "exFAT". 
    If "automatic file system recognising" feature is not supported, the list will consist of just one name, and 
    this will be the name returned by RFs::FileSystemName() API.

    Note that the file system name, returned in the aName descriptor shall be threated as case-insensitive string. I.e. 
    "fileSystem" and "FILESYSTEM" mean absolutely the same. Therefore, case-insensitive string methods (like TDesC::FindF(), TDesC::CompareF())
    shall be used to deal with the names.

    @param  aName           On successful return, contains the name of the file system that correspond to the aFsEnumerator value.
    m@param aDrive          The drive number 
    @param  aFsEnumerator   The supported file system enumerator. can be:
                            KRootFileSystem a special value; in this case the returned name will be the same as obtained by FileSystemName()
                            0,1,2... integer values specifying the sequential number of supported filesystem. See the return error code.
    
    @return KErrNone        success, aName contains a valid name for the supported file system number "aFsEnumerator" on this drive.
            KErrNotFound    the end of the supported file names list; "aFsEnumerator-1" was the last correct value
            KErrArgument    incorrect arguments
            
    
    @see FileSystemName()
    @see KRootFileSystem   
*/
EFSRV_EXPORT_C TInt RFs::SupportedFileSystemName(TDes& aName, TInt aDrive, TInt aFsEnumerator) const
    {
	if(aFsEnumerator < 0)
        return KErrArgument; //-- see RFs::FileSystemName(). "-1" is a reserved value

    TRACE2(UTF::EBorder, UTraceModuleEfsrv::EFsFileSystemName, MODULEUID, Handle(), aDrive);
    
    TInt r = SendReceive(EFsFileSystemName,TIpcArgs(&aName, aDrive, aFsEnumerator));

	TRACERETMULT2(UTF::EBorder, UTraceModuleEfsrv::EFsFileSystemNameReturn, MODULEUID, r, aName);
	return r;
    }





EFSRV_EXPORT_C TInt RFs::AddExtension(const TDesC& aFileName)
/**
Loads the specified extension.

@param aFileName The file name of the extension

@return KErrNone, if successful; otherwise one of the other system wide error codes.
*/
	{
	TRACEMULT2(UTF::EBorder, UTraceModuleEfsrv::EFsAddExtension, MODULEUID, Handle(), aFileName);
	RLoader loader;
	TInt r = loader.Connect();
	if (r==KErrNone)
		{
		r = loader.SendReceive(ELoadFSExtension, TIpcArgs(0, &aFileName, 0));
		loader.Close();
		}

	TRACERET1(UTF::EBorder, UTraceModuleEfsrv::EFsAddExtensionReturn, MODULEUID, r);
	return r;
	}




EFSRV_EXPORT_C TInt RFs::MountExtension(const TDesC& aExtensionName,TInt aDrive)
/**
Mounts the the specified extension.

The extension must first have been loaded using AddExtension().

@param aExtensionName  The fullname of the extension, as returned from
                       a call to ExtensionName().
@param aDrive          The drive on which the extension is to be mounted;

@return KErrNone if successful;
        KErrNotFound, if the extension cannot be found;
        otherwise one of the other system-wide error codes.

@see RFs::ExtensionName
*/
	{
	TRACEMULT3(UTF::EBorder, UTraceModuleEfsrv::EFsMountExtension, MODULEUID, Handle(), aExtensionName, aDrive);
	TInt r = SendReceive(EFsMountExtension,TIpcArgs(&aExtensionName,aDrive));

	TRACERET1(UTF::EBorder, UTraceModuleEfsrv::EFsMountExtensionReturn, MODULEUID, r);
	return r;
	}




/**
Dismounts the specified extension.

@param aExtensionName  The fullname of the extension, as returned from a call to ExtensionName().
@param aDrive          The drive this extension is to be dismounted from.

@return KErrNone if successful;
        KErrNotFound    if the extension cannot be found;
        otherwise one of the other system-wide error codes.
        
@see RFs::ExtensionName
*/
EFSRV_EXPORT_C TInt RFs::DismountExtension(const TDesC& aExtensionName,TInt aDrive)
	{
	TRACEMULT3(UTF::EBorder, UTraceModuleEfsrv::EFsDismountExtension, MODULEUID, Handle(), aExtensionName, aDrive);
	TInt r = SendReceive(EFsDismountExtension,TIpcArgs(&aExtensionName,aDrive));

	TRACERET1(UTF::EBorder, UTraceModuleEfsrv::EFsDismountExtensionReturn, MODULEUID, r);
	return r;
	}


EFSRV_EXPORT_C TInt RFs::RemoveExtension(const TDesC& aExtensionName)
/**
Removes the specified extension.

@param aExtensionName The fullname of the extension, as returned from
                      a call to ExtensionName().

@return KErrNone, if successful;
        KErrNotFound, if aExtensionName is not found;
        otrherwise one of the other system-wide error codes.
*/
	{
	TRACEMULT2(UTF::EBorder, UTraceModuleEfsrv::EFsRemoveExtension, MODULEUID, Handle(), aExtensionName);
	TInt r = SendReceive(EFsRemoveExtension,TIpcArgs(&aExtensionName));

	TRACERET1(UTF::EBorder, UTraceModuleEfsrv::EFsRemoveExtensionReturn, MODULEUID, r);
	return r;
	}




EFSRV_EXPORT_C TInt RFs::ExtensionName(TDes& aExtensionName,TInt aDrive,TInt aPos)
/**
Gets the name of the extension on the specified drive at the specified position
in the extension hierarchy.
			 
@param aExtensionName  On successful return, contains the name of the extension.
@param aDrive          The drive for which the extension name is required.
@param aPos            The position of the extension in the extension hierarchy.

@return KErrNone, if successful;
        KErrNotFound if the extension name is not found;
*/
	{
	TRACEMULT4(UTF::EBorder, UTraceModuleEfsrv::EFsExtensionName, MODULEUID, Handle(), aExtensionName, aDrive, aPos);
	TInt r = SendReceive(EFsExtensionName,TIpcArgs(&aExtensionName,aDrive,aPos));

	TRACERET1(UTF::EBorder, UTraceModuleEfsrv::EFsExtensionNameReturn, MODULEUID, r);
	return r;
	}




EFSRV_EXPORT_C TInt RFs::RemountDrive(TInt aDrive,const TDesC8* aMountInfo,TUint aFlags)
/**
Forces a remount of the specified drive.

@param aDrive     The drive for which a remount is to be forced.
@param aMountInfo Information passed down to the media driver. The meaning of
                  this information depends on the media driver, for example,
                  keys for secure areas.
@param aFlags     When the flag is set to
                  0x00000001 - Used to simulate ejecting and re-inserting the media.
                  0x80000000 - used to force the media driver for the specified logical
                               drive to be closed and reopened.

@return KErrNone if successful, otherwise one of
        the other system wide error codes.
*/
	{
	TRACE4(UTF::EBorder, UTraceModuleEfsrv::EFsRemountDrive, MODULEUID, Handle(), aDrive, aMountInfo, aFlags);
	TInt r = SendReceive(EFsRemountDrive,TIpcArgs(aDrive,aMountInfo,aFlags));

	TRACERET1(UTF::EBorder, UTraceModuleEfsrv::EFsRemountDriveReturn, MODULEUID, r);
	return r;
	}



EFSRV_EXPORT_C void RFs::NotifyChange(TNotifyType aType,TRequestStatus& aStat)
/**
Requests a notification of change to files or directories.

Changes are notified either:

1. following any change in the file system

or

2. only following the addition or deletion of a directory entry, or after
   a disk has been formatted or changed.
 
Such notification is useful for programs that maintain displays
of file lists which must be dynamically updated. The alternative is to do
no updating, or to perform periodic monitoring for change, which
is inefficient.

This is an asynchronous request and, as such, results in precisely one signal
to the request status passed as a parameter. To avoid missing any change, this
request should be issued before the first file list is constructed. When
the request completes, a new request should be issued before the next file
list is constructed. When the file server session is
closed, this request is implicitly cancelled.

Call NotifyChangeCancel() to explicitly cancel a notification request.

@param aType Indicates the kind of change that should result in notification.
			 For example:
			 ENotifyEntry causes notification only when an entry is added or
             deleted, or when a disk is formatted or changed;
             ENotifyAll causes notification following any type of change, such
             as when a file is written to, or when a file's attributes
             are changed.
@param aStat The request status.
             This is set to KErrNone on completion, otherwise one of the other
             system-wide error codes.

*/
	{
	TRACE3(UTF::EBorder, UTraceModuleEfsrv::EFsNotifyChange1, MODULEUID, Handle(), aType, &aStat);
	aStat=KRequestPending;
	// for backward compatibility
	TNotifyType type = (aType == 0 ? ENotifyEntry : aType);
	RSessionBase::SendReceive(EFsNotifyChange, TIpcArgs(type,&aStat) , aStat );
	//This call is to synchronise with the file server when this functions stack varibles can go out of scope
	SendReceive(EFsSynchroniseDriveThread, TIpcArgs(-1));		

	TRACE0(UTF::EBorder, UTraceModuleEfsrv::EFsNotifyChange1Return, MODULEUID);
	}




EFSRV_EXPORT_C void RFs::NotifyChange(TNotifyType aType,TRequestStatus& aStat,const TDesC& aPathName)
/**
Requests a notification of change to files or directories, allowing
a directory/file path to be specified.

Changes are notified either:

1. following any change in the file system

or

2. only following the addition or deletion of a directory entry, or after
   a disk has been formatted or changed.
 
Such notification is useful for programs that maintain displays
of file lists which must be dynamically updated. The alternative is to do
no updating, or to perform periodic monitoring for change, which
is inefficient.

This is an asynchronous request and, as such, results in precisely one signal
to the request status passed as a parameter. To avoid missing any change, this
request should be issued before the first file list is constructed. When
the request completes, a new request should be issued before the next file
list is constructed. When the file server session is
closed, this request is implicitly cancelled.

Call NotifyChangeCancel() to explicitly cancel a notification request.

@param aType     Indicates the kind of change that should result in
                 notification. For example:
			     ENotifyEntry causes notification only when an entry is added
			     or deleted, or when a disk is formatted or changed;
                 ENotifyAll causes notification following any type of change,
                 such as when a file is written to, or when a file's attributes
                 are changed.
@param aStat     The request status.
                 This is set to KErrNone on completion, otherwise one of
                 the other system-wide error codes.
@param aPathName The directory or file for which notification is required. By
                 specifying a drive as a wildcard, for example
                 "?:\\Resource\\apps\\", or
				 "*:\\Resource\\apps\\", a client can ask to be notified of changes
				 to a given directory on any drive.
				 As with all directory paths aPathName must be terminated with '\\',
				 Please refer to "Structure of paths and filenames" section in the
				 Symbian OS Library.

@capability Dependent If aName is /Sys then AllFiles capability is required.
@capability Dependent If aName begins with /Private and does not match this process' SID
					  then AllFiles capability is required.

*/
	{
	TRACEMULT4(UTF::EBorder, UTraceModuleEfsrv::EFsNotifyChange2, MODULEUID, Handle(), (TUint) aType, (TUint) &aStat, aPathName);
	aStat=KRequestPending;
	// for backward compatibility
	TNotifyType type = (aType == 0 ? ENotifyEntry : aType);
	RSessionBase::SendReceive(EFsNotifyChangeEx,TIpcArgs(type,&aPathName,&aStat),aStat);
	//This call is to synchronise with the file server when this functions stack varibles can go out of scope
	SendReceive(EFsSynchroniseDriveThread, TIpcArgs(-1));

	TRACE0(UTF::EBorder, UTraceModuleEfsrv::EFsNotifyChange2Return, MODULEUID);
	}




EFSRV_EXPORT_C void RFs::NotifyChangeCancel()
/**
Cancels all outstanding requests for notification of change
to files or directories.

All outstanding requests complete with KErrCancel.

Note that this is a synchronous function.

*/
	{
	TRACE1(UTF::EBorder, UTraceModuleEfsrv::EFsNotifyChangeCancel1, MODULEUID, Handle());
	RSessionBase::SendReceive(EFsNotifyChangeCancel);

	TRACE0(UTF::EBorder, UTraceModuleEfsrv::EFsNotifyChangeCancel1Return, MODULEUID);
	}




EFSRV_EXPORT_C void RFs::NotifyChangeCancel(TRequestStatus& aStat)
/**
Cancels the specific request for notification of change
to files or directories.

The outstanding request completes with KErrCancel.

Note that this is an asynchronous function.

@param aStat The request status object associated with the request
             to be cancelled. Note that the function does not change
             this parameter.

*/
	{
	TRACE2(UTF::EBorder, UTraceModuleEfsrv::EFsNotifyChangeCancel2, MODULEUID, Handle(), &aStat);
	if (aStat==KRequestPending)			//	May be better to ASSERT this?
		SendReceive(EFsNotifyChangeCancelEx,TIpcArgs(&aStat));

	TRACE0(UTF::EBorder, UTraceModuleEfsrv::EFsNotifyChangeCancel2Return, MODULEUID);
	}




EFSRV_EXPORT_C void RFs::NotifyDiskSpace(TInt64 aThreshold,TInt aDrive,TRequestStatus& aStat)
/**
Requests notification when the free disk space on the specified
drive crosses the specified threshold value.

The threshold is crossed if free disk space increases to a value above
the threshold value or decreases to a value below the threshold value.

This is an asynchronous request that completes if any of the
following events occur:

1. the threshold is crossed 

2. any drive is formatted 

3. there is a media change on any socket 

4. power up 

5. the scandrive utility is run on any drive

5. the specified threshold value is outside its limits 

7. the outstanding request is cancelled. 

Note that free disk space notification is not supported for
drives using remote file systems.

@param aThreshold The threshold value. This must be greater than zero and less
                  than the total size of the disk.
@param aDrive     The drive number. This is an explicit drive defined by one of
				  the TDriveNumber enum values or the value
				  KDefaultDrive. If KDefaultDrive is specified, then
                  the drive monitored is the session path drive.
@param aStat      The request status object. On request completion, contains:
				  KErrNone, if the threshold value is crossed, if any drive is
                  formatted, if there is a media change on any socket, if there is a power up or
				  if the scandrive utility is run on any drive;
				  KErrCancel, if the outstanding request is cancelled by a call to
                  NotifyDiskSpaceCancel();
                  KErrArgument, if the threshold value is outside its limits.

@see TDriveNumber
*/
	{
	TRACE5(UTF::EBorder, UTraceModuleEfsrv::EFsNotifyDiskSpace, MODULEUID, 
		Handle(), I64LOW(aThreshold),I64HIGH(aThreshold), aDrive,(TUint) &aStat);
	aStat=KRequestPending;
	TPtrC8 tBuf((TUint8*)&aThreshold,sizeof(TInt64));
	RSessionBase::SendReceive(EFsNotifyDiskSpace,TIpcArgs(&tBuf,aDrive,&aStat), aStat);
	//This call is to synchronise with the driver thread as corresponding cancel function (NotifyDiskSpaceCancel)
	//is synchronous, so it can complete before this notify request has even been added to TDiskSpaceQue.
	//This call guarantees that the notify request has been added to queue.
	SendReceive(EFsSynchroniseDriveThread, TIpcArgs(aDrive));


	TRACE0(UTF::EBorder, UTraceModuleEfsrv::EFsNotifyDiskSpaceReturn, MODULEUID);
	}




EFSRV_EXPORT_C void RFs::NotifyDiskSpaceCancel(TRequestStatus& aStat)
/**
Cancels a specific outstanding request for free disk space
notification.

The outstanding request completes with KErrCancel.

@param aStat The request status object identified with the original
			 notification request.
*/
	{
	TRACE2(UTF::EBorder, UTraceModuleEfsrv::EFsNotifyDiskSpaceCancel1, MODULEUID, Handle(), &aStat);
	
	if(aStat==KRequestPending)
		SendReceive(EFsNotifyDiskSpaceCancel,TIpcArgs(&aStat));

	TRACE0(UTF::EBorder, UTraceModuleEfsrv::EFsNotifyDiskSpaceCancel1Return, MODULEUID);
	}




EFSRV_EXPORT_C void RFs::NotifyDiskSpaceCancel()
/**
Cancels all outstanding requests for free disk space
notification.

Outstanding requests complete with KErrCancel.
*/
	{
	TRACE1(UTF::EBorder, UTraceModuleEfsrv::EFsNotifyDiskSpaceCancel2, MODULEUID, Handle());
	SendReceive(EFsNotifyDiskSpaceCancel,TIpcArgs(NULL));

	TRACE0(UTF::EBorder, UTraceModuleEfsrv::EFsNotifyDiskSpaceCancel2Return, MODULEUID);
	}




EFSRV_EXPORT_C TInt RFs::DriveList(TDriveList& aList) const
/**
Gets a list of the available (not remote and non hidden) drives.

The drive list consists of an array of 26 bytes. Array index zero corresponds
to drive A, index one equals B etc.

Each byte with a non zero value signifies that the corresponding drive is available
to the system. In the case of removable media, RFs::Drive should be used to determine
whether the media is inserted or not.

The local file system always reserves drive letters A through I.
Drive letter Z is always used for the ROM which means that letters J through Y
are available to be used by SetSubst() or for redirecting.				

@param aList On return, contains a list of drive attributes (only the first 8 bits) for the available non-remote and non-hidden drives.

@return KErrNone, successful, otherwise one of the other system-wide error codes.
*/
	{
	TRACE1(UTF::EBorder, UTraceModuleEfsrv::EFsDriveList1, MODULEUID, Handle());
	TInt r = SendReceive(EFsDriveList,TIpcArgs(&aList, KDriveAttExclude|KDriveAttRemote|KDriveAttHidden));

	TRACERET1(UTF::EBorder, UTraceModuleEfsrv::EFsDriveList1Return, MODULEUID, r);
	return r;
	}



EFSRV_EXPORT_C TInt RFs::DriveList(TDriveList& aList, TUint aFlags) const
/**
Gets a list of the available drives that match a combination of drive attributes,specified in aFlags.
This combination may include,exclude or exclusively specify the attributes that that drives to be returned 
should match.

The drive list consists of an array of 26 bytes. Array index zero corresponds
to drive A, index one equals B etc.

Each byte with a non zero value signifies that the corresponding drive is available
to the system. In the case of removable media, RFs::Drive should be used to determine
whether the media is inserted or not.

The local file system always reserves drive letters A through I.
Drive letter Z is always used for the ROM which means that letters J through Y
are available to be used by SetSubst() or for redirecting.				

@param aList On return, contains a list of available drives that qualify aFlags.

@param aFlags A combination of drive attributes that drives to be returned must qualify. 

@return KErrNone, successful, otherwise one of the other system-wide error codes;
		KErrArgument, If aFlags contains an invalid attribute combination.
*/
	{
	TRACE2(UTF::EBorder, UTraceModuleEfsrv::EFsDriveList2, MODULEUID, Handle(), aFlags);
	TInt r = SendReceive(EFsDriveList,TIpcArgs(&aList,aFlags));

	TRACERET1(UTF::EBorder, UTraceModuleEfsrv::EFsDriveList2Return, MODULEUID, r);
	return r;
	}




EFSRV_EXPORT_C TInt RFs::Drive(TDriveInfo& anInfo,TInt aDrive) const
/**
Gets information about a drive and the medium mounted on it.

Note that Volume() can also be used to give information about the drive and
the volume mounted on it. These two functions are separate because, while
the characteristics of a drive cannot change, those of a
volume can, by mounting different media, reformatting etc.
			 
@param anInfo On return, contains information describing the drive
			  and the medium mounted on it. The value of TDriveInfo::iType
			  shows whether the drive contains media.
@param aDrive The drive for which information is requested.
              Specify KDefaultDrive for the session default drive.
			  Specify a drive in the range EDriveA to EDriveZ for drives
			  A to Z respectively.

@return       KErrNone, if successful, otherwise one of the other
              system-wide error codes.
			 
@see RFs::Volume
*/
	{
	TRACE2(UTF::EBorder, UTraceModuleEfsrv::EFsDrive, MODULEUID, Handle(), aDrive);

	TPckg<TDriveInfo> m(anInfo);
	TInt r = SendReceive(EFsDrive,TIpcArgs(&m,aDrive));

	TRACERET4(UTF::EBorder, UTraceModuleEfsrv::EFsDriveReturn, MODULEUID, r, anInfo.iDriveAtt, anInfo.iMediaAtt, anInfo.iType);
	return r;
	}




EFSRV_EXPORT_C TInt RFs::Volume(TVolumeInfo& aVol,TInt aDrive) const
/**
Gets volume information for a formatted device.

This function provides additional information to that given by Drive(),
including the volume label, if set, and the amount of free space on the
disk.

Note, use Drive() to get information about the drive without reference to
a volume. These two functions are separate because, while the characteristics
of a drive cannot change, those of a volume can, by mounting different media,
reformatting etc. A volume may not even be present if the media is removable.

@param aVol   On return, contains the volume information.
@param aDrive The drive which contains the media for which volume information is to be displayed.
              Specify a drive in the range EDriveA to EDriveZ for drives A to Z respectively. 
              The default drive is the session default drive KDefaultDrive.

@return KErrNone, if successful;
        KErrNotReady, if the drive contains no media;
        otherwise one of the other system-wide error codes.
			
@see RFs::Drive
*/
	{
	TRACE2(UTF::EBorder, UTraceModuleEfsrv::EFsVolume1, MODULEUID, Handle(), aDrive);
	TPckg<TVolumeInfo> v(aVol);
    TInt r = SendReceive(EFsVolume,TIpcArgs(&v,aDrive,NULL));

	TRACE7(UTF::EBorder, UTraceModuleEfsrv::EFsVolume1Return, MODULEUID, 
		r, aVol.iUniqueID, I64LOW(aVol.iSize), I64HIGH(aVol.iSize),
		I64LOW(aVol.iFree), I64HIGH(aVol.iFree), aVol.iFileCacheFlags);
	return r;
	}

/**
Gets volume information for a formatted device asynchronously.
@see TInt RFs::Volume(TVolumeInfo& aVol,TInt aDrive) for the synchronous version.

"Asynchronously" corresponds to the amount of free space on the volume in TVolumeInfo::iFree.
I.e. this function returns the _current_ amount of free space on the volume, which can be changing due to some
filesystems' activities. For example, some filesystems can be performing free space calculations in the background. 
Comparing to the RFs::Volume(TVolumeInfo& aVol,TInt aDrive), this method doesn't block the client until background filesystem 
activity finishes, which can be useful in some situations. 

@param aVol   On return, contains the volume information with the _current_ value in the TVolumeInfo::iFree.
@param aDrive Drive number to query. Specify a drive in the range EDriveA to EDriveZ for drives A to Z respectively. 
@param aStat  request status. At present is used just for indication of the asynchronous version and gets immediately completed, so there is no reason to analyse its value.                                                                                                                      
			
@publishedPartner
@prototype
*/
EFSRV_EXPORT_C void RFs::Volume(TVolumeInfo& aVol,TInt aDrive, TRequestStatus& aStat) const
    {
	TRACE3(UTF::EBorder, UTraceModuleEfsrv::EFsVolume2, MODULEUID, Handle(), aDrive, &aStat);
	TPckg<TVolumeInfo> v(aVol);
    aStat=KRequestPending;
    RSessionBase::SendReceive(EFsVolume,TIpcArgs(&v,aDrive,&aStat), aStat);

	TRACE0(UTF::EBorder, UTraceModuleEfsrv::EFsVolume2Return, MODULEUID);
    }


EFSRV_EXPORT_C TInt RFs::SetVolumeLabel(const TDesC& aName,TInt aDrive)
/**
Sets the label for a volume.

Note that similar to file names, volume labels can be set with unicode characters.
However it may not be recognized properly if correct code page is not
loaded or it is mounted onto a system that does not support DBCS volume
labels

@param aName  The volume label.
@param aDrive The drive containing the media whose label is to be set.
	          Specify a drive in the range EDriveA to EDriveZ for
			  drives A to Z.
              The default drive is the session default drive KDefaultDrive.

@return KErrNone, if successful;
        KErrNotReady, if the drive contains no media;
        otherwise one of the other system-wide error codes.

@capability DiskAdmin

@see TDriveNumber
@see TVolumeInfo::iName
@see RFs::Volume
*/
	{
	TRACEMULT3(UTF::EBorder, UTraceModuleEfsrv::EFsSetVolumeLabel, MODULEUID, 
		Handle(), aName, aDrive);

	TInt r = SendReceive(EFsSetVolume,TIpcArgs(&aName,aDrive));

	TRACERET1(UTF::EBorder, UTraceModuleEfsrv::EFsSetVolumeLabelReturn, MODULEUID, r);
	return r;
	}




EFSRV_EXPORT_C TInt RFs::Subst(TDes& aPath,TInt aDrive) const
/**
Gets the path assigned to a drive letter by an earlier call to SetSubst().

To find out whether a drive letter has been substituted, first get the drive
information, using Drive(), and then test the value of the KDriveAttSubsted bit
provided by TDriveInfo::iDriveAtt.

@param aPath  On return, contains the path which has been assigned to the
              drive. If the drive letter has not been substituted, this argument
              returns an empty descriptor.
@param aDrive The drive which is the subject of the enquiry. Specify a number
			  in the range 0 (EDriveA) to 25 (>EDriveZ) for drives
			  A to Z. The default drive is the session default
              drive KDefaultDrive.
              
@return KErrNone, if successful, otherwise one of the other
        system-wide error codes.

@see RFs::SetSubst
@see TDriveInfo
@see RFs::Drive
*/
	{
	TRACEMULT3(UTF::EBorder, UTraceModuleEfsrv::EFsSubst, MODULEUID, Handle(), aPath, aDrive);
	TInt r = SendReceive(EFsSubst,TIpcArgs(&aPath,aDrive));

	TRACERET1(UTF::EBorder, UTraceModuleEfsrv::EFsSubstReturn, MODULEUID, r);
	return r;
	}




EFSRV_EXPORT_C TInt RFs::SetSubst(const TDesC& aPath,TInt aDrive)
/**
Assigns a path to a drive letter.

Whenever that drive letter is used, it will be translated into a reference
to the path specified here. To	clear a drive substitution, specify an empty
descriptor for aPath. 

Note that the substituted path is text-only. Its components need not
be syntactically correct, nor must they be valid at the time the substitution
is set. Any component may be deleted, removed or unmounted while the
substitution is set.

@param aPath  The path to be assigned to the drive letter. If a drive letter
			  is specified in the path, it must not itself be substituted or
			  redirected, or the function will return an error. If no drive is
			  specified, the drive contained in the default session path is
			  used, and if no path is specified, the default session path is
			  used. If a filename or extension is included in the  path,
			  the function will return an error. Therefore, the final component
			  in the path must have a trailing backslash to indicate that it is
			  a directory.
			 
@param aDrive The drive to which a path is to be assigned. Specify a number
			  in the range 0 (EDriveA) to 25 (EDriveZ) for drives
			  A to Z. Must not be local, ROM, or redirected, otherwise an
              error is returned. May be substituted, but only if the function
              is being used	to clear the substitution. If the same drive is
              specified in the path, the function will return an error.
              The default drive is the session default drive
			  KDefaultDrive.
			 
@return KErrNone, if successful; otherwise one of the other	system-wide
        error codes.

@capability DiskAdmin
@capability Dependent If aPath is /Sys then Tcb capability is required.
@capability Dependent If aPath begins with /Private and does not match this process' SID
					  then AllFiles capability is required.
@capability Dependent If aPath is /Resource then Tcb capability is required.
*/
	{
	TRACEMULT3(UTF::EBorder, UTraceModuleEfsrv::EFsSetSubst, MODULEUID, Handle(), aPath, aDrive);
	TInt r = SendReceive(EFsSetSubst,TIpcArgs(&aPath,aDrive));

	TRACERET1(UTF::EBorder, UTraceModuleEfsrv::EFsSetSubstReturn, MODULEUID, r);
	return r;
	}




EFSRV_EXPORT_C TInt RFs::RealName(const TDesC& aName,TDes& aResult) const
/**
Gets the real name of a file.

This is used in circumstances where a file system needs to
mangle Symbian OS natural names so that it can store them on that file
system.

@param aName   Contains the name by which the file is normally referred.
@param aResult On return, contains the real name of the file, comprising the
               full path, including the drive letter.

@return KErrNone if successful, otherwise one of the other
        system-wide error codes.

@capability Dependent If aName is /Sys then AllFiles capability is required.
@capability Dependent If aName begins with /Private and does not match this process' SID
					  then AllFiles capability is required.

*/
	{
	TRACEMULT2(UTF::EBorder, UTraceModuleEfsrv::EFsRealName, MODULEUID, Handle(), aName);
	TInt r = SendReceive(EFsRealName,TIpcArgs(&aName,&aResult));

	TRACERETMULT2(UTF::EBorder, UTraceModuleEfsrv::EFsRealNameReturn, MODULEUID, r, aResult);
	return r;
	}




/**
Gets the serial number of media.

Only local drive is allowed. Substed drive number will return KErrNotSupported.

@param aSerialNum Contains serial number on successful return.
@param aDrive     Drive number.

@return KErrNone            if successful;
        KErrNotSupported    if media doesn't support serial number (e.g. substed drives);
        KErrBadName         if drive number is invalid;
        otherwise one of system-wide error codes.

@see TMediaSerialNumber
*/
EFSRV_EXPORT_C TInt RFs::GetMediaSerialNumber(TMediaSerialNumber& aSerialNum, TInt aDrive)
    {
	TRACE2(UTF::EBorder, UTraceModuleEfsrv::EFsGetMediaSerialNumber, MODULEUID, Handle(), aDrive);
    TInt r = SendReceive(EFsGetMediaSerialNumber, TIpcArgs(&aSerialNum, aDrive));

	TRACERETMULT2(UTF::EBorder, UTraceModuleEfsrv::EFsGetMediaSerialNumberReturn, MODULEUID, r, aSerialNum);
	return r;
    }




EFSRV_EXPORT_C TInt RFs::SessionPath(TDes& aPath) const
/**
Gets the session path.

When a client connects to the file server, its session path is initialised to
the system default path. The session path of an existing client can only be
changed by this function.

@param aPath On return, contains the session path, including a trailing
             backslash.

@return KErrNone if successful, otherwise one of the other
        system-wide error codes.
*/
	{
	TRACE1(UTF::EBorder, UTraceModuleEfsrv::EFsSessionPath, MODULEUID, Handle());
	TInt r = SendReceive(EFsSessionPath,TIpcArgs(&aPath));

	TRACERETMULT2(UTF::EBorder, UTraceModuleEfsrv::EFsSessionPathReturn, MODULEUID, r, aPath);
	return r;
	}




EFSRV_EXPORT_C TInt RFs::SetSessionPath(const TDesC& aPath)
/**
Sets the session path for the current file server client.

When the client first connects to the file server, its session path
is initialised to the system default path.

Note that the session path is text-only. It does not cause any locking.
Thus, although the path must be syntactically correct, its components
do not need to be valid at the time the path is set, and any component may be
deleted, removed or unmounted while the path is set.

@param aPath The new session path. Consists of a drive and path. Normally, a
             drive should be specified, but if not, the drive specified in
             the existing session path is preserved. If a file is specified,
             then the function fails and returns an error code. Therefore,
             the final component in the path must have a trailing backslash
             to indicate that it is a directory. All components of the
             path must be syntactically correct, for example, wildcard
             characters and double backslashes are not allowed in any
             part of it.

@return KErrNone if successful, otherwise one of the other
        system-wide error codes.

@capability Dependent If aPath is /Sys then AllFiles capability is required.
@capability Dependent If aPath begins with /Private and does not match this process' SID
					  then AllFiles capability is required.

*/
	{
	TRACEMULT2(UTF::EBorder, UTraceModuleEfsrv::EFsSetSessionPath, MODULEUID, Handle(), aPath);
	TInt r = SendReceive(EFsSetSessionPath,TIpcArgs(&aPath));

	TRACERET1(UTF::EBorder, UTraceModuleEfsrv::EFsSetSessionPathReturn, MODULEUID, r);
	return r;
	}





/**
Makes a directory.

It should be a sub-directory of an existing	directory and its name should be
unique within its parent directory, otherwise the function returns error code KErrAlreadyExists.
				
Note that if a filename is specified in the argument, it is	ignored.
Therefore, there should be a trailing backslash after the final
directory name in the argument to indicate that it is a directory, not a filename. 

For example, following code will create directory "C:\\DIR1\\"
   
@code
fs.MkDir(_L("C:\\DIR1\\"));
@endcode

The last line in the following example will result in KErrAlreadyExists because "DIR2" doesn't have a trailing backslash, 
therefore is considered as a file name and discarded. Directory "C:\\DIR1\\" has already been created.

@code
fs.MkDir(_L("C:\\DIR1\\"));     // shall create DIR1 in the root directory
fs.MkDir(_L("C:\\DIR1\\DIR2")); // No trailing backslash, fails with KErrAlreadyExists
@endcode

This example will always fail because "DIR1" doesn't have a trailing backslash and discarded while the root
directory always exists. 

@code
fs.MkDir(_L("C:\\DIR1"));  // No trailing backslash, will always fail with KErrAlreadyExists
@endcode

Note, the following case

@code
fs.MkDir(_L("C:\\example.txt\\"));	// would normally create a directory "c:\\example.txt\\" with KErrNone
@endcode
 
But if there is a file named "example.txt", which exists at the same location, KErrAccessDenied is returned.    

Note also that because this method can return an error code (eg. because
the disk is full) before checking whether the path already exists, it
is not appropriate to use it just to work out whether a path exists or not.

See MkDirAll(), which may also create intermediate directories.

@param aPath The name of the new directory. Any path components which are
             not specified here will be taken from the session path.
			 The directory name shall not contain wild cards ('?' or '*' characters) 
			 and illegal characters like '<', '>', ':', '"', '/', '|' and '\000'.
			 The directory name containing only whilte space characters 
			 (See TChar::IsSpace()) is also illegal. 

@return KErrNone if successful, otherwise one of the other
        system-wide error codes. Even if another error code is returned,
		(for example, if the disk is full) it is still possible that the 
		path may already exist.

@capability Dependent If aPath is /Sys then Tcb capability is required.
@capability Dependent If aPath begins with /Private and does not match this process' SID
					  then AllFiles capability is required.
@capability Dependent If aPath is /Resource then Tcb capability is required.
        
@see RFs::MkDirAll       
*/
EFSRV_EXPORT_C TInt RFs::MkDir(const TDesC& aPath)
	{
	TRACEMULT2(UTF::EBorder, UTraceModuleEfsrv::EFsMkDir, MODULEUID, Handle(), aPath);
	TInt r = SendReceive(EFsMkDir,TIpcArgs(&aPath,NULL));

	TRACERET1(UTF::EBorder, UTraceModuleEfsrv::EFsMkDirReturn, MODULEUID, r);
	return r;
	}





/**
Makes one or more directories.

Any valid path component specified in aPath which does not already exist is
created as a directory.
 
Note that if a filename is specified in the argument, it is	ignored.
Therefore, there should be a trailing backslash after the final
directory name in the argument to indicate that it is a directory, not a
filename.

See also notes on RFs::MkDir() about trailing backslashes in directory names.

Note also that because this method can return an error code (eg. because
the disk is full) before checking whether the path already exists, it
is not appropriate to use it just to work out whether a path exists or not.
		
See MkDir(), which creates only a single new directory.

@param aPath The path name specifiying the directory or directories to
             create. If the function completes successfully, this path
             identifies a valid	directory. Any path components which are not
             specified here are taken from the session path.

@return KErrNone if successful, otherwise one of the other
        system-wide error codes. Even if another error code is returned,
		(for example, if the disk is full) it is still possible that the 
		path may already exist. 


@capability Dependent If aPath is /Sys then Tcb capability is required.
@capability Dependent If aPath begins with /Private and does not match this process' SID
					  then AllFiles capability is required.
@capability Dependent If aPath is /Resource then Tcb capability is required.

@see RFs::MkDir
*/
EFSRV_EXPORT_C TInt RFs::MkDirAll(const TDesC& aPath)
	{
	TRACEMULT2(UTF::EBorder, UTraceModuleEfsrv::EFsMkDirAll, MODULEUID, Handle(), aPath);
	TInt r = SendReceive(EFsMkDir,TIpcArgs(&aPath,TRUE));

	TRACERET1(UTF::EBorder, UTraceModuleEfsrv::EFsMkDirAllReturn, MODULEUID, r);
	return r;
	}




EFSRV_EXPORT_C TInt RFs::RmDir(const TDesC& aPath)
/**
Removes a directory.

The directory must be empty and cannot be the root directory. 

Note that if a filename is specified in the argument, it is
ignored. 

For example, following code will result in directory "C:\\SRC\\" being removed as long as 
it is empty, the existance of "ENTRY" will not be checked:

@code
fs.RmDir(_L("C:\\SRC\\ENTRY"));
@endcode

Similarly, following code will try to remove "C:\\SRC\\" instead of "C:\\SRC\DIR\\":
@code
fs.RmDir(_L("C:\\SRC\\DIR"));
@endcode

Therefore, there should be a trailing backslash after the final
directory name in the argument to indicate that it is a directory, not a
filename.

See class CFileMan for information on deleting a
non-empty directory and all of its contents.
				
@param aPath The path name of the directory to be removed. Any path	components
             which are not specified here are taken from the session path. Only
             the lowest-level directory identified is removed.

@return KErrNone, if successful;
        KErrInUse, if trying to remove a non-empty directory or root directory;
        otherwise, one of the other system-wide error codes.
              
@capability Dependent If aPath is /Sys then Tcb capability is required.
@capability Dependent If aPath begins with /Private and does not match this process' SID
					  then AllFiles capability is required.
@capability Dependent If aPath is /Resource then Tcb capability is required.

@see CFileMan
*/
	{
	TRACEMULT2(UTF::EBorder, UTraceModuleEfsrv::EFsRmDir, MODULEUID, Handle(), aPath);
	TInt r = SendReceive(EFsRmDir,TIpcArgs(&aPath));

	TRACERET1(UTF::EBorder, UTraceModuleEfsrv::EFsRmDirReturn, MODULEUID, r);
	return r;
	}




void RFs::GetDirL(const TDesC& aName,const TUidType& aUidType,TUint aKey,CDir*& aFileList,RDir& aDir) const
//
// Create a dir array. Leave on any error.
//
	{
	aFileList=NULL;
	User::LeaveIfError(aDir.Open((RFs& )*this,aName,aUidType));
	DoGetDirL(aKey,aFileList,aDir);
	}

void RFs::GetDirL(const TDesC& aName,TUint anAttMask,TUint aKey,CDir*& aFileList,RDir& aDir) const
//
// Create a dir array. Leave on any error.
//
	{

	aFileList=NULL;
	User::LeaveIfError(aDir.Open((RFs& )*this,aName,anAttMask));
	DoGetDirL(aKey,aFileList,aDir);
	}

void RFs::GetDirL(const TDesC& aName,TUint anAttMask,TUint aKey,CDir*& aFileList,CDir*& aDirList,RDir& aDir) const
//
// Create a dir array. Leave on any error.
//
	{
	
	aDirList=NULL;
	GetDirL(aName,anAttMask|KEntryAttDir,aKey,aFileList,aDir);
	aFileList->ExtractL(!(anAttMask&KEntryAttDir),aDirList);
	}

void RFs::DoGetDirL(TUint aKey,CDir*& aFileList,RDir& aDir) const
//
// Create a dir array. Leave on any error.
//
	{

	aFileList=CDir::NewL();
	TInt r;
	TEntryArray* pArray=new(ELeave) TEntryArray;
	CleanupStack::PushL(pArray);
	
	TEntryArray& array=*pArray;
	do
		{
		r=aDir.Read(array);
		if (r==KErrNone || r==KErrEof)
			{
			TInt count=array.Count();
			if (count==0)
				break;
			TInt i=0;
			while (i<count)
				aFileList->AddL(array[i++]);
			}
		}while (r==KErrNone);
	
	CleanupStack::PopAndDestroy();
	if (!(r==KErrNone || r==KErrEof))
		User::Leave(r);
	aFileList->Compress();
	if (aKey==ESortNone)
		return;
	
	r=aFileList->Sort(aKey);
	if (r!=KErrNone)
		User::Leave(r);
	}




EFSRV_EXPORT_C TInt RFs::GetDir(const TDesC& aName,const TUidType& aUidType,TUint aKey,CDir*& aFileList) const
/**
Gets a filtered list of a directory's contents by UID type.

The aUidType parameter determines which file entry types should be listed.
The sort key determines the order in which they are listed.

Notes:

1. The function sets aFileList to NULL, and then allocates memory for it before
   appending entries to the list. Therefore, aFileList should have no memory
   allocated to it before this function is called, otherwise this memory
   will become orphaned.
   
2. The caller of this function is responsible for deleting aFileList after
   the function has returned.
   
@param aName     The name of the directory for which a listing is required.
                 Wildcards may be used to specify particular files.
@param aUidType  Only those files whose UIDs match those specified within this
                 UID type will be included in the file list. Any, or all, of
                 the three UIDs within the UID type may be omitted.
				 Any UID which is omitted acts in a similar manner to
				 a wildcard character, matching to all UIDs.
@param aKey      The sort key. This is a set of flags indicating the order in
                 which the entries are to be sorted. These flags are defined
                 by TEntryKey.
@param aFileList On return contains a filtered list of directory and file entries.

@return KErrNone if successful, otherwise one of the other
        system-wide error codes.
        
@see TEntryKey
*/
	{
	TRACEMULT6(UTF::EBorder, UTraceModuleEfsrv::EFsGetDir1, MODULEUID, 
		Handle(), aName, aUidType[0].iUid, aUidType[1].iUid, aUidType[2].iUid, aKey);

	RDir d;
	TRAPD(r,GetDirL(aName,aUidType,aKey,aFileList,d))
	d.Close();
	if (r!=KErrNone)
		{
		delete aFileList;
		aFileList=NULL;
		}

	TRACERET1(UTF::EBorder, UTraceModuleEfsrv::EFsGetDir1Return, MODULEUID, r);
	return r;
	}




EFSRV_EXPORT_C TInt RFs::GetDir(const TDesC& aName,TUint anAttMask,TUint aKey,CDir*& aFileList) const
/**
Gets a filtered list of a directory's contents.

The	bitmask determines which file and directory entry types should be listed.
The sort key determines the order in which they are listed.

Notes:

1. If sorting by UID (as indicated when the ESortByUid bit is OR'ed with
   the sort key), then UID information will be included in the listing
   whether or not KEntryAttAllowUid is specified in anAttMask.

2. The function sets aFileList to NULL, and then allocates memory for it before
   appending entries to the list. Therefore, aFileList should have no memory
   allocated to it before this function is called, otherwise this memory will
   become orphaned.

3. The caller of this function is responsible for deleting aFileList after
   the function has returned.

@param aName     The name of the directory for which a listing is required.
                 Wildcards may be used to specify particular files.
@param anAttMask Bitmask indicating the attributes of interest. Only files and
                 directories whose attributes match those specified here can be
                 included in the listing. For more information,
                 see KEntryAttMatchMask and the other directory entry details.
                 Also see KEntryAttNormal and the other file or directory attributes.
@param aKey      The sort key. This is a set of flags indicating the order in
                 which the entries are to be sorted. These flags are defined
                 by TEntryKey.
@param aFileList On return contains a filtered list of directory and file entries.

@return KErrNone if successful, otherwise one of the other
        system-wide error codes.
        
@see TEntryKey
*/
	{
	TRACEMULT4(UTF::EBorder, UTraceModuleEfsrv::EFsGetDir2, MODULEUID, Handle(), aName, anAttMask, aKey);

	RDir d;
	if ((aKey&0xff)==ESortByUid)
		anAttMask|=KEntryAttAllowUid;
	TRAPD(r,GetDirL(aName,anAttMask,aKey,aFileList,d))
	d.Close();
	if (r!=KErrNone)
		{
		delete aFileList;
		aFileList=NULL;
		}

	TRACERET1(UTF::EBorder, UTraceModuleEfsrv::EFsGetDir2Return, MODULEUID, r);
	return r;
	}




EFSRV_EXPORT_C TInt RFs::GetDir(const TDesC& aName,TUint anAttMask,TUint aKey,CDir*& aFileList,CDir*& aDirList) const
/**
Gets a filtered list of the directory and file entries contained in
a directory, and a list of the directory entries only.

The bitmask	determines which file and directory entry types should be listed in
aFileList. The contents of the second list, aDirList are not affected by the bitmask; it
returns all directory entries contained in directory aName. The
sort key determines the order in which both lists are sorted.

Notes:

1. If sorting by UID (as indicated when the ESortByUid bit is OR'ed with
   the sort key), then UID information will be included in the listing
   whether or not KEntryAttAllowUid is specified in anAttMask.
   
2. The function sets both aFileList and aDirList to NULL, and then allocates
   memory to them before appending entries to the lists. Therefore, aFileList
   and aDirList should have no memory allocated to them before this
   function is called, otherwise the allocated memory will become orphaned.

3. The caller of this function is responsible for deleting aFileList
   and aDirList after the function has returned.
   
@param aName     The name of the directory for which a listing is required.
                 Wildcards may be used to specify particular files.
@param anAttMask Bitmask indicating the attributes of interest. Only files and
                 directories whose attributes match those specified here can be
                 included in aFileList. aDirList is unaffected by this mask. 
                 For more information, see KEntryAttMatchMask and the other
                 directory entry details.
                 Also see KEntryAttNormal and the other file or directory
                 attributes.
@param aKey      The sort key. This is a set of flags indicating the order in
                 which the entries in both lists are to be sorted. These flags
                 are defined by TEntryKey.
@param aFileList On return contains a filtered list of directory and
                 file entries.
@param aDirList  On return contains a filtered list of directory entries only.

@return KErrNone if successful, otherwise one of the other
        system-wide error codes.

@see TEntryKey
*/
	{
	TRACEMULT4(UTF::EBorder, UTraceModuleEfsrv::EFsGetDir3, MODULEUID, Handle(), aName, anAttMask, aKey);

	RDir d;
	if (aKey&ESortByUid)
		anAttMask|=KEntryAttAllowUid;
	TRAPD(r,GetDirL(aName,anAttMask,aKey,aFileList,aDirList,d))
	d.Close();
	if (r!=KErrNone)
		{
		delete aFileList;
		aFileList=NULL;
		delete aDirList;
		aDirList=NULL;
		}

	TRACERET1(UTF::EBorder, UTraceModuleEfsrv::EFsGetDir3Return, MODULEUID, r);
	return r;
	}




EFSRV_EXPORT_C TInt RFs::Parse(const TDesC& aName,TParse& aParse) const
/**
Parses a filename specification.

Parsing is done with wildcard resolution, using the session path as
the default. You can then use TParse's getter functions to extract individual
components of the resulting name. All the path components that are included
in aName are put into the resulting	filename. Any components that are still
missing are taken from the session path.

Specifying:

@code
TParse fp;
@endcode
@code
fs.Parse(name,fp);
@endcode

is equivalent to 

@code
TParse fp;
@endcode
@code
fp.Set(name,NULL,&fs.SessionPath());
@endcode

Note that the function does not check for illegal characters, or for
illegal path components in either of the paths specified.

@param aName  The file name to be parsed, using the session path to provide
              the missing components.
@param aParse A TParse objct that provides functions for
              extracting individual components of the resulting file name.

@return KErrNone if successful, otherwise one of the other
        system-wide error codes.
*/
	{
	TRACEMULT2(UTF::EBorder, UTraceModuleEfsrv::EFsParse1, MODULEUID, Handle(), aName);
	TFileName session_path;
	TInt r = SessionPath(session_path);
	if (r==KErrNone)
		r = aParse.Set(aName, NULL, &session_path);

	TRACERET1(UTF::EBorder, UTraceModuleEfsrv::EFsParse1Return, MODULEUID, r);
	return r;
	}




EFSRV_EXPORT_C TInt RFs::Parse(const TDesC& aName,const TDesC& aRelated,TParse& aParse) const
/**
Parses a filename specification, specifying related file path components.

Parsing is done with wildcard resolution, using the session path as
the default. You can then use TParse's getter functions to extract individual
components of the resulting name. All the path components that are included
in aName are put into the resulting	filename. Any missing components are taken
from the optional aRelated argument, which has the next order of precedence.
Finally, any components that are still missing are taken from the session path.

Specifying:

@code
TParse fp;
@endcode
@code
fs.Parse(name,related,fp);
@endcode

is equivalent to 

@code
TParse fp;
@endcode
@code
fp.Set(name,related,&fs.SessionPath());
@endcode

Note that the function does not check for illegal characters, or for
illegal path components in any of the paths specified.

@param aName    The file name to be parsed, using the session path and the
                related path to provide the missing components.
@param aRelated The related file specification.
@param aParse   A TParse objct that provides functions for
                extracting individual components of the resulting file name.

@return KErrNone if successful, otherwise one of the other
        system-wide error codes.
*/
	{
	TRACEMULT3(UTF::EBorder, UTraceModuleEfsrv::EFsParse2, MODULEUID, Handle(), aName, aRelated);
	TFileName session_path;
	TInt r = SessionPath(session_path);
	if (r==KErrNone)
		r = aParse.Set(aName, &aRelated, &session_path);

	TRACERET1(UTF::EBorder, UTraceModuleEfsrv::EFsParse2Return, MODULEUID, r);
	return r;
	}




EFSRV_EXPORT_C TInt RFs::Delete(const TDesC& aName)
/**
Deletes a single file.

Wildcards are not allowed in either the	file name or the extension,
otherwise an error is returned.

Note that the file must be closed and must not be read-only.
Hidden files can be deleted but system files cannot.

See class CFileMan for information on deleting multiple files.
		  
@param aName The name of the file to be deleted. Any path components which
             are not specified here will be taken from the session path.

@return KErrNone if successful, otherwise one of the other
        system-wide error codes.

@capability Dependent If aName is /Sys then Tcb capability is required.
@capability Dependent If aName begins with /Private and does not match this process' SID
					  then AllFiles capability is required.
@capability Dependent If aName is /Resource then Tcb capability is required.
        
@see CFileMan        
*/
	{
	TRACEMULT2(UTF::EBorder, UTraceModuleEfsrv::EFsDelete, MODULEUID, Handle(), aName);
	TInt r = SendReceive(EFsDelete,TIpcArgs(&aName));

	TRACERET1(UTF::EBorder, UTraceModuleEfsrv::EFsDeleteReturn, MODULEUID, r);
	return r;
	}




EFSRV_EXPORT_C TInt RFs::Rename(const TDesC& anOldName,const TDesC& aNewName)
/**
Renames a single file or directory.

It can also be used to move a file or directory by specifying different
destination and source directories.	If so, the destination and source
directories must be on the same drive. If a	directory is moved, then
the directory structure beneath it is also	moved.

If a directory specified by aNewName is different from one specified
by anOldName, then the file or directory is	moved to the new directory.
The file or directory cannot be moved to another device by this means,
either explicitly (by another drive	specified in the name) or implicitly
(because the directory has been mapped to another device with SetSubst().

The function fails and returns an error code in the following
circumstances:

1. If either the old or new name includes wildcards.

2. If a file or directory with the new name already exists in
   the target directory. Overwriting is not permitted.

3. If file anOldName does not exist, or is open.

Read-only, system and hidden files may be renamed. The renamed
file's attributes are preserved.

Note that when this function is operating on directories, a	trailing backslash
is not required after the final directory name in either anOldName or aNewName.

See class CFileMan for information on renaming multiple files.
		  				
@param anOldName File or directory to be renamed. Any path components which are
                 not specified here will be taken from the session path.
@param aNewName  Path specifying the new name for the file or directory and/or
				 its new parent directory. All directories specified in this path
				 must exist.
				 Any path components which are not specified here will be taken
				 from the session path.

@return KErrNone if successful, otherwise one of the other
        system-wide error codes.

@capability Dependent If either anOldName or aNewName is /Sys then Tcb capability is required.
@capability Dependent If either anOldName or aNewName begins with /Private and does not match
					  this process' SID then AllFiles capability is required.
@capability Dependent If either anOldName or aNewName is /Resource then Tcb capability is required.
        
@see CFileMan        
*/
	{
	TRACEMULT3(UTF::EBorder, UTraceModuleEfsrv::EFsRename, MODULEUID, Handle(), anOldName, aNewName);

	TInt r;
	if (anOldName.Length() <= 0 || aNewName.Length() <= 0 )
		r = KErrBadName;
	else
		r = SendReceive(EFsRename,TIpcArgs(&anOldName,&aNewName));

	TRACERET1(UTF::EBorder, UTraceModuleEfsrv::EFsRenameReturn, MODULEUID, r);
	return r;
	}




EFSRV_EXPORT_C TInt RFs::Replace(const TDesC& anOldName,const TDesC& aNewName)
/**
Replaces a single file with another.

This function does not support the use of wildcards. Unlike Rename(), it only
applies to files.

This function operates as follows:

1. if the aNewName file does not exist, it is created.

2. anOldName's contents, attributes and the date and time of its last
   modification are copied to file aNewName, overwriting any existing contents
   and attribute details.

3. anOldName is deleted.
				 
anOldName may be hidden, read-only or a system file. However,
neither anOldName, nor, if it exists, aNewName, can be open;
aNewName must not be read-only.
Both files must be on the same drive.

@param anOldName The file to be replaced. Must exist and must be closed. It is
                 deleted by this function.
@param aNewName  The file to replace anOldName. Does not need to exist, but if
                 it does exist, it must be closed. If it exists, its name
                 remains unchanged but its contents, attributes and the date
                 and time of its last modification are replaced by those
                 of anOldName.
                 If it does not exist, it will be created and is assigned
                 the contents and attributes of anOldName. Must not be followed
                 by a trailing backslash.

@return KErrNone, if successful;
        KErrAccessDenied, if an attempt is made to replace a directory;
        otherwise one of the other system-wide error codes. 

@capability Dependent If either anOldName or aNewName is /Sys then Tcb capability is required.
@capability Dependent If either anOldName or aNewName begins with /Private and does not match
					  this process' SID then AllFiles capability is required.
@capability Dependent If either anOldName or aNewName is /Resource then Tcb capability is required.

*/
	{
	TRACEMULT3(UTF::EBorder, UTraceModuleEfsrv::EFsReplace, MODULEUID, Handle(), anOldName, aNewName);
	TInt r = SendReceive(EFsReplace,TIpcArgs(&anOldName,&aNewName));

	TRACERET1(UTF::EBorder, UTraceModuleEfsrv::EFsReplaceReturn, MODULEUID, r);
	return r;
	}




EFSRV_EXPORT_C TInt RFs::Att(const TDesC& aName,TUint& aVal) const
/**
Gets a file's attributes.

@param aName The filename. Any path components which are not specified here
             will be taken from the session path.
@param aVal  On return, the individual bits within the byte indicate which
             attributes have been set. For more information see KEntryAttNormal
	         and the other file/directory attributes.

@return KErrNone if successful, otherwise one of the other
        system-wide error codes.

@capability Dependent If aName contains /sys/ then AllFiles capability is required.
@capability Dependent If aName contains /Private/ and does not match
					  this process' SID then AllFiles capability is required.
        
@see KEntryAttNormal
*/
	{
	TRACEMULT2(UTF::EBorder, UTraceModuleEfsrv::EFsAtt, MODULEUID, Handle(), aName);

	TEntry e;
	TInt r=Entry(aName,e);
	if (r==KErrNone)
		aVal=e.iAtt;

	TRACERET2(UTF::EBorder, UTraceModuleEfsrv::EFsAttReturn, MODULEUID, r, aVal);
	return r;
	}




EFSRV_EXPORT_C TInt RFs::SetAtt(const TDesC& aName,TUint aSetAttMask,TUint aClearAttMask)
/**
Sets or clears the attributes of a single file or directory.

The function uses two bitmasks. The first bitmask specifies the	attributes
to be set; the second specifies the attributes to be cleared.

An attempt to set or clear the KEntryAttDir, KEntryAttVolume or KEntryAttRemote
attributes have no effect.

@param aName          File or directory name. Any path components which are not
                      specified here will be taken from the session path. Must
                      not include wildcard characters. The file must be closed.
@param aSetAttMask    Bitmask indicating the attributes to be set.
@param aClearAttMask  Bitmask indicating the attributes to be cleared. For more
				      information, see KEntryAttNormal and the other file or
				      directory attributes.

@return KErrNone if successful, otherwise one of the other
        system-wide error codes.

@panic FSCLIENT 21 if any attribute appears in both bitmasks.


@capability Dependent If aName is /Sys then Tcb capability is required.
@capability Dependent If aName begins with /Private and does not match
					  this process' SID then AllFiles capability is required.
@capability Dependent If aName is /Resource then Tcb capability is required.
	
@see RFs::SetEntry

*/
	{
	TRACEMULT4(UTF::EBorder, UTraceModuleEfsrv::EFsSetAtt, MODULEUID, 
		Handle(), aName, aSetAttMask, aClearAttMask);

 	TInt r = SetEntry(aName,TTime(0),aSetAttMask,aClearAttMask);

	TRACERET1(UTF::EBorder, UTraceModuleEfsrv::EFsSetAttReturn, MODULEUID, r);
	return r;
	}




EFSRV_EXPORT_C TInt RFs::Modified(const TDesC& aName,TTime& aTime) const
/**
Gets the last modification date and time of a file or a directory,
in UTC.

If there has been no modification, the function gets the date and
time of the file or directory's creation.

@param aName File or directory name.
@param aTime On return, contains the date and time of the file or
             directory's last modification in universal time.

@return KErrNone if successful, otherwise one of the other
        system-wide error codes.

@capability Dependent If aName contains /sys/ then AllFiles capability is required.
@capability Dependent If aName contains /Private/ and does not match
					  this process' SID then AllFiles capability is required.

*/
	{
	TRACEMULT2(UTF::EBorder, UTraceModuleEfsrv::EFsModified, MODULEUID, Handle(), aName);

	TEntry e;
	TInt r=Entry(aName,e);
	if (r==KErrNone)
		aTime=e.iModified;

	TRACERET3(UTF::EBorder, UTraceModuleEfsrv::EFsModifiedReturn, MODULEUID, r, I64LOW(aTime.Int64()), I64HIGH(aTime.Int64()));
	return r;
	}




EFSRV_EXPORT_C TInt RFs::SetModified(const TDesC& aName,const TTime& aTime)
/**
Sets the date and time that the contents of a file or directory
were modified, in UTC.

@param aName File or directory name.
@param aTime The new date and time that the file or directory was modified
             in universal time.

@return KErrNone if successful;
        KErrInUse, if the file is open;
        otherwise one of the other system-wide error codes.

@capability Dependent If aName is /Sys then Tcb capability is required.
@capability Dependent If aName begins with /Private and does not match
					  this process' SID then AllFiles capability is required.
@capability Dependent If aName is /Resource then Tcb capability is required.

*/
	{
	TRACEMULT4(UTF::EBorder, UTraceModuleEfsrv::EFsSetModified, MODULEUID, Handle(), aName, I64LOW(aTime.Int64()), I64HIGH(aTime.Int64()) );

	TInt r = SetEntry(aName,aTime,KEntryAttModified,0);

	TRACERET1(UTF::EBorder, UTraceModuleEfsrv::EFsSetModifiedReturn, MODULEUID, r);
	return r;
	}




EFSRV_EXPORT_C TInt RFs::Entry(const TDesC& aName,TEntry& anEntry) const
/**
Gets the entry details for a file or directory.

This information includes UID information.

@param aName   Name of file or directory.
@param anEntry On return, contains the entry details for the file or directory. TEntry::iModified contains UTC date and time.

@return KErrNone if successful, otherwise one of the other
        system-wide error codes.

@capability Dependent If aName contains "\\Sys\\" and includes an additional file or directory then AllFiles capability 
					  is required. For example, the paths "c:\\sys" and "c:\\sys\\" will always be readable, whereas
					  the path "c:\\sys\\abc\\" will only be readable with AllFiles capability.

@capability Dependent If aName contains \\Private\\ and includes an additional file, or a directory which does not match
					  this process' SID, then AllFiles capability is required. For example, the paths "c:\\private" and 
					  "c:\\private\\" will always be readable, whereas the path "c:\\private\\<n>\\" will only be 
					  readable with AllFiles capability or if <n> matches the process' SID.
*/
	{
	TRACEMULT2(UTF::EBorder, UTraceModuleEfsrv::EFsEntry, MODULEUID, Handle(), aName);
	TPckg<TEntry> e(anEntry);
	TInt r = SendReceive(EFsEntry,TIpcArgs(&aName,&e));

	TRACE5(UTF::EBorder, UTraceModuleEfsrv::EFsEntryReturn, MODULEUID, 
		r, anEntry.iAtt, 
		I64LOW(anEntry.iModified.Int64()), I64HIGH(anEntry.iModified.Int64()), 
		anEntry.iSize);
	return r;
	}




EFSRV_EXPORT_C TInt RFs::SetEntry(const TDesC& aName,const TTime& aTime,TUint aSetAttMask,TUint aClearAttMask)
/**
Sets both the attributes and the last modified date and time for a file or directory.

The function uses two bitmasks. The first bitmask determines
which attributes should be set. The second bitmask determines which should be cleared.

An attempt to set or clear the KEntryAttDir, KEntryAttVolume or KEntryAttRemote
attributes have no effect.
			 
@param aName          File or directory name.
@param aTime	      New date and time. UTC date and time should be used.
@param aSetAttMask    Bitmask indicating which attributes are to be set.
@param aClearAttMask  Bitmask indicating which attributes are cleared. For more
                      information, see KEntryAttNormal, and the other file
                      or directory attributes.

@return KErrNone, if successful;
        KErrInUse, if the file is open;
        otherwise one of the other system-wide error codes.

@panic FSCLIENT 21 if any attribute appears in both bitmasks.        

@capability Dependent If aName is /Sys then Tcb capability is required.
@capability Dependent If aName begins with /Private and does not match
					  this process' SID then AllFiles capability is required.
@capability Dependent If aName is /Resource then Tcb capability is required.

@see KEntryAttNormal
@see KEntryAttDir
@see KEntryAttVolume
*/
	{
	TRACEMULT6(UTF::EBorder, UTraceModuleEfsrv::EFsSetEntry, MODULEUID, 
		Handle(), aName, 
		I64LOW(aTime.Int64()), I64HIGH(aTime.Int64()), 
		aSetAttMask, aClearAttMask);

	__ASSERT_ALWAYS((aSetAttMask&aClearAttMask)==0,Panic(EAttributesIllegal));
	TPtrC8 timeBuf((TUint8*)&aTime,sizeof(TTime));
	TInt r = SendReceive(EFsSetEntry,TIpcArgs(&aName,&timeBuf,aSetAttMask,aClearAttMask));

	TRACERET1(UTF::EBorder, UTraceModuleEfsrv::EFsSetEntryReturn, MODULEUID, r);
	return r;
	}

/**
Reads data from a file without opening it.

The contents of the	file can be accessed regardless of the file's lock state.

The file may be open by any number of other clients for reading	or writing.
In allowing such access to a file, the fileserver makes no guarantees as to
the validity of the data it returns.

@param aName    Name of the file to be accessed.
@param aPos     The offset, in bytes, from the start of the file where
				reading is to start.
@param aDes     On return, contains the data read from the file. The length of
				the descriptor is set to the number of bytes read. If the
				specified offset lies beyond the end of the file, no data is
				read and the length of this descriptor is set to zero.
@param aLength  The number of bytes to be read from the file.

@return KErrNone if successful, 
		KErrArgument if aLength is negative,
		otherwise one of the other system-wide error codes.

@panic FSCLIENT 19 if aPos negative.
@panic FSCLIENT 27 if aLength is greater than the maximum length of
       the target descriptor.

@capability Dependent If the path for aName starts with /Sys capability AllFiles is required
@capability Dependent If the path for aName starts with /Private and this process does not have 
                      the relevant SID capability AllFiles is required

*/
EFSRV_EXPORT_C TInt RFs::ReadFileSection(const TDesC& aName,TInt64 aPos,TDes8& aDes,TInt aLength) const
	{
	TRACEMULT5(UTF::EBorder, UTraceModuleEfsrv::EFsReadFileSection, MODULEUID, 
		Handle(), aName, I64LOW(aPos), I64HIGH(aPos), aLength);

	__ASSERT_ALWAYS(aPos>=0,Panic(EPosNegative));
	
#ifndef SYMBIAN_ENABLE_64_BIT_FILE_SERVER_API
	if(aPos > KMaxTInt)
		{
		TRACE1(UTF::EBorder, UTraceModuleEfsrv::EFsReadFileSectionReturn, MODULEUID, KErrTooBig);
		return KErrTooBig;
		}
	if((aPos + aLength) > KMaxTInt)
		aLength = KMaxTInt - (TInt)aPos;
#endif
	if (aLength)	//	Number of characters to read
		{
		__ASSERT_ALWAYS(aDes.MaxLength()>=aLength,Panic(EBadLength));
  	 	}
	else
		{
		aDes.Zero();
		TRACE1(UTF::EBorder, UTraceModuleEfsrv::EFsReadFileSectionReturn, MODULEUID, KErrNone);
		return(KErrNone);
		}
		
	__ASSERT_ALWAYS(aDes.MaxLength()>=aLength,Panic(EBadLength));
	
	TInt r;
	if(!(I64HIGH(aPos)))
		{
		r = SendReceive(EFsReadFileSection,TIpcArgs(&aDes,&aName,I64LOW(aPos),aLength));
		}
	else
		{
		TPckgC<TInt64> pkPos(aPos);
		r = SendReceive(EFsReadFileSection|KIpcArgSlot2Desc,TIpcArgs(&aDes,&aName,&pkPos,aLength));
		}
	TRACERET1(UTF::EBorder, UTraceModuleEfsrv::EFsReadFileSectionReturn, MODULEUID, r);
	return r;
	}
/**
Maintained for BC

@internalTechnology
*/
EFSRV_EXPORT_C TInt RFs::ReadFileSection_RESERVED(const TDesC& aName,TInt aPos,TDes8& aDes,TInt aLength) const
	{
	TRACEMULT5(UTF::EBorder, UTraceModuleEfsrv::EFsReadFileSection, MODULEUID, 
		Handle(), aName, aPos, 0, aLength);

	__ASSERT_ALWAYS(aPos>=0,Panic(EPosNegative));
	
	if (aLength)	//	Number of characters to read
		{
		__ASSERT_ALWAYS(aDes.MaxLength()>=aLength,Panic(EBadLength));
  	 	}
	else
		{
		aDes.Zero();
		TRACE1(UTF::EBorder, UTraceModuleEfsrv::EFsReadFileSectionReturn, MODULEUID, KErrNone);
		return(KErrNone);
		}
		
	__ASSERT_ALWAYS(aDes.MaxLength()>=aLength,Panic(EBadLength));
		
	TInt r = SendReceive(EFsReadFileSection,TIpcArgs(&aDes,&aName,aPos,aLength));

	TRACERET1(UTF::EBorder, UTraceModuleEfsrv::EFsReadFileSectionReturn, MODULEUID, r);
	return r;
	}




EFSRV_EXPORT_C void RFs::ResourceCountMarkStart() const
/**
Marks the start of resource count checking.

Typically, this function is called immediately after a client is connected
to the file server, and before any resources are opened.
*/
	{
	TRACE1(UTF::EBorder, UTraceModuleEfsrv::EFsResourceCountMarkStart, MODULEUID, Handle());
	
	RSessionBase::SendReceive(EFsResourceCountMarkStart);

	TRACE0(UTF::EBorder, UTraceModuleEfsrv::EFsResourceCountMarkStartReturn, MODULEUID);
	}




EFSRV_EXPORT_C void RFs::ResourceCountMarkEnd() const
/**
Ends resource count checking. Typically, this function is called immediately 
before closing a session with the file server.

@panic CSessionFs 2 if the number of resources opened since the start of resource 
       count checking is not equal to the number of resources closed.
*/
	{
	TRACE1(UTF::EBorder, UTraceModuleEfsrv::EFsResourceCountMarkEnd, MODULEUID, Handle());

	RSessionBase::SendReceive(EFsResourceCountMarkEnd);

	TRACE0(UTF::EBorder, UTraceModuleEfsrv::EFsResourceCountMarkEndReturn, MODULEUID);
	}




EFSRV_EXPORT_C TInt RFs::ResourceCount() const
/**
Gets the number of currently open resources.

The resource count is incremented by one: when a file or directory
is opened, when a device is opened in preparation for formatting, when a direct access channel
to a disk is opened.

@return The number of resources currently open.
*/
	{
	TRACE1(UTF::EBorder, UTraceModuleEfsrv::EFsResourceCount, MODULEUID, Handle());

	TInt count;
	TPckg<TInt> pckg(count);
	SendReceive(EFsResourceCount,TIpcArgs(&pckg));
	TInt r = *(TInt*)pckg.Ptr();

	TRACERET1(UTF::EBorder, UTraceModuleEfsrv::EFsResourceCountReturn, MODULEUID, r);
	return r;
	}



/**
Checks the integrity of the disk on the specified drive.
On FAT, this checks if a cluster number is invalid, if a cluster is allocated to
more than one file entry, if an unallocated cluster is not set free, and if size
of an entry is invalid.

@param aDrive Path indicating the drive which contains the disk to be checked. If the drive 
        information is not specified the current session drive is taken by default.
		Checkdisk is performed on the requested drive irrespective of the correctness or
		existance of the given path.

@return KErrNone, if successful;
		1, if successful but a file cluster contains a bad value;
		2, if successful but two files are linked to the same cluster;
		3, if successful but an unallocated cluster contains a value;
		4, if successful but the size of a file is not equal to the number of clusters in chain;
        KErrNotReady, if the specified drive is empty;
        KErrNotSupported, if the drive cannot handle this request;
        KErrPermissionDenied, if the caller doesn't have DiskAdmin capability;
        Other system wide error codes may also be returned.

@capability DiskAdmin
*/
EFSRV_EXPORT_C TInt RFs::CheckDisk(const TDesC& aDrive) const
	{
	TRACEMULT2(UTF::EBorder, UTraceModuleEfsrv::EFsCheckDisk, MODULEUID, Handle(), aDrive);
	TInt r = SendReceive(EFsCheckDisk,TIpcArgs(&aDrive));

	TRACERET1(UTF::EBorder, UTraceModuleEfsrv::EFsCheckDiskReturn, MODULEUID, r);
	return r;
	}




EFSRV_EXPORT_C TInt RFs::ScanDrive(const TDesC& aDrive) const
/**
Checks the specified drive for errors and corrects them. Specifically, it
checks if long file name entries' IDs are in sequence and short name is valid,
and file's allocated clusters are not used by other files.

This does not run on the internal RAM drive, and only applies to a
FAT file system.

@param aDrive Path indicating the drive which contains the disk to be checked. If the drive 
        information is not specified the current session drive is taken by default.
		ScanDrive is performed on the requested drive irrespective of the correctness or
		existance of the given path.

@return KErrNone if successful,
        KErrPermissionDenied if caller doesn't have capability DiskAdmin,
        KErrInUse if drive is in use,
        otherwise one of the other system-wide error codes

@capability DiskAdmin
*/
	{
	TRACEMULT2(UTF::EBorder, UTraceModuleEfsrv::EFsScanDrive, MODULEUID, Handle(), aDrive);
	TInt r = SendReceive(EFsScanDrive,TIpcArgs(&aDrive));

	TRACERET1(UTF::EBorder, UTraceModuleEfsrv::EFsScanDriveReturn, MODULEUID, r);
	return r;
	}




EFSRV_EXPORT_C TInt RFs::GetShortName(const TDesC& aLongName,TDes& aShortName) const
/**
Gets the short filename associated with a VFAT long filename.

The short filename has a limit of eight characters for the file name and three
characters for the extension.

@param aLongName  The long filename. Any path components which are not
                  specified	here will be taken from the session path.
                  If the path specifies a directory, it may or may not be
                  followed by a trailing backslash.
@param aShortName On return, contains the short filename associated with the file
                  or directory specified in aLongName.

@return KErrNone if successful, otherwise one of the other
        system-wide error codes.

@capability Dependent If the path for aLongName starts with /Sys capability AllFiles is required
@capability Dependent If the path for aLongName starts with /Private and this process does not
					  have the relevant SID capability AllFiles is required
*/
	{
	TRACEMULT2(UTF::EBorder, UTraceModuleEfsrv::EFsGetShortName, MODULEUID, Handle(), aLongName);
	TInt r = SendReceive(EFsGetShortName,TIpcArgs(&aLongName,&aShortName));

	TRACERETMULT2(UTF::EBorder, UTraceModuleEfsrv::EFsGetShortNameReturn, MODULEUID, r, aShortName);
	return r;
	}




EFSRV_EXPORT_C TInt RFs::GetLongName(const TDesC& aShortName,TDes& aLongName) const
/**
Gets the long filename associated with a short (8.3) filename.

A long filename has a limit of 256 characters for each component, as well as a
limit of 256 characters for the entire path.

@param aShortName The short file name. Any path components which are not
                  specified here will be taken from the session path. If
                  the path specifies a directory, it may or may not be followed
                  by a trailing backslash.
@param aLongName  On return, contains the long version of the name.
				
@return KErrNone if successful, otherwise one of the other
        system-wide error codes.

@capability Dependent If the path for aShortName starts with /Sys capability AllFiles is required
@capability Dependent If the path for aShortName starts with /Private and this process does not
					  have the relevant SID capability AllFiles is required

*/
	{
	TRACEMULT2(UTF::EBorder, UTraceModuleEfsrv::EFsGetLongName, MODULEUID, Handle(), aShortName);
	TInt r = SendReceive(EFsGetLongName,TIpcArgs(&aShortName,&aLongName));

	TRACERETMULT2(UTF::EBorder, UTraceModuleEfsrv::EFsGetLongNameReturn, MODULEUID, r, aLongName);
	return r;
	}




EFSRV_EXPORT_C TInt RFs::IsFileOpen(const TDesC& aFileName,TBool& anAnswer) const
/**
Tests whether a file is open.

This function is useful because several file based operations provided by
the RFs class, for example: Delete(), Replace() and Rename(), require that
the file be closed.

@param aFileName The name of the file to test. Any path components which are
                 not specified here will be taken from the session path. If a
				 directory is specified instead of a file then KErrNone will be
				 returned and anAnswer will be set to EFalse.
@param anAnswer  On return, true if the file is open, false if closed.

@return KErrNone if successful, otherwise one of the other
        system-wide error codes.

@capability Dependent If the path for aFileName starts with /Sys capability AllFiles is required
@capability Dependent If the path for aFileName starts with /Private and this process does not
					  have the relevant SID capability AllFiles is required
        
@see RFs::Delete
@see RFs::Rename 
@see RFs::Replace
*/
	{
	TRACEMULT2(UTF::EBorder, UTraceModuleEfsrv::EFsIsFileOpen, MODULEUID, Handle(), aFileName);
	TPckg<TBool> b(anAnswer);
	TInt r = SendReceive(EFsIsFileOpen,TIpcArgs(&aFileName,&b));

	TRACERET2(UTF::EBorder, UTraceModuleEfsrv::EFsIsFileOpenReturn, MODULEUID, r, anAnswer);
	return r;
	}




TInt RFs::GetOpenFileList(TInt& aSessionNumber,TInt& aLocalPos,TThreadId& aThreadId,TEntryArray& anArray) const
//
// Private function to get a list of open files 
//
	{
	TOpenFileListPos s(aSessionNumber,aLocalPos);
	TPckg<TOpenFileListPos> pS(s);
	TPckg<TThreadId> threadId(aThreadId);
	anArray.iCount=KCountNeeded;
	TInt r=SendReceive(EFsListOpenFiles,TIpcArgs(&pS,&threadId,&anArray.iBuf));
	aSessionNumber=s.iSession;
	aLocalPos=s.iEntryListPos;
	return r;
	}




EFSRV_EXPORT_C TBool RFs::GetNotifyUser()
/**
Tests whether user notification of file read or write failure is in effect.

@return True if notification in effect, false if not.
*/
	{
	TRACE1(UTF::EBorder, UTraceModuleEfsrv::EFsGetNotifyUser, MODULEUID, Handle());

	TInt notifyUser;
	TPckg<TInt> pckgNotify(notifyUser);
	SendReceive(EFsGetNotifyUser,TIpcArgs(&pckgNotify));
	TBool r = notifyUser;

	TRACERET1(UTF::EBorder, UTraceModuleEfsrv::EFsGetNotifyUserReturn, MODULEUID, r);
	return r;
	}




EFSRV_EXPORT_C void RFs::SetNotifyUser(TBool aValue)
/**
Sets whether the user should be notified of file read or write failure.
Note that if some drive is mounted as synchronous (see RFs::MountFileSystem), the user won't be 
notified about read/write failures on it. 

@param aValue ETrue, if user is to be notified of read or write failures;
              EFalse, for no notification.
*/
	{
	TRACE2(UTF::EBorder, UTraceModuleEfsrv::EFsSetNotifyUser, MODULEUID, Handle(), aValue);
	SendReceive(EFsSetNotifyUser,TIpcArgs(aValue));

	TRACE0(UTF::EBorder, UTraceModuleEfsrv::EFsSetNotifyUserReturn, MODULEUID);
	}




EFSRV_EXPORT_C TUint8* RFs::IsFileInRom(const TDesC& aFileName) const
/**
Gets a pointer to the specified file, if it is in ROM.

Note that this is not a test of whether the file is on the Z: drive, as
the Z: drive may consist of a ROM and another file system, using the composite
file system. For example, the file system may be ROFS, and the underlying media
NAND flash.

@param aFileName The filename whose address is sought. Cannot include wildcards.
                 Any path components which are not specified here will be taken
                 from the session path.

@return Address of the start of the file, if it is in ROM. This is NULL, if
        the file is not in ROM. Note that for the composite file system, the file
        might be on the Z: drive but in a non-ROM file system (i.e. ROFS), in
        which case the function still returns NULL.

@capability Dependent If the path for aFileName starts with /Sys capability AllFiles is required
@capability Dependent If the path for aFileName starts with /Private and this process does not
					  have the relevant SID capability AllFiles is required

*/
	{
	TRACEMULT2(UTF::EBorder, UTraceModuleEfsrv::EFsIsFileInRom, MODULEUID, Handle(), aFileName);

	TPckgBuf<TUint8*> start;

	TUint8* r;
	if (SendReceive(EFsIsFileInRom,TIpcArgs(&aFileName,&start))!=KErrNone)
		r = NULL;
	else
		r = start();

	TRACE1(UTF::EBorder, UTraceModuleEfsrv::EFsIsFileInRomReturn, MODULEUID, r);
	return r;
	}




/**
Tests whether a filename and path are syntactically correct.

The following restrictions apply to the path and to its components:

1.  Wildcards are not allowed in any path component, including the filename and extension.
2.  Double backslashes are not allowed anywhere in the path
3.  The following 6 characters cannot appear in the path: < > : " / |
4.  Either or both of a filename or extension must be present. This means that a valid aFileName can not 
    end with backslash (like "c:\\SomeName\\"), because it will mean that "SomeName" is a directory.

5.  The entire component following the final backslash (the filename and extension) cannot consist solely of space characters, 
    or of a single or double dot.

6.  Spaces between the drive, if specified, and the first directory in the path are illegal, although there may be 
    spaces between other path components, for example, between directories.

7.  If the path in aFileName is not fully specified, i.e. doesn't look like "c:\\Dir1\\File1.bin", all missing 
    parts of the full path will be taken from the session path, @see RFs::SetSessionPath, @see RFs::SessionPath.
    In this case the session path must be set, otherwise this method will return EFalse.
    For example: for the case "\\file1.txt" only the drive letter will be taken from the session path;
                 for the case "file1.txt"   whole session path will be internally prepended to the "file1.txt" and whole path checked.
    Note that in this case total length of the name in the aFileName parameter and the session path shall not exceed KMaxFileName characters.
    
   
@param aFileName    The path to be checked for validity. 
                    May specify a filename alone, or an entire path specification, including drive letter. 
                    If a path is specified, all components are checked for validity.

@return ETrue, if the name is valid (conforms to the mentioned above criteria); EFalse otherwise.
*/
EFSRV_EXPORT_C TBool RFs::IsValidName(const TDesC& aFileName) const
	{
	TRACEMULT2(UTF::EBorder, UTraceModuleEfsrv::EFsIsValidName1, MODULEUID, Handle(), aFileName);
	TBool returnInvalidChar=EFalse;
	TPckg<TBool> bPckg(returnInvalidChar);
	TBool b;
	if (SendReceive(EFsIsValidName,TIpcArgs(&aFileName,&bPckg,NULL,NULL))!=KErrNone)
		b = EFalse;
	else
		b = ETrue;
	TRACE1(UTF::EBorder, UTraceModuleEfsrv::EFsIsValidName1Return, MODULEUID, b);
	return b;
	}




/**
The following restrictions apply to the path and to its components:

1.  Wildcards are not allowed in any path component, including the filename and extension.
2.  Double backslashes are not allowed anywhere in the path
3.  The following 6 characters cannot appear in the path: < > : " / |
4.  Either or both of a filename or extension must be present. This means that a valid aFileName can not 
    end with backslash (like "c:\\SomeName\\"), because it will mean that "SomeName" is a directory.

5.  The entire component following the final backslash (the filename and extension) cannot consist solely of space characters, 
    or of a single or double dot.

6.  Spaces between the drive, if specified, and the first directory in the path are illegal, although there may be 
    spaces between other path components, for example, between directories.

7.  If the path in aFileName is not fully specified, i.e. doesn't look like "c:\\Dir1\\File1.bin", all missing 
    parts of the full path will be taken from the session path, @see RFs::SetSessionPath, @see RFs::SessionPath.
    In this case the session path must be set, otherwise this method will return EFalse.
    For example: for the case "\\file1.txt" only the drive letter will be taken from the session path;
                 for the case "file1.txt"   whole session path will be internally prepended to the "file1.txt" and whole path checked.
    Note that in this case total length of the name in the aFileName parameter and the session path shall not exceed KMaxFileName characters.
   
@param aFileName    The path to be checked for validity. 
                    May specify a filename alone, or an entire path specification, including drive letter. 
                    If a path is specified, all components are checked for validity.

@param aBadChar     reference to the variable that on return can contain illegal character from aFileName.
                    1. if the filename and optional path in aFileName are valid, this method will return ETrue and aBadChar will be set to 0x00.
                    2. if there is an illegal character in aFileName, this method will return EFalse and aBadChar will contain this illegal character.
                    3. if there is no illegal characters in aFileName, but this is still not a valid filename (like "\\SomeName\\") 
                        this method will return EFalse and aBadChar will contain space ' ' or code 0x20.

@return ETrue, if the name is valid (conforms to the mentioned above criteria); EFalse otherwise.
*/
EFSRV_EXPORT_C TBool RFs::IsValidName(const TDesC& aFileName,TText& aBadChar) const
	{
	TRACEMULT2(UTF::EBorder, UTraceModuleEfsrv::EFsIsValidName2, MODULEUID, Handle(), aFileName);
	TBool returnInvalidChar=ETrue;
	TPckg<TBool> boolPckg(returnInvalidChar);
	TPckg<TText> textPckg(aBadChar);
	TBool b;
	if (SendReceive(EFsIsValidName,TIpcArgs(&aFileName,&boolPckg,&textPckg,NULL))!=KErrNone)
		b = EFalse;
	else 
		b = ETrue;
	TRACE2(UTF::EBorder, UTraceModuleEfsrv::EFsIsValidName2Return, MODULEUID, b, aBadChar);
	return b;
	}
/**
This API can be used to validate both directory and file names.
If the name ends with a trailing backslash '\\' then it is considered to be a directory
else a filename.
For example: "C:\\test\\" would mean a directory, whereas
			 "C:\\test" would mean a file, both of which would be returned as a Valid Name.
However a name such as "C:\\test\\\\" would be returned as an Invalid name with error code TError::ErrBadName

The following restrictions apply to the path and to its components:

1.  Wildcards are not allowed in any path component, including the name and extension.
2.  Double backslashes are not allowed anywhere in the path
3.  The following 6 characters cannot appear in the path: < > : " / |
4.  The entire component following the final backslash (the filename and extension) cannot consist solely of space characters, 
    or of a single or double dot.
5.  Spaces between the drive, if specified, and the first directory in the path are illegal, although there may be 
    spaces between other path components, for example, between directories.
6.  If TNameValidParam::iUseSesssionPath is set to ETrue, and if the path in aName is not fully specified, 
	i.e. doesn't look like "c:\\Dir1\\File1.bin", all missing parts of the full path will be taken from the session path,
 	@see RFs::SetSessionPath, @see RFs::SessionPath.
    In this case the session path must be set, otherwise this method will return EFalse.
    For example: for the case "\\file1.txt" only the drive letter will be taken from the session path;
                 for the case "file1.txt"   whole session path will be internally prepended to the "file1.txt" and whole path checked.
    Note that in this case total length of the name in the aName parameter and the session path shall not exceed KMaxFileName characters.
7. If TNameValidParam::iUseSesssionPath is set to EFalse, which is the default value, then
   the session path is not used to fill in the missing parts of the name as stated above.
   For example: for the case "file1.txt", session path will not be used to check the validity of the name. 
@param aName    The path to be checked for validity. 
                May specify a name alone, or an entire path specification, including drive letter. 
                If a path is specified, all components are checked for validity.

@param aParam     reference to the variable that on return can contain details of the error if any.
				  While constructing an object of this type one could specify whether one wants to use the sessionPath for filling up missing parts of aName,
				  or one would want to test aName as it is without prepending the sessionPath.
				  By default the sessionPath is NOT used. 
                    1. if the name and optional path in aName are valid, this method will return ETrue and TError::iError will contain ErrNone.
                    2. if there is an illegal character in aName, this method will return EFalse and TError::iError will contain KErrBadCharacter.
                       Also TError::iInvalidCharPos will indicate the position of the rightmost invalid character. 
                    3. if there is no illegal characters in aName, but this is still not a valid name (like "") 
                       this method will return EFalse and TError::iError will contain KErrBadCharacter, while iInvalidCharPos will be set to 0
					4. if length of the name exceeds 256 characters, this method will return EFalse and TError::iError will contain KErrTooLong.
					   if the optional sessionPath is used, then the length of the sessionPath is also used to determine whether the length exceeds 256 characters.
@return ETrue, if the name is valid (conforms to the mentioned above criteria); EFalse otherwise.
*/
EFSRV_EXPORT_C TBool RFs::IsValidName(const TDesC& aName, TNameValidParam& aParam )
	{
	TRACEMULT2(UTF::EBorder, UTraceModuleEfsrv::EFsIsValidName3, MODULEUID, Handle(), aName);
	TPckg<TNameValidParam> paramPckg(aParam);
	TBool b;
	if (SendReceive(EFsIsValidName,TIpcArgs(&aName,NULL,NULL,&paramPckg))!=KErrNone)
		b = EFalse;
	else
		b = ETrue;
	TRACE2(UTF::EBorder, UTraceModuleEfsrv::EFsIsValidName3Return, MODULEUID, b, aParam.ErrorCode());
	return b;
	}




EFSRV_EXPORT_C TInt RFs::GetDriveName(TInt aDrive,TDes& aDriveName) const
/**
Gets the name of a drive.

Drive naming is optional. If the drive specified has not been assigned a name,
this function returns a descriptor whose length is zero.

@param aDrive     The drive number. Specify a drive in the range
                  EDriveA to EDriveZ for drives A to Z, respectively.
                  The default drive is the session default drive represented
                  by KDefaultDrive.
@param aDriveName On return, the drive name.

@return KErrNone if successful, otherwise one of the other
        system-wide error codes.

@see TDriveNumber
*/
	{
	TRACE2(UTF::EBorder, UTraceModuleEfsrv::EFsGetDriveName, MODULEUID, Handle(), aDrive);
	TInt r = SendReceive(EFsGetDriveName,TIpcArgs(aDrive,&aDriveName));

	TRACERETMULT2(UTF::EBorder, UTraceModuleEfsrv::EFsGetDriveNameReturn, MODULEUID, r, aDriveName);
	return r;
	}




EFSRV_EXPORT_C TInt RFs::SetDriveName(TInt aDrive,const TDesC& aDriveName)
/**
Sets the name of a drive.

Drive naming is optional. Any drive can be assigned a name, and more than
one drive can share the same name.

@param aDrive     The drive number. Specify a drive in the range
                  EDriveA to EDriveZ for drives A to Z, respectively.
                  Specify KDefaultDrive for the session default drive.
@param aDriveName The name of the drive, with a maximum of 256 characters.
                  The name cannot contain the 6 characters < > : " / |

@return KErrNone if successful;
        KErrBadName, if the name contains illegal characters;
        otherwise one of the other system-wide error codes.

@capability DiskAdmin

*/
	{
	TRACEMULT3(UTF::EBorder, UTraceModuleEfsrv::EFsSetDriveName, MODULEUID, Handle(), aDrive, aDriveName);
	TInt r = SendReceive(EFsSetDriveName,TIpcArgs(aDrive,&aDriveName));

	TRACERET1(UTF::EBorder, UTraceModuleEfsrv::EFsSetDriveNameReturn, MODULEUID, r);
	return r;
	}




EFSRV_EXPORT_C TInt RFs::LockDrive(TInt aDrv, const TMediaPassword &aOld, const TMediaPassword &aNew, TBool aStore)
/**
Sets the password for the media in the specified drive. 

The media is not necessarily locked afterwards. Accessibility is determined 
by the following rules:

 - The media may not become locked until power is removed (such as with MMC cards)
 - If the password is added to the password store (the aStore parameter is ETrue), the 
   media will be automatically unlocked on the next access.

@param aDrv   The drive.
@param aOld   The existing password. If no password is set, this must be a zero-length descriptor.
@param aNew   The new password.
@param aStore ETrue if the new password is to be saved to the controller password store; 
              EFalse if not.

@return KErrNone if successful;
        KErrNotSupported if the media does not support password locking.

@capability DiskAdmin

*/
	{
	TRACE3(UTF::EBorder, UTraceModuleEfsrv::EFsLockDrive, MODULEUID, Handle(), aDrv, aStore);
	TInt r = SendReceive(EFsLockDrive,TIpcArgs(aDrv,&aOld,&aNew,aStore));

	TRACERET1(UTF::EBorder, UTraceModuleEfsrv::EFsLockDriveReturn, MODULEUID, r);
	return r;
	}




EFSRV_EXPORT_C TInt RFs::UnlockDrive(TInt aDrive, const TMediaPassword &aPassword, TBool aStore)
/**
Unlocks the media in the specified drive.

The password must be added to the MultiMedia card controller's password store
so that the controller can subsequently issue the password without the user
having to be prompted for it again.

@param aDrive    The drive.
@param aPassword The password.
@param aStore    Specify ETrue to add the password to the
                 controller's password store. 
             
@return KErrNone, if successful;
        KErrAccessDenied, if the password is incorrect;
        KErrAlreadyExists, if the card has already been unlocked;
        KErrNotSupported, if the media does not support password locking.

@capability DiskAdmin

*/
	{
	TRACE3(UTF::EBorder, UTraceModuleEfsrv::EFsUnlockDrive, MODULEUID, Handle(), aDrive, aStore);
	TInt r = SendReceive(EFsUnlockDrive,TIpcArgs(aDrive,&aPassword,aStore));

	TRACERET1(UTF::EBorder, UTraceModuleEfsrv::EFsUnlockDriveReturn, MODULEUID, r);
	return r;
	}




EFSRV_EXPORT_C TInt RFs::ClearPassword(TInt aDrv, const TMediaPassword &aPswd)
/**
Clears the password from the locked MultiMedia card in the specified drive.

Clearing the password causes the MultiMedia card controller to set
the password to null.

@param aDrv  The drive.
@param aPswd The current password, which is required to perform this
             operation.
             
@return KErrNone, if successful;
        KErrAccessDenied, if the password is wrong or the card is still locked;              
        otherwise one of the other system-wide error codes.

@capability DiskAdmin

*/
	{
	TRACE2(UTF::EBorder, UTraceModuleEfsrv::EFsClearPassword, MODULEUID, Handle(), aDrv);
	TInt r = SendReceive(EFsClearPassword,TIpcArgs(aDrv,&aPswd));

	TRACERET1(UTF::EBorder, UTraceModuleEfsrv::EFsClearPasswordReturn, MODULEUID, r);
	return r;
	}




EFSRV_EXPORT_C TInt RFs::ErasePassword(TInt aDrv)
/**
Erase the password from the locked MultiMedia card in the specified drive.

Used when the password is unknown, and may result in the media being erased.

Successful execution of this call may result in leaving the media in unformatted state.
Hence, it is recommended to format the Multimedia card after calling RFs::ErasePassword().

@param aDrv  The drive.
             
@return KErrNone, if successful;
        otherwise one of the other system-wide error codes.

@capability DiskAdmin

*/
	{
	TRACE2(UTF::EBorder, UTraceModuleEfsrv::EFsErasePassword, MODULEUID, Handle(), aDrv);
	TInt r = SendReceive(EFsErasePassword,TIpcArgs(aDrv));

	TRACERET1(UTF::EBorder, UTraceModuleEfsrv::EFsErasePasswordReturn, MODULEUID, r);
	return r;
	}




EFSRV_EXPORT_C void RFs::StartupInitComplete(TRequestStatus& aStat)
/**
Noifies the file server that startup initialisation is complete.

@param aStat Request status object.
*/
//
// Notify file server that startup initialisation has been completed
//
	{
	TRACE2(UTF::EBorder, UTraceModuleEfsrv::EFsStartupInitComplete, MODULEUID, Handle(), &aStat);
	aStat=KRequestPending;
	RSessionBase::SendReceive(EFsStartupInitComplete,aStat);

	TRACE0(UTF::EBorder, UTraceModuleEfsrv::EFsStartupInitCompleteReturn, MODULEUID);
	}




EFSRV_EXPORT_C TInt RFs::SetLocalDriveMapping(const TDesC8& aMapping)
//
// Set the local drive mapping
//
	{
	TRACEMULT2(UTF::EBorder, UTraceModuleEfsrv::EFsSetLocalDriveMapping, MODULEUID, Handle(), aMapping);

	TInt r = SendReceive(EFsSetLocalDriveMapping,TIpcArgs(&aMapping));

	TRACERET1(UTF::EBorder, UTraceModuleEfsrv::EFsSetLocalDriveMappingReturn, MODULEUID, r);
	return r;
	}


/**
    Finalise the given drive. This operation is intended to put the drive into the consistent state when it is
    safe to remove it physically or switch the power off.

    @param  aDriveNo    drive number
    @param  aMode       describes the finalisation operation, see RFs::TFinaliseDrvMode enum

    @return KErrNone on success,
            KErrArgument if the function arguments are invalid
            KErrInUse    if the drive has opened objects (files, directories etc.) and therefore can not be finalised
            KErrCorrupt  if the drive is corrupt.
            System wide error codes otherwise.

    @capability DiskAdmin
*/
EFSRV_EXPORT_C TInt RFs::FinaliseDrive(TInt aDriveNo, TFinaliseDrvMode aMode) const
    {
	TRACE3(UTF::EBorder, UTraceModuleEfsrv::EFsFinaliseDrive, MODULEUID, Handle(), aDriveNo, aMode);
    TInt r = SendReceive(EFsFinaliseDrive,TIpcArgs(aDriveNo, (TInt)aMode));

	TRACERET1(UTF::EBorder, UTraceModuleEfsrv::EFsFinaliseDriveReturn, MODULEUID, r);
	return r;
    }


/**
    Makes the best effort to finalise all drives in the system. 
    Effectively calls RFs::FinaliseDrive(..., EFinal_RW) to all present drives in the system. This makes impossible to 
    analyse the error code if the finalisation of some fails.
    It is much better to use RFs::FinaliseDrive(...) specifying concrete drive number and desired finalisation mode.

    @return KErrNone, if successful; otherwise one of the other system-wide error codes.
    @capability DiskAdmin
*/
EFSRV_EXPORT_C TInt RFs::FinaliseDrives()
	{
	TRACE1(UTF::EBorder, UTraceModuleEfsrv::EFsFinaliseDrives, MODULEUID, Handle());
	TInt nRes;
	TDriveList driveList;
	TDriveInfo driveInfo;
	
	nRes=DriveList(driveList);
	if(nRes != KErrNone)
		{
		TRACE1(UTF::EBorder, UTraceModuleEfsrv::EFsFinaliseDrivesReturn, MODULEUID, nRes);
	    return nRes; //-- unable to obtain drives list
		}
	
	//-- walk through all drives in the system sending them "Finalise" request
	for (TInt i=0; i<KMaxDrives; ++i)
        {
	    if(!driveList[i])
	        continue;   //-- skip unexisting drive
	    
	    if(Drive(driveInfo, i) != KErrNone)
	        continue;   //-- skip this drive, can't get information about it
        
        const TUint KDrvAttExclude = KDriveAttRom | KDriveAttRedirected; //-- the drive attributes to exlcude from the finalisation

        if(driveInfo.iDriveAtt & KDrvAttExclude) 
            continue;
        	 
	    nRes = FinaliseDrive(i, EFinal_RW);
	    }
	

	TRACE1(UTF::EBorder, UTraceModuleEfsrv::EFsFinaliseDrivesReturn, MODULEUID, KErrNone);
	return 	KErrNone;
	}



EFSRV_EXPORT_C TInt RFs::SwapFileSystem(const TDesC& aOldFileSystemName,const TDesC& aNewFileSystemName,TInt aDrive) const
/**
Dismount aOldFileSystemName and mount aNewFileSystemName in an atomic operation

If swapping in the composite filesystem, and no mounts have been added to it,
then ROFS is added to it by default.  The synchronous state of the composite filesystem
will be used in preference to that of the old filesystem when it is mounted.

@param aOldFileSystemName The filesystem name that is currently on the drive.
@param aNewFileSystemName The filesystem name that is to be swapped onto the drive.
@param aDrive The drive for which the filesystem is to be swapped.

@return KErrNone if successful
		KErrInUse if a dismount is pending on the drive
		KErrNotSupported if swapping Z drive with something other then composite or if the drive is asynchronous
		KErrAlreadyExists if swapping the composite filesystem, and it is already mounted
		KErrNotFound If the filesystem name provided could not be found.
		
@capability DiskAdmin
*/
	{
	TRACEMULT4(UTF::EBorder, UTraceModuleEfsrv::EFsSwapFileSystem, MODULEUID, Handle(), aOldFileSystemName, aNewFileSystemName, aDrive);
	TInt r = SendReceive(EFsSwapFileSystem,TIpcArgs(&aNewFileSystemName,aDrive,&aOldFileSystemName));

	TRACERET1(UTF::EBorder, UTraceModuleEfsrv::EFsSwapFileSystemReturn, MODULEUID, r);
	return r;
	}


EFSRV_EXPORT_C TInt RFs::AddCompositeMount(const TDesC& aFileSystemName,TInt aLocalDriveToMount,TInt aCompositeDrive, TBool aSync) const
/**
Adds a local drive to the composite filesystem.  This can only be used before 
the composite filesystem is mounted.  The local drive is mounted with the
filesystem provided.  If any local drive added is marked to be asynchronous,
then the whole composite drive will be treated asynchronous.

@param aFileSystemName The filesystem of the local drive to be added.
@param aLocalDriveToMount The local drive to be added.
@param aCompositeDrive The drive the composite filesystem will be mounted on.
@param aSync If the filesystem added here is preferred to be synchronous.

@return KErrNone if successful
		KErrNotFound If the filesystem name provided could not be found.
		KErrNotReady If the composite filesystem has not been initialised.
		KErrNotSupported If the composite filesystem is already mounted or the parameters passed are unsupported
		
@capability DiskAdmin
*/
	{
	TRACEMULT5(UTF::EBorder, UTraceModuleEfsrv::EFsAddCompositeMount, MODULEUID, 
		Handle(), aFileSystemName, aLocalDriveToMount, aCompositeDrive, aSync);
	TInt r = SendReceive(EFsAddCompositeMount,TIpcArgs(&aFileSystemName,aLocalDriveToMount,aCompositeDrive,aSync));

	TRACERET1(UTF::EBorder, UTraceModuleEfsrv::EFsAddCompositeMountReturn, MODULEUID, r);
	return r;
	}


EFSRV_EXPORT_C TInt RFs::ReserveDriveSpace(TInt aDriveNo, TInt aSpace)
/**
Reserves an area of a drive. It is intended that sensible (tm) apps will reserve a small
area of disk for 'emergency' use in case of later out of disk situations. If the amount of 
reserved space later needs to be readjusted, this method should be called again with 
aSpace as the amount of extra space needed. 

Once space has been reserved via this method, an application can use RFs::GetReserveAccess
to gain access to the reserved area prior to executing disk space critical sections of code.
After the section of code is complete, the application should release access to the reserved
area.

For internal drives, reserved space will be lost if a reboot occurs. For removeable drives,
reserved space may be lost if there is a media change.

Reserved space will be cleaned up automatically when the RFs is closed.

Each drive has a max amount of space available to be reserved, and individual sessions also 
have a max amount of space. These are defined in f32/sfile/sf_std.h as KMaxTotalDriveReserved
and KMaxSessionDriveReserved respectively. Once space is reserved, it is only available to 
the reserving session until that session releases the reserved space.

@param aDriveNo Which drive to reserve space on

@param aSpace Amount of space to reserve

@return KErrNone if successful
        KErrInUse if the session already has reserved access
        KErrArgument if aSpace is invalid (greater than KMaxSessionDriveReserved, negative number, etc.)
        KErrDiskFull if insufficient space is left on the drive to service the request
        KErrTooBig if this request would overflow the available reserve (greater than KMaxTotalDriveReserved)
        any of the possible error return codes from TDrive::Volume()
*/
	{
	TRACE3(UTF::EBorder, UTraceModuleEfsrv::EFsReserveDriveSpace, MODULEUID, Handle(), aDriveNo, aSpace);
	TInt r = SendReceive(EFsReserveDriveSpace, TIpcArgs(aDriveNo, aSpace));

	TRACERET1(UTF::EBorder, UTraceModuleEfsrv::EFsReserveDriveSpaceReturn, MODULEUID, r);
	return r;
	}




EFSRV_EXPORT_C TInt RFs::GetReserveAccess(TInt aDriveNo)
/**
Get exclusive access for this session to overwrite a specific disk area, which has previously
been reserved via RFs::ReserveDriveSpace

@param aDriveNo drive on which to get reserved access

@return KErrNone if successful
        KErrPermissionDenied if the drive has no spare reserved space
*/
	{
	TRACE2(UTF::EBorder, UTraceModuleEfsrv::EFsGetReserveAccess, MODULEUID, Handle(), aDriveNo);
	TInt r = SendReceive(EFsGetReserveAccess, TIpcArgs(aDriveNo));

	TRACERET1(UTF::EBorder, UTraceModuleEfsrv::EFsGetReserveAccessReturn, MODULEUID, r);
	return r;
	}

EFSRV_EXPORT_C TInt RFs::ReleaseReserveAccess(TInt aDriveNo)
/**
Release exclusive access for this session to overwrite a specific disk area.

@param aDriveNo drive on which to release reserved access

@return KErrNone (always returned)

*/
	{
	TRACE2(UTF::EBorder, UTraceModuleEfsrv::EFsReleaseReserveAccess, MODULEUID, Handle(), aDriveNo);
	TInt r = SendReceive(EFsReleaseReserveAccess, TIpcArgs(aDriveNo));

	TRACERET1(UTF::EBorder, UTraceModuleEfsrv::EFsReleaseReserveAccessReturn, MODULEUID, r);
	return r;
	}




/**
    Controls file system dismounting on the specified drive, the way of control depends on the parameter TNotifyDismountMode. 
    
    This API allows interested parties to:
        1.  Subscribe for notification of file system dismounting events.
            This allows subscribers to commit their data to the media prior to the file system being dismounted.
            See TNotifyDismountMode::EFsDismountRegisterClient

        2.  Make a graceful attempt to dismount the file system by notifying the subscribers about a pending file system dismount
            and waiting until all subscribers have finished processing the notification and have signaled that they are ready. 
            If all clients don't respond in a reasonable time, the dismount request may be cancelled, followed by a forced dismount.
            If some client does not subscribe for dismounting notification and keeps handles opened, then after the file system dismounting all these
            handles will become invalid, any subsequent attempts to use them will result in KErrDismounted, and they should be closed(e.g. RFile::Close()). 
            See TNotifyDismountMode::EFsDismountNotifyClients

        3.  Dismount the file system by force even if there are opened handles (files, directories) on the volume being dismounted. 
            Any clients that kept handles opened, after forced file system dismounting will have them invalidated. And any further attempts to use 
            these handles will result in KErrDismounted, and they should be closed(e.g. RFile::Close()). 
            See TNotifyDismountMode::EFsDismountForceDismount

        * If there are clamped files on the volume, the file system dismounting will not happen until these files are unclamped.     
           

    The use case scenario:
    A 'Master' application that wants to dismount the file system on some drive 'aDrive'
    'Client1' and 'Client2' applications interested in file system dismount event notifications, because they need to commit their data before the file system is dismounted.
        
        1.  'Client1' and 'Client2' subscribe to the FS dismount notification using EFsDismountRegisterClient and start waiting on the request status objects.
        2.  'Master' decides to dismount the file system on the drive 'aDrive'.
            2.1 Graceful attempt: 'Master' calls RFs::NotifyDismount() with EFsDismountNotifyClients and starts waiting on 'aStat' for some time until all 'Client1' and 'Client2' respond or timeout occurs.
        
        3.  'Client1' and 'Client2' have their 'aStat' completed as the result of the 'Master' calling EFsDismountNotifyClients.
            3.1 'Client1' and 'Client2' flush data and close file handles.
            3.2 as soon as 'Client1' and 'Client2' decide that they are ready for the pending FS dismount, they signal the 'Master' that they are ready by calling RFs::AllowDismount()

        4.  As soon as _all_ subscribed clients ('Client1' and 'Client2') have called RFs::AllowDismount(), the file system on drive 'aDrive' is 
            dismounted and 'Master' has 'aStat' completed.

        If, for example, 'Client2' hasn't responded in a reasonable time by calling RFs::AllowDismount(), the 'Master' can cancel the pending 'aStat' and
        dismount the file system by force by calling this API with EFsDismountForceDismount. 
        In this case all subsequent attempts by 'Client2' to use its opened handles will result in KErrDismounted; these handles should be closed.



    @param aDriveNo The drive on which to request dismount
    @param aMode    specifies the behaviour of the notification API
    @param aStat    Asynchronous request state.
*/
EFSRV_EXPORT_C void RFs::NotifyDismount(TInt aDrive, TRequestStatus& aStat, TNotifyDismountMode aMode /*=EFsDismountRegisterClient*/) const
	{
	TRACE4(UTF::EBorder, UTraceModuleEfsrv::EFsNotifyDismount, MODULEUID, Handle(), aDrive, &aStat, aMode);
	aStat = KRequestPending;
	RSessionBase::SendReceive(EFsNotifyDismount, TIpcArgs(aDrive,aMode,&aStat), aStat);
	// This call is to synchronise with the driver thread as the corresponding cancel function (NotifyDismountCancel)
	// is synchronous, so it can complete before this notify request has even been added to TDismountNotifyQue.
	// This call guarantees that the notify request has been added to queue.
	SendReceive(EFsSynchroniseDriveThread, TIpcArgs(aDrive));

	TRACE0(UTF::EBorder, UTraceModuleEfsrv::EFsNotifyDismountReturn, MODULEUID);
	}




/**
    Cancels the oustanding dismount notifier, completing with KErrCancel.
 
    @param aStat The request status object associated with the request to be cancelled.

    @see RFs::NotifyDismount
*/
EFSRV_EXPORT_C void RFs::NotifyDismountCancel(TRequestStatus& aStat) const
	{
	TRACE2(UTF::EBorder, UTraceModuleEfsrv::EFsNotifyDismountCancel1, MODULEUID, Handle(), &aStat);
	
	if (aStat == KRequestPending)
		SendReceive(EFsNotifyDismountCancel, TIpcArgs(&aStat));

	TRACE0(UTF::EBorder, UTraceModuleEfsrv::EFsNotifyDismountCancel1Return, MODULEUID);
	}



/**
    Cancels all oustanding dismount notifiers for this session, completing with KErrCancel.

    @see RFs::NotifyDismount
*/
EFSRV_EXPORT_C void RFs::NotifyDismountCancel() const
	{
	TRACE1(UTF::EBorder, UTraceModuleEfsrv::EFsNotifyDismountCancel2, MODULEUID, Handle());

	SendReceive(EFsNotifyDismountCancel, TIpcArgs(NULL));

	TRACE0(UTF::EBorder, UTraceModuleEfsrv::EFsNotifyDismountCancel2Return, MODULEUID);
	}




/**
    Used by a client to indicate that it is safe to dismount the file system. This should be called after receiving a pending media removal notification.

    Not calling this does not guarantee that the dismount will not occur as the application requesting the dismount may decide to forcibly dismount
    after a given timeout period.

    @param aDriveNo The drive on which to allow the dismount.

    @return KErrNone if successful

    @see RFs::NotifyDismount
*/
EFSRV_EXPORT_C TInt RFs::AllowDismount(TInt aDrive) const
	{
	TRACE2(UTF::EBorder, UTraceModuleEfsrv::EFsAllowDismount, MODULEUID, Handle(), aDrive);
	TInt r = SendReceive(EFsAllowDismount, TIpcArgs(aDrive));

	TRACERET1(UTF::EBorder, UTraceModuleEfsrv::EFsAllowDismountReturn, MODULEUID, r);
	return r;
	}

EFSRV_EXPORT_C TInt RFs::SetStartupConfiguration(TInt aCommand,TAny* aParam1,TAny* aParam2) const
/**
@publishedPartner
@release

Only can be called in estart. Licensees could use this function to configure
file server at startup through their own version of estart.

Currently only loader thread priority can be specified.

@param aCommand Command indicating what aspect of file server should be configured.
       aParam1 Command specific parameter.
       aParam2 Command specific parameter.

@return KErrNone if successful, KErrPermissionDenied if called outside estart
*/
    {
	TRACE4(UTF::EBorder, UTraceModuleEfsrv::EFsSetStartupConfiguration, MODULEUID, Handle(), aCommand, aParam1, aParam2);
    TInt r = SendReceive(EFsSetStartupConfiguration, TIpcArgs(aCommand,aParam1,aParam2));

	TRACERET1(UTF::EBorder, UTraceModuleEfsrv::EFsSetStartupConfigurationReturn, MODULEUID, r);
	return r;
    }


EFSRV_EXPORT_C TInt RFs::SetNotifyChange(TBool aNotifyChange)
/**
Enables/Disables change notification on a per-session basis.  Change notification is enabled
by default, and can be disabled using this API.  Disabling change notification will result in
clients of the file server not being notified of events such as directory/file changes.
  
@param aNotifyChange ETrue to enable change notifications, EFalse to disable.

@return KErrNone if successful.

@capability DiskAdmin
        
@see RFs::NotifyChange
 */
	{
	TRACE2(UTF::EBorder, UTraceModuleEfsrv::EFsSetNotifyChange, MODULEUID, Handle(), aNotifyChange);
	TInt r = SendReceive(EFsSetSessionFlags, TIpcArgs(aNotifyChange ? EFsSessionNotifyChange: 0, aNotifyChange ? 0 : EFsSessionNotifyChange));

	TRACERET1(UTF::EBorder, UTraceModuleEfsrv::EFsSetNotifyChangeReturn, MODULEUID, r);
	return r;
	}


TInt RFs::Unclamp(const RFileClamp& aHandle)
/**
Makes available for paging-out the media space occupied by the file identified by
the supplied handle.
  
@param aHandle handle to the file on the media.

@return KErrNone if successful.

@capability ???
        
@see RFile::Clamp
 */
	{
	TPckg<RFileClamp> pkHandle(aHandle);
	return SendReceive(EFsUnclamp, TIpcArgs(& pkHandle));
	}

EFSRV_EXPORT_C TInt RFs::InitialisePropertiesFile(const TPtrC8& aPtr) const
/**
Sets the F32 properties file - Can only be called from the ESTART process

@param aPtr A descriptor pointing to an INI file in ROM.

@return KErrNone if successful.

@capability KDiskAdmin
*/
	{
	TRACE3(UTF::EBorder, UTraceModuleEfsrv::EFsInitialisePropertiesFile, MODULEUID, Handle(), aPtr.Ptr(), aPtr.Length());
	TInt r = SendReceive(EFsInitialisePropertiesFile, TIpcArgs(aPtr.Ptr(), aPtr.Length(), ETrue));

	TRACERET1(UTF::EBorder, UTraceModuleEfsrv::EFsInitialisePropertiesFileReturn, MODULEUID, r);
	return r;
	}

EFSRV_EXPORT_C TInt RFs::QueryVolumeInfoExt(TInt aDrive, TQueryVolumeInfoExtCmd aCommand, TDes8& aInfo) const
/**
@internalTechnology
Queries specific information on volumes by commands. Available commands is defined by TQueryVolumeInfoExtCmd.

@param aDriveNo The drive on which to query information.
@param aCommand A command to specify which information is under query
@param aInfo A TPckgBuf<> to carry returned results.

@return KErrNone if successful; otherwise another system-wide error code is returned.
        
@see TQueryVolumeInfoExtCmd
@see TVolumeIOParamInfo
 */
	{
	TRACE3(UTF::EBorder, UTraceModuleEfsrv::EFsQueryVolumeInfoExt, MODULEUID, Handle(), aDrive, aCommand);
	TInt r = SendReceive(EFsQueryVolumeInfoExt, TIpcArgs(aDrive, aCommand, &aInfo));

	TRACERET1(UTF::EBorder, UTraceModuleEfsrv::EFsQueryVolumeInfoExtReturn, MODULEUID, r);
	return r;
	}


EFSRV_EXPORT_C TInt RFs::VolumeIOParam(TInt aDrive, TVolumeIOParamInfo& aParamInfo) const
/**
This function queries a set of I/O parameters on the specified volume, this includes the block size of underlying media,
cluster size of the mounted file system and the recommended read/write buffer sizes. 

The volume information is retuned through aParamInfo. Even if VolumeIOParam() returns successful, errors 
can effect the return value of each field within aParamInfo.

@param aDrive A drive number, specifies which volume to query.
@param aParamInfo A TVolumeIOParamInfo containing the volume parameters.

@return KErrNone if successful; otherwise, another system wide error code is returned.
*/
	{
	TRACE2(UTF::EBorder, UTraceModuleEfsrv::EFsVolumeIOParam, MODULEUID, Handle(), aDrive);

	TInt r = KErrNone;

	if (!IsValidDrive(aDrive))
		r = KErrArgument;
	
	if (r == KErrNone)
		{
		TPckgBuf<TVolumeIOParamInfo> infoPckg;
		r = QueryVolumeInfoExt(aDrive, EIOParamInfo, infoPckg);
		if (r == KErrNone)
			aParamInfo = infoPckg();
		}

	TRACE5(UTF::EBorder, UTraceModuleEfsrv::EFsVolumeIOParamReturn, MODULEUID, 
		r, aParamInfo.iBlockSize, aParamInfo.iClusterSize, aParamInfo.iRecReadBufSize, aParamInfo.iRecWriteBufSize);
	return r;
	}


/**
    This function queries the sub type of the file system mounted on the specified volume. For example, 'FAT16' of the Fat file system. 
    TFSName is recommended as the type for aName when using this function.

    NOTE: For the file systems without a sub type (e.g. ROM file system), the  the file system name is returned (For example, 'Rom').
    Examples:
        "FAT"   file system; the subtypes can be "fat12", "fat16" or "fat32"
        "ROFS"  file system; the subtype will be "ROFS"

    Note also that the file system name, returned in the aName descriptor shall be threated as case-insensitive string. I.e. 
    "fileSystem" and "FILESYSTEM" mean absolutely the same. Therefore, case-insensitive string methods (like TDesC::FindF(), TDesC::CompareF())
    shall be used to deal with the names.


    @param  aDrive  drive number, specifies which volume to query.
    @param  aName   descriptor containing the returned sub type name or file system name.

    @return KErrNone if successful; KErrNotSuppoted if sub type is not supported; 
		    otherwise another system-wide error code is returned.

    @see TFSName
*/
EFSRV_EXPORT_C TInt RFs::FileSystemSubType(TInt aDrive, TDes& aName) const
	{
	TRACEMULT3(UTF::EBorder, UTraceModuleEfsrv::EFsFileSystemSubType, MODULEUID, Handle(), aDrive, aName);

	TInt r = KErrNone;

	if (!IsValidDrive(aDrive))
		r = KErrArgument;
		
	if (r == KErrNone)
		{
		TPckgBuf<TFSName> namePckg;
		r = QueryVolumeInfoExt(aDrive, EFileSystemSubType, namePckg);
		if (r == KErrNone || r == KErrNotSupported)
			aName = namePckg();
		}

	TRACERET1(UTF::EBorder, UTraceModuleEfsrv::EFsFileSystemSubTypeReturn, MODULEUID, r);
	return r;
	}

EXPORT_C TInt RFs::AddProxyDrive(const TDesC& aFileName)
/**
Loads the specified extension.

@param aFileName The file name of the extension

@return KErrNone, if successful; otherwise one of the other system wide error codes.
*/
	{
	RLoader loader;
	TInt r = loader.Connect();
	if (r==KErrNone)
		{
		r = loader.SendReceive(ELoadFSProxyDrive, TIpcArgs(0, &aFileName, 0));
		loader.Close();
		}
	return r;
	}


EXPORT_C TInt RFs::RemoveProxyDrive(const TDesC& aExtensionName)
/**
Removes the specified extension.

@param aExtensionName The fullname of the extension, as returned from
                      a call to ExtensionName().

@return KErrNone, if successful;
	KErrInUse if there are still drives mounted that are using it
       KErrNotFound, if aExtensionName is not found;
       otherwise one of the other system-wide error codes.
*/
	{
	return(SendReceive(EFsRemoveProxyDrive,TIpcArgs(&aExtensionName)));
	}

/**
Initialises a proxy drive.

@param aProxyDriveNumber drive number that will be used to mount the proxy drive
@param aName name of the proxy drive extension
@param anInfo initialisation information to be passed to the proxy drive extension to initialise the drive

@return If succesful the internal drive number used to represent the drive (equivalent to a local drive 
		number for normal drives) This number is obtained dynmically. The number will range between KMaxLocalDrives
		and KMaxDrives. 
		KErrInUse if aProxyDriveNumber is in use or if there are not proxy drive numbers left
		KErrArgument if aProxyDriveNumber or aName are invalid 
		Any other system wide error code.
		

*/
EXPORT_C TInt RFs::DoMountProxyDrive(const TIpcArgs& ipcArgs)
	{
	return SendReceive(EFsMountProxyDrive, ipcArgs);
	}


EXPORT_C TInt RFs::DismountProxyDrive(const TUint aProxyDriveNumber)
/**
Dismounts a proxy drive.

@param aProxyDriveNumber drive number that will be used to mount the proxy drive
@param aName name of the proxy drive extension
@param anInfo initialisation information to be passed to the proxy drive extension to initialise the drive

@return If succesful the internal drive number used to represent the drive (equivalent to a local drive 
		number for normal drives) This number is obtained dynmically. The number will range between KMaxLocalDrives
		and KMaxDrives. 
		KErrInUse if aProxyDriveNumber is in use or if there are not proxy drive numbers left
		KErrArgument if aProxyDriveNumber or aName are invalid 
		Any other system wide error code.
		

*/
	{
	return SendReceive(EFsDismountProxyDrive,TIpcArgs(aProxyDriveNumber));
	}

EFSRV_EXPORT_C void RFs::Close()
	{
	TRACE1(UTF::EBorder, UTraceModuleEfsrv::EFsClose, MODULEUID, Handle());
	RFTRACE_CLOSE;

	RSessionBase::Close();

	TRACE0(UTF::EBorder, UTraceModuleEfsrv::EFsCloseReturn, MODULEUID);
	}


