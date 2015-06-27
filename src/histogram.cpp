//------------------------------------------------------------------------------
// Histogram class
//
// Processes each frame and stores a histogram
//------------------------------------------------------------------------------

#include "histogram.hpp"
#include <string>
#include <iostream>
#include <cmath>
#include "memory.h"

//------------------------------------------------------------------------------
//--- Default constructor
//------------------------------------------------------------------------------
c_Histogram::c_Histogram()
{
    isInitialized = false;
}

//------------------------------------------------------------------------------
//--- Constructor 1
//--- Parameters:
//--- frame count(int)
//--- frame width(int)
//--- frame height(int)
//------------------------------------------------------------------------------
c_Histogram::c_Histogram(const int fc, const int w, const int h, const int c, const int sm)
{
    isInitialized = false;
	InitHist(fc, w * h, c, sm);
}

//------------------------------------------------------------------------------
//--- Constructor 2
//--- Parameters:
//--- frame count(int)
//--- frame size (int): width x height
//------------------------------------------------------------------------------
c_Histogram::c_Histogram(const int fc, const int fz, const int c, const int sm)
{
    isInitialized = false;
	InitHist(fc, fz, c, sm);
}

//--- Destructor
c_Histogram::~c_Histogram()
{
	delete[] histogram;
	delete[] rawSlice;
}

//------------------------------------------------------------------------------
//--- Initialize the histogram after the default constructor has been used
//--- Parameters:
//--- frame count(int)
//--- frame width(int)
//--- frame height(int)
//------------------------------------------------------------------------------
void c_Histogram::Initialize(const int fc, const int w, const int h, const int c, const int sm)
{
    InitHist(fc, w * h, c, sm);
}

//------------------------------------------------------------------------------
//--- Initialize the histogram after the default constructor has been used
//--- Parameters:
//--- frame count(int)
//--- frame size (int): width x height
//------------------------------------------------------------------------------
void c_Histogram::Initialize(const int fc, const int fz, const int c, const int sm)
{
    InitHist(fc, fz, c, sm);
}

//--- Private initializer for everything
void c_Histogram::InitHist(const int f, const int s, const int c, const int sm)
{
    if(!isInitialized)
    {
        histogram = new c_HistSlice[f]();
        rawSlice = new RGBA_uint32_t[256];
        curFrame = 0;
        totFrames = f;
        frameSize = s;
        channels = c;
        scaleMode = sm;
        ClearSlice();

        isInitialized = true;
    }
}

void c_Histogram::ClearSlice()
{
    memset(rawSlice, 0, 256 * sizeof(RGBA_uint32_t));
    /*
    for(int i = 0; i < 256; i++)
    {
        rawSlice[i].R = 0;
        rawSlice[i].G = 0;
        rawSlice[i].B = 0;
        rawSlice[i].A = 0;
    }
    */
}

//------------------------------------------------------------------------------
//--- Process new frame and set current frame index + 1
//--- It will read as many pixels as were specified by frame size
//--- 3 channels per pixel.
//------------------------------------------------------------------------------
void c_Histogram::ProcessFrame(const char* frameData)
{
    for(int px = 0; px < (frameSize * channels); px += 3)
    {
        //histogram[curFrame].Inc('r', (unsigned char)frameData[px + 0] + 128, rawSlice);
        //histogram[curFrame].Inc('g', (unsigned char)frameData[px + 1] + 128, rawSlice);
        //histogram[curFrame].Inc('b', (unsigned char)frameData[px + 2] + 128, rawSlice);
        histogram[curFrame].IncFastInline((unsigned char)frameData[px + 0] + 128,
                                    (unsigned char)frameData[px + 1] + 128,
                                    (unsigned char)frameData[px + 2] + 128,
                                     rawSlice);
    }

    //histogram->PrintMaxValues();
    histogram[curFrame].Normalize(scaleMode, rawSlice);
    ClearSlice();


    curFrame++;
}

void c_Histogram::ProcessFrame(const unsigned char* frameData)
{
    for(int px = 0; px < (frameSize * channels); px += 3)
    {
        //histogram[curFrame].Inc('r', frameData[px + 0], rawSlice);
        //histogram[curFrame].Inc('g', frameData[px + 1], rawSlice);
        //histogram[curFrame].Inc('b', frameData[px + 2], rawSlice);
        histogram[curFrame].IncFastInline((unsigned char)frameData[px + 0],
                                    (unsigned char)frameData[px + 1],
                                    (unsigned char)frameData[px + 2],
                                     rawSlice);
    }

    //histogram->PrintMaxValues();
    histogram[curFrame].Normalize(scaleMode, rawSlice);
    ClearSlice();


    curFrame++;
}

//------------------------------------------------------------------------------
//--- Normalize all slices
//--- Parameters:
//--- useLinear (bool): use linear normalizing, otherwise use square root
//------------------------------------------------------------------------------
void c_Histogram::NormalizeAll(bool useLinear)
{
    for(int curSlice = curFrame; curSlice >= 0; curSlice--)
        histogram[curSlice].Normalize(useLinear, rawSlice);
}

//------------------------------------------------------------------------------
//--- Create an image data, line by line horizontally
//--- Parameters:
//--- isRGBA (bool): default is RGBA, otherwise does BGRA
//------------------------------------------------------------------------------
unsigned char* c_Histogram::CreateImage(bool includeAlpha, bool isRGBA)
{
    unsigned char* imageData = new unsigned char[totFrames * 256 * 4];
    int curPos = 0;

    //--- Start at x0 (top), and move to x255 (bottom)
    for(int x = 0; x < 256; x++)
    {
        //--- Move across all frames
        for(int frame = 0; frame < totFrames; frame++)
        {
            curPos = (frame * 4) + ((totFrames * 4) * x);

            imageData[curPos + 0] = histogram[frame].GetElementFinal(x, 'r');
            imageData[curPos + 1] = histogram[frame].GetElementFinal(x, 'g');
            imageData[curPos + 2] = histogram[frame].GetElementFinal(x, 'b');

            if(includeAlpha)
                imageData[curPos + 3] = (unsigned char)((imageData[curPos + 0] + imageData[curPos + 1] + imageData[curPos + 2]) / 3.0f);
            else
                imageData[curPos + 3] = 255;

            /*
            unsigned char r, g, b, a;
            r = histogram[frame].GetElementFinal(x, 'r');
            g = histogram[frame].GetElementFinal(x, 'g');
            b = histogram[frame].GetElementFinal(x, 'b');
            a = histogram[frame].GetElementFinal(x, 'a');
            printf("curPos %i: [%c,%c,%c,%c]\n", curPos, r, g, b, a);
            */
        }
    }

    return imageData;
}

//------------------------------------------------------------------------------
//--- Create an image data, line by line horizontally
//--- Parameters:
//--- frame         (int): the frame to generate the image from
//--- paint mode    (int): #0 = start at brightness and fade. #1 = start at brightness and don't fade, #2 = use 255 or 0
//--- half heigh    (bool): create an image of 256x128, instead of 256x256
//--- isRGBA        (bool): default is RGBA, otherwise does BGRA
//------------------------------------------------------------------------------
unsigned char* c_Histogram::CreateImage2D(const int frame, const int paintMode, bool halfHeight, bool includeAlpha, bool isRGBA)
{

    unsigned char* imageData = new unsigned char[256 * 256 * 4];
    int curPos = 0;
    int painter = 0;
    int startColour = 0;
    std::string channels = "rgba";

    //--- Set memory to 0
    memset( imageData, 0, 256 * 256 * 4 );

    //--- Select a channel: [r | g | b | a]
    for(int chan = 0; chan < 4; chan++)
    {
        //--- Select a brightness sample: 0 to 255
        for(int sample = 0; sample < 256; sample++)
        {
            //--- Special average alpha treatment
            if(chan == 3)
            {
                if(includeAlpha)
                    painter = (unsigned char)(((float)histogram[frame].GetElementFinal(sample, 'r') +
                           (float)histogram[frame].GetElementFinal(sample, 'g') +
                           (float)histogram[frame].GetElementFinal(sample, 'b')) / 3.0f);
                else
                    painter = 255;

                startColour = painter;
            }

            else
            {
                //--- Assign the value to the painter
                painter = histogram[frame].GetElementFinal(sample, channels[chan]);
                startColour = painter;
            }

            //--- Paint the values vertically
            for(int y = 0; y < 256; y++)
            {
                //--- Current position
                curPos = (y * 256 * 4) + (sample * 4) + chan;

                //--- Fade the colour as it travels upwards
                if(paintMode == 0)
                {
                    imageData[curPos] = fmax((float)painter--, 0.0f);
                }

                //--- Paint starts at brightness value with no fade
                else if(paintMode == 1)
                {
                    if(painter > 0)
                        imageData[curPos] = startColour;
                    else
                        imageData[curPos] = 0;

                    painter--;
                }

                //--- Paint starts at 255 with no fade
                else if(paintMode == 2)
                {
                    if(painter > 0)
                        imageData[curPos] = 255;
                    else
                        imageData[curPos] = 0;

                    painter--;
                }
            }
        }
    }
    return imageData;
}

//------------------------------------------------------------------------------
//--- Create custom histogram file data
//--- Frame1[[rrrr... r], [gggg... g], [bbbb... b], [aaaa... a]
//--- Framr2[[rrrr... r], [gggg... g], [bbbb... b], [aaaa... a]
//--- Frame offset = 1024
//--- Channel offest = 256
//------------------------------------------------------------------------------
unsigned char* c_Histogram::CreateHist()
{
    std::string channels = "rgba";
    int chanWidth = 256;
    int chanNum = 4;
    int curPos = 0;
    unsigned char* imageData = new unsigned char[totFrames * chanWidth * chanNum];

    //--- Move across all frames
    for(int frame = 0; frame < totFrames; frame++)
    {
        for(int chan = 0; chan < chanNum; chan++)
        {
            for(int x = 0; x < chanWidth; x++)
            {
                curPos = (frame * (chanNum * chanWidth)) + (chan * chanWidth) + x;
                imageData[curPos] = histogram[frame].GetElementFinal(x, channels[chan]);
            }
        }
    }

    return imageData;
}

//------------------------------------------------------------------------------
//--- Return scale mode as string
//--- Return "error" is scale mode is not 0 - 3, AKA unsupported
//------------------------------------------------------------------------------
std::string c_Histogram::ScaleMode()
{
    if(scaleMode == 0)      return "linear";
    else if(scaleMode == 1) return "sqrt";
    else if(scaleMode == 2) return "cbrt";
    else if(scaleMode == 3) return "log";
    else                    return "error";
}

//------------------------------------------------------------------------------
//--- Operator =
//------------------------------------------------------------------------------
void c_Histogram::operator=(const c_Histogram &newHist)
{
    histogram = newHist.histogram;
    totFrames = newHist.totFrames;
    curFrame = newHist.curFrame;
    frameSize = newHist.frameSize;
    isInitialized = newHist.isInitialized;
}
