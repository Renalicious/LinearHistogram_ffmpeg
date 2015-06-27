//------------------------------------------------------------------------------
// Histogram class
//
// Processes each frame and stores a histogram slice
//------------------------------------------------------------------------------

#ifndef _HISTO_
#define _HISTO_

#include <string>
#include "histslice.hpp"

class c_Histogram
{
public:
    c_Histogram();
	c_Histogram(const int frameCount, const int width, const int height, const int channels, const int scaleMode);
	c_Histogram(const int frameCount, const int frameSize, const int channels, const int scaleMode);
	~c_Histogram();

	//--- Initialize histogram
	void Initialize(const int frameCount, const int width, const int height, const int channels, const int scaleMode);
	void Initialize(const int frameCount, const int frameSize, const int channels, const int scaleMode);

	//--- Process new frame and set current frame index + 1
	void ProcessFrame(const char* frameData);
	void ProcessFrame(const unsigned char* frameData);

	//--- Normalize all values between 0 and 255, using either linear or square root method
	void NormalizeAll(bool useLinear=false);

    //--- Return data as one more n * 256 RGBA image
	unsigned char* CreateImage(bool includeAlpha, bool isRGBA = true);

	//--- Return frame as a 2D image
	unsigned char* CreateImage2D(const int frame, const int paintMode, bool halfHeight, bool includeAlpha, bool isRGBA = true);

	//--- Retrn custom histogram file data
	unsigned char* CreateHist();

	std::string ScaleMode();

	//--- Operator =
	void operator=(const c_Histogram &newHist);

private:
    //--- Init
    void InitHist(const int frameCount, const int frameSize, const int chans, const int scaleMode);
    void ClearSlice();

    RGBA_uint32_t* rawSlice;
	c_HistSlice* histogram;
	int totFrames;
	int curFrame;
	int frameSize;
	int channels;
	int scaleMode;
	bool isInitialized;
};

#endif //_HISTO_
