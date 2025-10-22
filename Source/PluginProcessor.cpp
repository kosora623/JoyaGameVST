/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
JoyaGameVSTAudioProcessor::JoyaGameVSTAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
    : AudioProcessor(BusesProperties()
#if ! JucePlugin_IsMidiEffect
#if ! JucePlugin_IsSynth
        .withInput("Input", juce::AudioChannelSet::stereo(), true)
#endif
        .withOutput("Output", juce::AudioChannelSet::stereo(), true)
#endif
    )
#endif
{
    synth.addVoice(new juce::SamplerVoice());

    formatManager.registerBasicFormats();

    const char* bellSoundData = reinterpret_cast<const char*>(BinaryData::Bell_wav);
    int bellSoundDataSize = BinaryData::Bell_wavSize;

    auto* stream = new juce::MemoryInputStream(bellSoundData, bellSoundDataSize, false);

    std::unique_ptr<juce::InputStream> streamPtr (stream);
    juce::AudioFormatReader* reader = formatManager.createReaderFor(std::move(streamPtr));

    if (reader != nullptr)
    {
        juce::BigInteger allNotes;
        allNotes.setRange(0, 128, true);

        synth.addSound(new juce::SamplerSound(
            "bell",
            *reader, 
            allNotes,
            60,
            0.1,
            0.1,
            10.0
        ));
    }
}

JoyaGameVSTAudioProcessor::~JoyaGameVSTAudioProcessor()
{
}

//==============================================================================
const juce::String JoyaGameVSTAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool JoyaGameVSTAudioProcessor::acceptsMidi() const
{
#if JucePlugin_WantsMidiInput
    return true;
#else
    return false;
#endif
}

bool JoyaGameVSTAudioProcessor::producesMidi() const
{
#if JucePlugin_ProducesMidiOutput
    return true;
#else
    return false;
#endif
}

bool JoyaGameVSTAudioProcessor::isMidiEffect() const
{
#if JucePlugin_IsMidiEffect
    return true;
#else
    return false;
#endif
}

double JoyaGameVSTAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int JoyaGameVSTAudioProcessor::getNumPrograms()
{
    return 1;
}

int JoyaGameVSTAudioProcessor::getCurrentProgram()
{
    return 0;
}

void JoyaGameVSTAudioProcessor::setCurrentProgram(int index)
{
}

const juce::String JoyaGameVSTAudioProcessor::getProgramName(int index)
{
    return {};
}

void JoyaGameVSTAudioProcessor::changeProgramName(int index, const juce::String& newName)
{
}

//==============================================================================
void JoyaGameVSTAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    synth.setCurrentPlaybackSampleRate(sampleRate);
}

void JoyaGameVSTAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool JoyaGameVSTAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
#if JucePlugin_IsMidiEffect
    juce::ignoreUnused(layouts);
    return true;
#else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
        && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
#if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
#endif

    return true;
#endif
}
#endif

void JoyaGameVSTAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    synth.renderNextBlock(buffer, midiMessages, 0, buffer.getNumSamples());
}

//==============================================================================
bool JoyaGameVSTAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* JoyaGameVSTAudioProcessor::createEditor()
{
    return new JoyaGameVSTAudioProcessorEditor(*this);
}

//==============================================================================
void JoyaGameVSTAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
}

void JoyaGameVSTAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
}

void JoyaGameVSTAudioProcessor::triggerBellSound()
{
    synth.noteOn(0, 60, 1.0f);
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new JoyaGameVSTAudioProcessor();
}