//------------------------------------------------------------------------------
// Frame slice class
//
// A container which holds 256 c_RGBA variables
//------------------------------------------------------------------------------

#ifndef _SLICE_
#define _SLICE_

struct RGBA_f32_t
{
	float R;
	float G;
	float B;
	float A;
};

struct RGBA_uint32_t
{
	unsigned int R;
	unsigned int G;
	unsigned int B;
	unsigned int A;
};

struct RGBA_t
{
	unsigned char R;
	unsigned char G;
	unsigned char B;
	unsigned char A;
};

class c_HistSlice
{
public:
	//--- Default constructor
	c_HistSlice();

	//--- Increment the brightness of the channel by 1
	void Inc(const char channel, const unsigned char brightness, RGBA_uint32_t* tempSlice);

	//--- Increment the brightness of all channels by 1
	//--- This calls all channels at once, should speed things up
	void IncFast(const unsigned char rBright, const unsigned char gBright, const unsigned char bBright, RGBA_uint32_t* tempSlice);

    //--- Increment the brightness of all channels by 1
	//--- Inline method to save millions and function calls.
	void IncFastInline(const unsigned char rBright, const unsigned char gBright, const unsigned char bBright, RGBA_uint32_t* tempSlice);

	//--- Normalize the data between 0 and 255
	void Normalize(int scaleMode, RGBA_uint32_t* tempSlice);

	//--- Return an element
	unsigned int GetElementRaw(const int index, const char channel);
	unsigned char GetElementFinal(const int index, const char channel);

	//--- Some debug print
	void PrintMaxValues();

private:
	//RGBA_uint32_t   slice[256];
	RGBA_t          result[256];
	unsigned int    maxR;
	unsigned int    maxG;
	unsigned int    maxB;
	unsigned int    maxA;
};

inline void c_HistSlice::IncFastInline(const unsigned char rBright, const unsigned char gBright, const unsigned char bBright, RGBA_uint32_t* tempSlice)
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

#endif //_SLICE_
