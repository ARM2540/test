#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

class VUMisterNAMAudioProcessorEditor final : public juce::AudioProcessorEditor,
                                               private juce::Timer
{
public:
    explicit VUMisterNAMAudioProcessorEditor (VUMisterNAMAudioProcessor&);
    ~VUMisterNAMAudioProcessorEditor() override = default;
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;
    void drawMeter (juce::Graphics&, juce::Rectangle<float>, float, const juce::String&);
    VUMisterNAMAudioProcessor& audioProcessor;
    juce::Slider inputSlider, outputSlider, mixSlider;
    juce::ToggleButton bypassButton {"BYPASS"};
    juce::TextButton loadButton {"LOAD .NAM"};
    juce::Label titleLabel, modelLabel, statusLabel;
    std::unique_ptr<juce::FileChooser> fileChooser;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VUMisterNAMAudioProcessorEditor)
};
