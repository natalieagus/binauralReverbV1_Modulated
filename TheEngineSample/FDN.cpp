#include "FDN.h"
#include <math.h>
#include <cstdlib>
#include <cstring>
#include <assert.h>

#include <random>

//Initialize condition:
//Room size 5x5 metres
//RT60 value 0.7 seconds
int iii = 0;

// this is the function we call to process the audio from iFretless.
void  FDN::processIFretlessBuffer(float* input, size_t numFrames, float* outputL, float* outputR)
{
    // bypass reverb
    //for (int i = 0; i < numFrames; i++) {
    //    outputL[i] = outputR[i] = ioBuffer[i];
    //}
    // return;
    
    if (parameterNeedsUpdate){
        setParameterSafe(newParametersFDN);
        parameterNeedsUpdate = false;
        for (size_t i = 0; i < numFrames; i++){
            processReverb(&zero_f, outputL + i, outputR + i);
        }
    }
    
    else {
        if (reverbOn){
            for (UInt32 i = 0; i < numFrames; i++){
                float* inputPtr = input + i;
                processReverb(inputPtr, outputL + i, outputR + i);
            }

            
        }
        else{
            // spread the signal to the left and right channels
            memcpy(outputL, input, numFrames * sizeof(Float32));
            memcpy(outputR, input, numFrames * sizeof(Float32));
        }
    }
}


//The structure of the unit is different from the paper. In this case, there's input from previous unit + dry input which will be
//applied to a delay. Output taps are placed on this delay.
//Then, the output from this delay (both dry and previous unit) will be processed back to 4x4 mixing matrix.
//Apply high-shelf first order filter to this, somehow this filter depends on delays


inline void FDN::processReverb(float* pInput, float* pOutputL, float* pOutputR)
{
    
    
    // copy the filtered input so that we can process it without affecting the original value
	float xn = *pInput;

    // attenuate the input
    xn *= inputAttenuation;
    
    float fdnTankOutsNew[CHANNELS] = {0.0f, 0.0f, 0.0f, 0.0f,0.0f, 0.0f, 0.0f, 0.0f};
    
    float directRaysOutput[2] = { 0.0f, 0.0f };
    
    // copy output taps to pre-filtered output buffer
    //rwIndices are output taps pointer
    vDSP_vgathra (const_cast<const float**>(rwIndices), 1, outputsPF, 1, numTaps);

    // scale the output taps according to the feedBackTapGains
    vDSP_vmul(feedbackTapGains, 1, outputsPF, 1, outputsPF, 1, numTaps);
  
    processDirectRays(pInput, directRaysOutput);
    processTankOut(fdnTankOutsNew);
  
    float fdnTankOutLeft[CHANNELS] = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,};
    float fdnTankOutRight[CHANNELS] = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,};
    filterChannels(fdnTankOutsNew, directRaysOutput, fdnTankOutLeft, fdnTankOutRight);

    float reverbOut[2] = {0.0f, 0.0f};
    
    //add another delay here, before mixing them together, for left tank out, add delays on channel 0-3, for right tank out, add delays on channel 4-7
    addReverbDelay(fdnTankOutLeft, fdnTankOutRight);
    vDSP_sve(fdnTankOutLeft, 1, &reverbOut[0], CHANNELS);
    vDSP_sve(fdnTankOutRight, 1, &reverbOut[1], CHANNELS);

    *pOutputL = (directRaysOutput[0]*directMix - reverbOut[0]*reverbMix);
    *pOutputR = (directRaysOutput[1]*directMix - reverbOut[1]*reverbMix);
    
    // apply a first order high-shelf filter to the feedback path
    //
    // This filter structure is direct form 2 from figure 14 in section 1.1.6
    // of Digital Filters for Everyone by Rusty Alred, second ed.
    //
    // t = outputsPF + (a1 * z1);
    vDSP_vma (a1, 1, z1, 1, outputsPF + numUncirculatedTaps, 1, t, 1, numDelays);
    // outputsAF = b0*t + b1*z1;
    vDSP_vmma(b0, 1, t, 1, b1, 1, z1, 1, outputsAF + numUncirculatedTaps, 1, numDelays);
    // z1 = t;
    memcpy(z1, t, numDelays * sizeof(float));
    
    // apply the matrix attenuation before mixing
    vDSP_vsmul (outputsAF+numUncirculatedTaps, 1, &matrixAttenuation, outputsAF+numUncirculatedTaps, 1, numDelays);
    
    // process mixing matrices
    //if (DELAYSPERUNIT == 4) {
    /*
     // The following block of code is replaced by vDSP vectorized code in the section below
     //
     for (i = 0; i < NUMDELAYS; i += DELAYSPERUNIT){
     ppxx = outputsAF[i + 0] + outputsAF[i + 1];
     xxpp = outputsAF[i + 2] + outputsAF[i + 3];
     pnxx = outputsAF[i + 0] - outputsAF[i + 1];
     xxpn = outputsAF[i + 2] - outputsAF[i + 3];
     fbIn[i + 0] = ppxx + xxpp;
     fbIn[i + 1] = ppxx - xxpp;
     fbIn[i + 2] = pnxx + xxpn;
     fbIn[i + 3] = pnxx - xxpn;
     }
     */
    
   // for (i = 0; i < NUMDELAYS; i += DELAYSPERUNIT){
    //    j = (i + 4) % NUMDELAYS;
    //    ppxx = outputsAF[i + 0] + outputsAF[i + 1];
    //    xxpp = outputsAF[i + 2] + outputsAF[i + 3];
    //    pnxx = outputsAF[i + 0] - outputsAF[i + 1];
    //    xxpn = outputsAF[i + 2] - outputsAF[i + 3];
    //
    //  Here's a vectorised version of the code in the comment above:
    vDSP_vadd(outputsAF + 0 + numUncirculatedTaps, 4, outputsAF + 1 + numUncirculatedTaps, 4, ppxxV, 1, delayUnits);
    vDSP_vadd(outputsAF + 2 + numUncirculatedTaps, 4, outputsAF + 3 + numUncirculatedTaps, 4, xxppV, 1, delayUnits);
    vDSP_vsub(outputsAF + 0 + numUncirculatedTaps, 4, outputsAF + 1 + numUncirculatedTaps, 4, pnxxV, 1, delayUnits);
    vDSP_vsub(outputsAF + 2 + numUncirculatedTaps, 4, outputsAF + 3 + numUncirculatedTaps, 4, xxpnV, 1, delayUnits);
    //
    //  inputs[j + 0] += ppxx + xxpp;
    //  inputs[j + 1] += ppxx - xxpp;
    //  inputs[j + 2] += pnxx + xxpn;
    //  inputs[j + 3] += pnxx - xxpn;
    // }
    // Here's the vectorised version:
    vDSP_vadd(ppxxV, 1, xxppV, 1, inputs+0, 4, delayUnits);
    vDSP_vsub(ppxxV, 1, xxppV, 1, inputs+1, 4, delayUnits);
    vDSP_vadd(pnxxV, 1, xxpnV, 1, inputs+2, 4, delayUnits);
    vDSP_vsub(pnxxV, 1, xxpnV, 1, inputs+3, 4, delayUnits);
    //} else { // if DELAYSPERUNIT != 4, i.e. 2x2 mixing
    
    // mixing in 2x2 matrices
    //for (i = 0; i < NUMDELAYS; i+= 2) {
    //    float a = outputsAF[i];
    //    float b = outputsAF[i+1];
    //    inputs[i] = a + b;
    //    inputs[i+1] = a - b;
    //}
    // vectorized mixing of 2x2 matrices
    //    vDSP_vadd(outputsAF, 2, outputsAF + 1, 2, inputs, 2, DELAYUNITS);
    //    vDSP_vsub(outputsAF, 2, outputsAF + 1, 2, inputs + 1, 2, DELAYUNITS);
    //}
    
    // mix new input together with feedback input
    vDSP_vsadd(inputs, 1, &xn, inputs, 1, numDelays);
    
    // feedback the mixed audio into the tank, shifting the feedback path over to the next unit
    size_t j;//,k;
    for (j = DELAYSPERUNIT; j < numDelays; j++) *(rwIndices[j+ numUncirculatedTaps]) = inputs[j-DELAYSPERUNIT];
    for (j = 0;j<DELAYSPERUNIT; j++) *(rwIndices[j+ numUncirculatedTaps]) = inputs[j+numDelays-DELAYSPERUNIT];
    //for (j = 0; j < numDelays; j++) *(rwIndices[j+ numUncirculatedTaps]) = inputs[j];
    
    incrementIndices();
}

inline void FDN::addReverbDelay(float* fdnLeft, float*fdnRight){
    for(int i = 0; i < CHANNELS/2; i++){
        //left , delay channels 0-3
        fdnLeft[i] = reverbDelays[i].process(fdnLeft[i]);
    }
    for (int i = CHANNELS/2 ; i < CHANNELS; i++){
        //right, delay channels 4-7
        fdnRight[i] = reverbDelays[i].process(fdnRight[i]);
    }
}

inline void FDN::processDirectRays(float* input, float* directRaysOutput){
    directRaysOutput[0] = directRays[0].process(*input);
    directRaysOutput[1] = directRays[1].process(*input);
}


inline void FDN::processTankOut(float fdnTankOut[CHANNELS]){
    for (size_t i = 0; i < NUMTAPSSTD; i++){
        size_t channel = delayTimesChannel[i];
        fdnTankOut[channel] += outputsPF[i];
    }
}

inline void FDN::filterChannels(float fdnTankOut[8], float directRay[2], float fdnTankOutLeft[8], float fdnTankOutRight[8]){
    //Filter left & right ear
    for (size_t i = 0; i < CHANNELS; i++){
        fdnTankOutLeft[i] = leftEarFilter[i].process(fdnTankOut[i]);
        fdnTankOutRight[i] = rightEarFilter[i].process(fdnTankOut[i]);
    }
    //Filter direct rays
    directRay[0] = directRayFilter[0].process(directRay[0]);
    directRay[1] = directRayFilter[1].process(directRay[1]);
}

FDN::FDN(void) 
{
    bool powerSaveByDefault = false;
    initialise(powerSaveByDefault);
}

FDN::FDN(bool powerSaveMode)
{
    initialise(powerSaveMode);
}

void FDN::initialise(bool powerSaveMode){
    
    if (powerSaveMode){
        numDelays = DELAYUNITSSMALL*DELAYSPERUNIT;
        delayUnits = DELAYUNITSSMALL;
        numUncirculatedTaps = UNCIRCULATEDTAPSSMALL;
        //printf("powersave");
    }
    else {
        numDelays = DELAYUNITSSTD*DELAYSPERUNIT;
        delayUnits = DELAYUNITSSTD;
        numUncirculatedTaps = UNCIRCULATEDTAPSSTD;
    }
    
    numTaps = numDelays + numUncirculatedTaps;
 
    tone = 1.0;
    delayBuffers = NULL;
    
    parametersFDN =  Parameter();
    setParameterSafe(parametersFDN);

}

void FDN::resetReadIndices(){
    // set start, end indices for the first delay in the feedback tank
    rwIndices[numUncirculatedTaps] = startIndices[numUncirculatedTaps] = (float*)delayBuffers;
    endIndices[numUncirculatedTaps] = startIndices[numUncirculatedTaps] + delayTimes[numUncirculatedTaps];
    
    for (int i = 0; i < numTaps;i++) assert(delayTimes[i] > 0);
    
    // set start / end indices for the second feedback delay tap onwards
    for (int i = numUncirculatedTaps + 1; i < numTaps; i++){
        rwIndices[i] = startIndices[i] = startIndices[i-1] + delayTimes[i-1];
        endIndices[i] = startIndices[i] + delayTimes[i];
    }
    
    // set times for the uncirculated taps
    int firstMultiTappedDelayIdx = numUncirculatedTaps;
    for (int i = 0; i < numUncirculatedTaps; i++){
        // find the index of the delay line on which this tap operates
        int delayLineIdx = (i%numDelays)+firstMultiTappedDelayIdx;
       // printf("delayLine Idx: %d\n", delayLineIdx);
        // uncirculated delay times can not be longer than the circulated time on the same delay line
        if(delayTimes[i] > delayTimes[delayLineIdx]){
            int old = delayTimes[i];
            // usually, the taps time won't exceed the delay time but the longest one might so, we generate a random time for it.
            updateRand();
            int delayLineLength = delayTimes[delayLineIdx];
            delayTimes[i] = (rand % (delayLineLength-1)) + 1;
              printf("ERROR! Delay uncirculated > delay circulated. Need to sort. delayTimes[%d]: %d delayTimes[delayLineidx]: %d, new delay line: %d\n", i, old, delayTimes[delayLineIdx], delayTimes[i]);
        }
        
        startIndices[i] = startIndices[delayLineIdx];
        endIndices[i] = endIndices[delayLineIdx];
        rwIndices[i] = endIndices[i] - delayTimes[i];
    }
    
    samplesUntilNextWrap = 0; // Initially, we trigger an unncecssary wrap operation to calculate the number of samples until the next actual wrap
}

// see: http://software.intel.com/en-us/articles/fast-random-number-generator-on-the-intel-pentiumr-4-processor/
// rather than returning an output, this function updates a class variable so that we only have to generate 1 random number for each sample.
inline void FDN::updateRand()
{
	randomSeed = (214013 * randomSeed + 2531011);
	rand = (randomSeed >> 16) & 0x7FFF;
}


void FDN::resetTapAttenuation(float rt60){
	for (int i = 0; i < numTaps; i++){
        updateRand();
        feedbackTapGains[i] = gain(rt60, delayTimes[i]);
	}
    
    
    // randomly set half of the signs in each channel negative.
    int numNegativesL = 0;
    int numNegativesR = 0;
    
    // start all positive
    for (int i = 0; i < numTaps; i++) outTapSigns[i] = 1.0;
    
    // set half of the left channel signs negative
    while (numNegativesL < numTaps / 4) {
        int nextIdx = (2*rand) % numTaps;
        updateRand();
        if (outTapSigns[nextIdx] > 0.0f) {
            outTapSigns[nextIdx] = -1.0f;
            numNegativesL++;
        }
    }
    
    // set half of the right channel signs negative
    while (numNegativesR < numTaps / 4) {
        int nextIdx = ((2*rand)+1) % numTaps;
        updateRand();
        if (outTapSigns[nextIdx] > 0.0f) {
            outTapSigns[nextIdx] = -1.0f;
            numNegativesR++;
        }
    }
}

FDN::~FDN(){
    if (delayBuffers) delete[] delayBuffers;
}

void FDN::resetDelay(int totalDelayTime)
{
    if (delayBuffers) delete[] delayBuffers;
    delayBuffers = new float[totalDelayTime];
    this->totalDelayTime = totalDelayTime;
    
	// clear the buffers
	memset(delayBuffers, 0, sizeof(float) * totalDelayTime);
	memset(inputs, 0, sizeof(float) * numDelays);
	memset(outputsPF, 0, sizeof(float) * numTaps);
    memset(outputsAF, 0, sizeof(float) * numTaps);
    
    memset(z1, 0, sizeof(float) * numDelays); //DF2
    memset(z1a, 0, sizeof(float) * numDelays); //DF1
    memset(z1b, 0, sizeof(float) * numDelays); //DF1
    
    f0z1 = f1z1 = 0.0f;
	randomSeed = 0;
}

inline void FDN::incrementIndices(){
    for (int i = 0; i < numTaps; i++) {
        float** rwIndex = rwIndices + i;
        // increment
        (*rwIndex)++;
        // wrap around back to the beginning if we're off the end
        //if ((*rwIndex) >= endIndices[i]) (*rwIndex) = startIndices[i];
    }
    samplesUntilNextWrap--;
    
    // if any pointer is at the end of the buffer, check all pointers and wrap back to the beginning where necessary
    if (samplesUntilNextWrap <= 0) {
        samplesUntilNextWrap = LONG_MAX;
        for (long i = 0; i< numTaps; i++){
            float** rwIndex = rwIndices + i;
            float** endIndex = endIndices + i;
            
            // wrap all pointers that need to be wrapped
            if ((*rwIndex) >= (*endIndex)) (*rwIndex) = startIndices[i];
            
            // find the distance until the next pointer needs to wrap
            long iThDistanceToEnd = (*endIndex) - (*rwIndex);
            if (iThDistanceToEnd < samplesUntilNextWrap) samplesUntilNextWrap = iThDistanceToEnd;
        }
    }
}

// Multiplies the decay rate for high Frequencies
// hfMult >= 1
// 0 <= fc < ~18000
//
// 1 means HF decays at the same rate as mid frequencies, 2 means HF decay twice as fast,
// 3 is three times as fast, etc.
//
//
// the formulae for calculating the filter coefficients are from Digital Filters for Everyone,
// second ed., by Rusty Alred.  Section 2.3.10: Shelf Filters
void FDN::setHFDecayMultiplier(float fc, float hfMult, float rt60){
    double g[numDelays];
    double D[numDelays];
    double gamma;
    
    fc = 8000.0f;

    gamma = tan((M_PI * fc) / double(SAMPLINGRATEF));
    
    for (int i = 0; i < numDelays; i++){
        // set the filter gains
        //
        // here we find the gain already applied to each delay line
        double broadbandGain = gain(rt60, delayTimes[i+numUncirculatedTaps]);
        double desiredHFGain = gain(rt60 / hfMult, delayTimes[i+numUncirculatedTaps]);
        // and now find how much additional filter gain to apply for the given increase in HF decay speed
        g[i] = desiredHFGain / broadbandGain;
        
        // just a temp variable
        D[i] = 1.0 / ((g[i] * gamma) + 1.0);
        
        // set the filter coefficients
        b0[i] = g[i] * (gamma + 1.0) * D[i];
        b1[i] = g[i] * (gamma - 1.0) * D[i];
        // Rusty Allred omits the negative sign in the next line.  We use it to avoid a subtraction in the optimized filter code.
        a1[i] = -1.0 * ((g[i] * gamma) - 1.0) * D[i];
    }
}

// fc is in the interval [0,1]
void FDN::setHighPassCutoff(float fc){
    double D, gamma;
    
    gamma = tan(M_PI * fc);
    
    // first order butterworth low-pass
    D = gamma + 1.0;
    f1b0 = 1.0f / D;
    f1b1 = -1.0f / D;
    f1a1 = (gamma - 1.0 ) / D;
    
    f1z1 = 0.0f; // set the filter delay element to zero
}

// computes the appropriate feedback gain attenuation
// to get a decay envelope with the specified RT60 time (in seconds)
// from a delay line of the specified length.
//
// This formula comes from solving EQ 11.33 in DESIGNING AUDIO EFFECT PLUG-INS IN C++ by Will Pirkle
// which is attributed to Jot, originally.
double gain(double rt60, double delayLengthInSamples) {
    return pow(M_E, (-3.0 * delayLengthInSamples ) / (rt60 * SAMPLINGRATEF) );
}

void FDN::setParameter(Parameter params){
    newParametersFDN = params;
    parameterNeedsUpdate = true;
}

void FDN::setParameterSafe(Parameter params){
    parametersFDN = newParametersFDN;
    reverbOn = parametersFDN.roomSize > 0.05;
    
    float hpfCutoffHZ = 85.0;
    useHPF = true;
    minTone = 0.0025;
    maxTone = 0.49f;
    lowPassTone = true;
    
    randomSeed = 0; // reset the seed every time for consistent results
    
    // since we copy one input channel into many delay lines and tap each one several times for output,
    // we need to attenuate the input to compensate for the increased volume.
    inputAttenuation = 1.0/sqrt((float)numTaps);
    // this is required to keep the mixing matrices unitary
    matrixAttenuation = 1.0/sqrt((float)DELAYSPERUNIT);
    
    //Must be in this order
    setRoomBouncePoints();
    setDelayChannels();
    setDelayTimes();
    setTempPoints();
    calculateAdditionalDelays();
    setDirectDelayTimes();
    setDirectRayAngles();
    
    for (int i = 0; i <NUMTAPSSTD; i++){
        delayTimes[i] = delayTimesNew[i];
    }
    
    setSingleTapDelay();
    setFilters();
    setGainConstants();
    
    int totalDelayTime = 0;
    for(int i = numUncirculatedTaps; i < numTaps; i++) totalDelayTime += delayTimes[i];
    resetDelay(totalDelayTime);
    resetReadIndices();
    
    
    // set high-frequency attenuation
    setHFDecayMultiplier(8000.0f,1.5f,parametersFDN.RT60);
    
    // cutoff low frequencies below HPFcutoff
    setHighPassCutoff(hpfCutoffHZ / (SAMPLINGRATEF*0.5));
    
    resetTapAttenuation(parametersFDN.RT60);
}

inline int FDN::getRandom(){
    updateRand();
    int r = rand % 2;
    return r;
}

//Fix this
inline void FDN::setRoomBouncePoints(){
    std::random_device rd;     // only used once to initialise (seed) engine
    std::mt19937 rng(rd());    // random-number engine used (Mersenne-Twister in this case)
    std::uniform_int_distribution<int> uniW(0,parametersFDN.roomWidthCM); // guaranteed unbiased
    std::uniform_int_distribution<int> uniH(0,parametersFDN.roomHeightCM);
    
    for (int i = 0; i<NUMTAPSSTD; i++){
        float x = (float) uniW(rng);
        float y = (float) uniH(rng);
        
        roomBouncePoints[i] = Point2d(x,y);
    }
    
    
    int numPointsLeftWall = numTaps*parametersFDN.roomWidthCM/(parametersFDN.roomWidthCM+parametersFDN.roomHeightCM)/2;
    int numPointsRightWall = numPointsLeftWall;
    int numPointsFrontWall = (numTaps- (numPointsLeftWall*2))/2;
    int numPointsBackWall = numTaps - (numPointsFrontWall + numPointsLeftWall + numPointsRightWall);
    
    for (int i = 0; i < numPointsLeftWall; i++){
        roomBouncePoints[i].x = 0.0f;
    }
    
    for(int i = numPointsLeftWall; i< numPointsLeftWall+numPointsRightWall; i++){
        roomBouncePoints[i].x =parametersFDN.roomWidthCM;
    }
    for (int i = numPointsLeftWall+numPointsRightWall; i< numPointsLeftWall+numPointsRightWall+numPointsFrontWall; i++){
        roomBouncePoints[i].y = parametersFDN.roomHeightCM;
    }
    
    for (int i = numPointsFrontWall+numPointsLeftWall+numPointsRightWall; i<numTaps; i++){
        roomBouncePoints[i].y = 0;
    }
}

inline float FDN::getDistance(float x1, float y1, float x2, float y2){
    float xDist = x2 - x1;
    float yDist = y2 - y1;
    xDist = pow(xDist, 2);
    yDist = pow(yDist, 2);
    return sqrt(xDist + yDist);
}


inline void FDN::setDelayTimes(){
    
    for (int i = 0; i<NUMTAPSSTD; i++){
        
        //if on the left of listener, use left ear x_pts < x_listener
        //if on the right of listener, use right ear x_pts >= x_listener
        size_t ch = delayTimesChannel[i];
        
        if (ch < CHANNELS/2){
            //use right ear
            float d1 = roomBouncePoints[i].distance(parametersFDN.soundSourceLoc);
            float d2 = roomBouncePoints[i].distance(parametersFDN.listenerLocRightEar);
            float td = (d1 + d2) * CENTIMETRESTOMETRES;
            float delaySeconds = td / SOUNDSPEED;
            delayTimesNew[i] = delaySeconds * SAMPLINGRATEF;
        }
        else{
            //use left ear
            float d1 = parametersFDN.soundSourceLoc.distance(roomBouncePoints[i]);
            float d2 = parametersFDN.listenerLocLeftEar.distance(roomBouncePoints[i]);
            float td = (d1 + d2) * CENTIMETRESTOMETRES;
            float delaySeconds = td / SOUNDSPEED;
            delayTimesNew[i] = delaySeconds * SAMPLINGRATEF;
        }

    }
}

inline void FDN::setDirectDelayTimes(){
    //Calculate delay from source to receiver
    float directDelayLeft = parametersFDN.soundSourceLoc.distance(parametersFDN.listenerLocLeftEar)*CENTIMETRESTOMETRES / SOUNDSPEED;
    float directDelayRight = parametersFDN.soundSourceLoc.distance(parametersFDN.listenerLocRightEar)*CENTIMETRESTOMETRES / SOUNDSPEED;
    directDelayTimes[0] = directDelayLeft;
    directDelayTimes[1] = directDelayRight;
}

void FDN::setDirectRayAngles(){
    
    float yDiff2, xDiff2;
    yDiff2 = parametersFDN.soundSourceLoc.y - parametersFDN.listenerLoc.y;
    xDiff2 = parametersFDN.soundSourceLoc.x - parametersFDN.listenerLoc.x;
    float angle = atan2(xDiff2, yDiff2) * 180.0f / M_PI;
    directRayAngles[0] = angle;
    directRayAngles[1] = angle;
    
}

void FDN::setDelayChannels(){
    for (size_t i = 0; i < NUMTAPSSTD; i++){
        delayTimesChannel[i] = determineChannel(roomBouncePoints[i].x, roomBouncePoints[i].y);
    }
}

float FDN::channelToAngle(size_t channel){
    float channelWidth = 360.0f / (float)CHANNELS;
    size_t midChannel = CHANNELS / 2 - 1;
    
    if (channel <= midChannel){
    return (float)channel * channelWidth + (channelWidth / 2.0f);
    }
    else{
        return (-1.f  * (float) (CHANNELS - channel) * channelWidth )+ (channelWidth / 2.0f);
    }
}

size_t FDN::angleToChannel(float angleInDegrees){
    
    float channelWidth = 360.0f / CHANNELS;

    //Channel 0 to 7 from 12 o'clock, clockwise according from DFX
    return (size_t)floorf(angleInDegrees / channelWidth);
}

//azimuth is clockwise 0 to 360 degree, p. 149 DAFX
size_t FDN::determineChannel(float x,float y){
    float xL = parametersFDN.listenerLoc.x;
    float yL = parametersFDN.listenerLoc.y;
    
    float xDistance = x - xL;
    float yDistance = y - yL;
    
    float angle2 = atan2f(xDistance, yDistance) * 180.0f / M_PI;
    if (angle2 < 0.0f ){
        angle2 += 360.0f;
    }
    return angleToChannel(angle2);
    
}

//Use this if uncircullated tap > 0
inline void FDN::sortDelayTimes(){
    
    for (int i = 0; i < numDelays; i++){
        for (int j = 0; j < 2; j++){
            //Compare uncirculated with circulated
            int delay = delayTimesNew[numUncirculatedTaps + i];
            int tap = delayTimesNew[i + (j * numDelays)];
          //  printf("index %d delay %d indexTap %d tap %d \n", NUMTAPSSTD + i, delay, i + (j * numDelays), tap);
            if (delay < tap){
                //swap delays
                delayTimesNew[NUMTAPSSTD + i] = tap;
                delayTimesNew[i + (j * numDelays)] = delay;
                
              //  printf("%d %d %d %d\n", i , tap, delay, i + (j * numDelays));
                //swap points
                float x0 = roomBouncePoints[(i + (j*numDelays))].x;
                float y0 = roomBouncePoints[(i + (j*numDelays))].y;
                
                float x1 = roomBouncePoints[(numUncirculatedTaps + i)].x;
                float y1 = roomBouncePoints[(numUncirculatedTaps + i)].y;
                
                roomBouncePoints[(i + (j*numDelays))].x = x1;
                roomBouncePoints[(i + (j*numDelays))].y = y1;
                
                roomBouncePoints[(numUncirculatedTaps + i)].x = x0;
                roomBouncePoints[(numUncirculatedTaps + i)].y = y0;
                
            }
        }
    }
}

void FDN::setFilters(){
    //Right is negative, p.151 DAFX
    for (size_t i = 0; i < CHANNELS; i++){
        leftEarFilter[i].setAngle(channelToAngle(i), SAMPLINGRATEF);
        rightEarFilter[i].setAngle(-channelToAngle(i), SAMPLINGRATEF);
    }
    
    directRayFilter[0].setAngle(directRayAngles[0], SAMPLINGRATEF);
    directRayFilter[1].setAngle(-directRayAngles[1], SAMPLINGRATEF);
    
}

void FDN::setSingleTapDelay(){
    directRays[0].setTimeSafe(directDelayTimes[0]);
    directRays[1].setTimeSafe(directDelayTimes[1]);
}

//Simplify this
void FDN::setTempPoints(){
   // printf("Lis loc %f %f sound loc %f %f roomwidth %f room length %f\n", listenerLoc[0], listenerLoc[1], soundSourceLoc[0], soundSourceLoc[1], roomWidthCM, roomHeightCM);
    for (int i = 0; i < CHANNELS ; i++){
     //   printf("Channel # %d \n", i);
        float angle = channelToAngle(i);
       // printf("Initial angle is: %f \n", angle);
        Point2d point;
        if (angle < 0.0f and i >= (3*CHANNELS/4)){
         //   printf ("front left ears \n");
            //for front left ears
            angle = 90.0f - (angle);
            
            //find intersection with y = h
//            float v1[2] = {listenerLoc[0], listenerLoc[1] - roomHeightCM};
//            float v2[2] = {listenerLoc[0], 0.0f};
//            float v3[2] = {-sin(angle * M_PI / 180.f), cos(angle* M_PI / 180.f)};
            Point2d v1 = Point2d(parametersFDN.listenerLoc.x,parametersFDN.listenerLoc.y-parametersFDN.roomHeightCM);
            Point2d v2 = Point2d(parametersFDN.listenerLoc.x,0.0f);
            Point2d v3 = Point2d(-sin(angle * M_PI / 180.f),cos(angle* M_PI / 180.f));
         //   printf("v3: %f %f %f \n", v3[0], v3[1], angle);
            float t2 = v1.dotProduct(v3) / v2.dotProduct(v3);
            
            if (t2 > 1.0f or t2 < 0.0f){
                //find intersection with x = 0
//                v1[1] = listenerLoc[1];
//                v2[1] = roomHeightCM; v2[0] = 0.0f;
                v1.y = parametersFDN.listenerLoc.y;
                v2 = Point2d(0.0f,parametersFDN.roomHeightCM);
                
                t2 = v1.dotProduct(v3) / v2.dotProduct(v3);
                point = Point2d(0.0f,parametersFDN.roomHeightCM * t2);
            }
            else{
//                point[0] = listenerLoc[0] * t2; point[1] = roomHeightCM;
                point = Point2d(parametersFDN.listenerLoc.x * t2,parametersFDN.roomHeightCM);
            }
        }
        
        else if (angle<0.f and i >= CHANNELS/2 and i < 3*CHANNELS/4){
            angle = 90.0f - (angle);
            //for back left ears
            //  printf ("back left ears \n");
            //find intersection with x = 0
            Point2d v1 = parametersFDN.listenerLoc;
            Point2d v2 = Point2d(0.0f, parametersFDN.roomHeightCM);
            Point2d v3 = Point2d(-sin(angle* M_PI / 180.f), cos(angle* M_PI / 180.f));
         //     printf("v3: %f %f %f \n", v3[0], v3[1], angle);
            float t2 = v1.dotProduct(v3) / v2.dotProduct(v3);
            
            if (t2 > 1.0f or t2 < 0.0f){
                //find intersection with y = 0
                v2 = Point2d(parametersFDN.listenerLoc.x,0.0f);
                t2 = v1.dotProduct(v3) / v2.dotProduct(v3);
                point = Point2d(parametersFDN.listenerLoc.x * t2, 0.0f);
            }
            else{
                point = Point2d(0.0f, parametersFDN.roomHeightCM*t2);
            }
        }
        
        else if (angle >= 0.f and i < CHANNELS/4){
            //for right front ears
            angle = 90.f - angle;
           //   printf ("front right ears \n");
            //find intersection with y = h
            Point2d v1 = Point2d(parametersFDN.listenerLoc.x, parametersFDN.listenerLoc.y- parametersFDN.roomHeightCM);
            Point2d v2 = Point2d(parametersFDN.roomWidthCM, 0);
            Point2d v3 = Point2d(-sin(angle* M_PI / 180.f),cos(angle* M_PI / 180.f));
            
         //   printf("v3: %f %f %f \n", v3[0], v3[1], angle);
            float t2 = v1.dotProduct(v3) / v2.dotProduct(v3);
         //   printf("initial t: %f \n ", t2);
            if (t2 > 1.0f or t2 < 0.0f){
                //find intersection with x = width
                v1.x = parametersFDN.listenerLoc.x - parametersFDN.roomWidthCM; v1.y = parametersFDN.listenerLoc.y;
                v2.x = 0.0f; v2.y = parametersFDN.roomHeightCM;
                
                t2 = v1.dotProduct(v3) / v2.dotProduct(v3);
            //    printf("%f %f %f %f %f %f \n", v1[0], v1[1], v2[0], v2[1], dotProduct(v1, v3), dotProduct(v2, v3));
                point = Point2d(parametersFDN.roomWidthCM, t2*parametersFDN.roomHeightCM);
            }
            else{
                point = Point2d(parametersFDN.roomWidthCM*t2, parametersFDN.roomHeightCM);
               
            }
         //   printf("T: %f\n", t2);
        }
        
        else if (angle >= 0.f and i >= CHANNELS/4 and i < CHANNELS /2){
          //    printf ("back right ears \n");
            angle = 90.f - angle;
            //for right back ears
            //find intersection with x = w
            Point2d v1 = Point2d(parametersFDN.listenerLoc.x - parametersFDN.roomWidthCM, parametersFDN.listenerLoc.y);
            Point2d v2 = Point2d(0.0f, parametersFDN.roomHeightCM);
            Point2d v3 = Point2d(-sin(angle* M_PI / 180.f), cos(angle* M_PI / 180.f));
       //     printf("v3: %f %f %f \n", v3[0], v3[1], angle);
            float t2 = v1.dotProduct(v3)/ v2.dotProduct(v3);
            
            if (t2 > 1.0f or t2 < 0.0f){
                //find intersection with y = 0
                v1.x = parametersFDN.listenerLoc.x; v1.y = parametersFDN.listenerLoc.y;
                v2.x = parametersFDN.roomWidthCM; v2.y = 0.0f;
                
                t2 = v1.dotProduct(v3) / v2.dotProduct(v3);
                point = Point2d(parametersFDN.roomWidthCM*t2, 0.0f);
            }
            else{
                point = Point2d(parametersFDN.roomWidthCM, parametersFDN.roomWidthCM*t2);
            }
        }
        
    //    printf("For angle: %f, intersection at %f %f\n", angle, point[0], point[1]);
        
        tempPoints[i*2] = point.x;
        tempPoints[i*2+1] = point.y;
     //   printf( " ==========\n");
    }
}

void FDN::calculateAdditionalDelays(){
    for (int i = 0; i < CHANNELS; i++)
    {
        if (i < CHANNELS/2){ //CHANNEL 0 -3 FOR RIGHT EAR
             //near to right use temp0,leftear- temp0,rightear, 0 to 3, FOR LEFT EAR
            float d = (getDistance(tempPoints[i*2], tempPoints[i*2+1], parametersFDN.listenerLocLeftEar.x, parametersFDN.listenerLocLeftEar.y) - getDistance(tempPoints[i*2], tempPoints[i*2+1], parametersFDN.listenerLocRightEar.x, parametersFDN.listenerLocRightEar.y) ) * CENTIMETRESTOMETRES / SOUNDSPEED;
            additionalDelays[i] = d;
        }
        else{ //CHANNEL 4-7 FOR LEFT EAR
              //near to left use temp4,rightear - temp4,leftear, 4 to 7, FOR RIGHT EAR
           float d = (getDistance(tempPoints[i*2], tempPoints[i*2+1], parametersFDN.listenerLocRightEar.x, parametersFDN.listenerLocRightEar.y) - getDistance(tempPoints[i*2], tempPoints[i*2+1], parametersFDN.listenerLocLeftEar.x, parametersFDN.listenerLocLeftEar.y) ) * CENTIMETRESTOMETRES / SOUNDSPEED;
            additionalDelays[i] = d;
        }
        reverbDelays[i].setTimeSafe(additionalDelays[i]);
    }
}

void FDN::setGainConstants(){
    roomSA = 2.0f * (parametersFDN.roomWidthCM*parametersFDN.roomHeightCM * CENTIMETRESTOMETRESSQ + parametersFDN.roomWidthCM *parametersFDN.roomCeilingCM * CENTIMETRESTOMETRESSQ + parametersFDN.roomHeightCM  * parametersFDN.roomCeilingCM * CENTIMETRESTOMETRESSQ);
    directDistanceInMetres = parametersFDN.soundSourceLoc.distance(parametersFDN.listenerLoc) * CENTIMETRESTOMETRES;
    directMix = parametersFDN.directGain * 1.0f / (directDistanceInMetres*directDistanceInMetres);
    reverbMix = parametersFDN.reverbGain * 1.0f / roomSA;
}

