/*

FDN: a feedback delay network reverberator

*/

#pragma once

#define DELAYUNITSSTD 8
#define DELAYUNITSSMALL 3
#define DELAYSPERUNIT 4
#define UNCIRCULATEDTAPSSMALL 2*DELAYUNITSSMALL*DELAYSPERUNIT
//#define UNCIRCULATEDTAPSSTD 2*DELAYUNITSSTD*DELAYSPERUNIT
#define UNCIRCULATEDTAPSSTD 0
#define NUMDELAYSSTD (DELAYUNITSSTD * DELAYSPERUNIT)
#define NUMTAPSSTD NUMDELAYSSTD + UNCIRCULATEDTAPSSTD
#define AUDIOCHANNELS 2
#define SAMPLINGRATEF 44100.0f
#define SOUNDSPEED 340.29f
#define CENTIMETRESTOMETRES 0.01f
#define CENTIMETRESTOMETRESSQ CENTIMETRESTOMETRES*CENTIMETRESTOMETRES
#define CHANNELS 8
#define ROOMSIZE 3000 //30 metres max
#define RADIUSOFHEAD 8 //8cm radius of head
#import <Accelerate/Accelerate.h>
#import "FirstOrderFilter.h"
#import "SingleTapDelay.h"
#import "Point2d.hpp"

class FDN
{
public:
    // constructor
	FDN(void);
    FDN(bool powerSaveMode); // call this with powerSaveMode set to true for a more efficient reverb with lower quality sound.
    ~FDN();
    
    // processes a buffer with numFrames samples.  mono in, stereo out.
    void processIFretlessBuffer(float* input, size_t numFrames, float* outputL, float* outputR );

    //To set all the settable variable in FDN from UI
    // size must be between 1 and 10, unit in metres
    void setRoomSize(float size);
    void setWidthRatio(float size);
    void setListenerLoc(Point2d loc);
    void setSoundSourceLoc(Point2d loc);
    // tone must be between 0 and 1;  1 is no high frequency attenuation.  0 is max attenuation
    void setRT60(float RT60);

    bool listenerLocationChanged;
    bool soundLocationChanged;
    bool RT60Changed;
    bool roomSizeChanged;
    bool widthRatioChanged;
    

protected:
    

    float listenerX, listenerY, soundSourceX, soundSourceY;
    //Locations of listener and sound source, as well as room size (in metres, square)
    //Assumption: X location is fixed, both sound source and listener are in the middle of the room
    float roomSizeCM = 500; //min 3 metres, max 10 metres, a square room
    float roomWidthCM, roomHeightCM;
    //Take bottom left hand corner of the room as x[0] and y[0]
    Point2d soundSourceLoc;
    Point2d listenerLoc;
    Point2d listenerLocLeftEar;
    Point2d listenerLocRightEar;
    float directDelayTimes[2]; //unit = FREQ * seconds
    float delayTimesNew[NUMTAPSSTD]; //unit = FREQ * seconds
    //The location of all bounce points in the room, x and y coordinate
    Point2d roomBouncePoints[NUMTAPSSTD];
    //Stores the channel of each delay time of the same index
    size_t delayTimesChannel[NUMTAPSSTD];
   // float directMix = 0.5f; // set this with a UISlider that goes from 0 to 1.
   // float reverbMix = sqrt(1.0 - directMix*directMix);
    float directMix, reverbMix;
    
    void setTempPoints();
    float tempPoints[CHANNELS*2];
    void calculateAdditionalDelays();
    float additionalDelays[8];
    SingleTapDelay reverbDelays[8];
    void addReverbDelay(float* fdnLeft, float*fdnRight);
    
    float roomCeilingCM = 300.0f; //constant room ceiling of 3 metres tall
    float directGain = 0.5f; // constant, set by listening (until we can think of a formula for it)
    float reverbGain = 0.5f; // constant, set by listening
    float directDistanceInMetres;
    float roomSA;
    void recalculateReverDirectMix();
    void setGainConstants();
    
    //setting width and height
    float widthPct = 0.5; //50-50 is square
    bool widthPctChanged;
    float widthPctNew = 0.5;
    void setWidthRatioSafe(float size);
    
    //To set listener and sound source locations
    void setListenerLocSafe(float* loc);
    void setSoundSourceLocSafe(float* loc);
    bool listenerNeedsUpdate = false;
    bool soundSourceNeedsUpdate = false;
    Point2d soundSourceLocNew;
    Point2d listenerLocNew;
    void setListenerOrSoundSourceLocSafe();

    //To calculate delays
    //Method to calculate delay times based in randomised 72 points in the room, output to delayTimes[NUMTAPSSTD]
    void setDelayTimes();
    //Method to calculate delay times based on 2 direct signals
    void setDirectDelayTimes();
    //Method to generate random 72 points on the wall
    void setRoomBouncePoints();
    //Method to get random for wall points
    int getRandom();
    //Method to calculate line distance
    float getDistance(float x1, float x2, float y1, float y2);
    //Method to sort delay times
    void sortDelayTimes();
   // void quickSort(int arr[], int arr2[], int left, int right);
    
    //To handle direct Rays
    SingleTapDelay directRays[2];
    void setSingleTapDelay();
    void processDirectRays(float* input, float* directRaysOutput);
    
    //To set RT60
    void setRT60Safe(float RT60);
    bool RT60NeedsUpdate;
    float newRT60;
    
    //To do channel angle calculations
    void setDelayChannels();
    void setDirectRayAngles();
    size_t determineChannel(float x, float y);
    size_t angleToChannel(float angleInDegrees);
    float channelToAngle(size_t channel);
    
    //setting tankout of 8 channels
    void processTankOut(float fdnTankOut[CHANNELS]);

    //To do binaural filters
    FirstOrderFilter leftEarFilter[CHANNELS];
    FirstOrderFilter rightEarFilter[CHANNELS];
    FirstOrderFilter directRayFilter[2];
    void setFilters();
    //Direct ray is inclusive in one of the channels, so there's no need to have another channel angle for this
    float directRayAngles[2];
    //process the fdntankout[channels]
    void filterChannels(float fdnTankOut[CHANNELS], float directRay[2], float fdnTankOutLeft[CHANNELS], float fdnTankOutRight[CHANNELS]);
    
 
    // delay buffers
	//float delayBuffers [MAXBUFFERSIZE];
    float* delayBuffers;
    int delayTimes [NUMTAPSSTD];
    int totalDelayTime;
    int numDelays, delayUnits, numTaps, numUncirculatedTaps;
	//float feedbackAttenuation[NUMDELAYSSTANDARD];
    //int numDelays, delayUnits, numIndices;

	// read / write indices
    // we put all the indices in one array, then access them via pointers.
    // Keeping all indices in one array allows us to increment all of them with a single
    // optomised vector-scalar addition.
    float* rwIndices[NUMTAPSSTD];
    float* startIndices[NUMTAPSSTD];
    float* endIndices[NUMTAPSSTD];
    long samplesUntilNextWrap;
    //int writeIndex;
	//int* outTapReadIndices;
    //int* endIndex;

	// delay times
	//int delayTimes[NUMDELAYS];

    // variables for generating random numbers
	int rand, randomSeed;

	// we keep buffer one sample of input in an array so that we can keep a pointer to each input for
	// fast summing of feedback output.  This avoids calculating the array index of the 
	// write pointer for every source.  We'll sum the inputs here and then copy only once from here
	// to the write pointer location.
	float inputs[NUMDELAYSSTD];
	float outputsPF[NUMTAPSSTD];
    float outputsAF[NUMTAPSSTD];
    float outTapSigns[NUMTAPSSTD];
	float feedbackTapGains[NUMTAPSSTD];
    //float outTapTemp[NUMDELAYSSTANDARD][OUTPUTTAPSPERDELAY];
    //int fbTapIdxBase[NUMDELAYSSTANDARD];
	//float predelay;
    
    // temp variables for mixing feedback
    float ppxxV [DELAYUNITSSTD];
    float xxppV [DELAYUNITSSTD];
    float pnxxV [DELAYUNITSSTD];
    float xxpnV [DELAYUNITSSTD];
    
    // filter coefficients
    //
    // high-shelf filters for HF attenuation inside the tank
    float b0[NUMDELAYSSTD];
    float b1[NUMDELAYSSTD];
    float a1[NUMDELAYSSTD];
    float z1[NUMDELAYSSTD]; //DF2
    float t[NUMDELAYSSTD];
    float z1a[NUMDELAYSSTD]; //DF1
    float z1b[NUMDELAYSSTD]; //DF1
    int one_i = 1;
    float zero_f = 0.0f;
    
    // filter on dry input
    float f0b0, f0b1, f0a1, f0z1;
    
    // filter on reverb input
    float f1b0, f1b1, f1a1, f1z1;
    
    float inputAttenuation;
	float matrixAttenuation;
   // float wetPercent, dryPercent;
    float mix1, mix2;
    float tone;
   // float roomSize;
    
    void updateRand();
	//void resetFeedbackAttenuation(float rt60);
	void resetTapAttenuation(float rt60);
	void resetReadIndices();
    void setHFDecayMultiplier(float fc, float hfMult, float rt60);
    void incrementIndices();
    void processReverb(float* pInput, float* pOutputL, float* pOutputR);
    void processTone(float* pInput, float* pOutput);
    void resetDelay(int totalDelayTime);
    void setDryHighShelfAttenuation(float fc, float g);
    void setDryLowPassCutoff(float fc);
    void setHighPassCutoff(float fc);
    //void resetEarlyReflections(float erStart, float erEnd);
    void setRoomSizeSafe(float size); //unit is in CM
    void initialise(bool powerSaveMode);
    void randomPermutation(int* a, int length, int channels);
    void randomPermutation2channelGrouped(int* a, int length);
    void shiftPermutation1channel(int* a, int length, int shift);
    void cyclicGroupPermutation2channel(int* a, int length, int shift);
    void randomPermutation1Channel(int* a, int length, int audioChannels);
    
    bool roomSizeNeedsUpdate;
    bool toneFilterOn, reverbOn, useHPF, lowPassTone;
    float newRoomSize = 450;
    float minTone, maxTone;

    
};

double gain(double rt60, double delayLengthInSamples);