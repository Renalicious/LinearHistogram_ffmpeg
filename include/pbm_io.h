#ifndef IMAGE_IO_H_INCLUDED
#define IMAGE_IO_H_INCLUDED

#include <string>

//------------------------------------------------------------------------------
//--- Write a P3 PBM file, using ascii to represent colours
//--- Parameters:
//--- fileName (string): name of the file to create
//--- width     (int): width of the image, in pixels
//--- height    (int): hight of the image, in pixels
//--- data      (uchar array): The image data
//------------------------------------------------------------------------------
void PBM_WriteP3(const std::string fileName, const int width, const int height, const unsigned char* data);

//------------------------------------------------------------------------------
//--- Write a P6 PBM file, using bytes to represent colours
//--- Parameters:
//--- fileName (string): name of the file to create
//--- width     (int): width of the image, in pixels
//--- height    (int): hight of the image, in pixels
//--- data      (uchar array): The image data
//------------------------------------------------------------------------------
void PBM_WriteP6(const std::string fileName, const int width, const int height, const unsigned char* data);

//------------------------------------------------------------------------------
//--- Write TGA file, using bytes to represent colours
//--- Parameters:
//--- fileName (string): name of the file to create
//--- width     (int): width of the image, in pixels
//--- height    (int): hight of the image, in pixels
//--- data      (uchar array): The image data
//--- flipVertical(bool): TGA files canbe written upside down, this is the switch that does it
//------------------------------------------------------------------------------
void TGA_Write(const std::string fileName, const int width, const int height, const unsigned char *data, bool flipVertical = true);

//------------------------------------------------------------------------------
//--- Write histogram file, version 0002
//--- Parameters:
//--- fileName      (string): name of the file to create
//--- fps           (int): Framerate of the video x 100, 29.97 = 2997
//--- frame count      (int): number of frames processed
//--- channels          (int): Number of channels
//--- channelWidth      (int): Width of the channels (typically it's 256)
//--- channelMaxValue   (int): The "height" of the channel, typically it's 256, or 8bits, but we might want more
//--- scale          (string): The scale of the pixels (linear (lin), square root (sqrt), cube root (cbrt), log)
//--- pixel layout   (string): The pixel order, eg: RGBA, YUV, BGRA, etc
//--- data      (uchar array): The image data
//
//--- There are 5 parameters included
//------------------------------------------------------------------------------
struct HIST_Header
{
    std::string magicNum = "HISTFILE_v000002";
    unsigned int histParams;
    unsigned int histFrames;
    unsigned int histFps;
    unsigned int histChannels;
    unsigned int histChanWidth;
    unsigned int histChanMaxVal;
    std::string histScale;
    std::string histLayout;
};
void HIST_Write(const std::string fileName, const int fps, const int frameCount, const int channels, const int channelWidth, const int channelMaxValue, const std::string layout, const std::string scale, const unsigned char *data);
void HIST_Write(const std::string fileName, HIST_Header header, const unsigned char *data);
unsigned char* HIST_Read(const std::string fileName, HIST_Header* header);

#endif // IMAGE_IO_H_INCLUDED
