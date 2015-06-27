#include <fstream>
#include <iomanip>
#include "pbm_io.h"
#include "memory.h"

void PBM_WriteP3(const std::string fileName, const int width, const int height, const unsigned char *data)
{
    //--- Local vars
    std::ofstream myFile (fileName, std::ios::out);

    if(myFile.is_open())
    {
        //--- Write header
        myFile << "P3\n";
        myFile << width << "\n";
        myFile << height << "\n";
        myFile << "255\n";

        //--- Write data
        for(int i = 0; i < width * height * 4; i++)
            myFile << (int)data[i] << ' ';
    }

    myFile.close();
}

void PBM_WriteP6(const std::string fileName, const int width, const int height, const unsigned char *data)
{
    //--- Local vars
    std::ofstream myFile (fileName, std::ios::out | std::ios::binary);

    if(myFile.is_open())
    {
        //--- Write header
        myFile << "P6\n";
        myFile << width << "\n";
        myFile << height << "\n";
        myFile << "255\n";

        //--- Write data
        myFile.write((char *)data, width * height * 4);
    }

    myFile.close();
}

void TGA_Write(const std::string fileName, const int width, const int height, const unsigned char *data, bool flipVertical)
{
    std::ofstream myFile (fileName, std::ios::out | std::ios::binary);

    if(myFile.is_open())
    {
        unsigned char *buffer;
        int		i;
        int		bufferSize = width * height * 4 + 18;
        int     imgStart = 18;

        buffer = new unsigned char[bufferSize];
        memset( buffer, 0, 18 );
        buffer[2] = 2;		// uncompressed type
        buffer[12] = width & 255;
        buffer[13] = width >> 8;
        buffer[14] = height & 255;
        buffer[15] = height >> 8;
        buffer[16] = 32;	// pixel size

        if ( !flipVertical )
            buffer[17] = (1 << 5);	// flip bit, for normal top to bottom raster order

        // swap rgb to bgr
        for ( i = imgStart ; i < bufferSize ; i += 4 )
        {
            buffer[i]   = data[i - imgStart + 2];		// blue
            buffer[i+1] = data[i - imgStart + 1];		// green
            buffer[i+2] = data[i - imgStart + 0];		// red
            buffer[i+3] = data[i - imgStart + 3];		// alpha
        }
        //--- Write data
        myFile.write((char *)buffer, bufferSize);

        delete[] buffer;
    }

    myFile.close();
}

void HIST_Write(const std::string fileName, HIST_Header header, const unsigned char *data)
{
    HIST_Write(fileName,            header.histFps,         header.histFrames,
               header.histChannels, header.histChanWidth,   header.histChanMaxVal,
               header.histScale,    header.histLayout,      data);
}

void HIST_Write(const std::string fileName,     const int fps,              const int frameCount,
                const int channels,             const int channelWidth,     const int channelMaxValue,
                const std::string scale,        const std::string pxLayout, const unsigned char *data)
{
    //--- Local vars
    std::ofstream myFile (fileName, std::ios::out | std::ios::binary);

    if(myFile.is_open())
    {
        //--- Write header
        myFile << "HIST:v02:p07:w16";                                                            //Magic number: Version 2, 7 params, 16 bytes wide
        myFile << "FRAM=" << std::setw(11) << std::setfill('0') << (unsigned int)frameCount;     //Frame count
        myFile << "AFPS=" << std::setw(11) << std::setfill('0') << (unsigned int)fps;            //FPS (for playback)
        myFile << "CHAN=" << std::setw(11) << std::setfill('0') << (unsigned int)channels;       //Number of channels
        myFile << "CWDT=" << std::setw(11) << std::setfill('0') << (unsigned int)channelWidth;   //Size of the each channel in pixels
        myFile << "CMAX=" << std::setw(11) << std::setfill('0') << (unsigned int)channelMaxValue;//The max value of channel
        myFile << "SCAL=" << std::setw(11) << std::setfill('_') << scale;                        //The max value of channel
        myFile << "LAYO=" << std::setw(11) << std::setfill('_') << pxLayout;                     //Channel layout

        //--- Write data
        myFile.write((char *)data, frameCount * channels * channelWidth);
    }

    myFile.close();
}
