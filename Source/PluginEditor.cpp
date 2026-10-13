#include "PluginEditor.h"

static constexpr auto bg = juce::Colour (0xff17191b);
static constexpr auto panel = juce::Colour (0xff292c2e);
static constexpr auto cream = juce::Colour (0xfff0e4bd);
static constexpr auto amber = juce::Colour (0xffffa52f);

VUMisterNAMAudioProcessorEditor::VUMisterNAMAudioProcessorEditor (VUMisterNAMAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    setSize (760, 480);
    titleLabel.setText ("VU MISTER  /  NAM", juce::dontSendNotification);
    titleLabel.setFont (juce::Font (juce::FontOptions (25.0f, juce::Font::bold)));
    titleLabel.setColour (juce::Label::textColourId, cream);
    addAndMakeVisible (titleLabel);

    auto setupSlider = [this] (juce::Slider& s, const juce::String& name)
    {
        s.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
        s.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 76, 22);
        s.setColour (juce::Slider::rotarySliderFillColourId, amber);
        s.setColour (juce::Slider::rotarySliderOutlineColourId, juce::Colour (0xff51575b));
        s.setColour (juce::Slider::thumbColourId, cream);
        s.setColour (juce::Slider::textBoxTextColourId, cream);
        s.setColour (juce::Slider::textBoxBackgroundColourId, panel);
        s.setName (name);
        addAndMakeVisible (s);
    };
    setupSlider (inputSlider, "INPUT");
    setupSlider (outputSlider, "OUTPUT");
    setupSlider (mixSlider, "MIX");

    inputSlider.setRange (-24.0, 24.0, 0.1);
    outputSlider.setRange (-24.0, 24.0, 0.1);
    mixSlider.setRange (0.0, 100.0, 0.1);
    inputSlider.setTextValueSuffix (" dB");
    outputSlider.setTextValueSuffix (" dB");
    mixSlider.setTextValueSuffix (" %");

    inputSlider.setValue (p.parameters.getRawParameterValue ("input")->load());
    outputSlider.setValue (p.parameters.getRawParameterValue ("output")->load());
    mixSlider.setValue (p.parameters.getRawParameterValue ("mix")->load());

    inputSlider.onValueChange = [this] { audioProcessor.parameters.getParameterAsValue ("input").setValue (inputSlider.getValue()); };
    outputSlider.onValueChange = [this] { audioProcessor.parameters.getParameterAsValue ("output").setValue (outputSlider.getValue()); };
    mixSlider.onValueChange = [this] { audioProcessor.parameters.getParameterAsValue ("mix").setValue (mixSlider.getValue()); };

    bypassButton.setColour (juce::ToggleButton::textColourId, cream);
    bypassButton.onClick = [this] {
        audioProcessor.parameters.getParameterAsValue ("bypass").setValue (bypassButton.getToggleState());
    };
    addAndMakeVisible (bypassButton);

    loadButton.setColour (juce::TextButton::buttonColourId, amber);
    loadButton.setColour (juce::TextButton::textColourOffId, bg);
    loadButton.onClick = [this]
    {
        fileChooser = std::make_unique<juce::FileChooser> ("Choose a NAM model", juce::File{}, "*.nam");
        fileChooser->launchAsync (juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
            [this] (const juce::FileChooser& chooser)
            {
                const auto file = chooser.getResult();
                if (file != juce::File{})
                    audioProcessor.loadNamFile (file);
                modelLabel.setText (audioProcessor.getNamPath().isNotEmpty()
                    ? juce::File (audioProcessor.getNamPath()).getFileName() : "No model selected",
                    juce::dontSendNotification);
                statusLabel.setText (audioProcessor.getNamStatus(), juce::dontSendNotification);
            });
    };
    addAndMakeVisible (loadButton);

    modelLabel.setColour (juce::Label::textColourId, cream);
    modelLabel.setText ("No model selected", juce::dontSendNotification);
    addAndMakeVisible (modelLabel);
    statusLabel.setColour (juce::Label::textColourId, juce::Colour (0xffd0b875));
    statusLabel.setFont (juce::Font (juce::FontOptions (12.0f)));
    statusLabel.setText ("NAM model validation only — DSP engine not integrated", juce::dontSendNotification);
    addAndMakeVisible (statusLabel);

    startTimerHz (30);
}

void VUMisterNAMAudioProcessorEditor::drawMeter (juce::Graphics& g,
                                                  juce::Rectangle<float> r,
                                                  float db, const juce::String& label)
{
    g.setColour (juce::Colour (0xff080909));
    g.fillRoundedRectangle (r, 12.0f);
    auto face = r.reduced (8.0f);
    g.setColour (cream);
    g.fillRoundedRectangle (face, 7.0f);

    const auto center = face.getCentre();
    const float radius = face.getWidth() * 0.40f;
    g.setColour (juce::Colour (0xff333333));
    g.setFont (juce::Font (juce::FontOptions (12.0f, juce::Font::bold)));
    for (int i = -20; i <= 3; i += 5)
    {
        const float norm = (float) (i + 20) / 23.0f;
        const float angle = juce::degreesToRadians (-135.0f + norm * 180.0f);
        const juce::Point<float> outer (center.x + std::cos (angle) * radius,
                                        center.y + std::sin (angle) * radius);
        const juce::Point<float> inner (center.x + std::cos (angle) * (radius - 10.0f),
                                        center.y + std::sin (angle) * (radius - 10.0f));
        g.drawLine ({inner, outer}, 2.0f);
        const juce::Point<float> textPt (center.x + std::cos (angle) * (radius - 26.0f),
                                         center.y + std::sin (angle) * (radius - 26.0f));
        g.drawText (juce::String (i), juce::Rectangle<float> (textPt.x - 13, textPt.y - 9, 26, 18),
                    juce::Justification::centred);
    }

    const float clamped = juce::jlimit (-20.0f, 3.0f, db);
    const float norm = (clamped + 20.0f) / 23.0f;
    const float angle = juce::degreesToRadians (-135.0f + norm * 180.0f);
    const juce::Point<float> tip (center.x + std::cos (angle) * (radius - 13.0f),
                                  center.y + std::sin (angle) * (radius - 13.0f));
    g.setColour (juce::Colour (0xffb51f1f));
    g.drawLine ({center.x, center.y, tip.x, tip.y}, 3.0f);
    g.setColour (juce::Colour (0xff333333));
    g.fillEllipse (center.x - 6, center.y - 6, 12, 12);
    g.setColour (juce::Colour (0xff242424));
    g.setFont (juce::Font (juce::FontOptions (16.0f, juce::Font::bold)));
    g.drawText (label, face.withY (face.getBottom() - 31.0f).withHeight (22.0f),
                juce::Justification::centred);
}

void VUMisterNAMAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (bg);
    g.setColour (panel);
    g.fillRoundedRectangle (18.0f, 58.0f, 724.0f, 310.0f, 16.0f);
    g.setColour (amber);
    g.fillRect (18.0f, 58.0f, 724.0f, 4.0f);
    drawMeter (g, {38, 76, 330, 270}, audioProcessor.getInputMeterDb(), "INPUT");
    drawMeter (g, {392, 76, 330, 270}, audioProcessor.getOutputMeterDb(), "OUTPUT");
    g.setColour (juce::Colour (0xff73797c));
    g.drawText ("ANALOG STYLE  •  VST3  •  WINDOWS", 24, 448, 710, 18,
                juce::Justification::centred);
}

void VUMisterNAMAudioProcessorEditor::resized()
{
    titleLabel.setBounds (24, 10, 450, 40);
    inputSlider.setBounds (54, 365, 125, 90);
    outputSlider.setBounds (190, 365, 125, 90);
    mixSlider.setBounds (326, 365, 125, 90);
    loadButton.setBounds (480, 374, 120, 32);
    bypassButton.setBounds (610, 374, 105, 32);
    modelLabel.setBounds (480, 410, 240, 20);
    statusLabel.setBounds (480, 430, 240, 32);
}

void VUMisterNAMAudioProcessorEditor::timerCallback()
{
    repaint (38, 76, 684, 270);
}

