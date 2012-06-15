// Copyright (c) 1997-2009 Nokia Corporation and/or its subsidiary(-ies).
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
// e32test\locale\t_msgtxt.cpp
// Tests the TLocaleMessageText class
// 
//

#include <e32test.h>
#include <e32svr.h>

LOCAL_D	TBuf<KMaxLocaleMessageText>* localeBufs[ELocaleMessages_LastMsg];

RTest test(_L("T_MSGTXT"));

LOCAL_C void DisplayMessages()
//
// Display the locale messages
//
	{

	RNotifier notifier;
	TInt r=notifier.Connect();
	test(r==KErrNone);
	TRequestStatus status;
	TInt buttonVal=0;

	test.Next(_L("Test File Server Error Dialogs"));
	TLocaleMessageText msgTxt;
	msgTxt.Set(EFileServer_Button1);
	TBuf<KMaxLocaleMessageText> button1(msgTxt);
	msgTxt.Set(EFileServer_Button2);
	TBuf<KMaxLocaleMessageText> button2(msgTxt);

	test.Next(_L("DIALOG1: Displayed if a disk is removed during a write"));
	User::After(300000);
	test.Printf(_L("***Press SHIFT+R or SHIFT+S to confim dialogue***\n"));
	TInt count=2;
	msgTxt.Set((TLocaleMessage)count++);
	TBuf<KMaxLocaleMessageText> line1=msgTxt;
	msgTxt.Set((TLocaleMessage)count++);
	TBuf<KMaxLocaleMessageText> line2=msgTxt;
	notifier.Notify(line1,line2,button1,button2,buttonVal,status);
	User::WaitForRequest(status);
	User::After(300000);

	test.Next(_L("DIALOG2: Write failed due to low power"));
	User::After(300000);
	msgTxt.Set((TLocaleMessage)count++);
	line1=msgTxt;
	msgTxt.Set((TLocaleMessage)count++);
	line2=msgTxt;
	notifier.Notify(line1,line2,button1,button2,buttonVal,status);
	User::WaitForRequest(status);
	User::After(300000);

	test.Next(_L("DIALOG3: General error message - disk write failed"));
	User::After(300000);
	msgTxt.Set((TLocaleMessage)count++);
	line1=msgTxt;
	msgTxt.Set((TLocaleMessage)count++);
	line2=msgTxt;
	notifier.Notify(line1,line2,button1,button2,buttonVal,status);
	User::WaitForRequest(status);
	User::After(300000);
	test.Printf(_L("***End***\n\n"));

	msgTxt.Set((TLocaleMessage)count++);
	test.Printf(_L("ALARMNAME: 'Chimes' - %S\n"),&msgTxt);
	msgTxt.Set((TLocaleMessage)count++);
	test.Printf(_L("ALARMNAME: 'Rings' - %S\n"),&msgTxt);
	msgTxt.Set((TLocaleMessage)count++);
	test.Printf(_L("ALARMNAME: 'Signal' - %S\n"),&msgTxt);
	test.Printf(_L("***Press any key***\n\n"));
	test.Getch();

	msgTxt.Set((TLocaleMessage)count++);
	test.Printf(_L("DISKNAME: 'Internal' - %S\n"),&msgTxt);
	msgTxt.Set((TLocaleMessage)count++);
	test.Printf(_L("DISKNAME: 'External1' - %S\n"),&msgTxt);
	msgTxt.Set((TLocaleMessage)count++);
	test.Printf(_L("DISKNAME: 'External2' - %S\n"),&msgTxt);
	msgTxt.Set((TLocaleMessage)count++);
	test.Printf(_L("DISKNAME: 'External3' - %S\n"),&msgTxt);
	msgTxt.Set((TLocaleMessage)count++);
	test.Printf(_L("DISKNAME: 'External4' - %S\n"),&msgTxt);
	msgTxt.Set((TLocaleMessage)count++);
	test.Printf(_L("DISKNAME: 'External5' - %S\n"),&msgTxt);
	msgTxt.Set((TLocaleMessage)count++);
	test.Printf(_L("DISKNAME: 'External6' - %S\n"),&msgTxt);
	msgTxt.Set((TLocaleMessage)count++);
	test.Printf(_L("DISKNAME: 'External7' - %S\n"),&msgTxt);
	msgTxt.Set((TLocaleMessage)count++);
	test.Printf(_L("DISKNAME: 'External8' - %S\n"),&msgTxt);
	test.Printf(_L("***Press any key***\n\n"));
	test.Getch();
	
	msgTxt.Set((TLocaleMessage)count++);
	test.Printf(_L("SOCKETNAME0: - %S\n"),&msgTxt);
	msgTxt.Set((TLocaleMessage)count++);
	test.Printf(_L("SOCKETNAME1: - %S\n"),&msgTxt);
	msgTxt.Set((TLocaleMessage)count++);
	test.Printf(_L("SOCKETNAME2: - %S\n"),&msgTxt);
	msgTxt.Set((TLocaleMessage)count++);
	test.Printf(_L("SOCKETNAME3: - %S\n"),&msgTxt);
	notifier.Close();
	}

TInt E32Main()
//
// Main
//
	{

	test.Title();
//	TBuf<KMaxLocaleMessageText>* localeBufs[ELocaleMessages_LastMsg];    {Too big for local decleration}
																	   															
	test.Start(_L("Constructor"));
	TLocaleMessageText msgTxt;
	msgTxt.Set((TLocaleMessage)4);
	TInt count=0;
	for (count=0;count<ELocaleMessages_LastMsg;count++)
		{
		TLocaleMessageText msgTxt((TLocaleMessage)count);
		test.Printf(_L("Message %d = \"%S\"\n"),count,&msgTxt);
		localeBufs[count]=new TBuf<KMaxLocaleMessageText>(msgTxt);
		test(localeBufs[count]!=NULL);
		}

	test.Next(_L("Set"));
	for (count=0;count<ELocaleMessages_LastMsg;count++)
		{
		TLocaleMessageText msgTxt((TLocaleMessage)count);
		test((*localeBufs[count])==msgTxt);
		}

	for (count=0;count<ELocaleMessages_LastMsg;count++)
		{
		delete localeBufs[count];
		}

	test.Next(_L("Illegal values"));
	TLocaleMessageText error((TLocaleMessage)0x1000000);
	test(error.Length()==0);
	error.SetLength(20);
	error.Set((TLocaleMessage)0x1000000);
	test(error.Length()==0);
	TLocaleMessageText lastMsg(ELocaleMessages_LastMsg);
	test(lastMsg.Length()==0);
	lastMsg.SetLength(20);
	lastMsg.Set(ELocaleMessages_LastMsg);
	test(lastMsg.Length()==0);

	DisplayMessages();

	test.End();
	return KErrNone;
	}
