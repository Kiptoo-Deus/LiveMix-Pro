/*
  ==============================================================================

    Track.h
    Created: 24 Apr 2025 8:23:08pm
    Author:  Joel

  ==============================================================================
*/

#pragma once
#pragma once
#include <JuceHeader.h>

class AudioTrack
{
public:
    AudioTrack() = default;
    void addAudioData(const juce::AudioBuffer<float>& buffer)
    {
        auto numSamples = buffer.getNumSamples();
        auto numChannels = buffer.getNumChannels();
        for (int ch = 0; ch < numChannels && ch < 1; ++ch) // Mono
            audioData.addArray(buffer.getReadPointer(ch), numSamples);
    }
    void getAudioData(juce::AudioBuffer<float>& buffer, int startSample, int numSamples)
    {
        buffer.clear();
        auto* data = audioData.getRawDataPointer() + startSample;
        for (int ch = 0; ch < buffer.getNumChannels() && ch < 1; ++ch)
            buffer.copyFrom(ch, 0, data, juce::jmin(numSamples, audioData.size() - startSample));
    }
    int getNumSamples() const { return audioData.size(); }
    void clear() { audioData.clear(); }

private:
    juce::Array<float> audioData; // Mono
};

class MidiTrack
{
public:
    MidiTrack() = default;
    void addMidiMessage(const juce::MidiMessage& message, double timeStamp)
    {
        midiData.addEvent(message, timeStamp * 44100.0); // Convert seconds to samples
    }
    juce::MidiBuffer getMidiBuffer(int startSample, int numSamples)
    {
        juce::MidiBuffer buffer;
        for (const auto metadata : midiData)
        {
            int samplePos = static_cast<int>(metadata.samplePosition);
            if (samplePos >= startSample && samplePos < startSample + numSamples)
                buffer.addEvent(metadata.getMessage(), samplePos - startSample);
        }
        return buffer;
    }
    void clear() { midiData.clear(); }

private:
    juce::MidiBuffer midiData;
};