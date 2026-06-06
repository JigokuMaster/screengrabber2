#ifndef __SCREENGRABBER_NEWFORMATS_H__
#define __SCREENGRABBER_NEWFORMATS_H__

#include <e32std.h>
#include <e32base.h>
#include <fbs.h>
#include <f32file.h>
#include <stddef.h>
#include "SGGifAnimator.h"

#ifndef MSF_GIF_IMPL
struct MsfGifState;
#endif


TBool SaveAsGIFHQ(TRequestStatus& aStatus, RFs &aFs, const TDesC &aFilePath, CFbsBitmap *aBitmap);


class CGifHQAnimator : public CBase, public MDialogCallback
	{
public:

    static int CreateGifAnimation(const TDesC& aFileName, TSize aDimensions, CVideoFrameArray* aVideoFrameArray);
    ~CGifHQAnimator();

private:
    CGifHQAnimator();
    void StartL(const TDesC& aFileName, const TSize& aDimensions, CVideoFrameArray* aVideoFrameArray);

    CFbsBitmap* GetBitmapLC(TVideoFrame& aFrame, const TSize& aDimensions);
    TBool EncodeFrameL(CFbsBitmap* aBitmap, MsfGifState* aGifState, TUint aDelay);
    static size_t WriteFrame(const void* buffer, size_t size, size_t count, void* stream);

    void FinishL();
    void DialogDismissedL(TInt aButtonId); // from MDialogCallback
    
private:
    RFs                         iFs;
    RFile                       iOutFile;
    TBool        		iCancelEncoding;
    CSavingProgressDialog*      iSavingProgressDialog;
    };


#ifdef WITH_MP4VIDEO_SUPPORT
#include "VideoEnc.h"

class CMP4Exporter : public CBase, public MDialogCallback
	{
public:

    static int Create(const TDesC& aFileName, TSize aDimensions, CVideoFrameArray* aVideoFrameArray);
    ~CMP4Exporter();

private:
    CMP4Exporter();

    void StartL(const TDesC& aFileName, const TSize& aDimensions, CVideoFrameArray* aVideoFrameArray);

    CFbsBitmap* GetBitmapLC(TVideoFrame& aFrame, const TSize& aDimensions);

    TBool EncodeFrameL(CFbsBitmap* aBitmap, TUint aDelay);

    void ShowErrorL();

    void DialogDismissedL(TInt aButtonId); // from MDialogCallback
    
private:
    MP4Encoder*			iMP4Encoder;
    RFs                         iFs;
    TBool        		iCancelEncoding;
    CSavingProgressDialog*      iSavingProgressDialog;
    };



#endif // WITH_MP4VIDEO_SUPPORT


#endif // __SCREENGRABBER_NEWFORMATS_H__
