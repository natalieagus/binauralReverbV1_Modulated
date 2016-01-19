/*

FDN: a feedback delay network reverberator

*/

#pragma once


#define DELAYUNITSSMALL 3
#define DELAYSPERUNIT 4
#define UNCIRCULATEDTAPSSMALL (2*DELAYUNITSSMALL*DELAYSPERUNIT)
//#define UNCIRCULATEDTAPSSTD 2*DELAYUNITSSTD*DELAYSPERUNIT
#define UNCIRCULATEDTAPSSTD 0
#define EXTRADELAYS 2
#define FLOORUNITS 9 //4 * FLOORUNITS >= FLOORDELAYS
#define DELAYUNITSSTD (4 + EXTRADELAYS + FLOORUNITS)
#define NUMDELAYSSTD (DELAYUNITSSTD * DELAYSPERUNIT)  
#define FLOORDELAYS 36 //PUT min 16, can try more 25, 36, 64 HERE, ensure FLOORDELAYS < 0.5*NUMDELAYSSTD
#define SMOOTHINGDELAYS ((EXTRADELAYS * DELAYSPERUNIT) + (FLOORUNITS * DELAYSPERUNIT) - FLOORDELAYS)
#define NUMTAPSSTD (NUMDELAYSSTD + UNCIRCULATEDTAPSSTD)
#define AUDIOCHANNELS 2
#define SAMPLINGRATEF 44100.0f
#define SOUNDSPEED 340.29f
//#define CENTIMETRESTOMETRES 0.01f
//#define CENTIMETRESTOMETRESSQ CENTIMETRESTOMETRES*CENTIMETRESTOMETRES
#define CHANNELS 8


#import <Accelerate/Accelerate.h>
#import "FirstOrderFilter.h"
#import "SingleTapDelay.h"
#import "Point2d.hpp"
#import "Parameter.hpp"
#import "RoomRayModel.h"
#import "Delays.hpp"


class FDN
{
public:
    
    float directPortionOn = 1.0f;
    float reverbPortionOn = 1.0f;
    
    // constructor
	FDN(void);
    FDN(bool powerSaveMode); // call this with powerSaveMode set to true for a more efficient reverb with lower quality sound.
    ~FDN();
    
    // processes a buffer with numFrames samples.  mono in, stereo out.
    void processIFretlessBuffer(float* input, size_t numFrames, float* outputL, float* outputR );

    void setParameter(Parameter params);
    bool parameterNeedsUpdate;

protected:
    
    Point2d floorBouncePoints[FLOORDELAYS];
    
//    bool bouncepointSet = false;
    void configureRandomModel(float roomSize);
    
    Delays reverbDelayValues[NUMTAPSSTD];
    void shuffleDelays();
    
    RoomRayModel roomRayModel;
    float inputGains[NUMTAPSSTD];
    float outputGains[NUMTAPSSTD];
    
    void configureRoomRayModel();
    
    Parameter parametersFDN;
    Parameter newParametersFDN;
    void setParameterSafe(Parameter params);
    
    float directDelayTimes[2]; //unit = FREQ * seconds
    Point2d roomBouncePoints[NUMTAPSSTD];
    size_t delayTimesChannel[NUMTAPSSTD];
    float directMix;
    
    void setTempPoints();
    Point2d tempPoints[CHANNELS];
    void calculateAdditionalDelays();
    float additionalDelays[8];
    SingleTapDelay reverbDelays[8];
    void addReverbDelay(float* fdnLeft, float*fdnRight);

    void setDelayTimes();
    void setDirectDelayTimes();
    void sortDelayTimes();
    void shortenDelayTimes();
    
    //To handle direct Rays
    SingleTapDelay directRays[2];
    void setDirectSingleTapDelay();
    void processDirectRays(float* input, float* directRaysOutput);
    
    //To do channel angle calculations
    void setDelayChannels();
    void setDirectRayAngles();
    size_t determineChannel(float x, float y);
    size_t angleToChannel(float angleInDegrees);
    float channelToAngle(size_t channel);
    
    //setting tankout of 8 channels
    void processTankOut(float fdnTankOut[CHANNELS]);

    //Binaural filters
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
    
    float inputAttenuation;
	float matrixAttenuation;
    
    void updateRand();
	void resetTapAttenuation(float rt60);
	void resetReadIndices();
    void setHFDecayMultiplier(float fc, float hfMult, float rt60);
    void incrementIndices();
    void processReverb(float* pInput, float* pOutputL, float* pOutputR);
    void resetDelay(int totalDelayTime);

    void initialise(bool powerSaveMode);
    
    bool reverbOn;


    
};

double gain(double rt60, double delayLengthInSamples);