
#ifndef SCREENGRABBERVIEW3CONTAINER_H
#define SCREENGRABBERVIEW3CONTAINER_H

// INCLUDES
#include <coecntrl.h>
#include <eiklbo.h>

// FORWARD DECLARATIONS
class CEikColumnListBox;

// CLASS DECLARATION

/**
*  CSGSavedShotsContainer  container control class.
*  
*/
class CSGSavedShotsContainer : public CCoeControl, MCoeControlObserver, MEikListBoxObserver
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
        ~CSGSavedShotsContainer();

    protected: // Functions from base classes

        void SizeChanged();
        TInt CountComponentControls() const;
        CCoeControl* ComponentControl(TInt aIndex) const;
        void HandleControlEventL(CCoeControl* aControl,TCoeEvent aEventType);
        void HandleResourceChange(TInt aType);

    public:
	TBool IsEmpty();
	void ViewShotL(TInt aItemIndex);
	void DeleteShotL(TInt aItemIndex=0);
	void SendShotL(TInt aItemIndex=0);
    private:
	TKeyResponse OfferKeyEventL(const TKeyEvent& aKeyEvent, TEventCode /*aType*/);
	void HandlePointerEventL(const TPointerEvent &aPointerEvent);
	void HandleListBoxEventL(CEikListBox* aListBox, TListBoxEvent aEventType);
	void SetItemsL();
	void GetShotFilePath(TInt aItemIndex, TDes &aDes);

    private: //data
        CEikColumnListBox* iListBox;
	//CDesCArray* iItemArray;

    };

#endif

// End of File
