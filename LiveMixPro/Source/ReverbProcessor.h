/*
  ==============================================================================

    ReverbProcessor.h
    Created: 23 Apr 2025 12:17:19am
    Author:  Joel

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

class ReverbProcessor
{
public:
    ReverbProcessor();
    void prepareToPlay(double sampleRate, int samplesPerBlock);
    void processBlock(juce::AudioBuffer<float>& buffer);
    void setReverbMix(float mix);
    void setRoomSize(float size);

private:
    juce::Reverb reverb;
    juce::Reverb::Parameters reverbParams;
    float dryWetMix;
};