//------------------------------------------------------------------------------
// FFMPEG wrapper
//
// A simple wrapper for FFMPEG's functionality so that C++ code can be kept clean
//------------------------------------------------------------------------------

#include "ffwrapper.hpp"

//------------------------------------------------------------------------------
//--- Default constructor
//------------------------------------------------------------------------------
c_FFWrap::c_FFWrap(const char* fileName)
{
    /* register codecs and formats and other lavf/lavc components */
    av_register_all();

    /* Register all filters */
    //avfilter_register_all();
    //avfilter_register_all();

    /* Allocate context */
    pFormatCtx = avformat_alloc_context();

    /* Register frame */
    pFrame = av_frame_alloc();
    pFrameRGB = av_frame_alloc();

    /* Register file */
    vidFile = fileName;

    vidStream = -1;
    vidWidth = 0;
    vidHeight = 0;
    curFrame = 0;
}

//------------------------------------------------------------------------------
//--- Default destructor
//------------------------------------------------------------------------------
c_FFWrap::~c_FFWrap()
{
    delete vidFile;
    av_frame_free(&pFrame);
    av_frame_free(&pFrameRGB);
    sws_freeContext(sws_ctx);
    avformat_free_context(pFormatCtx);
    avcodec_free_context(&pCodecCtx);
    av_free_packet(&packet);
}

//------------------------------------------------------------------------------
//--- Open video file for reading, based on "https://www.ffmpeg.org/doxygen/2.3/filtering_video_8c-example.html"
//--- Returns a return code:
//--- 0 = nominal
//--- 1 = error
//------------------------------------------------------------------------------
int c_FFWrap::OpenVideo()
{
    AVCodec *dec;

    //--- Open input file
    returnCode = avformat_open_input(&pFormatCtx, vidFile, NULL, NULL);
    if(returnCode < 0)
    {
        std::cerr << "FFMPEG error: Cannot open input file\n";
        return returnCode;
    }

    //--- Find a stream info
    returnCode = avformat_find_stream_info(pFormatCtx, NULL);
    if (returnCode < 0)
    {
        std::cerr << "FFMPEG error: Cannot find stream information\n";
        return returnCode;
    }

    /* select the video stream */
    returnCode = av_find_best_stream(pFormatCtx, AVMEDIA_TYPE_VIDEO, -1, -1, &dec, 0);
    if (returnCode < 0)
    {
        std::cerr << "FFMPEG error: Cannot find a video stream in the input file\n";
        return returnCode;
    }

    //--- Found a video stream, get video information
    vidStream = returnCode;
    pCodecCtx = pFormatCtx->streams[vidStream]->codec;
    vidWidth = pCodecCtx->width;
    vidHeight = pCodecCtx->height;
    av_opt_set_int(pCodecCtx, "refcounted_frames", 1, 0);

    /* init the video decoder */
    returnCode = avcodec_open2(pCodecCtx, dec, NULL);
    if (returnCode < 0)
    {
        std::cerr << "FFMPEG error: Cannot open video decoder\n";
        return returnCode;
    }

    /* create scaling context */
    //--- We don't care about scaling, we want all frames as they are.
    //--- However, we have to find out what the source pixel format is using "pCodecCtx->pix_fmt"
    //--- We then hard code the destination format to RGB24, since it's the easiest to read and work with.
    sws_ctx = sws_getContext(vidWidth, vidHeight, pCodecCtx->pix_fmt, //AV_PIX_FMT_YUV420P,
                             vidWidth, vidHeight, AV_PIX_FMT_RGB24,
                             SWS_BICUBIC, nullptr, nullptr, nullptr);

    //--- Check that it works!
    if (!sws_ctx)
    {
        std::cerr << "FFMPEG error: Impossible to create scale context for the conversion\n";
        std::cerr << "src fmt:" << av_get_pix_fmt_name(pCodecCtx->pix_fmt) << " s:" << vidWidth << "x" << vidHeight << "\n";
        std::cerr << "dst fmt:" << av_get_pix_fmt_name(AV_PIX_FMT_RGB24) << " s:" << vidWidth << "x" << vidHeight << "\n";

        fprintf(stderr,
                "Impossible to create scale context for the conversion "
                "fmt:%s s:%dx%d -> fmt:%s s:%dx%d\n",
                av_get_pix_fmt_name(pCodecCtx->pix_fmt), vidWidth, vidHeight,
                av_get_pix_fmt_name(AV_PIX_FMT_RGB24), vidWidth, vidHeight);

        returnCode = AVERROR(EINVAL);
        return returnCode;
    }

    //--- Everything is good!
    return 0;
}

//------------------------------------------------------------------------------
//--- Read a frame and return an AVFrame object
//--- Return a histogram
//------------------------------------------------------------------------------
void c_FFWrap::ReadVideo(c_Histogram *curHist, const int frameLimit)
{
    uint8_t *buffer;
    int numBytes;
    int frameFinished;

    // Determine required buffer size and allocate buffer
    numBytes = avpicture_get_size(PIX_FMT_RGB24, pCodecCtx->width, pCodecCtx->height);
    buffer = (uint8_t *)av_malloc(numBytes * sizeof(uint8_t));

    // Assign appropriate parts of buffer to image planes in pFrameRGB
    // Note that pFrameRGB is an AVFrame, but AVFrame is a superset of AVPicture
    avpicture_fill((AVPicture *)pFrameRGB, buffer, PIX_FMT_RGB24, pCodecCtx->width, pCodecCtx->height);

    //--- Read through the entire video
    while(av_read_frame(pFormatCtx, &packet) >= 0)
    {
        // Is this a packet from the video stream?
        if(packet.stream_index == vidStream)
        {
            // Decode video frame
            returnCode = avcodec_decode_video2(pCodecCtx, pFrame, &frameFinished, &packet);
            if(returnCode < 0)
                std::cerr << "FFMPEG error: Could not decode video, return code = " << returnCode << "\n";

            //--- Did we get a video frame?
            if(frameFinished == 0)
            {
                //--- Don't panic! This frame still got read OK!
                //--- This usually means that there was very little (if any) change between frames.
                std::cerr << "FFMPEG warning: no frame decompressed\n";
            }

            else
            {
                pFrame->pts = av_frame_get_best_effort_timestamp(pFrame);

                // Convert the image from its native format to RGB
                returnCode = sws_scale(sws_ctx, pFrame->data, pFrame->linesize, 0, pCodecCtx->height, pFrameRGB->data, pFrameRGB->linesize);
                //std::cout << "FFMPEG info: Slice height from sws_scale() = " << returnCode << "\n";

                //--- Process the frame
                curHist->ProcessFrame(pFrameRGB->data[0]);

                //--- IMPORTANT: Free the packet or else FFMPEG will use up all available / allocated memory!
                av_free_packet(&packet);

                //--- Keep track of our frames.
                curFrame++;

                if(curFrame % 100 == 0)
                    std::cout << "FFMPEG info: current frame = " << curFrame << "\n";

                //--- Cut out early if requested.
                if(frameLimit > 0 && (curFrame +1) == frameLimit)
                {
                    std::cout << "FFMPEG info: hit max frame limit, exiting on frame " << curFrame << '\n';
                    break;
                }
            }
        }
    }
}

//------------------------------------------------------------------------------
//--- Convert AVFrame into an unsigned char array
//--- This is more of a pass through method just incase we don't want RGB right away
//---
//--- Parameters:
//--- Input frame (AVFrame): data from FFMPEG
//--- data index (int): which data to use, default is 0. There are a total of 8 available... sometimes 16!
//------------------------------------------------------------------------------
unsigned char* c_FFWrap::FrameToArray(AVFrame *inFrame, const int dataIndex)
{
    uint8_t *data = inFrame->data[dataIndex]; //0-7
    return data;
}

//------------------------------------------------------------------------------
//--- Print to screen additional info from FFMPEG
//------------------------------------------------------------------------------
void c_FFWrap::PrintInfo()
{
    av_dump_format(pFormatCtx, 0, vidFile, 0);
}

//------------------------------------------------------------------------------
//--- Returns the current frame read
//------------------------------------------------------------------------------
int c_FFWrap::GetCurrentFrame()
{
    return curFrame;
}

//------------------------------------------------------------------------------
//--- Get video width
//------------------------------------------------------------------------------
int c_FFWrap::GetWidth()
{
    return vidWidth;
}

//------------------------------------------------------------------------------
//--- Get video height
//------------------------------------------------------------------------------
int c_FFWrap::GetHeight()
{
    return vidHeight;
}

//------------------------------------------------------------------------------
//--- Get video average FPS
//------------------------------------------------------------------------------
int c_FFWrap::GetAvgFPS()
{
    double temp = pFormatCtx->streams[vidStream]->avg_frame_rate.num / (double)pFormatCtx->streams[vidStream]->avg_frame_rate.den;
    return rint(temp * 100);
}

//------------------------------------------------------------------------------
//--- Get video duration in frames
//------------------------------------------------------------------------------
int c_FFWrap::GetFrames()
{
    //--- Our variables
    uint64_t duration;
    double msPerFrame;
    int fps, frames;

    //--- Get duration in nanoseconds (ms * 1000)
    duration = pFormatCtx->duration;

    //--- Find average FPS
    fps = GetAvgFPS();

    //--- Get frame length in ms
    msPerFrame = (1000 / (double)fps);

    //--- Calculate frames based on duration (duration as milliseconds)
    frames = (int) (((double)duration / 1000.0) / msPerFrame) / 100.0;

    return frames;
}
