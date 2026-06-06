
#ifndef SCREENGRABBERVIEW3_H
#define SCREENGRABBERVIEW3_H

// INCLUDES
#include <aknview.h>

// CONSTANTS
// UID of view
const TUid KSavedShotsViewUID = {3};

// FORWARD DECLARATIONS
class CSGSavedShotsContainer;

// CLASS DECLARATION

/**
*  CSGSavedShotsView view class.
* 
*/
class CSGSavedShotsView : public CAknView
{
    public: // Constructors and destructor

        /**
        * EPOC default constructor.
        */
        void ConstructL();

        /**
        * Destructor.
        */
        ~CSGSavedShotsView();

    public: // Functions from base classes
        
        TUid Id() const;

        void HandleCommandL(TInt aCommand);

        void HandleClientRectChange();

    private:
        // From MEikMenuObserver
        void DynInitMenuPaneL(TInt aResourceId, CEikMenuPane* aMenuPane);
        
    private:

        /**
        * From AknView, ?member_description
        */
        void DoActivateL(const TVwsViewId& aPrevViewId,TUid aCustomMessageId,
            const TDesC8& aCustomMessage);

        /**
        * From AknView, ?member_description
        */
        void DoDeactivate();

    private: // Data
        CSGSavedShotsContainer* iContainer;
        
    };

#endif

// End of File
