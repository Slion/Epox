// Copyright (c) 2001-2009 Nokia Corporation and/or its subsidiary(-ies).
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

// System includes
#include <uri8.h>
#include <uri16.h>
#include <uriutils.h>
#include <uriutilscommon.h>


//User includes
#include "TUriParserInternal.h"
#include "UriUtilsInternal.h"
#include "TUriCInternal.h"
#include "GenericUriParser.h"
#include "SipUriParser.h"



// Constants
//

_LIT8(KSIP, "Sip");
_LIT8(KSIPS, "Sips");


//
//
// Implementation of TUriParser8
//
//

/**
	Constructor.
	
	@since			6.0
 */
EXPORT_C TUriParser8::TUriParser8()
: TUriC8()
	{
	}
	
/**
	Parses the descriptor aUri into uri components.
	
	@since			6.0
	@param			aUri A reference to a descriptor pointer to be parsed.
	@return			KErrNone if the descriptor has been parsed into uri components.
	KUriUtilsParserErrInvalidUri if the descriptor is an invalid uri.
	KErrNoMemory if out of memory
	@post			The object references the input descriptor.
 */
EXPORT_C TInt TUriParser8::Parse(const TDesC8& aUri)
	{
	// Reset the Uri information and then set the Uri
	if( iUriDes.Length() )
		{
		Reset();
		}
	iUriDes.Set(aUri);

	// Check uri is valid
	if( iUriDes.Length() && iUriDes[0]==KSchemeDelimiter )
		{
		return KUriUtilsErrInvalidUri;
		}
	
	TPtrC8 schemeComponent;
	RetrieveScheme(iUriDes,schemeComponent);
	CGenericUriParser* UriHandler = NULL;
	TInt err=KErrNone;
	if(schemeComponent.CompareF(KSIP()) == 0 || schemeComponent.CompareF(KSIPS()) == 0 )
		{
	    TRAP(err,UriHandler = CSIPUriParser::NewL());
		}
	else
		{
	    TRAP(err,UriHandler = CGenericUriParser::NewL());
 		}
	if(UriHandler)
		{
		UriHandler->DoParseUri(iUriDes, iComponent);
		delete UriHandler;
		}
       
		return err;      

	}

/**
	Parses the descriptor aUri into uri components.
	
	@param			aUri A reference to a descriptor pointer of an Uri.
	@param			aScheme A reference to a descriptor pointer for retieved 
					scheme component.
 */
void TUriParser8::RetrieveScheme(const TPtrC8& aUri, TPtrC8& aScheme)
	{
	TInt schemePos = aUri.Locate(KSchemeDelimiter);
	if(schemePos != KErrNotFound)
		{
		// Got a scheme - store information
		aScheme.Set(aUri.Left(schemePos));
		}
	}

//
//
// Implementation of TUriParser16
//
//

/**
	Constructor.
	
	@since			6.0
	@deprecated Deprecated in 9.1
 */
EXPORT_C TUriParser16::TUriParser16()
: TUriC16()
	{
	}

/**
	Parses the descriptor aUri into uri components.
	
	@since			6.0
	@deprecated Deprecated in 9.1
	@param			aUri A reference to a descriptor pointer to be parsed.
	@return			KErrNone if the descriptor has been parsed into uri components.
	EUriParserErrInvalidUri if the descriptor is an invalid uri.
	@post			The object references the input descriptor.
 */
EXPORT_C TInt TUriParser16::Parse(const TDesC16& aUri)
	{
	// Reset the Uri information and then set the Uri
	if( iUriDes.Length() )
		Reset();
	iUriDes.Set(aUri);

	// Check uri is valid
	if( iUriDes.Length() && iUriDes[0]==KSchemeDelimiter )
		return KUriUtilsErrInvalidUri;

	// Parse the uri.
	DoParseUri(iUriDes, iComponent);
	return KErrNone;
	}

//
//
// Implementation of templated LOCAL functions
//
//

/**
	Templated function that parses a descriptor into the components of a uri.
						
	@since			6.0
	@param			aUri		The descriptor with the data to parse.
	@param			aComponent	The output array of descriptors of each uri component.
	@pre 			Each descriptor pointer in aComponent has had the pointer to its
					associated descriptor buffer set to NULL.
	@post			The descriptor pointers in aComponent are updated to refer to the 
					appropriate sections of aUri that represent the components of a uri.
*/
template<class TPtrCType>
void DoParseUri(const TPtrCType& aUri, TPtrCType aComponent[])
	{
	// Parse the components
	TPtrCType uri = aUri;
	TInt consumed = 0;
	TPtrCType& scheme = aComponent[EUriScheme];
	if( (consumed = ParseScheme(uri, scheme)) > 0 )
		{
		uri.Set(uri.Mid(consumed));
		}
	if( (consumed = ParseAuthority(uri, aComponent[EUriUserinfo], 
		 aComponent[EUriHost], aComponent[EUriPort], IsNetworkScheme(scheme))) > 0 )
		{
		uri.Set(uri.Mid(consumed));
		}
	if( (consumed = ParsePath(uri, aComponent[EUriPath])) > 0 )
		{
		uri.Set(uri.Mid(consumed));
		}
	if( (consumed = ParseQuery(uri, aComponent[EUriQuery])) > 0 )
		{
		uri.Set(uri.Mid(consumed));
		}
	if( (consumed = ParseFragment(uri, aComponent[EUriFragment])) > 0 )
		{
		uri.Set(uri.Mid(consumed));
		}
	}

/**
	Templated function to parse a descriptor for a scheme component. If a scheme is found 
	then the output argument aScheme is set to refer to it.
						
	@since			6.0
	@param			aUri	The descriptor containing the uri to be parsed for 
	a scheme.
	@param			aScheme	The output descriptor to refer to the scheme
	@return			The number of characters consumed in parsing the scheme.
	@pre 			The output descriptor has been initialized so that the pointer
					to the associated descriptor buffer is NULL. The input descriptor 
					is set to the start of the uri.
	@post			If a scheme component exists then the output descriptor refers
					to it, otherwise the output descriptor is left unchanged.
*/
template<class TPtrCType>
TInt ParseScheme(const TPtrCType& aUri, TPtrCType& aScheme)
	{
	// Get the descriptor and look for scheme delimiter
	TInt consumed =0;
	TInt endSchemePos = FindFirstUriDelimiter(aUri, ESchemeDelimiterSearch);

	if( endSchemePos != KErrNotFound )
		{
		// Got a scheme - store information
		aScheme.Set(aUri.Left(endSchemePos));

		// Set consumed amount move past scheme delimiter
		consumed = endSchemePos + 1;
		}
	return consumed;
	}
	
/**
	Templated function to parse a descriptor for an authority component. If an authority is 
	found then the output arguments aUserinfo, aHost and aPort are set to refer to those parts 
	of the authority component. If an authority component exists then the host part exists. The 
	userinfo and port parts are optional.
						
	@since			6.0
	@param			aUri		The descriptor containing the uri to be parsed 
					for an authority.
	@param			aUserinfo	The output descriptor for the userinfo component.
	@param			aHost		The output descriptor for the host component.
	@param			aPort		The output descriptor for the port component.
	@param			aUseNetworkDelimiter Whether the scheme uses the network delimeter '//'
	@return			The number of characters consumed in parsing the authority.
	@pre 			The output descriptors have been initialized so that the pointer
					to the their associated descriptor buffers is NULL. The input descriptor 
					is set to the start of the start of the authority component.
	
	@post			If an authority component exists then the output descriptors refer
					to the userinfo part (if exists), the host part and the port part (if exists), 
					otherwise the output descriptors are left unchanged.
*/

template<class TPtrCType> 
TInt ParseAuthority(const TPtrCType& aUri, TPtrCType& aUserinfo, TPtrCType& aHost, TPtrCType& aPort, TBool aUseNetworkDelimiter)
	{
	// Get uri descriptor and see if authority exists - if aUseNetworkDelimiter is true it has to start with '//' 
	TInt consumed =0;
	const TInt prefixLength = aUseNetworkDelimiter ? KUriNetworkAuthorityDelimiterLength : 0;
  	if( !aUseNetworkDelimiter || 
  		(aUri.Length() >= prefixLength && aUri[0] == KSlashDelimiter && aUri[1] == KSlashDelimiter ))
		{
		// There is an authority
		TPtrCType authority = aUri.Mid(prefixLength);

		// Authority delimited by '/', '?', '#' or the end of the string
		TInt authorityEndPos = FindFirstUriDelimiter(authority, EAuthDelimiterSearch);

		// Got authority - parse it for its components
		authority.Set(authority.Left(authorityEndPos));

		// Get the userinfo...
		TInt userinfoEndPos = authority.Locate(KUserinfoDelimiter);
		if( userinfoEndPos != KErrNotFound )
			{
			// Store the information 
			aUserinfo.Set(authority.Left(userinfoEndPos));
			  
			// Move past the userinfo and the delimiter '@'
			authority.Set(authority.Mid(userinfoEndPos + 1));
			}

		// Authority is also delimited by ';' but this is valid in the userinfo so can only be checked now
		TInt semicolonPos = FindFirstUriDelimiter(authority, ESemiColonDelimiterFlag);
		if ( semicolonPos != KErrNotFound )
			{
			authority.Set(authority.Left(semicolonPos));
			authorityEndPos = semicolonPos + userinfoEndPos + 1;
			}
		
		// Set consumed amount to move past authority
		consumed += prefixLength + authorityEndPos;
		
		// Check if this is an IPv6 address	by looking for the opening '['
		TInt startHostIPv6 = authority.Locate(KIPv6UriOpenBrace);

		if (startHostIPv6==KErrNotFound)
			{
			// This isn't an IPv6 address.....

			// Get host...
			TInt hostEndPos = authority.Locate(KPortDelimiter);

			// Host also delimited by the end of the authority
			if( hostEndPos == KErrNotFound )
				hostEndPos = authority.Length();

			// There's always a host, but can be empty - store information
			aHost.Set(authority.Left(hostEndPos));

			// Move past the host
			authority.Set(authority.Mid(hostEndPos));
			}
		else
			{
		
	    // First, move past the opening brace
	        authority.Set(authority.Mid(startHostIPv6 + 1));
	            // auth now = X:X:X]?????
	            
			// This is an IPv6 address, so it MUST have the closing brace too....
			TInt endIPv6Host = authority.Locate(KIPv6UriCloseBrace);

			// Return an error if the closing IPv6 delimiter isn't there.
			if (endIPv6Host==KErrNotFound)
				return KUriUtilsErrInvalidUri;

			// It's an ipv6  address, with an opening and closing brace. So now just extract it
			// auth = [X:X:X]?????


			// Set the host, and need to remove the closing brace
			aHost.Set(authority.Left(endIPv6Host));
			// host = X:X:X

			// Move past the host
			authority.Set(authority.Mid(endIPv6Host + 1 ));
			}
		
		// Get the port...
		TInt portEndPos = authority.Length();
		if( portEndPos )
			{
			// Store the port - remove leading ':'
			aPort.Set(authority.Mid(1, portEndPos - 1));
			}
		}
	return consumed;
	}

/**
	Templated function to parse a descriptor for a path component.There is always a path component.
	The ouput argument aPath is set to the path component found.
						
	@since			6.0
	@param			aUri	The descriptor containing the uri to be parsed for 
	a path.
	@param			aComponent	The output descriptor to refer to the path.
	@return			The number of characters consumed in parsing the path.
	@pre 			The output descriptor has been initialized so that the pointer
					to the associated descriptor buffer is NULL. The input descriptor 
					is set to the start of the path.
					
	@post			The output descriptor refers to the path component.
*/
template<class TPtrCType>
TInt ParsePath(const TPtrCType& aUri, TPtrCType& aComponent)
	{
	// Get descriptor with the path
	TInt consumed =0;

	// Path is delimited by '?'. '#' or the end of the string
	TInt pathEndPos = FindFirstUriDelimiter(aUri, EPathDelimiterSearch);

	// Check for presence of path
	if( pathEndPos != KErrNotFound )
		{
		// Got path - store information
		aComponent.Set(aUri.Left(pathEndPos));
	
		// Set consumed amount to move past path
		consumed = pathEndPos;
		}
	return consumed;
	}

/**
	Templated function to parse a descriptor for a query component.If a query is found then 
	the output argument aQuery is set to refer to it.
						
	@since			6.0
	@param			aUri	The descriptor containing the uri to be parsed for 
	a query.
	@param			aComponent	The output descriptor to refer to the query
	@return			The number of characters consumed in parsing the query.
	@pre 			The output descriptor has been initialized so that the pointer
					to the associated descriptor buffer is NULL. The input descriptor is set to
					the start of the query.
	@post			If a query component exists then the output descriptor refers
					to it, otherwise the output descriptor is left unchanged.
*/
template<class TPtrCType>
TInt ParseQuery(const TPtrCType& aUri, TPtrCType& aComponent)
	{
	// Get descriptor with the query
	TInt consumed =0;

	// Query is delimited by '#' or end of the string 
	TInt queryEndPos = FindFirstUriDelimiter(aUri, EQueryDelimiterSearch);

	// Check for presence of query
	if( queryEndPos )
		{
		// Got query - store information; need to excluded leading '?'
		aComponent.Set(aUri.Mid(1, queryEndPos - 1));
	
		// Set consumed amount to move past query
		consumed = queryEndPos;
		}
	return consumed;
	}

/**
	Templated function to parse a descriptor for a fragment component. If a fragment is found then 
	the output argument aFragment is set to refer to it.
						
	@since			6.0
	@param			aUri		The descriptor containing the uri to be parsed for 
					a fragment.
	@param			aComponent	The output descriptor to refer to the fragment.
	@return			The number of characters consumed in parsing the fragment.
	@pre 			The output descriptor has been initialized so that the pointer
					to the associated descriptor buffer is NULL. The input descriptor is set to
					the start of the fragment.
	@post			If a fragment component exists then the output descriptor refers
					to it, otherwise the output descriptor is left unchanged.
*/
template<class TPtrCType>
TInt ParseFragment(const TPtrCType& aUri, TPtrCType& aComponent)
	{
	// Get descriptor with the fragment
	TInt consumed =0;

	// Fragment is delimited by end of the string
	TInt fragmentEndPos = aUri.Length();

	// Check for presence of fragment
	if( fragmentEndPos )
		{
		// Got fragment - store information; need to excluded leading '#'
		aComponent.Set(aUri.Mid(1, fragmentEndPos - 1));
	
		// Set consumed amount to move past fragment
		consumed = fragmentEndPos;
		}
	return consumed;
	}
	
/**
	Templated function to find the position of the first delimiter in the descriptor specified 
	by the delimiter flags. Note that the end of the descriptor is also a delimiter if there are 
	no others. In the case of the scheme delimiter search, the position returned depends on the 
	position of the colon delimiter with respect to the other delimiters for a scheme.
						
	@since			6.0
	@param			aUri		The descriptor containing the section of a uri to be searched.
	@param			aSearchFlag	The enum specifying the delimiters to search for.
	@return			The position of nearest delimiter to start of the descriptor, where
					zero is the start (left-most) position.
*/
template<class TPtrCType> 
TInt FindFirstUriDelimiter(const TPtrCType& aUri, TDelimiterSearchFlag aSearchFlag)
	{
	// Set ultimate delimiter - string length
	TInt endPos = aUri.Length();
	if( aSearchFlag & EHashDelimiterFlag )
		{
		TInt fragmentPos = aUri.Locate(KFragmentDelimiter);
		if( fragmentPos != KErrNotFound && fragmentPos < endPos )
			endPos = fragmentPos;
		}
	if( aSearchFlag & EQueryDelimiterFlag )
		{
		TInt queryPos = aUri.Locate(KQueryDelimiter);
		if( queryPos != KErrNotFound && queryPos < endPos )
			endPos = queryPos;
		}
	if( aSearchFlag & ESlashDelimiterFlag )
		{
		TInt slashPos = aUri.Locate(KSlashDelimiter);
		if( slashPos != KErrNotFound && slashPos < endPos )
			endPos = slashPos;
		}
	if( aSearchFlag & ESemiColonDelimiterFlag )
		{
		TInt semiColonPos = aUri.Locate(KParamDelimiter);
		if( semiColonPos != KErrNotFound && semiColonPos < endPos )
			endPos = semiColonPos;
		}
	if( aSearchFlag & EColonDelimiterFlag )
		{
		TInt schemePos = aUri.Locate(KSchemeDelimiter);
		if( schemePos != KErrNotFound && schemePos < endPos )
			{
			// There is a scheme
			endPos = schemePos;
			}
		else if( aSearchFlag == ESchemeDelimiterSearch )
			{
			// Ok different if looking for scheme delimiter - no scheme, return KErrNotFound
			endPos = KErrNotFound;
			}
		}
	return endPos;
	}
