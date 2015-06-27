//------------------------------------------------------------------------------
// Frame slice class
//
// A container which holds 256 c_RGBA variables
//------------------------------------------------------------------------------

#include "histslice.hpp"
#include <math.h>
#include <iostream>

//------------------------------------------------------------------------------
//--- Default constructor
//------------------------------------------------------------------------------
c_HistSlice::c_HistSlice()
{
	for(int i = 0; i < 256; i++)
	{
		result[i].R = 0;
		result[i].G = 0;
		result[i].B = 0;
		result[i].A = 0;
	}

	maxR = 0;
	maxG = 0;
	maxB = 0;
	maxA = 0;
}

//------------------------------------------------------------------------------
//--- Increment the value of the histogram by 1, based on the channel provided.
//--- Parameters:
//--- c          (char) : the pixel channel 'r', 'g', 'b', or 'a'
//--- brightness (uchar): the pixel channel brightness from 0 to 255
//------------------------------------------------------------------------------
void c_HistSlice::Inc(const char c, const unsigned char brightness, RGBA_uint32_t* tempSlice)
{
	if(c == 'r' || c == 'R')
	{
		tempSlice[(int)brightness].R++;
		if(tempSlice[(int)brightness].R > maxR)
			maxR = tempSlice[(int)brightness].R;
	}

	else if(c == 'g' || c == 'G')
	{
		tempSlice[(int)brightness].G++;
		if(tempSlice[(int)brightness].G > maxG)
			maxG = tempSlice[(int)brightness].G;
	}

	else if(c == 'b' || c == 'B')
	{
		tempSlice[(int)brightness].B++;
		if(tempSlice[(int)brightness].B > maxB)
			maxB = tempSlice[(int)brightness].B;
	}

	else if(c == 'a' || c == 'A')
	{
		tempSlice[(int)brightness].A++;
		if(tempSlice[(int)brightness].A > maxA)
			maxA = tempSlice[(int)brightness].A;
	}
}

void c_HistSlice::IncFast(const unsigned char rBright, const unsigned char gBright, const unsigned char bBright, RGBA_uint32_t* tempSlice)
{
    tempSlice[(int)rBright].R++;
    if(tempSlice[(int)rBright].R > maxR)
        maxR = tempSlice[(int)rBright].R;

    tempSlice[(int)gBright].G++;
    if(tempSlice[(int)gBright].G > maxG)
        maxG = tempSlice[(int)gBright].G;

    tempSlice[(int)bBright].B++;
    if(tempSlice[(int)bBright].B > maxB)
        maxB = tempSlice[(int)bBright].B;
}

//------------------------------------------------------------------------------
//--- Normalize all channels between 0 and 255
//--- linear result = (raw value)       * (255.0f / max value)
//---   sqrt result = (sqrt(raw value)) * (255.0f / sqrt(max value))
//---   cube result = (cbrt(raw value)) * (255.0f / cbrt(max value))
//---    log result = (log(raw value))  * (255.0f / log(max value))
//---
//--- Parameters:
//--- scaleMode (int): 0 = linear, 1 = sqrt, 2 = cbrt, 3 = log
//------------------------------------------------------------------------------
void c_HistSlice::Normalize(int scaleMode, RGBA_uint32_t* tempSlice)
{
    //--- Local variables
    float fMaxR = 0;
    float fMaxG = 0;
    float fMaxB = 0;
    float fMaxA = 0;

    if(scaleMode == 0)
    {
        fMaxR = 255.0f / maxR;
        fMaxG = 255.0f / maxG;
        fMaxB = 255.0f / maxB;
        fMaxA = 255.0f / maxA;

        for(int i = 0; i < 256; i++)
        {
            result[i].R = (unsigned char)(tempSlice[i].R * fMaxR);
            result[i].G = (unsigned char)(tempSlice[i].G * fMaxG);
            result[i].B = (unsigned char)(tempSlice[i].B * fMaxB);
            result[i].A = (unsigned char)(tempSlice[i].A * fMaxA);
        }
    }

    else if(scaleMode == 1)
    {
        fMaxR = 255.0f / sqrt((float)maxR);
        fMaxG = 255.0f / sqrt((float)maxG);
        fMaxB = 255.0f / sqrt((float)maxB);
        fMaxA = 255.0f / sqrt((float)maxA);

      for(int i = 0; i < 256; i++)
        {
            result[i].R = (unsigned char)(sqrt((float)tempSlice[i].R) * fMaxR);
            result[i].G = (unsigned char)(sqrt((float)tempSlice[i].G) * fMaxG);
            result[i].B = (unsigned char)(sqrt((float)tempSlice[i].B) * fMaxB);
            result[i].A = (unsigned char)(sqrt((float)tempSlice[i].A) * fMaxA);
        }
    }

    else if(scaleMode == 2)
    {
        fMaxR = 255.0f / cbrtf((float)maxR);
        fMaxG = 255.0f / cbrtf((float)maxG);
        fMaxB = 255.0f / cbrtf((float)maxB);
        fMaxA = 255.0f / cbrtf((float)maxA);

      for(int i = 0; i < 256; i++)
        {
            result[i].R = (unsigned char)(cbrtf((float)tempSlice[i].R) * fMaxR);
            result[i].G = (unsigned char)(cbrtf((float)tempSlice[i].G) * fMaxG);
            result[i].B = (unsigned char)(cbrtf((float)tempSlice[i].B) * fMaxB);
            result[i].A = (unsigned char)(cbrtf((float)tempSlice[i].A) * fMaxA);
        }
    }

    else if(scaleMode == 3)
    {
        fMaxR = 255.0f / log((float)maxR);
        fMaxG = 255.0f / log((float)maxG);
        fMaxB = 255.0f / log((float)maxB);
        fMaxA = 255.0f / log((float)maxA);

      for(int i = 0; i < 256; i++)
        {
            result[i].R = (unsigned char)(log((float)tempSlice[i].R) * fMaxR);
            result[i].G = (unsigned char)(log((float)tempSlice[i].G) * fMaxG);
            result[i].B = (unsigned char)(log((float)tempSlice[i].B) * fMaxB);
            result[i].A = (unsigned char)(log((float)tempSlice[i].A) * fMaxA);
        }
    }
}

//------------------------------------------------------------------------------
//--- Get an element from the normalized data
//------------------------------------------------------------------------------
unsigned char c_HistSlice::GetElementFinal(const int index, const char channel)
{
    if(channel == 'r' || channel == 'R')
        return result[index].R;

    else if(channel == 'g' || channel == 'G')
        return result[index].G;

    else if(channel == 'b' || channel == 'B')
        return result[index].B;

    else if(channel == 'a' || channel == 'A')
        return result[index].A;

    else
        return 0;
}

//--- Print some debug info
void c_HistSlice::PrintMaxValues()
{
    std::cout << "Maximum values [R, G, B] = " << maxR << ", " << maxG << ", " << maxB << "\n";
}
