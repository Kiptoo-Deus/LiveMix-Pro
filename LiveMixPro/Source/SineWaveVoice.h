/*
  ==============================================================================

    SineWaveVoice.h
    Created: 23 Apr 2025 7:51:12pm
    Author:  Joel

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

class SineWaveVoice : public juce::SynthesiserVoice
{
public:
    SineWaveVoice(double initialSampleRate = 44100.0);

    bool canPlaySound(juce::SynthesiserSound* sound) override;
    void startNote(int midiNoteNumber, float velocity, juce::SynthesiserSound* sound, int currentPitchWheelPosition) override;
    void stopNote(float velocity, bool allowTailOff) override;
    void pitchWheelMoved(int newPitchWheelValue) override;
    void controllerMoved(int controllerNumber, int newControllerValue) override;
    void renderNextBlock(juce::AudioBuffer<float>& outputBuffer, int startSample, int numSamples) override;

private:
    double currentAngle = 0.0;
    double angleDelta = 0.0;
    double level = 0.0;
    double tailOff = 0.0;
    double sampleRate;
};