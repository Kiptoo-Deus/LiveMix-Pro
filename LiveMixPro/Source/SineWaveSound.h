/*
  ==============================================================================

    SineWaveSound.h
    Created: 23 Apr 2025 7:50:44pm
    Author:  Joel

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

class SineWaveSound : public juce::SynthesiserSound
{
public:
    SineWaveSound() {}

    bool appliesToNote(int /*midiNoteNumber*/) override { return true; }
    bool appliesToChannel(int /*midiChannel*/) override { return true; }
};