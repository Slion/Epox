/*
* Copyright (c) 2005-2009 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
* This component and the accompanying materials are made available
* under the terms of "Eclipse Public License v1.0"
* which accompanies this distribution, and is available
* at the URL "http://www.eclipse.org/legal/epl-v10.html".
*
* Initial Contributors:
* Nokia Corporation - initial contribution.
*
* Contributors:
*
* Description:
*
*/





#include "sysif.h"
#include "ipcclient.h"
#include "sysreent.h"
#include "sysusrinclude.h"


extern "C" {

// -----------------------------------------------------------------------------
// Funcation name: mkfifo
// Description: Provides a FIFO special file (a named pipe).
// Returns: 0     : On success
//          -1    : On error
// In case of error, errno value set to 
//          EACCES: One  of  the  directories in pathname did not allow search 
//                  (exe-cute) permission.
//          EEXIST: pathname already exists.
//          ENAMETOOLONG: Either the total length of pathname is greater 
//                        than PATH_MAX, or an individual  file  name  component  
//                        has a length greater than NAME_MAX.  In the GNU system, 
//                        there is no imposed limit on over-all  file name length,
//                        but some file systems may place limits on the length of a component.
//          ENOENT: A directory component in pathname does not exist or  is  a  dan-gling
//                  symbolic link.
//          ENOSPC: The directory or filesystem has no room for the new file.
//          ENOTDIR: A  component  used as a directory in pathname is not, in fact, a
//                   directory.
//          EROFS:  pathname refers to a read-only filesystem.
// To be removed
//          EFAULT: Pathname is not valid.
//          EMFILE: Too many file descriptors are in use by the process.
//          ENFILE: System limit on the total number of open files has been 
//                  reached.
// -----------------------------------------------------------------------------
//

EXPORT_C int _mkfifo_r(int *aErrno, const wchar_t *_widename, mode_t mode)
    {
	return Backend()->mkfifo(_widename, mode, *aErrno);
    }


// -----------------------------------------------------------------------------
// Funcation name: pipe
// Description: It provides mechanism to flow data in one direction.
// Returns: 0     : On success
//          -1    : On error
// In case of error, errno value set to 
//          EFAULT: filedes is not valid.
//          EMFILE: Too many file descriptors are in use by the process.
//          ENFILE: System limit on the total number of open files has been 
//                  reached.
// -----------------------------------------------------------------------------
//

EXPORT_C int _pipe_r(int *aErrno, int fildes[2])
    {
    return Backend()->pipe(fildes, *aErrno);
    }

// -----------------------------------------------------------------------------
// Funcation name: semget
// Description: Get semaphore identifier using the IPC key generated by ftok.
// Returns: None zero number (semaphore identifier): On success
//          -1              : On error.
// In case of error, errno value set to 
//          EACCES - Semaphore exists but access denied.
//          EEXIST - A semaphore identifier exists for the argument key but 
//                   ((semflg&IPC_CREAT)&&(semflg&IPC_EXCL)) is non-zero.
//          EINVAL - The value of nsems is either less than or equal to 0 or 
//                   greater than the system-imposed limit, or a semaphore 
//                   identifier exists for the argument key, but the number of 
//                   semaphores in the set associated with it is less than nsems 
//                   and nsems is not equal to 0.
//          ENOENT - A semaphore identifier does not exist for the argument key 
//                   and (semflg&IPC_CREAT) is equal to 0.
//          ENOSPC - A semaphore identifier is to be created but the system-imposed 
//                   limit on the maximum number of allowed semaphores system-wide 
//                   would be exceeded.
// -----------------------------------------------------------------------------
//


EXPORT_C int _semget_r(int *aErrno, key_t akey, int nsems, int asemflgs)
    {
	return Backend()->iIpcS.semget(akey, nsems, asemflgs, *aErrno); 
    }

// -----------------------------------------------------------------------------
// Funcation name: semop
// Description: Perform atomically a user-defined array of semaphore operations 
//              on the set of semaphores associated with the semaphore identifier 
//              specified by the argument semid.
// Returns: 0  : On success
//          -1 : On error.
// In case of error, errno value set to 
//          E2BIG - The value of nsops is greater than the system-imposed maximum.
//          EACCES - Operation permission is denied to the calling process, see IPC.
//          EAGAIN - The operation would result in suspension of the calling process 
//                   but (sem_flg&IPC_NOWAIT) is non-zero.
//          EFBIG - The value of sem_num is less than 0 or greater than or equal to 
//                   the number of semaphores in the set associated with semid.
//          EIDRM - The semaphore identifier semid is removed from the system.
//          EINTR - The semop() function was interrupted by a signal.
//          EINVAL - The value of semid is not a valid semaphore identifier, or the 
//                   number of individual semaphores for which the calling process 
//                   requests a SEM_UNDO would exceed the system-imposed limit.
//          ENOSPC - The limit on the number of individual processes requesting a 
//                   SEM_UNDO would be exceeded.
//          ERANGE - An operation would cause a semval to overflow the system-imposed 
//                   limit, or an operation would cause a semadj value to overflow the 
//                   system-imposed limit
// -----------------------------------------------------------------------------
//
EXPORT_C int _semop_r(int *aErrno, int semid, struct sembuf *sops, unsigned nsops)
    {
	return Backend()->iIpcS.semop(semid, sops, nsops, *aErrno);	
    }

// -----------------------------------------------------------------------------
// Funcation name: semctl
// Description: Provides a variety of semaphore control operations as specified by cmd.
// Returns: 
//          If successful, the value returned by semctl() depends on cmd as follows:
//                 GETVAL - Value of semval. 
//                 GETPID - Value of sempid. 
//                 GETNCNT - Value of semncnt. 
//                 GETZCNT - Value of semzcnt. 
//                 All others, it returns 0. 
//          -1 : On error.
// In case of error, errno value set to 
//          EACCES - Operation permission is denied to the calling process.
//          EINVAL - The value of semid is not a valid semaphore identifier, 
//                   or the value of semnum is less than 0 or greater than or 
//                   equal to sem_nsems, or the value of cmd is not a valid command.
//          EPERM - The argument cmd is equal to IPC_RMID or IPC_SET and the 
//                   effective user ID of the calling process is not equal to 
//                   that of a process with appropriate privileges and it is not 
//                   equal to the value of sem_perm.cuid or sem_perm.uid in the 
//                   data structure associated with semid.
//          ERANGE - The argument cmd is equal to SETVAL or SETALL and the value 
//                   to which semval is to be set is greater than the system-imposed maximum
// -----------------------------------------------------------------------------
//
EXPORT_C int _semctl_r(int *aErrno, int semid, int semnum, int cmd, ...)
    {
    
	va_list ap;
	union semun semun;
	union semun *semun_ptr;

	
	if (cmd == IPC_SET || cmd == IPC_STAT || cmd == GETALL
	    || cmd == SETVAL || cmd == SETALL) 
	    {
		va_start(ap, cmd);
		semun = va_arg(ap, union semun);
		semun_ptr = &semun;
		va_end(ap);
	    } 
	else 
	    {
		semun_ptr = NULL;
	    }

	return Backend()->iIpcS.semctl(semid, semnum, cmd, semun_ptr, *aErrno);
    }

// -----------------------------------------------------------------------------
// Funcation name: shmget
// Description: Get shared memory identifier using the IPC key generated by ftok.
// Returns: None zero number (shared memory identifier): On success
//          -1              : On error.
// In case of error, errno value set to 
//          EACCES - Operation permission is denied to the calling process, see IPC.
//          EINVAL - The value of semid is not a valid semaphore identifier, or the 
//                   value of semnum is less than 0 or greater than or equal to 
//                   sem_nsems, or the value of cmd is not a valid command.
//          EPERM -  The argument cmd is equal to IPC_RMID or IPC_SET and the effective 
//                   user ID of the calling process is not equal to that of a process 
//                   with appropriate privileges and it is not equal to the value of 
//                   sem_perm.cuid or sem_perm.uid in the data structure associated 
//                   with semid.
//          ERANGE - The argument cmd is equal to SETVAL or SETALL and the value to 
//                   which semval is to be set is greater than the system-imposed 
//                   maximum.
// -----------------------------------------------------------------------------
//

EXPORT_C int _shmget_r(int *aErrno, key_t key, int size, int shmflg)
    {
    return Backend()->iIpcS.shmget(key, size, shmflg, *aErrno);
    }

// -----------------------------------------------------------------------------
// Funcation name: shmat
// Description: Attaches the shared memory segment associated with the shared 
//              memory identifier specified by shmid to the address space of 
//              the calling process.
// Returns: Segment's start address: On success
//          -1   : On error.
// In case of error, errno value set to 
//          EACCES - Operation permission is denied to the calling process.
//          EINVAL - The value of shmid is not a valid shared memory identifier; 
//                   the shmaddr is not a null pointer and the value of 
//                   (shmaddr-((ptrdiff_t)shmaddr%SHMLBA)) is an illegal 
//                   address for attaching shared memory; or the shmaddr is not 
//                   a null pointer, (shmflg&SHM_RND) is 0 and the value of 
//                   shmaddr is an illegal address for attaching shared memory. 
//          EMFILE - The number of shared memory segments attached to the calling 
//                   process would exceed the system-imposed limit.
//          ENOMEM - The available data space is not large enough to accommodate 
//                   the shared memory segment.
// -----------------------------------------------------------------------------
//
EXPORT_C void* _shmat_r(int *aErrno, int shmid, const void *shmaddr, int shmflg)
    {
    return Backend()->iIpcS.shmat(shmid, shmaddr, shmflg, *aErrno);
    }

// -----------------------------------------------------------------------------
// Funcation name: shmdt
// Description: Detaches the shared memory segment located at the address 
//              specified by shmaddr from the address space of the calling process.
// Returns: 0    : On success.
//          -1   : On error.
// In case of error, errno value set to 
//          EINVAL - The value of shmaddr is not the data segment start 
//                   address of a shared memory segment.
// -----------------------------------------------------------------------------
//
EXPORT_C int _shmdt_r(int *aErrno, const void *shmaddr)
    {
    return Backend()->iIpcS.shmdt(shmaddr, *aErrno);
    }

// -----------------------------------------------------------------------------
// Funcation name: shmctl
// Description: Provides a variety of shared memory control operations as 
//              specified by cmd.
// Returns: 0  : On success
//          -1 : On error.
// In case of error, errno value set to 
//          EACCES - The argument cmd is equal to IPC_STAT and the calling 
//                   process does not have read permission.
//          EINVAL- The value of shmid is not a valid shared memory 
//                   identifier, or the value of cmd is not a valid command. 
//          EPERM - The argument cmd is equal to IPC_RMID or IPC_SET and the 
//                  effective user ID of the calling process is not equal to 
//                  that of a process with appropriate privileges and it is 
//                  not equal to the value of shm_perm.cuid or shm_perm.uid 
//                  in the data structure associated with shmid. 
//          EOVERFLO - The cmd argument is IPC_STAT and the gid or uid value 
//                     is too large to be stored in the structure pointed to 
//                     by the buf argument
// -----------------------------------------------------------------------------
//
EXPORT_C int _shmctl_r(int *aErrno, int shmid, int cmd, struct shmid_ds *buf)
    {
    return Backend()->iIpcS.shmctl(shmid, cmd, buf, *aErrno);
    }


//the message queue functions
EXPORT_C int _msgget_r(key_t aKey, int aMsgFlag, int *aErrno)
	{
	return Backend()->iIpcS.msgget(aKey, aMsgFlag, *aErrno);
	}

EXPORT_C int _msgsnd_r(int msqid, const void *msgp, size_t msgsz, int msgflg, int *aErrno)
    {
    return Backend()->iIpcS.msgsnd(msqid, msgp, msgsz, msgflg, *aErrno);
    }
    
EXPORT_C int _msgrcv_r(int msqid, void* msgp, size_t msgsz, long msgtyp, int msgflg, int *aErrno)
    {
    return Backend()->iIpcS.msgrcv(msqid, msgp, msgsz, msgtyp, msgflg, *aErrno);
    }
    
EXPORT_C int _msgctl_r(int msqid, int cmd, struct msqid_ds *buf, int *aErrno)
    {
    return Backend()->iIpcS.msgctl(msqid, cmd, buf, *aErrno);
    }

} //end extern "C"
