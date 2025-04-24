#include "MainComponent.h"

MainComponent::MainComponent()
        : midiKeyboard(keyboardState, juce::MidiKeyboardComponent::horizontalKeyboard)
{
    // Request microphone permission
    juce::RuntimePermissions::request(juce::RuntimePermissions::recordAudio,
                                      [](bool granted) {
                                          juce::Logger::writeToLog("Microphone permission " + juce::String(granted ? "granted" : "denied"));
                                      });

    setAudioChannels(1, 2); // 1 input (mic), 2 outputs (stereo)
    setSize(800, 600);

    // Setup reverb mix slider
    reverbMixSlider.setRange(0.0, 1.0, 0.01);
    reverbMixSlider.setValue(0.5);
    reverbMixSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 80, 40);
    reverbMixSlider.onValueChange = [this] {
        reverbProcessor.setReverbMix(static_cast<float>(reverbMixSlider.getValue()));
        juce::Logger::writeToLog("Slider value: " + juce::String(reverbMixSlider.getValue()));
    };
    addAndMakeVisible(reverbMixSlider);

    reverbMixLabel.setText("Reverb Mix", juce::dontSendNotification);
    reverbMixLabel.attachToComponent(&reverbMixSlider, true);
    reverbMixLabel.setFont(juce::Font(20.0f));
    addAndMakeVisible(reverbMixLabel);

    // Setup MIDI keyboard
    midiKeyboard.setMidiChannel(1);
    midiKeyboard.setKeyWidth(40.0f); // ~27 keys fit in ~1080px
    midiKeyboard.setAvailableRange(48, 72); // C3 to C5
    keyboardState.addListener(this);
    addAndMakeVisible(midiKeyboard);

    // Log audio device
    if (auto* device = deviceManager.getCurrentAudioDevice())
        juce::Logger::writeToLog("Audio device: " + device->getName() +
                                 ", inputChannels=" + juce::String(device->getActiveInputChannels().countNumberOfSetBits()) +
                                 ", outputChannels=" + juce::String(device->getActiveOutputChannels().countNumberOfSetBits()));
    else
        juce::Logger::writeToLog("No audio device available");
}

MainComponent::~MainComponent()
{
    keyboardState.removeListener(this);
    shutdownAudio();
}

void MainComponent::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
    juce::Logger::writeToLog("prepareToPlay: sampleRate=" + juce::String(sampleRate) +
                             ", blockSize=" + juce::String(samplesPerBlockExpected));
    reverbProcessor.prepareToPlay(sampleRate, samplesPerBlockExpected);
    reverbProcessor.setReverbMix(static_cast<float>(reverbMixSlider.getValue()));
}

void MainComponent::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill)
{
    auto* inputBuffer = bufferToFill.buffer;
    auto* outputBuffer = bufferToFill.buffer;

    juce::Logger::writeToLog("getNextAudioBlock: inputChannels=" + juce::String(inputBuffer->getNumChannels()) +
                             ", outputChannels=" + juce::String(outputBuffer->getNumChannels()) +
                             ", samples=" + juce::String(bufferToFill.numSamples));

    if (inputBuffer->getNumChannels() < 1)
    {
        juce::Logger::writeToLog("No input channels available");
        outputBuffer->clear();
        return;
    }

    inputBuffer->applyGain(2.0f);
    float maxSample = inputBuffer->getMagnitude(0, 0, inputBuffer->getNumSamples());
    juce::Logger::writeToLog("Input max sample: " + juce::String(maxSample));

    reverbProcessor.processBlock(*inputBuffer);

    for (int channel = 0; channel < outputBuffer->getNumChannels(); ++channel)
    {
        outputBuffer->copyFrom(channel, bufferToFill.startSample,
                               inputBuffer->getReadPointer(0),
                               bufferToFill.numSamples);
    }
}

void MainComponent::releaseResources()
{
    juce::Logger::writeToLog("Releasing audio resources");
}

void MainComponent::paint(juce::Graphics& g)
{
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
    g.setColour(juce::Colours::white);
    g.setFont(20.0f);
    g.drawText("LiveMixPro: Reverb & MIDI", 0, 0, getWidth(), 40, juce::Justification::centred);
}

void MainComponent::resized()
{
    auto area = getLocalBounds().reduced(20); // Margin
    juce::Logger::writeToLog("Screen bounds: " + area.toString());

    area.removeFromTop(40);

    // Reverb slider
    reverbMixSlider.setBounds(100, area.getY() + 10, area.getWidth() - 120, 40);
    area.removeFromTop(60);

    // MIDI keyboard
    midiKeyboard.setBounds(area.getX(), area.getY(), area.getWidth(), 150);
}

void MainComponent::handleNoteOn(juce::MidiKeyboardState*, int midiChannel, int midiNoteNumber, float velocity)
{
    juce::Logger::writeToLog("MIDI Note On: channel=" + juce::String(midiChannel) +
                             ", note=" + juce::String(midiNoteNumber) +
                             ", velocity=" + juce::String(velocity));
}

void MainComponent::handleNoteOff(juce::MidiKeyboardState*, int midiChannel, int midiNoteNumber, float velocity)
{
    juce::Logger::writeToLog("MIDI Note Off: channel=" + juce::String(midiChannel) +
                             ", note=" + juce::String(midiNoteNumber) +
                             ", velocity=" + juce::String(velocity));
}
;