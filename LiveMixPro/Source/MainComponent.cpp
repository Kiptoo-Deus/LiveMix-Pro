#include "MainComponent.h"

MainComponent::MainComponent()
        : midiKeyboard(keyboardState, juce::MidiKeyboardComponent::horizontalKeyboard)
{
    juce::RuntimePermissions::request(juce::RuntimePermissions::recordAudio,
                                      [](bool granted) {
                                          juce::Logger::writeToLog("Microphone permission " + juce::String(granted ? "granted" : "denied"));
                                      });

    setAudioChannels(1, 2);
   // setSize(800, 600);  I don't need this because it's an app

    // Setup reverb mix slider
    reverbMixSlider.setRange(0.0, 1.0, 0.01);
    reverbMixSlider.setValue(0.5);
    reverbMixSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 80, 40);
    reverbMixSlider.onValueChange = [this] {
        double value = reverbMixSlider.getValue();
        if (juce::isPositiveAndBelow(value, 1.0)) {
            reverbProcessor.setReverbMix(static_cast<float>(value));
            juce::Logger::writeToLog("Reverb mix slider value: " + juce::String(value));
        }
    };
    addAndMakeVisible(reverbMixSlider);

    reverbMixLabel.setText("Reverb Mix", juce::dontSendNotification);
    reverbMixLabel.attachToComponent(&reverbMixSlider, true);
    reverbMixLabel.setFont(juce::Font(20.0f));
    addAndMakeVisible(reverbMixLabel);

    // Setup room size slider
    roomSizeSlider.setRange(0.0, 1.0, 0.01);
    roomSizeSlider.setValue(0.5);
    roomSizeSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 80, 40);
    roomSizeSlider.onValueChange = [this] {
        double value = roomSizeSlider.getValue();
        if (juce::isPositiveAndBelow(value, 1.0)) {
            reverbProcessor.setRoomSize(static_cast<float>(value));
            juce::Logger::writeToLog("Room size slider value: " + juce::String(value));
        }
    };
    addAndMakeVisible(roomSizeSlider);

    roomSizeLabel.setText("Room Size", juce::dontSendNotification);
    roomSizeLabel.attachToComponent(&roomSizeSlider, true);
    roomSizeLabel.setFont(juce::Font(20.0f));
    addAndMakeVisible(roomSizeLabel);

    // Setup delay mix slider
    delayMixSlider.setRange(0.0, 1.0, 0.01);
    delayMixSlider.setValue(0.5);
    delayMixSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 80, 40);
    delayMixSlider.onValueChange = [this] {
        double value = delayMixSlider.getValue();
        if (juce::isPositiveAndBelow(value, 1.0)) {
            delayProcessor.setDelayMix(static_cast<float>(value));
            juce::Logger::writeToLog("Delay mix slider value: " + juce::String(value));
        }
    };
    addAndMakeVisible(delayMixSlider);

    delayMixLabel.setText("Delay Mix", juce::dontSendNotification);
    delayMixLabel.attachToComponent(&delayMixSlider, true);
    delayMixLabel.setFont(juce::Font(20.0f));
    addAndMakeVisible(delayMixLabel);

    // Setup buttons
    recordButton.setButtonText("Record");
    recordButton.onClick = [this] {
        if (isRecording) stopRecording();
        else startRecording();
    };
    addAndMakeVisible(recordButton);

    playButton.setButtonText("Play");
    playButton.onClick = [this] {
        if (isPlaying) stopPlayback();
        else startPlayback();
    };
    addAndMakeVisible(playButton);

    // Setup MIDI keyboard
    midiKeyboard.setMidiChannel(1);
    midiKeyboard.setKeyWidth(40.0f);
    midiKeyboard.setAvailableRange(48, 72);
    keyboardState.addListener(this);
    addAndMakeVisible(midiKeyboard);

    // Initialize tracks
    audioTrack = std::make_unique<AudioTrack>();
    midiTrack = std::make_unique<MidiTrack>();

    // Setup synth
    synth.setCurrentPlaybackSampleRate(44100.0);
    synth.addSound(new SineWaveSound());
    synth.addVoice(new SineWaveVoice());

    // Log audio device
    if (auto* device = deviceManager.getCurrentAudioDevice())
        juce::Logger::writeToLog("Audio device: " + device->getName() +
                                 ", inputChannels=" + juce::String(device->getActiveInputChannels().countNumberOfSetBits()) +
                                 ", outputChannels=" + juce::String(device->getActiveOutputChannels().countNumberOfSetBits()));
    else
        juce::Logger::writeToLog("No audio device available");

    reverbProcessor.setReverbMix(0.5f);
    reverbProcessor.setRoomSize(0.5f);
    delayProcessor.setDelayMix(0.5f);
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
    delayProcessor.prepareToPlay(sampleRate, samplesPerBlockExpected);
    synth.setCurrentPlaybackSampleRate(sampleRate);
    this->sampleRate = sampleRate;
}

void MainComponent::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill)
{
    auto* inputBuffer = bufferToFill.buffer;
    auto* outputBuffer = bufferToFill.buffer;

    juce::Logger::writeToLog("getNextAudioBlock: inputChannels=" + juce::String(inputBuffer->getNumChannels()) +
                             ", outputChannels=" + juce::String(outputBuffer->getNumChannels()) +
                             ", samples=" + juce::String(bufferToFill.numSamples));

    outputBuffer->clear();

    if (isRecording && inputBuffer->getNumChannels() >= 1)
    {
        juce::AudioBuffer<float> tempBuffer(1, bufferToFill.numSamples);
        tempBuffer.copyFrom(0, 0, inputBuffer->getReadPointer(0), bufferToFill.numSamples);
        audioTrack->addAudioData(tempBuffer);
        tempBuffer.applyGain(2.0f);
        reverbProcessor.processBlock(tempBuffer);
        delayProcessor.processBlock(tempBuffer);
        for (int ch = 0; ch < outputBuffer->getNumChannels(); ++ch)
            outputBuffer->copyFrom(ch, bufferToFill.startSample, tempBuffer.getReadPointer(0), bufferToFill.numSamples);
    }

    if (isPlaying)
    {
        if (playbackPosition < audioTrack->getNumSamples())
        {
            juce::AudioBuffer<float> tempBuffer(1, bufferToFill.numSamples);
            audioTrack->getAudioData(tempBuffer, playbackPosition, bufferToFill.numSamples);
            reverbProcessor.processBlock(tempBuffer);
            delayProcessor.processBlock(tempBuffer);
            for (int ch = 0; ch < outputBuffer->getNumChannels(); ++ch)
                outputBuffer->copyFrom(ch, bufferToFill.startSample, tempBuffer.getReadPointer(0), bufferToFill.numSamples);
        }

        juce::MidiBuffer midiBuffer = midiTrack->getMidiBuffer(playbackPosition, bufferToFill.numSamples);
        synth.renderNextBlock(*outputBuffer, midiBuffer, 0, bufferToFill.numSamples);

        playbackPosition += bufferToFill.numSamples;
        if (playbackPosition >= audioTrack->getNumSamples())
            stopPlayback();
    }

    currentTime += bufferToFill.numSamples / sampleRate;
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
    g.drawText("LiveMixPro DAW - DroidCon 2025", 0, 0, getWidth(), 40, juce::Justification::centred);
}

//void MainComponent::resized()
//{
//    auto area = getLocalBounds().reduced(20);
//    area.removeFromTop(40);
//
//    recordButton.setBounds(area.getX(), area.getY() + 10, 80, 40);
//    playButton.setBounds(area.getX() + 90, area.getY() + 10, 80, 40);
//    area.removeFromTop(60);
//
//    reverbMixSlider.setBounds(100, area.getY() + 10, area.getWidth() - 120, 40);
//    area.removeFromTop(60);
//
//    roomSizeSlider.setBounds(100, area.getY() + 10, area.getWidth() - 120, 40);
//    area.removeFromTop(60);
//
//    delayMixSlider.setBounds(100, area.getY() + 10, area.getWidth() - 120, 40);
//    area.removeFromTop(60);
//
//    midiKeyboard.setBounds(area.getX(), area.getY(), area.getWidth(), 150);
//}

void MainComponent::resized()
{
    auto bounds = getLocalBounds().reduced(10);
    bounds.removeFromTop(40); //  space for the header/title

    juce::FlexBox flexBox;
    flexBox.flexDirection = juce::FlexBox::Direction::column;
    flexBox.justifyContent = juce::FlexBox::JustifyContent::flexStart;
    flexBox.alignItems = juce::FlexBox::AlignItems::stretch;


    juce::FlexBox buttonRow;
    buttonRow.flexDirection = juce::FlexBox::Direction::row;
    buttonRow.justifyContent = juce::FlexBox::JustifyContent::spaceBetween;

    buttonRow.items.add(juce::FlexItem(recordButton).withMinWidth(80.0f).withHeight(40.0f).withMargin(5));
    buttonRow.items.add(juce::FlexItem(playButton).withMinWidth(80.0f).withHeight(40.0f).withMargin(5));

    flexBox.items.add(juce::FlexItem(buttonRow).withHeight(50.0f));

    // Helper lambda to add sliders with labels
    auto addSliderRow = [&](juce::Label& label, juce::Slider& slider)
    {
        juce::FlexBox row;
        row.flexDirection = juce::FlexBox::Direction::row;
        row.items.add(juce::FlexItem(label).withMinWidth(100.0f).withMargin(juce::FlexItem::Margin(5)));
        row.items.add(juce::FlexItem(slider).withFlex(1.0f).withMinHeight(40.0f).withMargin(juce::FlexItem::Margin(5)));
        flexBox.items.add(juce::FlexItem(row).withHeight(50.0f));
    };

    addSliderRow(reverbMixLabel, reverbMixSlider);
    addSliderRow(roomSizeLabel, roomSizeSlider);
    addSliderRow(delayMixLabel, delayMixSlider);

    // MIDI Keyboard
    flexBox.items.add(juce::FlexItem(midiKeyboard).withHeight(150.0f).withMargin(juce::FlexItem::Margin(10.0f, 5.0f, 5.0f, 5.0f)));

    // Apply layout
    flexBox.performLayout(bounds);
}



void MainComponent::handleNoteOn(juce::MidiKeyboardState*, int midiChannel, int midiNoteNumber, float velocity)
{
    juce::Logger::writeToLog("MIDI Note On: channel=" + juce::String(midiChannel) +
                             ", note=" + juce::String(midiNoteNumber) +
                             ", velocity=" + juce::String(velocity));
    if (isRecording)
    {
        juce::MidiMessage msg = juce::MidiMessage::noteOn(midiChannel, midiNoteNumber, velocity);
        midiTrack->addMidiMessage(msg, currentTime);
    }
}

void MainComponent::handleNoteOff(juce::MidiKeyboardState*, int midiChannel, int midiNoteNumber, float velocity)
{
    juce::Logger::writeToLog("MIDI Note Off: channel=" + juce::String(midiChannel) +
                             ", note=" + juce::String(midiNoteNumber) +
                             ", velocity=" + juce::String(velocity));
    if (isRecording)
    {
        juce::MidiMessage msg = juce::MidiMessage::noteOff(midiChannel, midiNoteNumber);
        midiTrack->addMidiMessage(msg, currentTime);
    }
}

void MainComponent::startRecording()
{
    isRecording = true;
    audioTrack->clear();
    midiTrack->clear();
    currentTime = 0.0;
    recordButton.setButtonText("Stop");
    juce::Logger::writeToLog("Recording started");
}

void MainComponent::stopRecording()
{
    isRecording = false;
    recordButton.setButtonText("Record");
    juce::Logger::writeToLog("Recording stopped");
}

void MainComponent::startPlayback()
{
    isPlaying = true;
    playbackPosition = 0;
    playButton.setButtonText("Stop");
    juce::Logger::writeToLog("Playback started");
}

void MainComponent::stopPlayback()
{
    isPlaying = false;
    playButton.setButtonText("Play");
    juce::Logger::writeToLog("Playback stopped");
}