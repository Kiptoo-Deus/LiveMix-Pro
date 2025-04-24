/*
  ==============================================================================

    DelayProcessor.cpp
    Created: 24 Apr 2025 7:56:41pm
    Author:  Joel

  ==============================================================================
*/

#include "DelayProcessor.h"


DelayProcessor::DelayProcessor()
{
    delayMix = 0.5f;
}

void DelayProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    this->sampleRate = sampleRate;
    int delaySamples = static_cast<int>(delayTimeSeconds * sampleRate);
    delayBuffer.setSize(1, delaySamples + samplesPerBlock);
    delayBuffer.clear();
    writePosition = 0;
    juce::Logger::writeToLog("Delay prepared: sampleRate=" + juce::String(sampleRate) +
                             ", delaySamples=" + juce::String(delaySamples));
}

void DelayProcessor::processBlock(juce::AudioBuffer<float>& buffer)
{
    auto* channelData = buffer.getWritePointer(0);
    int numSamples = buffer.getNumSamples();
    int delaySamples = static_cast<int>(delayTimeSeconds * sampleRate);

    for (int i = 0; i < numSamples; ++i)
    {
        float inputSample = channelData[i];
        int readPosition = (writePosition - delaySamples + delayBuffer.getNumSamples()) % delayBuffer.getNumSamples();
        float delayedSample = delayBuffer.getSample(0, readPosition);

        float outputSample = (1.0f - delayMix) * inputSample + delayMix * delayedSample;
        channelData[i] = outputSample;
        delayBuffer.setSample(0, writePosition, inputSample);

        writePosition = (writePosition + 1) % delayBuffer.getNumSamples();
    }
}

void DelayProcessor::setDelayMix(float mix)
{
    delayMix = juce::jlimit(0.0f, 1.0f, mix);
    juce::Logger::writeToLog("Delay mix set to: " + juce::String(delayMix));
}