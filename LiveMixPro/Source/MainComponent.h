#pragma once
#include <JuceHeader.h>
#include "ReverbProcessor.h"

class MainComponent : public juce::AudioAppComponent, private juce::MidiKeyboardStateListener
{
public:
    MainComponent();
    ~MainComponent() override;

    void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override;
    void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) override;
    void releaseResources() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    void handleNoteOn(juce::MidiKeyboardState*, int midiChannel, int midiNoteNumber, float velocity) override;
    void handleNoteOff(juce::MidiKeyboardState*, int midiChannel, int midiNoteNumber, float velocity) override;

    ReverbProcessor reverbProcessor;
    juce::Slider reverbMixSlider;
    juce::Label reverbMixLabel;
    juce::MidiKeyboardState keyboardState;
    juce::MidiKeyboardComponent midiKeyboard;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};