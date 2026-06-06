/*
 ============================================================================
 Name		: SGAboutView.h
 Author	  : JigokuMaster
 Copyright   : Your copyright notice
 Description : Declares view class for application.
 ============================================================================
 */

#ifndef __SGABOUTVIEW_H__
#define __SGABOUTVIEW_H__

// INCLUDES
#include <aknview.h>
#include <brctlinterface.h>

const TUid KAboutViewUID = {4};

// CLASS DECLARATION

/**
 *  CWebView
 * 
 */
class CWebView : public CCoeControl
	{
public:
	// Constructors and destructor

	/**
	 * Destructor.
	 */
	~CWebView();

	/**
	 * Two-phased constructor.
	 */
	static CWebView* NewL(const TRect& aRect);

	/**
	 * Two-phased constructor.
	 */
	static CWebView* NewLC(const TRect& aRect);
	
private:

	/**
	 * Constructor for performing 1st stage construction
	 */
	CWebView();

	/**
	 * EPOC default constructor for performing 2nd stage construction
	 */
	void ConstructL(const TRect& aRect);

	virtual void SizeChanged();

        virtual TInt CountComponentControls() const; 

        virtual CCoeControl* ComponentControl(TInt aIndex) const; 


	void HandlePointerEventL(const TPointerEvent& aPointerEvent);

public:
        TKeyResponse OfferKeyEventL(const TKeyEvent& aKeyEvent,TEventCode aType);

	void LoadPageL();

public: //data 
        // Pointer to the browser control interface
        CBrCtlInterface* iBrCtlInterface;
};

class CSGAboutView : public CAknView
{
public:
	// New methods
	static CSGAboutView* NewL();
	static CSGAboutView* NewLC();

	/**
	 * ~SGAboutView
	 * Virtual Destructor.
	 */
	virtual ~CSGAboutView();

public:
    
        /**
        * From CAknView.
        * Returns views id.
        *
        * @return Id for this view.
        */
        TUid Id() const;
    
        /**
        * From CAknView.
        * Handles commands
        *
        * @param aCommand A command to be handled.
        */
        void HandleCommandL( TInt aCommand );

	void HandleViewRectChange();

        /**
        * From CAknView.
        * Activates view
        *
        * @param aPrevViewId Specifies the view previously active.
        * @param aCustomMessageId Specifies the message type.
        * @param aCustomMessage The activation message.
        */
        void DoActivateL( const TVwsViewId& aPrevViewId,
                          TUid aCustomMessageId,
                          const TDesC8& aCustomMessage );
    
        /**
        * From CAknView.
        * Deactivates view.
        */
        void DoDeactivate();

private:
	// Constructors
	void ConstructL();

	/**
	 * SGAboutView.
	 * C++ default constructor.
	 */
	CSGAboutView();


private:
	CWebView* iContainer;
};

#endif // __SGABOUTVIEW_H___
// End of File
