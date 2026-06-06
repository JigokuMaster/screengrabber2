// INCLUDE FILES
#include <aknlists.h>
#include <eikclbd.h>
#include <documenthandler.h> // CDocumentHandler
#include <stringloader.h>
#include <bautils.h>
#include <cmessagedata.h>
#include <sendui.h>
#include <ScreenGrabber2.rsg>
#include <AknQueryDialog.h>
#include "SGSavedShotsContainer.h"
#include "SGDocument.h" 
#include "SGModel.h"

// ================= MEMBER FUNCTIONS =======================


CSGSavedShotsContainer::~CSGSavedShotsContainer()
    {
	if (iListBox) delete iListBox;
    }

void CSGSavedShotsContainer::ConstructL(const TRect& aRect)
    {

    CreateWindowL();
    SetRect(aRect);
    iListBox = new ( ELeave) CAknSingleStyleListBox();
    iListBox->ConstructL(this);
    iListBox->SetContainerWindowL(*this);
    iListBox->SetListBoxObserver(this);
    iListBox->CreateScrollBarFrameL(ETrue);
    iListBox->ScrollBarFrame()->SetScrollBarVisibilityL(
	    CEikScrollBarFrame::EOff, CEikScrollBarFrame::EAuto );

    iListBox->ItemDrawer()->ColumnData()->EnableMarqueeL(ETrue);
    iListBox->SetRect(Rect());
    SetItemsL();
    ActivateL();
    }

void CSGSavedShotsContainer::SetItemsL()
    {
    
    CDir* fileList;

    CScreenGrabberModel* SGModel = static_cast<CScreenGrabberDocument*>(reinterpret_cast<CEikAppUi*>(iEikonEnv->AppUi())->Document())->Model();

    User::LeaveIfError(iCoeEnv->FsSession().GetDir( 
            SGModel->SavePath(),
            KEntryAttMaskSupported,
            ESortByName,
            fileList));

    CDesCArray* itemArray = new ( ELeave ) CDesCArrayFlat(4);
    CleanupStack::PushL(itemArray);

    TBuf<KMaxFileName+10> itemText;
    
    for( TInt i = 0; i < fileList->Count(); ++i)
    {
	TEntry e = (*fileList)[i];
	if (e.IsDir()) continue;

	TParse p;
	p.Set(e.iName, NULL, NULL);
	TPtrC ext = p.Ext();
	if (!ext.CompareF(_L(".gif"))  ||
	    !ext.CompareF(_L(".png"))  || 
	    !ext.CompareF(_L(".jpg"))  ||
	    !ext.CompareF(_L(".bmp"))  ||
	    !ext.CompareF(_L(".mbm"))  
#ifdef WITH_MP4VIDEO_SUPPORT
	    || !ext.CompareF(_L(".mp4")) 
#endif
	    )
	{
	    itemText.Format(_L("\t%S\t\t"), &e.iName);
	    itemArray->AppendL( itemText );
	}
    }

    CleanupStack::Pop(itemArray);
    iListBox->Model()->SetItemTextArray( itemArray );
    iListBox->Model()->SetOwnershipType( ELbmOwnsItemArray );
    iListBox->HandleItemAdditionL();
    iListBox->SetCurrentItemIndex(0);
    }

TBool CSGSavedShotsContainer::IsEmpty()
{
    if (!iListBox) return ETrue;
    return iListBox->Model()->NumberOfItems() == 0;
}


void CSGSavedShotsContainer::GetShotFilePath(TInt aItemIndex, TDes &aDes)
{
    if (aItemIndex == 0) aItemIndex = iListBox->CurrentItemIndex();
    TFileName fn(iListBox->Model()->ItemText(aItemIndex));
    CScreenGrabberModel* SGModel = static_cast<CScreenGrabberDocument*>(reinterpret_cast<CEikAppUi*>(iEikonEnv->AppUi())->Document())->Model();
    
    aDes.Append(SGModel->SavePath());
    if (aDes.Right(1) != _L("\\")) aDes.Append(_L("\\"));  
    if (fn.Left(1) == _L("\t")) fn = fn.Mid(1); 
    if (fn.Right(2) == _L("\t\t")) fn = fn.Mid(0, fn.Length()-2); 
    
    aDes.Append(fn);

}
void CSGSavedShotsContainer::ViewShotL(TInt aItemIndex)
{
    if (IsEmpty()) return;

    RFile sharableFile;
    CDocumentHandler* dh = CDocumentHandler::NewL();
    CleanupStack::PushL(dh);
    TFileName fp;
    GetShotFilePath(aItemIndex, fp);
  
    dh->OpenTempFileL(fp, sharableFile);
    CleanupClosePushL(sharableFile);

    TDataType dataType;   
    dh->OpenFileEmbeddedL(sharableFile, dataType);
    //dh->OpenFileL(sharableFile, dataType);
    CleanupStack::PopAndDestroy();  //dh, sharableFile
    
}

void CSGSavedShotsContainer::DeleteShotL(TInt aItemIndex)
{
    if (IsEmpty()) return;

    TFileName fp;
    GetShotFilePath(aItemIndex, fp);

    HBufC* prompt = StringLoader::LoadL(R_DELSHOT_QUERY_PROMPT, fp, iEikonEnv);
    CleanupStack::PushL(prompt);
    CAknQueryDialog* delQuery = CAknQueryDialog::NewL();
    delQuery->SetPromptL(*prompt);  
    CleanupStack::PopAndDestroy(prompt);
    if (delQuery->ExecuteLD(R_MY_GENERAL_CONFIRMATION_QUERY))
    {
	if (!BaflUtils::DeleteFile(iEikonEnv->FsSession(), fp))
	{
	    SetItemsL();
	}
    }

}

void CSGSavedShotsContainer::SendShotL(TInt aItemIndex)
{

    if (IsEmpty()) return;

    CSendUi* sendUi = CSendUi::NewL();
    CleanupStack::PushL(sendUi);
        
    CMessageData* messageData = CMessageData::NewL();
    CleanupStack::PushL(messageData);

    TFileName fp;
    GetShotFilePath(aItemIndex, fp);
    messageData->AppendAttachmentL(fp);

    sendUi->ShowQueryAndSendL(messageData, TSendingCapabilities(0, 0, TSendingCapabilities::ESupportsAttachments));

    CleanupStack::PopAndDestroy(2); // sendUi, messageData  
}


void CSGSavedShotsContainer::HandleListBoxEventL(CEikListBox* aListBox , TListBoxEvent aEventType)
{
    if ((aEventType == EEventEnterKeyPressed) || (aEventType == EEventItemClicked))
    {
	ViewShotL(aListBox->CurrentItemIndex());
    }
}


TInt CSGSavedShotsContainer::CountComponentControls() const
    {
    return (iListBox ? 1 : 0); // return number of controls inside this container
    }

CCoeControl* CSGSavedShotsContainer::ComponentControl(TInt aIndex) const
    {
    switch (aIndex)
        {
        case 0:
            return iListBox;
        default:
            return NULL;
        }
    }

void CSGSavedShotsContainer::SizeChanged()
    {
	if (iListBox) iListBox->SetRect(Rect());
    }

// ---------------------------------------------------------
// CSGSavedShotsContainer::HandleControlEventL(
//     CCoeControl* aControl,TCoeEvent aEventType)
// ---------------------------------------------------------
//
void CSGSavedShotsContainer::HandleControlEventL(
    CCoeControl* /*aControl*/,TCoeEvent /*aEventType*/)
    {
    }


// ---------------------------------------------------------
// CSGSavedShotsContainer::HandleResourceChange(TInt aType)
// Handle layout change of the screen
// ---------------------------------------------------------
//
void CSGSavedShotsContainer::HandleResourceChange(TInt aType)
    {

    if (aType == KEikDynamicLayoutVariantSwitch || aType == KAknsMessageSkinChange)
        {
        TRect mainPaneRect;
        AknLayoutUtils::LayoutMetricsRect(AknLayoutUtils::EMainPane, mainPaneRect);
        SetRect(mainPaneRect);
        }
    else
        {
        CCoeControl::HandleResourceChange(aType);
        }

    }


// ---------------------------------------------------------
// CSGSavedShotsContainer::OfferKeyEventL(const TKeyEvent& aKeyEvent,
//                                    TEventCode aType)
// Handle key event. Only up and down key arrow events are
// consumed in order to enable scrolling in output window.
// ---------------------------------------------------------
//

TKeyResponse CSGSavedShotsContainer::OfferKeyEventL(const TKeyEvent& aKeyEvent, TEventCode aType)
    {
	switch(aKeyEvent.iCode){
	    case EKeyDelete:
	    case EKeyBackspace:
		DeleteShotL();
		break;
	    case EStdKeyYes:
		SendShotL();
		break;
	}
	return iListBox->OfferKeyEventL(aKeyEvent, aType);

    }



void CSGSavedShotsContainer::HandlePointerEventL(const TPointerEvent &aPointerEvent)
{
    CCoeControl::HandlePointerEventL(aPointerEvent);
    if (iListBox)
    {
	iListBox->HandlePointerEventL(aPointerEvent);
    }
}


// End of File  
