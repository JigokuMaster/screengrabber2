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
#include "SGMainContainer.h"


#include <AknBidiTextUtils.h>
#include <AknUtils.h>
#include <aknnotewrappers.h>
#include <eiksbfrm.h>
#include <AknsDrawUtils.h> 
#include <AknsBasicBackgroundControlContext.h>
#include <akntitle.h>
#include <eikspane.h>  
#include <AknDef.h>
#include <eikgted.h>
#include <ScreenGrabber2.rsg>

const TUint KLeftMargin = 2;
const TUint KRightMargin = 2;


// ================= MEMBER FUNCTIONS =======================


void CScreenGrabberMainContainer::ConstructL(const TRect& aRect)
    {
    iSkinContext = NULL;

    CreateWindowL();
    Window().SetShadowDisabled(EFalse);

    EnableDragEvents();

    iTextView = new (ELeave) CEikGlobalTextEditor;
    iTextView->SetAknEditorFlags( EAknEditorFlagEnableScrollBars);
    iTextView->ConstructL(
        this,
        0,  // number of lines
        0,  // text limit
        (EEikEdwinReadOnly | EEikEdwinNoAutoSelection | CEikEdwin::EAvkonDisableCursor), // edwin flags
        EGulFontControlAll, // font control flags
        EGulAllFonts ); // font name flags
    
    iTextView->SetFocus(ETrue);

    SetBlank();
    
    // set title of the app
    CEikStatusPane* statusPane = iEikonEnv->AppUiFactory()->StatusPane();
    CAknTitlePane* title = static_cast<CAknTitlePane*>( statusPane->ControlL( TUid::Uid( EEikStatusPaneUidTitle ) ) );
    title->SetTextL( _L("Screen Grabber") );
   
    iText = iCoeEnv->AllocReadResourceL(R_USAGE_TEXT);    
    SetRect(aRect);
    ActivateL(); 
    UpdateVisualContentL();
    }

CScreenGrabberMainContainer::~CScreenGrabberMainContainer()
    {
    if (iSkinContext) delete iSkinContext;
    if (iText) delete iText;
    if (iTextView) delete iTextView;
    }


// ---------------------------------------------------------
// CScreenGrabberMainContainer::UpdateVisualContentL()
// ---------------------------------------------------------
//
void CScreenGrabberMainContainer::UpdateVisualContentL()
    {
    TSize rectSize;
    AknLayoutUtils::LayoutMetricsSize(AknLayoutUtils::EMainPane, rectSize);
    TRect rect(rectSize);

    // set the correct drawing area for the skin background
    if(iSkinContext)
        {
        delete iSkinContext;
        iSkinContext = NULL;
        } 
    
 
    iSkinContext = CAknsBasicBackgroundControlContext::NewL(KAknsIIDQsnBgAreaMain, rectSize, EFalse);
    iTextView->SetSkinBackgroundControlContextL(iSkinContext);
    TCharFormat charFormat;
    TCharFormatMask charMask;

    // get the text color from the skin   
    TRgb textColor;
    if (AknsUtils::GetCachedColor(AknsUtils::SkinInstance(), textColor, KAknsIIDQsnTextColors, EAknsCIQsnTextColorsCG6 ) == KErrNone)
    {
	charMask.SetAttrib(EAttColor);
	charMask.SetAttrib(EAttFontHeight);
	charMask.SetAttrib(EAttFontPosture); 
	charMask.SetAttrib(EAttFontStrokeWeight); 
	charMask.SetAttrib(EAttFontPrintPos);  
	charMask.SetAttrib(EAttFontTypeface); 
	const CFont* font = AknLayoutUtils::FontFromId(EAknLogicalFontSecondaryFont);
	charFormat.iFontSpec = font->FontSpecInTwips();
	charFormat.iFontPresentation.iTextColor = textColor;
	iTextView->ApplyCharFormatL(charFormat, charMask);
    }

    iTextView->SetTextL(&(*iText));
    // update the screen
    DrawNow();
    }

TInt CScreenGrabberMainContainer::CountComponentControls() const
    {
    return (iTextView ? 1 : 0); // return number of controls inside this container
    }

CCoeControl* CScreenGrabberMainContainer::ComponentControl(TInt aIndex) const
    {
    switch (aIndex)
        {
        case 0:
            return iTextView;
        default:
            return NULL;
        }
    }

void CScreenGrabberMainContainer::SizeChanged()
    {
    
    if (!iTextView) return;
    TSize size = Size();

    if (iTextView->ScrollBarFrame())
        {
        size.iWidth -= iTextView->ScrollBarFrame()->VerticalScrollBar()->ScrollBarBreadth();
        }
    iTextView->SetExtent(TPoint(0,0), size);
    }

// ---------------------------------------------------------
// CScreenGrabberMainContainer::MopSupplyObject(TTypeUid aId)
// Pass skin information if needed
// ---------------------------------------------------------
//
TTypeUid::Ptr CScreenGrabberMainContainer::MopSupplyObject(TTypeUid aId)
    {
    if (aId.iUid == MAknsControlContext::ETypeId && iSkinContext)
        {
        return MAknsControlContext::SupplyMopObject(aId, iSkinContext);
        }

    return CCoeControl::MopSupplyObject(aId);
    }


// ---------------------------------------------------------
// CScreenGrabberMainContainer::HandleControlEventL(
//     CCoeControl* aControl,TCoeEvent aEventType)
// ---------------------------------------------------------
//
void CScreenGrabberMainContainer::HandleControlEventL(
    CCoeControl* /*aControl*/,TCoeEvent /*aEventType*/)
    {
    }

// ---------------------------------------------------------
// CScreenGrabberMainContainer::HandleResourceChange(TInt aType)
// Handle layout change of the screen
// ---------------------------------------------------------
//
void CScreenGrabberMainContainer::HandleResourceChange(TInt aType)
    {
    if (aType == KEikDynamicLayoutVariantSwitch || aType == KAknsMessageSkinChange)
        {
        TRect mainPaneRect;
        AknLayoutUtils::LayoutMetricsRect(AknLayoutUtils::EMainPane, mainPaneRect);
        SetRect(mainPaneRect);
        UpdateVisualContentL();
        }
    else
        {
        CCoeControl::HandleResourceChange(aType);
        }
    }


// ---------------------------------------------------------
// CScreenGrabberMainContainer::OfferKeyEventL(const TKeyEvent& aKeyEvent,
//                                    TEventCode aType)
// Handle key event. Only up and down key arrow events are
// consumed in order to enable scrolling in output window.
// ---------------------------------------------------------
//

TKeyResponse CScreenGrabberMainContainer::OfferKeyEventL(const TKeyEvent& aKeyEvent,TEventCode aType)
    {


    if (iTextView  && aType == EEventKey) {
	
	if(aKeyEvent.iCode == EKeyUpArrow)
        {
	    iTextView->MoveDisplayL(TCursorPosition::EFLineUp);
	    return EKeyWasConsumed;
        }

	else if(aKeyEvent.iCode == EKeyDownArrow)
        {
	    iTextView->MoveDisplayL(TCursorPosition::EFLineDown);
	    return EKeyWasConsumed;
        }
    }
    return EKeyWasNotConsumed;
    }



void CScreenGrabberMainContainer::HandlePointerEventL(const TPointerEvent &aPointerEvent)
{
    if (iTextView)
    {
	static TInt oldY = 0;
	TInt currY = aPointerEvent.iPosition.iY;
	TCursorPosition::TMovementType move = TCursorPosition::EFLineUp;  

	if (currY != oldY )
	{
	    move = currY > oldY ? TCursorPosition::EFLineUp : TCursorPosition::EFLineDown;
	    oldY = currY;
	}

	if (aPointerEvent.iType == TPointerEvent::EDrag)
	{
	    iTextView->MoveDisplayL(move);
	}
    }
}


    
// End of File  
