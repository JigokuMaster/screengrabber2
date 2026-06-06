#include "VideoEnc.h"

#include <stdlib.h>
#include <string.h> // memcpy

#ifdef ENABLE_DEBUG
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#define DEBUG( ... ) fprintf(stderr, __VA_ARGS__)
#else 
#define DEBUG( ... )
#endif 

#define MP4_FAIL_IF_ERR( cond, msg )\
do\
{\
    if( cond )\
    {\
        fprintf(stderr, msg);\
        return 0;\
    }\
} while( 0 )


#define MP4_FAIL_WITH_ERR( msg )\
do\
{\
    fprintf(stderr, msg);\
    return 0;\
} while( 0 )



#define H264_NALU_LENGTH_SIZE 4


///////////////////////////////////////////////////// 
///
/// this function was created by Gemini and tweaked by me. 
///
void bgr_to_yuv420p(
    const uint8_t *bgr_data, 
    int width,
    int height,
    int pixel_bytes,
    int bgr_stride,
    uint8_t *y_plane,
    uint8_t *u_plane,
    uint8_t *v_plane) 
{
    int y_stride = width;
    int uv_stride = width / 2;

    // Process 2 rows at a time
    for (int y = 0; y < height; y += 2) {
        const uint8_t *bgr_row0 = bgr_data + (y * bgr_stride);
        const uint8_t *bgr_row1 = bgr_data + ((y + 1) * bgr_stride);

        uint8_t *y_row0 = y_plane + (y * y_stride);
        uint8_t *y_row1 = y_plane + ((y + 1) * y_stride);

        uint8_t *u_row = u_plane + ((y / 2) * uv_stride);
        uint8_t *v_row = v_plane + ((y / 2) * uv_stride);

        // Process 2 pixels horizontally at a time
        for (int x = 0; x < width; x += 2) {
            
            // --- Pixel Offsets ---
            int offset00 = x * pixel_bytes;
            int offset01 = (x + 1) * pixel_bytes;

            // --- Row 0 (Top 2 Pixels) ---
            uint8_t b00 = bgr_row0[offset00 + 0], g00 = bgr_row0[offset00 + 1], r00 = bgr_row0[offset00 + 2];
            uint8_t b01 = bgr_row0[offset01 + 0], g01 = bgr_row0[offset01 + 1], r01 = bgr_row0[offset01 + 2];

            // --- Row 1 (Bottom 2 Pixels) ---
            uint8_t b10 = bgr_row1[offset00 + 0], g10 = bgr_row1[offset00 + 1], r10 = bgr_row1[offset00 + 2];
            uint8_t b11 = bgr_row1[offset01 + 0], g11 = bgr_row1[offset01 + 1], r11 = bgr_row1[offset01 + 2];

            // 1. Calculate Y (Luma) for all 4 pixels independently
            y_row0[x]     = (uint8_t)((77 * r00 + 150 * g00 + 29 * b00) >> 8);
            y_row0[x + 1] = (uint8_t)((77 * r01 + 150 * g01 + 29 * b01) >> 8);
            y_row1[x]     = (uint8_t)((77 * r10 + 150 * g10 + 29 * b10) >> 8);
            y_row1[x + 1] = (uint8_t)((77 * r11 + 150 * g11 + 29 * b11) >> 8);

            // 2. Average the BGR colors across the 2x2 cluster for proper Chroma subsampling
            int r_avg = (r00 + r01 + r10 + r11) >> 2;
            int g_avg = (g00 + g01 + g10 + g11) >> 2;
            int b_avg = (b00 + b01 + b10 + b11) >> 2;

            // 3. Calculate 1 pair of U/V values for this 2x2 block
            u_row[x / 2] = (uint8_t)(((-43 * r_avg - 85 * g_avg + 128 * b_avg) >> 8) + 128);
            v_row[x / 2] = (uint8_t)(((128 * r_avg - 107 * g_avg - 21 * b_avg) >> 8) + 128);
        }
    }
}


///////////////////////////////////////////////////// 

// Implementation of MP4Muxer

MP4Muxer::MP4Muxer()
{
    mRoot = NULL;
    mSummary = NULL;
    mSEIBuffer = NULL;
    mSEIBufferSize = 0;
    mStartOffset = 0;
    mFirstCTS = 0;
}

MP4Muxer::~MP4Muxer()
{
    cleanup();
}

void MP4Muxer::cleanup()
{
    lsmash_cleanup_summary((lsmash_summary_t *)mSummary);
    if ( mRoot ) lsmash_destroy_root(mRoot);
    if ( mSEIBuffer ) free(mSEIBuffer);

    mRoot = NULL;
    mSEIBuffer = NULL;
}

void MP4Muxer::setError(const char* msg)
{
    if (msg) strncpy(mError, msg, sizeof(mError)-1);
    else memset(mError, 0, sizeof(mError));
}

char* MP4Muxer::getError()
{
    return ( strlen(mError) ? mError : NULL);
}
	

int MP4Muxer::init(const char* fp)
{
    mRoot = lsmash_create_root();
 
    MP4_FAIL_IF_ERR( lsmash_open_file(fp, 0, &mFileParam) < 0, "failed to open an output file.\n" );

    mSummary = (lsmash_video_summary_t*)lsmash_create_summary(LSMASH_SUMMARY_TYPE_VIDEO);

    MP4_FAIL_IF_ERR( !mSummary, "failed to allocate memory for summary information of video.\n");
    
    mSummary->sample_type = ISOM_CODEC_TYPE_AVC1_VIDEO;

    return 1;
}



int MP4Muxer::setParam(x264_param_t *param)
{

    int i_delay_frames = param->i_bframe ? (param->i_bframe_pyramid ? 2 : 1) : 0;
    int dts_compress_multiplier = i_delay_frames + 1;

    uint64_t i_media_timescale = (uint64_t)param->i_timebase_den * dts_compress_multiplier;
    mTimeInc = (uint64_t)param->i_timebase_num * dts_compress_multiplier;
    
    //MP4_FAIL_IF_ERR( i_media_timescale > UINT32_MAX, "MP4 media timescale exceeds maximum\n");


    /* Select brands. */
    lsmash_brand_type brands[6] = { (lsmash_brand_type)0 };
    uint32_t brand_count = 0;
    brands[brand_count++] = ISOM_BRAND_TYPE_MP42;
    brands[brand_count++] = ISOM_BRAND_TYPE_MP41;
    brands[brand_count++] = ISOM_BRAND_TYPE_ISOM;
    
    /* Set file */
    lsmash_file_parameters_t *file_param = &mFileParam;
    file_param->major_brand   = brands[0];
    file_param->brands        = brands;
    file_param->brand_count   = brand_count;
    file_param->minor_version = 0;


    MP4_FAIL_IF_ERR( !lsmash_set_file(mRoot, file_param), "failed to add an output file into a ROOT.\n" );


    /* Set movie parameters. */
    lsmash_movie_parameters_t movie_param;
    lsmash_initialize_movie_parameters( &movie_param );


    MP4_FAIL_IF_ERR( lsmash_set_movie_parameters( mRoot, &movie_param ),
                     "failed to set movie parameters.\n" );

    mMovieTimeScale = lsmash_get_movie_timescale( mRoot);
    MP4_FAIL_IF_ERR( !mMovieTimeScale, "movie timescale is broken.\n" );

    /* Create a video track. */
    mVideoTrack = lsmash_create_track(mRoot, ISOM_MEDIA_HANDLER_TYPE_VIDEO_TRACK );

    MP4_FAIL_IF_ERR( !mVideoTrack, "failed to create a video track.\n" );

    mSummary->width = param->i_width;
    mSummary->height = param->i_height;
    uint32_t i_display_width = param->i_width << 16;
    uint32_t i_display_height = param->i_height << 16;
    if( param->vui.i_sar_width && param->vui.i_sar_height )
    {
        double sar = (double)param->vui.i_sar_width / param->vui.i_sar_height;
        if( sar > 1.0 )
            i_display_width *= sar;
        else
            i_display_height /= sar;
        mSummary->par_h = param->vui.i_sar_width;
        mSummary->par_v = param->vui.i_sar_height;
    }

    mSummary->color.primaries_index = param->vui.i_colorprim;
    mSummary->color.transfer_index  = param->vui.i_transfer;
    mSummary->color.matrix_index    = param->vui.i_colmatrix >= 0 ? param->vui.i_colmatrix : ISOM_MATRIX_INDEX_UNSPECIFIED;
    mSummary->color.full_range      = param->vui.b_fullrange >= 0 ? param->vui.b_fullrange : 0;

    /* Set video track parameters. */
    lsmash_track_parameters_t track_param;
    lsmash_initialize_track_parameters( &track_param );
    lsmash_track_mode track_mode = (lsmash_track_mode) (ISOM_TRACK_ENABLED | ISOM_TRACK_IN_MOVIE | ISOM_TRACK_IN_PREVIEW);
    track_param.mode = track_mode;
    track_param.display_width = i_display_width;
    track_param.display_height = i_display_height;
    MP4_FAIL_IF_ERR( lsmash_set_track_parameters( mRoot, mVideoTrack, &track_param ),
                     "failed to set track parameters for video.\n" );

    /* Set video media parameters. */
    lsmash_media_parameters_t media_param;
    lsmash_initialize_media_parameters( &media_param );
    media_param.timescale = i_media_timescale;

    media_param.media_handler_name = "L-SMASH Video Media Handler";

    MP4_FAIL_IF_ERR( lsmash_set_media_parameters( mRoot, mVideoTrack, &media_param),
                     "failed to set media parameters for video.\n" );

    mVideoTimeScale = lsmash_get_media_timescale(mRoot, mVideoTrack);
    MP4_FAIL_IF_ERR( !mVideoTimeScale, "media timescale for video is broken.\n" );


    DEBUG("mMovieTimeScale: %d\n", mMovieTimeScale);

    DEBUG("mVideoTimeScale: %d\n", mVideoTimeScale);

    return 1;
}



int MP4Muxer::writeHeaders(x264_nal_t *p_nal)
{
    uint32_t sps_size = p_nal[0].i_payload - H264_NALU_LENGTH_SIZE;
    uint32_t pps_size = p_nal[1].i_payload - H264_NALU_LENGTH_SIZE;
    uint32_t sei_size = p_nal[2].i_payload;

    uint8_t *sps = p_nal[0].p_payload + H264_NALU_LENGTH_SIZE;
    uint8_t *pps = p_nal[1].p_payload + H264_NALU_LENGTH_SIZE;
    uint8_t *sei = p_nal[2].p_payload;

    lsmash_codec_specific_t *cs = lsmash_create_codec_specific_data( LSMASH_CODEC_SPECIFIC_DATA_TYPE_ISOM_VIDEO_H264,
                                                                     LSMASH_CODEC_SPECIFIC_FORMAT_STRUCTURED );

    lsmash_h264_specific_parameters_t *param = (lsmash_h264_specific_parameters_t *)cs->data.structured;
    param->lengthSizeMinusOne = H264_NALU_LENGTH_SIZE - 1;

    /* SPS
     * The remaining parameters are automatically set by SPS. */
    if( lsmash_append_h264_parameter_set( param, H264_PARAMETER_SET_TYPE_SPS, sps, sps_size ) )
    {
        MP4_FAIL_WITH_ERR("failed to append SPS.\n");
    }

    /* PPS */
    if( lsmash_append_h264_parameter_set( param, H264_PARAMETER_SET_TYPE_PPS, pps, pps_size ) )
    {
	MP4_FAIL_WITH_ERR("failed to append PPS.\n");
    }

    if( lsmash_add_codec_specific_data( (lsmash_summary_t*)mSummary, cs ) )
    {
        MP4_FAIL_WITH_ERR( "failed to add H.264 specific info.\n" );
    }

    lsmash_destroy_codec_specific_data(cs);

    /* Additional extensions */
    /* Bitrate info */
    cs = lsmash_create_codec_specific_data( LSMASH_CODEC_SPECIFIC_DATA_TYPE_ISOM_VIDEO_H264_BITRATE,
                                            LSMASH_CODEC_SPECIFIC_FORMAT_STRUCTURED );
    if( cs )
        lsmash_add_codec_specific_data( (lsmash_summary_t *)mSummary, cs );

    lsmash_destroy_codec_specific_data( cs );


    mSampleEntry = lsmash_add_sample_entry(mRoot, mVideoTrack, mSummary);

    MP4_FAIL_IF_ERR( !mSampleEntry, "failed to add sample entry for video.\n" );

    /* SEI */
    mSEIBuffer = (uint8_t*) malloc(sei_size);

    MP4_FAIL_IF_ERR( !mSEIBuffer, "failed to allocate sei transition buffer.\n" );

    memcpy(mSEIBuffer, sei, sei_size);
    mSEIBufferSize = sei_size;

    return sei_size + sps_size + pps_size;
}



int MP4Muxer::writeFrame(uint8_t *data, int size, x264_picture_t *picture)
{

    uint64_t dts, cts;

    if( mFirstCTS == 0)
    {
        mStartOffset = picture->i_dts * -1;
	mFirstCTS = mStartOffset * mTimeInc;
    }


    lsmash_sample_t *sample = lsmash_create_sample( size + mSEIBufferSize);

    MP4_FAIL_IF_ERR( !sample, "failed to create a video sample data.\n" );

    if(mSEIBuffer)
    {
        memcpy(sample->data, mSEIBuffer, mSEIBufferSize);
        free(mSEIBuffer);
        mSEIBuffer = NULL;
    }

    memcpy(sample->data + mSEIBufferSize, data, size);
    
    mSEIBufferSize = 0;

    dts = (picture->i_dts + mStartOffset) * mTimeInc;
    cts = (picture->i_pts + mStartOffset) *  mTimeInc;
    
    sample->dts = dts;
    sample->cts = cts;

    sample->index = mSampleEntry;
    sample->prop.ra_flags = picture->b_keyframe ? ISOM_SAMPLE_RANDOM_ACCESS_FLAG_SYNC : ISOM_SAMPLE_RANDOM_ACCESS_FLAG_NONE;

    /* Append data per sample. */

    lsmash_append_sample(mRoot, mVideoTrack, sample);
    return 1;
}

int MP4Muxer::flushFrames(int64_t largest_pts, int64_t second_largest_pts)
{

    if( mRoot )
    {
        double actual_duration = 0;
        if( mVideoTrack )
        {
            /* Flush the rest of samples and add the last sample_delta. */
            uint32_t last_delta = largest_pts - second_largest_pts;

            MP4_FAIL_IF_ERR( lsmash_flush_pooled_samples(mRoot, mVideoTrack, (last_delta ? last_delta : 1) * mTimeInc),
                            "failed to flush the rest of samples.\n" );

            if( mMovieTimeScale != 0 && mVideoTimeScale != 0 )    /* avoid zero division */
                actual_duration = ((double)((largest_pts + last_delta) * mTimeInc) / mVideoTimeScale) * mMovieTimeScale;
            else
	    {
                MP4_FAIL_WITH_ERR( "timescale is broken.\n" );
	    }

	    MP4_FAIL_IF_ERR( lsmash_finish_movie(mRoot, NULL ), "failed to finish movie.\n" );
	}
    }

    lsmash_close_file(&mFileParam);
    DEBUG("MP4Muxer::flushFrames() ok\n");
    return 1;
}


///////////////////////////////////////////////////// 

// Implementation of MP4Encoder

MP4Encoder::MP4Encoder()
{
    mx264 = NULL;
    mYUVBuffer = NULL;
    mLargestPts = -1;
    mSecondLargestPts = -1;
}

MP4Encoder::~MP4Encoder()
{
    cleanup();
}

void MP4Encoder::cleanup()
{
    if (mMuxer)
    {
	delete mMuxer;
	mMuxer = NULL;
    }
    
    if (mx264)
    {
	x264_encoder_close(mx264);
	mx264 = NULL;
    }
    
    //x264_picture_clean(&mx264Picture);
    if (mYUVBuffer)
    {
	free(mYUVBuffer);
	mYUVBuffer = NULL;
    }
}


void MP4Encoder::setError(const char* msg)
{
    if (msg) strncpy(mError, msg, sizeof(mError)-1);
    else memset(mError, 0, sizeof(mError));
}

char* MP4Encoder::getError()
{
    if ( strlen(mError) ) return mError;
    return mMuxer->getError();
}
	

int MP4Encoder::init(int width, int height, const char* fp)
{
    x264_param_t param;

#ifdef __SYMBIAN32__
    const char* logFile = "SG2.log";
    int fd = open(logFile, O_WRONLY | O_CREAT | O_TRUNC);
    dup2(fd, 1); 
    dup2(fd, 2); 
#endif 

    DEBUG("MP4Encoder::init(%d, %d)\n", width, height);

    /* Get default params for preset/tuning */
    if( x264_param_default_preset(&param, "superfast", NULL ) < 0 ) {

	MP4_FAIL_WITH_ERR("failed to get default params for preset/tuning");

    }

    /* Configure non-default params */
    param.i_bitdepth = 8;
    param.i_csp = X264_CSP_I420;
    param.i_width = width;
    param.i_height = height;
    param.b_vfr_input = 1;
    param.b_repeat_headers = 0;
    param.b_annexb = 0;
    param.i_level_idc = 13;

    param.i_timebase_num = 1;
    param.i_timebase_den = 1000;
    
    DEBUG("param.i_fps_num: %d\n", param.i_fps_num);

    DEBUG("param.i_timebase_den: %d\n", param.i_timebase_den);

    DEBUG("param.i_timebase_num: %d\n", param.i_timebase_num);

    DEBUG("param.i_frame_total: %d\n", param.i_frame_total);

    DEBUG("param.b_vfr_input: %d\n", param.b_vfr_input);


    /* Apply profile restrictions. */
    if(x264_param_apply_profile(&param, "baseline") < 0 ){
	MP4_FAIL_WITH_ERR("failed to x264 apply profile");
    }

    //mx264Picture = (x264_picture_t*) malloc(sizeof(x264_picture_t));
    if( x264_picture_alloc(&mx264Picture, param.i_csp, param.i_width, param.i_height) < 0 )
    {

	MP4_FAIL_WITH_ERR("failed to alloc x264 picture");
    }

    mx264Picture.i_pts = 0;

    if ( !(mx264 = x264_encoder_open(&param)) )
    {
	MP4_FAIL_WITH_ERR("failed to open x264 encoder");

    }

    x264_encoder_parameters(mx264, &param);

    // Write SPS/PPS/SEI
    int i_nal;
    x264_nal_t *headers;
    if ( x264_encoder_headers(mx264, &headers, &i_nal ) < 0)
    {
	MP4_FAIL_WITH_ERR("failed to write x264 headers");
    }

    mx264Param = param;

    mMuxer = new MP4Muxer();

    if (!mMuxer->init(fp)) return 0; 

    if (!mMuxer->setParam(&mx264Param)) return 0;

    if( !mMuxer->writeHeaders(headers)) return 0;

    return 1;

}


int MP4Encoder::encodeFrame(uint8_t* rgb, int bpp, int stride, int64_t duration)
{
  
    x264_picture_t outPic;

    int luma_size = mx264Param.i_width  * mx264Param.i_height;
    int chroma_size = luma_size / 4;

    if ( !mYUVBuffer )
    {
	DEBUG("encodeFrame(bpp=%d, stride=%d)\n", bpp, stride);
	mYUVBuffer = (uint8_t*) malloc(luma_size*3 / 2);
	MP4_FAIL_IF_ERR(!mYUVBuffer, "failed to allocate YUV buffer\n");
    }


    uint8_t *y = mYUVBuffer, *u = y + luma_size, *v = u + chroma_size;
  
    //rgb_to_yuv420p(rgb, y, u, v, mx264Param.i_width, mx264Param.i_height);

    bgr_to_yuv420p(rgb, mx264Param.i_width, mx264Param.i_height, bpp, stride, y, u, v);

    //bgr24_to_yuv420p(rgb,mx264Param.i_width, mx264Param.i_height, stride, y, u, v);
   
    mx264Picture.img.plane[0] = y;
    mx264Picture.img.plane[1] = u;
    mx264Picture.img.plane[2] = v;
 
    // Encode frame

    x264_nal_t *nal;
    int i_nal;  

    if (mx264Param.b_vfr_input) 
    {
	mx264Picture.i_pts += (duration * mx264Param.i_timebase_den)/1000;
    }

    else mx264Picture.i_pts++; 

    mSecondLargestPts = mLargestPts;
    mLargestPts = mx264Picture.i_pts;

    int frame_size = x264_encoder_encode(mx264, &nal, &i_nal, &mx264Picture, &outPic);
    
    //DEBUG("mx264Picture.i_pts: %ld\n", mx264Picture.i_pts);
    
    if(frame_size < 0 )
    {
	return 0;
    }

    else if (frame_size)
    {
	return mMuxer->writeFrame(nal[0].p_payload, frame_size, &outPic);
    }

    return 1;

}


int MP4Encoder::flushFrames()
{
    x264_picture_t outPic;
    x264_nal_t *nal;
    int i_nal;
    int frame_size;

    /* Flush delayed frames */
    while(x264_encoder_delayed_frames(mx264))
    {
        frame_size = x264_encoder_encode(mx264, &nal, &i_nal, NULL,  &outPic);
        
	if( frame_size < 0 )
	{
	    return 0;
	}

        else if(frame_size)
        {
            if (!mMuxer->writeFrame(nal[0].p_payload, frame_size,  &outPic)) return 0;
        }
    }

    DEBUG("MP4Encoder::flushFrames() ok\n");

    return mMuxer->flushFrames(mLargestPts, mSecondLargestPts);
}




