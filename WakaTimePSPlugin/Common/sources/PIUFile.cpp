// ADOBE SYSTEMS INCORPORATED
// Copyright  1993 - 2002 Adobe Systems Incorporated
// All Rights Reserved
//
// NOTICE:  Adobe permits you to use, modify, and distribute this 
// file in accordance with the terms of the Adobe license agreement
// accompanying it.  If you have received this file from a source
// other than Adobe, then your use, modification, or distribution
// of it requires the prior written permission of Adobe.
//-------------------------------------------------------------------
//-------------------------------------------------------------------------------
//
//	File:
//		PIUFile.cpp
//
//	Description:
//		File utilities for alias values acquired from the automation system.
//
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
//	Includes
//-------------------------------------------------------------------------------
#include "PIUFile.h"


void AliasToFullPath(Handle aliasValue, char* fullPath, int32 maxPathLength)
{
	if (fullPath != NULL)
	{
		*fullPath = 0;
#ifdef __PIMac__
		OSErr err;
		Boolean wasChanged = false;
		FSRef fileRef;

		err = FSResolveAlias(nil, (AliasHandle)aliasValue, &fileRef, &wasChanged);
		if (err != noErr) return;

		err = FSRefMakePath(&fileRef, (unsigned char*)fullPath, maxPathLength);
			
#elif defined(__PIWin__)
		int32 length = sPSHandle->GetSize(aliasValue);

		if (length > maxPathLength - 1) return;
			
		Boolean oldLock = FALSE;
		Ptr pointer = NULL;
			
		sPSHandle->SetLock(aliasValue, true, &pointer, &oldLock);
			
		if (pointer != NULL)
		{
			strncpy(fullPath, pointer, maxPathLength);
			fullPath[maxPathLength-1] = '\0';
			sPSHandle->SetLock(aliasValue, oldLock, &pointer, &oldLock);
		}
			
		
	#endif
		}
}

void FullPathToAlias(char* fullPath, Handle& aliasValue)
{
	aliasValue = NULL;
#if __PIMac__
	AliasHandle aHandle = NULL;
	// this could error with fnfErr, file not found error, ignore
	(void) sPSAlias->MacNewAliasFromCString(fullPath, &aHandle);
	aliasValue = (Handle)aHandle;
#else
	Boolean oldLock = FALSE;
	Ptr address = NULL;
	size_t pathLength = strlen(fullPath)+1;
    aliasValue = sPSHandle->New((int32)pathLength);
    if (aliasValue != NULL)
	{
		sPSHandle->SetLock(aliasValue, true, &address, &oldLock);
		if (address != NULL)
		{
			strncpy(address, fullPath, pathLength);
			address[pathLength-1] = '\0';
			sPSHandle->SetLock(aliasValue, false, &address, &oldLock);
		}
    }
#endif
}


int32 GetFullPathToDesktop(char * fullPath, int32 maxPathLength)
{
	int32 error = 0;
	if (fullPath == NULL) return kSPBadParameterError;
	
	#if __PIMac__

		FSRef fsRef;
	
		error = FSFindFolder(kOnSystemDisk, 
			 				 kDesktopFolderType, 
							 kDontCreateFolder, 
							 &fsRef);
		if (error) return error;

		error = FSRefMakePath(&fsRef, (unsigned char*)fullPath, maxPathLength);

		strncat(fullPath, "/", maxPathLength);
		fullPath[maxPathLength-1]= '\0';

	#else
		
		HRESULT hr = SHGetFolderPathA( NULL, CSIDL_DESKTOPDIRECTORY, NULL, 0, fullPath );
		if (FAILED(hr))
		{
			strncpy(fullPath, "C:\\", maxPathLength);
			fullPath[maxPathLength-1]= '\0';
		}
		else
		{
			strncat(fullPath, "\\", maxPathLength);
			fullPath[maxPathLength-1]= '\0';
		}

	#endif
	
	return error;
}
// end PIUFile.cpp