//------------------------------------------------------------------------------
// FFMPEG read video wrapper
//
// A simple wrapper for FFMPEG's functionality so that C++ code can be kept clean
// The major purpose for this wrapper is to be able to extract video frames
// for processing.
//
// This wrapper, currently, has no functionality to WRITE or ENCODE files.
//------------------------------------------------------------------------------

#ifndef __FFWRAP__
#define __FFWRAP__

#include <iostream>
#include "histogram.hpp"

extern "C" {
    #include <libavcodec/avcodec.h>
    #include <libavformat/avformat.h>
    #include <libavformat/avio.h>
    #include <libswscale/swscale.h>
    #include <libavutil/pixdesc.h>
    #include <libavutil/file.h>
    #include <libavutil/opt.h>
};

class c_FFWrap
{
public:
    c_FFWrap(const char* fileName);
    ~c_FFWrap();

    //--- Open video file for processing
    int OpenVideo();

    //--- Read a frame
    void ReadVideo(c_Histogram *, const int frameLimit = -1);
    int GetCurrentFrame();

    //--- Convert AVFrame data into an unsignes char array
    unsigned char* FrameToArray(AVFrame *inFrame, const int dataIndex = 0);

    //--- Print additional info
    void PrintInfo();

    //--- Get additional parameters
    int GetWidth();
    int GetHeight();
    int GetAvgFPS();
    int GetFrames();

private:
    AVFormatContext *pFormatCtx;
    AVCodecContext *pCodecCtx;
    AVFrame *pFrame;
    AVFrame *pFrameRGB;
    AVPacket packet;
    struct SwsContext *sws_ctx;
    const char* vidFile;
    int vidStream;
    int returnCode;
    int vidWidth;
    int vidHeight;
    int curFrame;
};

#endif // __FFWRAP__
