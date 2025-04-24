/*
  ==============================================================================

    DelayProcessor.h
    Created: 24 Apr 2025 7:56:41pm
    Author:  Joel

  ==============================================================================
*/

#pragma once
#pragma once
#include <JuceHeader.h>

class DelayProcessor
{
public:
    DelayProcessor();
    void prepareToPlay(double sampleRate, int samplesPerBlock);
    void processBlock(juce::AudioBuffer<float>& buffer);
    void setDelayMix(float mix);

private:
    juce::AudioBuffer<float> delayBuffer;
    int writePosition = 0;
    float delayMix = 0.5f;
    double sampleRate = 44100.0;
    static constexpr float delayTimeSeconds = 0.3f; // 300ms delay
};