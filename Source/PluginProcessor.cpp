#include "PluginProcessor.h"
#include "PluginEditor.h"

VUMisterNAMAudioProcessor::VUMisterNAMAudioProcessor()
    : AudioProcessor (BusesProperties()
        .withInput ("Input", juce::AudioChannelSet::stereo(), true)
        .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      parameters (*this, nullptr, "PARAMETERS", createParameterLayout())
{
}

juce::AudioProcessorValueTreeState::ParameterLayout
VUMisterNAMAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    layout.add (std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID {"input", 1}, "Input", juce::NormalisableRange<float> (-24.0f, 24.0f, 0.1f), 0.0f, " dB"));
    layout.add (std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID {"output", 1}, "Output", juce::NormalisableRange<float> (-24.0f, 24.0f, 0.1f), 0.0f, " dB"));
    layout.add (std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID {"mix", 1}, "Mix", juce::NormalisableRange<float> (0.0f, 100.0f, 0.1f), 100.0f, "%"));
    layout.add (std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID {"bypass", 1}, "Bypass", false));
    return layout;
}

void VUMisterNAMAudioProcessor::prepareToPlay (double sampleRate, int)
{
    currentSampleRate = sampleRate;
}

void VUMisterNAMAudioProcessor::releaseResources() {}

bool VUMisterNAMAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto in = layouts.getMainInputChannelSet();
    const auto out = layouts.getMainOutputChannelSet();
    return (in == juce::AudioChannelSet::mono() || in == juce::AudioChannelSet::stereo())
        && in == out;
}

void VUMisterNAMAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer,
                                               juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    const int channels = getTotalNumInputChannels();
    const int samples = buffer.getNumSamples();
    for (int ch = getTotalNumOutputChannels(); ch < buffer.getNumChannels(); ++ch)
        buffer.clear (ch, 0, samples);

    float peakIn = 0.0f;
    float peakOut = 0.0f;
    const float inGain = juce::Decibels::decibelsToGain (
        parameters.getRawParameterValue ("input")->load());
    const float outGain = juce::Decibels::decibelsToGain (
        parameters.getRawParameterValue ("output")->load());
    const float mix = parameters.getRawParameterValue ("mix")->load() * 0.01f;
    const bool bypass = parameters.getRawParameterValue ("bypass")->load() > 0.5f;

    for (int ch = 0; ch < channels; ++ch)
    {
        auto* data = buffer.getWritePointer (ch);
        for (int i = 0; i < samples; ++i)
        {
            const float dry = data[i];
            peakIn = juce::jmax (peakIn, std::abs (dry));
            const float processed = dry * inGain; // NAM inference is not wired into this starter build.
            const float wet = bypass ? dry : (dry + (processed - dry) * mix) * outGain;
            data[i] = wet;
            peakOut = juce::jmax (peakOut, std::abs (wet));
        }
    }
    inputMeterDb.store (juce::Decibels::gainToDecibels (peakIn, -60.0f));
    outputMeterDb.store (juce::Decibels::gainToDecibels (peakOut, -60.0f));
}

juce::AudioProcessorEditor* VUMisterNAMAudioProcessor::createEditor()
{
    return new VUMisterNAMAudioProcessorEditor (*this);
}

void VUMisterNAMAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = parameters.copyState();
    state.setProperty ("namPath", namPath, nullptr);
    if (auto xml = state.createXml())
        copyXmlToBinary (*xml, destData);
}

void VUMisterNAMAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary (data, sizeInBytes))
    {
        if (xml->hasTagName (parameters.state.getType()))
        {
            auto state = juce::ValueTree::fromXml (*xml);
            parameters.replaceState (state);
            namPath = state.getProperty ("namPath").toString();
            if (namPath.isNotEmpty() && juce::File (namPath).existsAsFile())
                loadNamFile (juce::File (namPath));
            else if (namPath.isNotEmpty())
                namStatus = "Saved NAM file not found";
        }
    }
}

juce::String VUMisterNAMAudioProcessor::getNamPath() const { return namPath; }
juce::String VUMisterNAMAudioProcessor::getNamStatus() const { return namStatus; }

bool VUMisterNAMAudioProcessor::loadNamFile (const juce::File& file)
{
    if (! file.existsAsFile() || file.getFileExtension().compareIgnoreCase (".nam") != 0)
    {
        namStatus = "Invalid file: choose a .nam file";
        return false;
    }

    // Validate the basic JSON envelope only. This does NOT instantiate a NAM neural model.
    juce::var parsed;
    const auto parseResult = juce::JSON::parse (file.loadFileAsString(), parsed);
    if (parseResult.failed() || ! parsed.isObject()
        || ! parsed.getDynamicObject()->hasProperty ("architecture")
        || ! parsed.getDynamicObject()->hasProperty ("weights"))
    {
        namStatus = "Invalid NAM JSON/model structure";
        return false;
    }

    namPath = file.getFullPathName();
    const auto architecture = parsed.getProperty ("architecture", {}).toString();
    namStatus = "Validated " + architecture + " file (not running model)";
    return true;
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new VUMisterNAMAudioProcessor();
}
