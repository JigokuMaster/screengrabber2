
// INCLUDE FILES
#include  <aknViewAppUi.h>
#include  <avkon.hrh>
#include  <apgtask.h>
#include  <aknmessagequerydialog.h> 
#include  <ScreenGrabber2.rsg>
#include  "SGMainView.h"
#include  "SG.hrh" 
#include  "SGSavedShotsView.h"
#include  "SGSavedShotsContainer.h"
#include  "SGDocument.h" 
#include  "SGModel.h"

void CSGSavedShotsView::ConstructL()
    {
    BaseConstructL( R_SCREENGRABBER_VIEW3 );
    }

// ---------------------------------------------------------
// CSGSavedShotsView::~CSGSavedShotsView()
// ---------------------------------------------------------
//
CSGSavedShotsView::~CSGSavedShotsView()
    {
    if ( iContainer )
        {
        AppUi()->RemoveFromViewStack( *this, iContainer );
        }

    delete iContainer;
    }

// ---------------------------------------------------------
// TUid CSGSavedShotsView::Id()
// ---------------------------------------------------------
//
TUid CSGSavedShotsView::Id() const
    {
    return KSavedShotsViewUID;
    }



// ---------------------------------------------------------
// TUid CSGSavedShotsView::DynInitMenuPaneL(TInt aResourceId, CEikMenuPane* aMenuPane)
// ---------------------------------------------------------
//


void CSGSavedShotsView::DynInitMenuPaneL(TInt aResourceId, CEikMenuPane* aMenuPane)
{
  
    if (aResourceId == R_SCREENGRABBER_VIEW3_MENU )
        {
	    TBool dimm = iContainer->IsEmpty();
	    aMenuPane->SetItemDimmed(EScreenGrabberCmdDeleteShot, dimm);
	    aMenuPane->SetItemDimmed(EScreenGrabberCmdSendShot, dimm);
	    //Cba()->MakeCommandVisible(EAknSoftkeyOptions, dimm);
	    //Cba()->DrawNow();
        }
    else AppUi()->DynInitMenuPaneL(aResourceId, aMenuPane);
    }

   
// ---------------------------------------------------------
// CSGSavedShotsView::HandleCommandL(TInt aCommand)
// ---------------------------------------------------------
//
void CSGSavedShotsView::HandleCommandL(TInt aCommand)
    {
    switch ( aCommand )
        {
        case EAknSoftkeyBack:
            // switch back to the main view
            AppUi()->ActivateLocalViewL(KMainViewUID);
            break;

        case EScreenGrabberCmdDeleteShot:
            iContainer->DeleteShotL();
            break;

        case EScreenGrabberCmdSendShot:
            iContainer->SendShotL();
            break;

        default:
            AppUi()->HandleCommandL( aCommand );
            break;
        }

    }

// ---------------------------------------------------------
// CSGSavedShotsView::HandleClientRectChange()
// ---------------------------------------------------------
//
void CSGSavedShotsView::HandleClientRectChange()
    {
    if ( iContainer )
        {
        iContainer->SetRect( ClientRect() );
        }
    }

// ---------------------------------------------------------
// CSGSavedShotsView::DoActivateL(...)
// ---------------------------------------------------------
//
void CSGSavedShotsView::DoActivateL(
   const TVwsViewId& /*aPrevViewId*/,TUid /*aCustomMessageId*/,
   const TDesC8& /*aCustomMessage*/)
    {
    if (!iContainer)
        {
	iContainer = new (ELeave) CSGSavedShotsContainer;
        iContainer->SetMopParent(this);
        iContainer->ConstructL( ClientRect() );
	Cba()->MakeCommandVisible(EAknSoftkeyOptions, !iContainer->IsEmpty());

        AppUi()->AddToStackL( *this, iContainer );
        } 
   }

// ---------------------------------------------------------
// CSGSavedShotsView::HandleCommandL(TInt aCommand)
// ---------------------------------------------------------
//
void CSGSavedShotsView::DoDeactivate()
    {
    if ( iContainer )
        {
        AppUi()->RemoveFromViewStack( *this, iContainer );
        }
    
    delete iContainer;
    iContainer = NULL;
    }

// End of File

