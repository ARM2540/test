#pragma once
#include <JuceHeader.h>
#include <atomic>

class VUMisterNAMAudioProcessor final : public juce::AudioProcessor
{
public:
    VUMisterNAMAudioProcessor();
    ~VUMisterNAMAudioProcessor() override = default;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }
    const juce::String getName() const override { return JucePlugin_Name; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState parameters;
    float getInputMeterDb() const noexcept { return inputMeterDb.load(); }
    float getOutputMeterDb() const noexcept { return outputMeterDb.load(); }
    juce::String getNamPath() const;
    juce::String getNamStatus() const;
    bool loadNamFile (const juce::File& file);

private:
    std::atomic<float> inputMeterDb {-60.0f};
    std::atomic<float> outputMeterDb {-60.0f};
    juce::String namPath;
    juce::String namStatus {"No NAM model selected"};
    double currentSampleRate = 44100.0;

    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VUMisterNAMAudioProcessor)
};
