#include "MainComponent.h"

MainComponent::MainComponent()
{
    juce::RuntimePermissions::request(juce::RuntimePermissions::recordAudio,
                                      [](bool granted) {
                                          juce::Logger::writeToLog("Microphone permission " + juce::String(granted ? "granted" : "denied"));
                                      });

    setAudioChannels(1, 2);
    setSize(800, 600);

    if (auto* device = deviceManager.getCurrentAudioDevice())
        juce::Logger::writeToLog("Audio device: " + device->getName() +
                                 ", inputChannels=" + juce::String(device->getActiveInputChannels().countNumberOfSetBits()) +
                                 ", outputChannels=" + juce::String(device->getActiveOutputChannels().countNumberOfSetBits()));
    else
        juce::Logger::writeToLog("No audio device available");
}

MainComponent::~MainComponent()
{
    shutdownAudio();
}

void MainComponent::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
    juce::Logger::writeToLog("prepareToPlay: sampleRate=" + juce::String(sampleRate) +
                             ", blockSize=" + juce::String(samplesPerBlockExpected));
    reverbProcessor.prepareToPlay(sampleRate, samplesPerBlockExpected);
    reverbProcessor.setReverbMix(0.5f);
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
    g.setFont(15.0f);
    g.drawText("LiveMixPro", getLocalBounds(), juce::Justification::centred);
}

void MainComponent::resized()
{
    // No UI components yet
};