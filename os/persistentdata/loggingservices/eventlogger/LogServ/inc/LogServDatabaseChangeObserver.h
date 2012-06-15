// Copyright (c) 2002-2009 Nokia Corporation and/or its subsidiary(-ies).
// All rights reserved.
// This component and the accompanying materials are made available
// under the terms of "Eclipse Public License v1.0"
// which accompanies this distribution, and is available
// at the URL "http://www.eclipse.org/legal/epl-v10.html".
//
// Initial Contributors:
// Nokia Corporation - initial contribution.
//
// Contributors:
//
// Description:
//

#ifndef __LOGSERVDATABASECHANGEOBSERVER_H__
#define __LOGSERVDATABASECHANGEOBSERVER_H__

class CLogChangeDefinition;
class TLogServDatabaseChangeDefinition;

/////////////////////////////////////////////////////////////////////////////////////////
// -----> MLogServDatabaseChangeObserver (header)
/////////////////////////////////////////////////////////////////////////////////////////
/**
@internalComponent
*/
class MLogServDatabaseChangeObserver
	{
/////////////////////////////////////////////////////////////////////////////////////////
public: // FROM MLogServDatabaseChangeObserver
/////////////////////////////////////////////////////////////////////////////////////////

	/**
	 * Notify the change interface about some kind of change in the database
	 */
	virtual void DCOHandleChangeEventsL(const CLogChangeDefinition& aChanges);

	/**
	 * Handle a non-event specific change, e.g. the log has been cleared.
	 */
	virtual void DCOHandleGlobalChangeEventL(const TLogServDatabaseChangeDefinition& aChange);

	};


#endif