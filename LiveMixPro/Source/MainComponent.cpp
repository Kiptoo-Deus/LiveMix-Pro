#include "MainComponent.h"

MainComponent::MainComponent()
{
    setAudioChannels(1, 2); // 1 input (mic), 2 outputs (stereo)
    setSize(800, 600);
}

MainComponent::~MainComponent()
{
    shutdownAudio();
}

void MainComponent::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
    // Log sample rate and block size for latency debugging
    juce::Logger::writeToLog("prepareToPlay: sampleRate=" + juce::String(sampleRate) +
                             ", blockSize=" + juce::String(samplesPerBlockExpected));
}

void MainComponent::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill)
{
    // Get the input buffer from the audio device manager
    auto* inputBuffer = bufferToFill.buffer;

    // Ensure input is available (1 channel expected)
    if (inputBuffer->getNumChannels() < 1) return;

    // Get the output buffer
    auto* outputBuffer = bufferToFill.buffer;

    // Copy mono input to both stereo output channels
    for (int channel = 0; channel < outputBuffer->getNumChannels(); ++channel)
    {
        outputBuffer->copyFrom(channel, bufferToFill.startSample,
                               inputBuffer->getReadPointer(0),
                               bufferToFill.numSamples);
    }
}

void MainComponent::releaseResources()
{
}

void MainComponent::paint(juce::Graphics& g)
{
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
    g.setColour(juce::Colours::white);
    g.setFont(15.0f);
    g.drawText("LiveMix Pro: Audio Passthrough", getLocalBounds(), juce::Justification::centred);
}

void MainComponent::resized()
{

}