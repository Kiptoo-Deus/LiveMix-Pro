/*
  ==============================================================================

    ReverbProcessor.cpp
    Created: 23 Apr 2025 12:17:19am
    Author:  Joel

  ==============================================================================
*/
#include "ReverbProcessor.h"

ReverbProcessor::ReverbProcessor()
{
    dryWetMix = 0.5f;
    reverbParams.roomSize = 0.5f;
    reverbParams.damping = 0.5f;
    reverbParams.wetLevel = dryWetMix;
    reverbParams.dryLevel = 1.0f - dryWetMix;
    reverb.setParameters(reverbParams);
}

void ReverbProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    reverb.reset();
    reverb.setSampleRate(sampleRate);
}

void ReverbProcessor::processBlock(juce::AudioBuffer<float>& buffer)
{
    reverb.processMono(buffer.getWritePointer(0), buffer.getNumSamples());
}

void ReverbProcessor::setReverbMix(float mix)
{
    dryWetMix = juce::jlimit(0.0f, 1.0f, mix);
    reverbParams.wetLevel = dryWetMix;
    reverbParams.dryLevel = 1.0f - dryWetMix;
    reverb.setParameters(reverbParams);
}

void ReverbProcessor::setRoomSize(float size)
{
    reverbParams.roomSize = juce::jlimit(0.0f, 1.0f, size);
    reverb.setParameters(reverbParams);
    juce::Logger::writeToLog("Room size set to: " + juce::String(size));
}