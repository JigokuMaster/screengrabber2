/*
* Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies). 
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



// INCLUDE FILES
#include    "SGApp.h"
#include    "SGDocument.h"

#include <eikstart.h>
#include <bautils.h>
#include <coemain.h>
#include <s32file.h>


_LIT(KLangSetFileName, "screengrabber2_lang.ini");
_LIT(KEngResFileNameFormat, "%S:\\Resource\\Apps\\ScreenGrabber2.rsc");
_LIT(KResFileNameFormat0, "%S:\\Resource\\Apps\\ScreenGrabber2.r0%d");
_LIT(KResFileNameFormat, "%S:\\Resource\\Apps\\ScreenGrabber2.r%d");

static void GetPrivateFilePathL(RFs& aRfs, TFileName &aFilePath, const TDesC &aFileName)
{

    User::LeaveIfError(aRfs.PrivatePath(aFilePath));
    // insert the "drive:" into the private path.
    aFilePath.Insert(0, RProcess().FileName().Left(2));
    aFilePath.Append(aFileName);
}


// ================= MEMBER FUNCTIONS =======================

// ---------------------------------------------------------
// CScreenGrabberApp::AppDllUid()
// Returns application UID
// ---------------------------------------------------------
//
TUid CScreenGrabberApp::AppDllUid() const
    {
    return KUidScreenGrabber;
    }


// ---------------------------------------------------------
// TFileName CScreenGrabberApp::ResourceFileName() const
// 
// ---------------------------------------------------------
//

TFileName CScreenGrabberApp::ResourceFileName() const
{
    TFileName res;
    TLanguage systemLang = User::Language();
    iCurrentLanguage = systemLang;
    TBool ok = ReadLanguageL(res);
    if (ok && BaflUtils::FileExists(CCoeEnv::Static()->FsSession(), res)) return res;
    else iCurrentLanguage = User::Language();
    return CAknApplication::ResourceFileName(); 
}


TBool CScreenGrabberApp::ReadLanguageL(TFileName &aFileName) const
{
    TFileName storePath;
    RFs& fs = iCoeEnv->FsSession();
    GetPrivateFilePathL(fs, storePath, KLangSetFileName);
    if (BaflUtils::FileExists(fs, storePath))
    {

	RDictionaryReadStream in;

        CDictionaryFileStore* settingsStore = CDictionaryFileStore::OpenLC(fs, storePath, KUidScreenGrabber);
        in.OpenLC(*settingsStore, TUid::Uid(0x1));
        
	TRAPD(err, iCurrentLanguage = (TLanguage)in.ReadUint32L());
        CleanupStack::PopAndDestroy(2); // in, settingsStore 
	if (err == KErrNone)
	{
	    TPtrC driveChar = AppFullName().Left(1);
	    if (iCurrentLanguage > 9) aFileName.Format(KResFileNameFormat, &driveChar, iCurrentLanguage);
	    else 
	    {
		if (iCurrentLanguage == ELangEnglish) aFileName.Format(KEngResFileNameFormat, &driveChar);
		else aFileName.Format(KResFileNameFormat0, &driveChar, iCurrentLanguage);
	    }
	    return ETrue;
	}
    }
    
    return EFalse;

}

TBool CScreenGrabberApp::SaveLanguageL(const TLanguage aLanguage)
{
    if (iCurrentLanguage == aLanguage) return EFalse;

    RFs& fs = iCoeEnv->FsSession();
    TFileName res;

    TPtrC driveChar = AppFullName().Left(1);
    if (aLanguage > 9) res.Format(KResFileNameFormat, &driveChar, aLanguage);

    else{
	if (aLanguage == ELangEnglish) res.Format(KEngResFileNameFormat, &driveChar);
	else res.Format(KResFileNameFormat0, &driveChar, aLanguage);
    }

    if (!BaflUtils::FileExists(CCoeEnv::Static()->FsSession(), res)) return EFalse;

    TFileName storePath;
    GetPrivateFilePathL(fs, storePath, KLangSetFileName);

    // delete existing store to make sure that it is clean and not eg corrupted
    if (BaflUtils::FileExists(fs, storePath))
    {
	fs.Delete(storePath);
    }

    RDictionaryWriteStream out;

    CDictionaryFileStore* settingsStore = CDictionaryFileStore::OpenLC(fs, storePath, KUidScreenGrabber);
    out.AssignLC(*settingsStore, TUid::Uid(0x1));
    out.WriteUint32L(aLanguage);
    out.CommitL(); 	
    settingsStore->CommitL();
    CleanupStack::PopAndDestroy(2); // out, settingsStore 
    iCurrentLanguage = aLanguage;
    return ETrue;
}


// ---------------------------------------------------------
// CDictionaryStore* CScreenGrabberApp::OpenIniFileLC(RFs& aFs) const
// overrides CAknApplication::OpenIniFileLC to enable INI file support
// ---------------------------------------------------------
//
CDictionaryStore* CScreenGrabberApp::OpenIniFileLC(RFs& aFs) const
{
    return CEikApplication::OpenIniFileLC(aFs);
}
   
// ---------------------------------------------------------
// CScreenGrabberApp::CreateDocumentL()
// Creates CScreenGrabberDocument object
// ---------------------------------------------------------
//
CApaDocument* CScreenGrabberApp::CreateDocumentL()
    {
    return CScreenGrabberDocument::NewL( *this );
    }

// ================= OTHER EXPORTED FUNCTIONS ==============

LOCAL_C CApaApplication* NewApplication()
    {
    return new CScreenGrabberApp;
    }


GLDEF_C TInt E32Main()
    {
    return EikStart::RunApplication(NewApplication);
    }
   

// End of File  

