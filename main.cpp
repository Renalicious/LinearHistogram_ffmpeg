#include <fstream>
#include <iostream>
#include "histogram.hpp"
#include "ffwrapper.hpp"
#include "pbm_io.h"
#include <stdlib.h>

using namespace std;

void PrintCompilerSettings(bool);

int main(int argc, char *argv[])
{
    cout << "***** Welcome to Linear Histogram, now with FFMPEG! *****\n";
    cout << "***** Running version 1.0                           *****\n";
    PrintCompilerSettings(true);

    string fileName = "";

    //--- Check arguments for a good file name
    //--- Quit on error, return code 100
    if(argc > 1)
    {
        fileName = argv[1];
    }
    else
    {
        cerr << "\tError, no file provided!\n";
        //return 100;
    }

    //--- Control variables and containers
    //--- Eventually these will be fed via arguments
    //int frameLimit = -1;//2 << 10; //was 16
    int frameLimit = -1;
    int maxFrames;
    int scaleMode = 1; //sqrt
    int paintMode = 1;
    bool writeImage = true;
    bool writeHist = true;
    bool write2D = false;
    c_FFWrap ff = c_FFWrap(fileName.c_str());
    c_Histogram *hist;

    //--- Attempt to open the video
    //--- Quit on error, return code 101
    if(ff.OpenVideo() < 0)
    {
        cout << "Could not open video, exiting application\n";
        return 101;
    }

    else
    {
        //--- Get our maximum number of frames.
        if(frameLimit > 0 && frameLimit < ff.GetFrames())
        {
            cout << "\tWarning: Reached maximum memory limits!\n";
            maxFrames = frameLimit;
        }
        else
        {
            maxFrames = ff.GetFrames();
        }

        cout << "\tProcessing file   : " << fileName << "\n";
        cout << "\tVideo duration    : " << maxFrames << " frames" << "\n";
        cout << "\tVideo average fps : " << (double)ff.GetAvgFPS() / 100.0 << "\n";
        cout << "\tVideo resolution  : " << ff.GetWidth() <<  "x" << ff.GetHeight() << "\n";

        cout << "\n\tInitializing histogram processor...\n\n";

        //--- Try to initialize the histogram with the number of frames
        //--- Quit on error, return code 102
        try
        {
            hist = new c_Histogram(maxFrames, ff.GetWidth(), ff.GetHeight(), 3, scaleMode);
        }
        catch (std::ios::failure& e)
        {
            cerr << "\tError: Could not allocate enough memory for histogram!";
            return 102;
        }

        cout << "\tReading video frames...\n\n";
        ff.ReadVideo(hist, maxFrames);

        //--- Image buffer
        unsigned char* imageBuffer = new unsigned char[maxFrames * 256 * 4];
        memset(imageBuffer, 0, maxFrames * 256 * 4 * sizeof(unsigned char));

        //--- This section deals with writing a regular image
        if(writeImage)
        {
            imageBuffer = hist->CreateImage(false);

            if(maxFrames < (2 << 14))
            {
                string histName = fileName + ".tga";
                cout << "\tWriting histogram file as TGA: " << histName << "\n";

                TGA_Write(histName, maxFrames, 256, imageBuffer);
            }

            else
            {
                string histName = fileName + ".raw";
                cout << "\tWriting histogram file as RAW: " << histName << "\n";

                PBM_WriteP6(histName, maxFrames, 256, imageBuffer);
            }
        } //END writeImage

        //--- This section deals with writing a histogram file
        if(writeHist)
        {
            memset(imageBuffer, 0, maxFrames * 256 * 4 * sizeof(unsigned char));
            imageBuffer = hist->CreateHist();

            string histName2 = fileName + ".histogram";
            cout << "\tWriting histogram file as " << histName2 << "\n";
            HIST_Write(histName2, ff.GetAvgFPS(), maxFrames, 4, 256, 256, "RGBA", hist->ScaleMode(), hist->CreateHist());

            memset(imageBuffer, 0, sizeof(unsigned char));
        }//END writeHist

        //--- This section deals with writing a series of 2D images
        //--- Too many frames to write might cause the application to crash from IO errors
        if(write2D)
        {
            cout << "\tWriting 2D histogram files...\n";
            unsigned char* imgBuf2D = new unsigned char[256 * 256 * 4];
            char buffer [256];

            for(int i = 0; i < maxFrames; i++)
            {
                memset(imgBuf2D, 0, 256 * 256 * 4 * sizeof(unsigned char));
                imgBuf2D = hist->CreateImage2D(i, paintMode, false, true, true);

                try
                {
                    memset(&buffer, 0, 256 * sizeof(char));
                    snprintf( buffer, 256, "%s_hist2d.%05d.tga", fileName.substr(0, fileName.length() - 4).c_str(), i );
                    TGA_Write(buffer, 256, 256, imgBuf2D);
                }
                catch(...)
                {
                    cerr << "\tWarning, caught exception trying to write 2D histogram image #" << i <<" , attempting to continue\n";
                    continue;
                }
            }

        }//END write2D

        cout << "\tAll done!";

        //delete hist;
        //delete[] imageBuffer;
    }

    //--- All is well! Yay! :)
    return 0;
}

void PrintCompilerSettings(bool printStuff)
{
#ifdef _CORE2_
    cout << "\tCompiled with core2 instructions\n";
#endif
}
