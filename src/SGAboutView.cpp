/*
   ============================================================================
Name		: SGAboutView.cpp
Author	  : JigokuMaster
Copyright   : Your copyright notice
Description : Application view implementation
============================================================================
*/

// INCLUDE FILES
#include <coemain.h>
#include <bautils.h>
#include "SGAppUi.h"
#include  "SGMainView.h"
#include "SGAboutView.h"
#include  <ScreenGrabber2.rsg>
#include  "SG.hrh"


#include <eikenv.h>
#include <f32file.h>

#ifdef __WINSCW__
_LIT(KHTMLFile, "C:\\System\\Apps\\SG2\\about.html");
#else
_LIT(KHTMLFile, "about.html");
#endif

CWebView::CWebView()
{
    iBrCtlInterface = NULL;
}

CWebView::~CWebView()
{
    if (iBrCtlInterface) delete iBrCtlInterface;

}

CWebView* CWebView::NewLC(const TRect& aRect)
{
    CWebView* self = new (ELeave) CWebView();
    CleanupStack::PushL(self);
    self->ConstructL(aRect);
    return self;
}

CWebView* CWebView::NewL(const TRect& aRect)
{
    CWebView* self = CWebView::NewLC(aRect);
    CleanupStack::Pop(); // self;
    return self;
}

void CWebView::ConstructL(const TRect& aRect)
{
    CreateWindowL();
    iBrCtlInterface = CreateBrowserControlL(this, 
	aRect, 
	0,
	TBrCtlDefs::ECommandIdBase, 
        NULL, 
        NULL, 
        NULL,
        NULL,
	NULL
    );

    SetRect(aRect);
    LoadPageL();
    SetBlank();
    ActivateL();
}


void CWebView::LoadPageL()
{
    iBrCtlInterface->SetBrowserSettingL(TBrCtlDefs::ESettingsTextWrapEnabled, 1);
#ifdef __WINSCW__
    if (BaflUtils::FileExists(iEikonEnv->FsSession(), KHTMLFile))
    {
	iBrCtlInterface->LoadFileL(KHTMLFile);
    }
#else
    // load from private folder
    TFileName fp; // full path
    TFileName privatePath;
    iEikonEnv->FsSession().PrivatePath(privatePath);
    fp.Append(RProcess().FileName().Left(2)); // C: or E: etc...
    fp.Append(privatePath);     
    fp.Append(KHTMLFile);
    if (BaflUtils::FileExists(iEikonEnv->FsSession(), fp))
    {
	iBrCtlInterface->LoadFileL(fp);
    }
   
#endif

}


TInt CWebView::CountComponentControls() const
{
    if (iBrCtlInterface)
    {
        return 1;
    }
    return 0;

}

CCoeControl* CWebView::ComponentControl(TInt aIndex) const
{
    switch ( aIndex )
    {
        case 0:
            return iBrCtlInterface; // Could be NULL
        default:
            return NULL;
    }
}

void CWebView::SizeChanged()
{
    if (iBrCtlInterface)
    {
        iBrCtlInterface->SetRect(Rect());
    }
}

void CWebView::HandlePointerEventL(const TPointerEvent& aPointerEvent)
{

    CCoeControl::HandlePointerEventL(aPointerEvent);
    if (iBrCtlInterface) iBrCtlInterface->HandlePointerEventL(aPointerEvent);

}

TKeyResponse CWebView::OfferKeyEventL(const TKeyEvent& aKeyEvent,TEventCode aType)
{
    if (iBrCtlInterface)
    {
	return iBrCtlInterface->OfferKeyEventL(aKeyEvent, aType);
    }
    return EKeyWasConsumed;
}


// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CSGAboutView::NewL()
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CSGAboutView* CSGAboutView::NewL()
{
    CSGAboutView* self = CSGAboutView::NewLC();
    CleanupStack::Pop(self);
    return self;
}

// -----------------------------------------------------------------------------
// CSGAboutView::NewLC()
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CSGAboutView* CSGAboutView::NewLC()
{
    CSGAboutView* self = new (ELeave) CSGAboutView;
    CleanupStack::PushL(self);
    self->ConstructL();
    return self;
}

// -----------------------------------------------------------------------------
// CSGAboutView::ConstructL()
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
//
void CSGAboutView::ConstructL()
{
    BaseConstructL(R_SCREENGRABBER_VIEW4);
}

// -----------------------------------------------------------------------------
// CSGAboutView::SGAboutView()
// C++ default constructor can NOT contain any code, that might leave.
// -----------------------------------------------------------------------------
//
CSGAboutView::CSGAboutView()
{
    iContainer = NULL;

}

// -----------------------------------------------------------------------------
// CSGAboutView::~SGAboutView()
// Destructor.
// -----------------------------------------------------------------------------
//
CSGAboutView::~CSGAboutView()
{
    if (iContainer)
    {
	delete iContainer;
	iContainer = NULL;
    }
}


TUid CSGAboutView::Id() const 
{ 
    return KAboutViewUID; 
}


void CSGAboutView::HandleCommandL( TInt aCommand )
{
    switch ( aCommand )
        {
        case EAknSoftkeyBack:
            // switch back to the main view
            AppUi()->ActivateLocalViewL(KMainViewUID);
            break;

        default:
            AppUi()->HandleCommandL( aCommand );
            break;
        }
}


void CSGAboutView::HandleViewRectChange()
{
    if (iContainer)
    {
	iContainer->SetRect(ClientRect());
    }   
}


void CSGAboutView::DoActivateL( const TVwsViewId& /*aPrevViewId*/,
	TUid /*aCustomMessageId*/,
	const TDesC8& /*aCustomMessage*/ )
{

    if (!iContainer) iContainer = CWebView::NewL(ClientRect());
    iContainer->SetMopParent(this);
    AppUi()->AddToStackL(*this, iContainer); 
    Cba()->MakeCommandVisible(EAknSoftkeyOptions, EFalse);  
}


void CSGAboutView::DoDeactivate()
{
    if ( iContainer )
    {
	AppUi()->RemoveFromViewStack(*this, iContainer);
	delete iContainer;
	iContainer = NULL;
    }  
}

// End of File
