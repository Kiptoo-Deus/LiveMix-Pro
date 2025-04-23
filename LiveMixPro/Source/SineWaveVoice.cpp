/*
  ==============================================================================

    SineWaveVoice.cpp
    Created: 23 Apr 2025 7:51:12pm
    Author:  Joel

  ==============================================================================
*/

#include "SineWaveVoice.h"
#include "SineWaveSound.h"

SineWaveVoice::SineWaveVoice(double initialSampleRate)
        : sampleRate(initialSampleRate)
{
}
bool SineWaveVoice::canPlaySound(juce::SynthesiserSound* sound)
{
    return dynamic_cast<SineWaveSound*>(sound) != nullptr;
}

void SineWaveVoice::startNote(int midiNoteNumber, float velocity, juce::SynthesiserSound*, int /*currentPitchWheelPosition*/)
{
    currentAngle = 0.0;
    level = velocity * 0.15; // Scale velocity
    tailOff = 0.0;

    double cyclesPerSecond = juce::MidiMessage::getMidiNoteInHertz(midiNoteNumber);
    angleDelta = cyclesPerSecond * 2.0 * juce::MathConstants<double>::pi / sampleRate;
}

void SineWaveVoice::stopNote(float /*velocity*/, bool allowTailOff)
{
    if (allowTailOff)
    {
        if (tailOff == 0.0)
            tailOff = 1.0;
    }
    else
    {
        clearCurrentNote();
        angleDelta = 0.0;
    }
}

void SineWaveVoice::pitchWheelMoved(int /*newPitchWheelValue*/) {}

void SineWaveVoice::controllerMoved(int /*controllerNumber*/, int /*newControllerValue*/) {}

void SineWaveVoice::renderNextBlock(juce::AudioBuffer<float>& outputBuffer, int startSample, int numSamples)
{
    if (angleDelta != 0.0)
    {
        if (tailOff > 0.0)
        {
            while (--numSamples >= 0)
            {
                auto currentSample = (float)(std::sin(currentAngle) * level * tailOff);
                for (auto i = outputBuffer.getNumChannels(); --i >= 0;)
                    outputBuffer.addSample(i, startSample, currentSample);

                currentAngle += angleDelta;
                startSample++;

                tailOff *= 0.999; // Exponential decay
                if (tailOff <= 0.005)
                {
                    clearCurrentNote();
                    angleDelta = 0.0;
                    break;
                }
            }
        }
        else
        {
            while (--numSamples >= 0)
            {
                auto currentSample = (float)(std::sin(currentAngle) * level);
                for (auto i = outputBuffer.getNumChannels(); --i >= 0;)
                    outputBuffer.addSample(i, startSample, currentSample);

                currentAngle += angleDelta;
                startSample++;
            }
        }
    }
}
;