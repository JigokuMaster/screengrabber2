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



#ifndef SCREENGRABBERMAINCONTAINER_H
#define SCREENGRABBERMAINCONTAINER_H

// INCLUDES
#include <coecntrl.h>
#include <eiksbobs.h>

// FORWARD DECLARATIONS
class CEikScrollBarFrame;
class CAknsBasicBackgroundControlContext;
class CEikGlobalTextEditor;

// CLASS DECLARATION

/**
*  CScreenGrabberMainContainer  container control class.
*  
*/
class CScreenGrabberMainContainer : public CCoeControl, MCoeControlObserver
    {
    public: // Constructors and destructor
        
        /**
        * EPOC default constructor.
        * @param aRect Frame rectangle for container.
        */
        void ConstructL(const TRect& aRect);

        /**
        * Destructor.
        */
        ~CScreenGrabberMainContainer();

    protected: // Functions from base classes

        void SizeChanged();
        TInt CountComponentControls() const;
        CCoeControl* ComponentControl(TInt aIndex) const;
        void HandleControlEventL(CCoeControl* aControl,TCoeEvent aEventType);
	TTypeUid::Ptr MopSupplyObject(TTypeUid aId);
    public:
        void HandleResourceChange(TInt aType);
       
    private:
	TKeyResponse OfferKeyEventL(const TKeyEvent& aKeyEvent, TEventCode /*aType*/);
	void HandlePointerEventL(const TPointerEvent &aPointerEvent);
    
    private:
        void UpdateVisualContentL();

    private: //data
        HBufC*                  iText;
        CAknsBasicBackgroundControlContext* iSkinContext;
	CEikGlobalTextEditor*	iTextView;


    };

#endif

// End of File
