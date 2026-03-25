
#include <e32std.h>
#include <s32file.h>
#include <avkon.hrh> // EAknSoftkeyCancel

#ifndef MSF_GIF_IMPL
#define MSF_GIF_IMPL
#include "msf_gif.h"
#endif

#include "SGNewFormats.h"

static void CompleteRequest(TRequestStatus& aStatus, TInt aError)
{
    TRequestStatus* status = &aStatus;
    User::RequestComplete(status, aError);
}

TBool SaveAsGIFHQ(TRequestStatus& aStatus, RFs &aFs, const TDesC &aFilePath, CFbsBitmap *aBitmap)
{

     
    TInt bpp = TDisplayModeUtils::NumDisplayModeBitsPerPixel(aBitmap->DisplayMode());   
    if (bpp < 32) 
    {
	CompleteRequest(aStatus, KErrNotSupported);
	return EFalse;
    }

    TSize size = aBitmap->SizeInPixels(); 
    TInt width = size.iWidth, height = size.iHeight;
    TInt stride = aBitmap->DataStride();
    int centisecondsPerFrame = 1, quality = 16;   
    msf_gif_bgra_flag = true; 
    msf_gif_alpha_threshold = 0;
    MsfGifState gifState = {};

    if (!msf_gif_begin(&gifState, width, height))
    {
	CompleteRequest(aStatus, KErrNotReady);
	return EFalse;
    }

    //aBitmap->LockHeap();
    TUint32 *data = aBitmap->DataAddress();
    //aBitmap->UnlockHeap();

    if (!msf_gif_frame(&gifState, (uint8_t*)data, centisecondsPerFrame, quality, stride/* width*4 */))
    {
	CompleteRequest(aStatus, KErrNoMemory);
	return EFalse;
    }

    MsfGifResult result = msf_gif_end(&gifState);

    TInt err = KErrNone;
    if (result.data)
    {
	RFile f; 
	err = f.Replace(aFs, aFilePath, EFileWrite);

	if (err == KErrNone)
	{
	    //f.Write(TPtrC8((TUint8*)result.data, result.dataSize), aStatus);
	    err = f.Write(TPtrC8((TUint8*)result.data, result.dataSize));
	}

	f.Close();
	msf_gif_free(result);
    }

    else{
	err = KErrNoMemory;
    }
    
    CompleteRequest(aStatus, err);
    return err==KErrNone;
}




// ---------------------------------------------------------------------------

TInt CGifHQAnimator::CreateGifAnimation(const TDesC& aFileName, TSize aDimensions, CVideoFrameArray* aVideoFrameArray)
    {
	CGifHQAnimator* self = new(ELeave) CGifHQAnimator;
	CleanupStack::PushL(self);
	TRAPD(err, self->StartL(aFileName, aDimensions, aVideoFrameArray));
	CleanupStack::PopAndDestroy();
	return err;
    }

// ---------------------------------------------------------------------------

CGifHQAnimator::CGifHQAnimator():iCancelEncoding(EFalse)
    {
    }

// ---------------------------------------------------------------------------

CGifHQAnimator::~CGifHQAnimator()
    {
   
    delete iSavingProgressDialog;
    }
 

// ---------------------------------------------------------------------------

void CGifHQAnimator::StartL(const TDesC& aFileName, const TSize& aDimensions, CVideoFrameArray* aVideoFrameArray)
    {
    __ASSERT_ALWAYS(aFileName.Length() > 0, User::Panic(_L("GifAnim"), 100));
    __ASSERT_ALWAYS(aDimensions.iHeight > 0, User::Panic(_L("GifAnim"), 101));
    __ASSERT_ALWAYS(aDimensions.iWidth > 0, User::Panic(_L("GifAnim"), 102));
    __ASSERT_ALWAYS(aVideoFrameArray != NULL, User::Panic(_L("GifAnim"), 103));
    


    // setup the gif encoder   

    TInt width = aDimensions.iWidth, height = aDimensions.iHeight;

    msf_gif_bgra_flag = true; //optionally, set this flag if your pixels are in BGRA format instead of RGBA
   
    MsfGifState gifState = {};

    if (!msf_gif_begin_to_file(&gifState, width, height, (MsfGifFileWriteFunc) WriteFrame, &iOutFile))
    {
	User::Leave(KErrNotReady);
	return;
    }

   // show a progress dialog
    iSavingProgressDialog = CSavingProgressDialog::NewL(this);
    iSavingProgressDialog->StartDisplayingL(aVideoFrameArray->Count()-1);

    // open the file for writing
    User::LeaveIfError(iFs.Connect());
    User::LeaveIfError(iOutFile.Replace(iFs, aFileName, EFileWrite));

    // process each frame of the animation
    for (TInt i=0; !iCancelEncoding && i<aVideoFrameArray->Count(); i++)
        {
	    
        // write headers and raster block
        TVideoFrame frame = aVideoFrameArray->At(i);
        CFbsBitmap* bitmap = GetBitmapLC(frame, aDimensions);

	//msf_gif_alpha_threshold = frame.iEnableTransparency ? 255 : 0;
 	
        EncodeFrameL(bitmap, &gifState, frame.iDelay);

        CleanupStack::PopAndDestroy(); //bitmap
        // update the progress bar
        iSavingProgressDialog->IncreaseProgressValueWithOne();
        }


    if (!msf_gif_end_to_file(&gifState))
    {
	User::Leave(KErrNoMemory);
    }

    FinishL();
    
    // remove the progress dialog from the screen
    iSavingProgressDialog->ProcessFinished();
    }    


// ---------------------------------------------------------------------------

CFbsBitmap* CGifHQAnimator::GetBitmapLC(TVideoFrame& aFrame, const TSize& aDimensions)
    {
    CFbsBitmap* bitmap = new(ELeave) CFbsBitmap;
    CleanupStack::PushL(bitmap);
    
    // read the bitmap from the temporary file
    RFile bitmapFile;
    User::LeaveIfError( bitmapFile.Open(iFs, aFrame.iFileStorePath, EFileRead) );
    RFileReadStream readStream(bitmapFile);
    bitmap->InternalizeL(readStream);
    readStream.Close();
    bitmapFile.Close();

    // delete the temporary file since it's not needed anymore
    iFs.Delete(aFrame.iFileStorePath);
    
    // resize the bitmap to match the video dimensions if it is a full screen one
    if (aFrame.iFillsWholeScreen && (aFrame.iWidth != aDimensions.iWidth || aFrame.iHeight != aDimensions.iHeight))
        {
        if (bitmap->Resize(aDimensions) == KErrNone)
            {
            // also update dimensions of this frame to match the dimensions of the video            
            aFrame.iWidth = aDimensions.iWidth;
            aFrame.iHeight = aDimensions.iHeight;
            }
        }
    
    return bitmap;
    }
 
TBool CGifHQAnimator::EncodeFrameL(CFbsBitmap* aBitmap, MsfGifState* aGifState, TUint aDelay)
{
    TInt bpp = TDisplayModeUtils::NumDisplayModeBitsPerPixel(aBitmap->DisplayMode());   
    if (bpp < 32) 
    {

	User::Leave(KErrNotSupported);
	return EFalse;
    }
    
    //TInt width = aBitmap->SizeInPixels().iWidth;

    TInt stride = aBitmap->DataStride();

    int centisecondsPerFrame = aDelay, quality = 16;   

    aBitmap->LockHeap();
    uint8_t* data = (uint8_t*)aBitmap->DataAddress();  
    int ok = msf_gif_frame_to_file(aGifState, data, centisecondsPerFrame, quality, stride);
    aBitmap->UnlockHeap();
    
    if (!ok)
    {
	User::Leave(KErrNoMemory);
	return EFalse;
    }
    return ETrue;
}


size_t CGifHQAnimator::WriteFrame(const void * buffer, size_t size, size_t count, void* stream)
{
    RFile* f = (RFile*) stream;
    TInt err = f->Write(TPtrC8((TUint8*)buffer, count*size));

    if (err != KErrNone) return 0;

    return count*size;
}    
// ---------------------------------------------------------------------------

void CGifHQAnimator::FinishL()
    {
    iOutFile.Close();
    iFs.Close();
    }

// ---------------------------------------------------------------------------

void CGifHQAnimator::DialogDismissedL(TInt aButtonId)
    {
    // check if cancel button was pressed
    if (aButtonId == EAknSoftkeyCancel)
        {
	    iCancelEncoding = ETrue;
        }
    }

