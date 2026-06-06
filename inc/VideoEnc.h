#ifndef __VIDEOENC_H__
#define __VIDEOENC_H__

#include <stdint.h>
#include <x264.h>
#include <lsmash.h>

////////////////////////////////////
//// the encoder and muxer are heavily based on the code of x264 cli tool. 
//////////////////////////////////

class MP4Muxer
{

public:
	MP4Muxer();
	~MP4Muxer();
public:
	char* getError();
	
	int init(const char* fp);

	int setParam(x264_param_t *param);

	int writeHeaders(x264_nal_t *p_nal);

	int writeFrame(uint8_t *data, int size, x264_picture_t *picture);

	int flushFrames(int64_t largest_pts, int64_t second_largest_pts);

private:
	void cleanup();
	void setError(const char* msg);

private:

	lsmash_root_t *mRoot;
	lsmash_video_summary_t *mSummary;
	int64_t	 mStartOffset;  
	uint64_t mFirstCTS;
	uint64_t mTimeInc;
	uint32_t mVideoTimeScale;
	uint32_t mMovieTimeScale;	
	uint32_t mVideoTrack;
	uint32_t mSampleEntry;
	uint8_t *mSEIBuffer;
	uint32_t mSEIBufferSize;
	lsmash_file_parameters_t mFileParam;
	char mError[1024];
};


class MP4Encoder
{

public:
	MP4Encoder();
	~MP4Encoder();
public:
	char* getError();
	
	int init(int width, int height, const char* fp);

	int encodeFrame(uint8_t* rgb, int bpp, int stride, int64_t duration);

	int flushFrames();

private:
	void cleanup();
	void setError(const char* msg);

private:
	MP4Muxer *mMuxer;
	x264_t *mx264;
	x264_picture_t mx264Picture;
	x264_param_t mx264Param;
	uint8_t* mYUVBuffer;
	int64_t mLargestPts;
	int64_t mSecondLargestPts;
	char mError[1024];

};


#endif // __VIDEOENC_H__
