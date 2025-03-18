#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
PluginV3AudioProcessorEditor::PluginV3AudioProcessorEditor (PluginV3AudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Set up the level meters
    leftMeter.setVertical(true);
    leftMeter.showPeakMarker(true);
    leftMeter.setMeterColour(juce::Colours::green, juce::Colours::yellow, juce::Colours::red);
    addAndMakeVisible(leftMeter);
    
    rightMeter.setVertical(true);
    rightMeter.showPeakMarker(true);
    rightMeter.setMeterColour(juce::Colours::green, juce::Colours::yellow, juce::Colours::red);
    addAndMakeVisible(rightMeter);
    
    // Set up meter labels
    leftMeterLabel.setText("L", juce::dontSendNotification);
    leftMeterLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(leftMeterLabel);
    
    rightMeterLabel.setText("R", juce::dontSendNotification);
    rightMeterLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(rightMeterLabel);
    
    // Set up common properties for all knobs
    auto setupGainKnob = [this](juce::Slider& knob, juce::Label& label, const juce::String& text, bool isMaster = false) {
        knob.setSliderStyle(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
        knob.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 15);
        knob.setDoubleClickReturnValue(true, 1.0f); // Reset to 0dB on double click
        knob.setTextValueSuffix(" dB");
        
        // Make master knob use a different color to stand out
        if (isMaster) {
            knob.setColour(juce::Slider::thumbColourId, juce::Colours::orangered.brighter(0.2f));
            knob.setColour(juce::Slider::rotarySliderFillColourId, juce::Colours::orange.brighter(0.2f));
        } else {
            knob.setColour(juce::Slider::thumbColourId, juce::Colours::orangered);
            knob.setColour(juce::Slider::rotarySliderFillColourId, juce::Colours::orange);
        }
        
        knob.setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colours::darkgrey);
        
        // Set the rotation parameters to make noon at 0dB
        knob.setRotaryParameters(juce::MathConstants<float>::pi * 1.2f, 
                                juce::MathConstants<float>::pi * 2.8f, 
                                true);
        
        // Custom value to text conversion for dB display
        knob.textFromValueFunction = [](double value) {
            if (value <= 0.001)
                return juce::String("-inf dB");
            return juce::String(20.0 * std::log10(value), 1) + " dB";
        };
        
        addAndMakeVisible(knob);
        
        label.setText(text, juce::dontSendNotification);
        label.setJustificationType(juce::Justification::centred);
        label.attachToComponent(&knob, false);
        addAndMakeVisible(label);
    };
    
    // Set up the master gain knob (make it larger and distinctive)
    setupGainKnob(masterGainKnob, masterGainLabel, "Master", true);
    
    // Set up the left channel gain knob
    setupGainKnob(leftGainKnob, leftGainLabel, "Left");
    
    // Set up the right channel gain knob
    setupGainKnob(rightGainKnob, rightGainLabel, "Right");
    
    // Set up the mid gain knob with unique color
    midGainKnob.setSliderStyle(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
    midGainKnob.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 15);
    midGainKnob.setDoubleClickReturnValue(true, 1.0f);
    midGainKnob.setTextValueSuffix(" dB");
    midGainKnob.setColour(juce::Slider::thumbColourId, juce::Colours::purple);
    midGainKnob.setColour(juce::Slider::rotarySliderFillColourId, juce::Colours::mediumpurple);
    midGainKnob.setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colours::darkgrey);
    midGainKnob.setRotaryParameters(juce::MathConstants<float>::pi * 1.2f, 
                              juce::MathConstants<float>::pi * 2.8f, 
                              true);
    midGainKnob.textFromValueFunction = [](double value) {
        if (value <= 0.001)
            return juce::String("-inf dB");
        return juce::String(20.0 * std::log10(value), 1) + " dB";
    };
    addAndMakeVisible(midGainKnob);
    
    midGainLabel.setText("Mid", juce::dontSendNotification);
    midGainLabel.setJustificationType(juce::Justification::centred);
    midGainLabel.attachToComponent(&midGainKnob, false);
    addAndMakeVisible(midGainLabel);
    
    // Set up the side gain knob with unique color
    sideGainKnob.setSliderStyle(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
    sideGainKnob.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 15);
    sideGainKnob.setDoubleClickReturnValue(true, 1.0f);
    sideGainKnob.setTextValueSuffix(" dB");
    sideGainKnob.setColour(juce::Slider::thumbColourId, juce::Colours::magenta);
    sideGainKnob.setColour(juce::Slider::rotarySliderFillColourId, juce::Colours::hotpink);
    sideGainKnob.setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colours::darkgrey);
    sideGainKnob.setRotaryParameters(juce::MathConstants<float>::pi * 1.2f, 
                              juce::MathConstants<float>::pi * 2.8f, 
                              true);
    sideGainKnob.textFromValueFunction = [](double value) {
        if (value <= 0.001)
            return juce::String("-inf dB");
        return juce::String(20.0 * std::log10(value), 1) + " dB";
    };
    addAndMakeVisible(sideGainKnob);
    
    sideGainLabel.setText("Side", juce::dontSendNotification);
    sideGainLabel.setJustificationType(juce::Justification::centred);
    sideGainLabel.attachToComponent(&sideGainKnob, false);
    addAndMakeVisible(sideGainLabel);
    
    // Set up enable Mid/Side toggle button
    enableMidSideButton.setButtonText("Enable Mid/Side");
    enableMidSideButton.setColour(juce::ToggleButton::tickColourId, juce::Colours::magenta);
    enableMidSideButton.setColour(juce::ToggleButton::tickDisabledColourId, juce::Colours::darkgrey);
    addAndMakeVisible(enableMidSideButton);
    
    // Set up the link gain button
    linkGainButton.setButtonText("Link L/R");
    linkGainButton.setColour(juce::ToggleButton::tickColourId, juce::Colours::orangered);
    linkGainButton.setColour(juce::ToggleButton::tickDisabledColourId, juce::Colours::darkgrey);
    linkGainButton.onClick = [this]() {
        gainKnobsLinked = linkGainButton.getToggleState();
        
        // If linked and values are different, sync the right knob to the left knob's value
        if (gainKnobsLinked && std::abs(leftGainKnob.getValue() - rightGainKnob.getValue()) > 0.001) {
            rightGainKnob.setValue(leftGainKnob.getValue());
        }
    };
    addAndMakeVisible(linkGainButton);
    
    // Set up value change listeners for the gain knobs to implement linking
    leftGainKnob.onValueChange = [this]() {
        if (gainKnobsLinked) {
            // Temporarily remove the right knob's listener to avoid feedback loops
            auto rightListener = rightGainKnob.onValueChange;
            rightGainKnob.onValueChange = nullptr;
            
            // Sync right knob value to left knob
            rightGainKnob.setValue(leftGainKnob.getValue());
            
            // Restore the right knob's listener
            rightGainKnob.onValueChange = rightListener;
        }
    };
    
    rightGainKnob.onValueChange = [this]() {
        if (gainKnobsLinked) {
            // Temporarily remove the left knob's listener to avoid feedback loops
            auto leftListener = leftGainKnob.onValueChange;
            leftGainKnob.onValueChange = nullptr;
            
            // Sync left knob value to right knob
            leftGainKnob.setValue(rightGainKnob.getValue());
            
            // Restore the left knob's listener
            leftGainKnob.onValueChange = leftListener;
        }
    };
    
    // Set up phase invert buttons
    invertLeftButton.setButtonText("Invert Phase");
    invertLeftButton.setColour(juce::ToggleButton::tickColourId, juce::Colours::orangered);
    invertLeftButton.setColour(juce::ToggleButton::tickDisabledColourId, juce::Colours::darkgrey);
    invertLeftButton.onClick = [this]() {
        // Update the stereo placement visualization when phase inversion changes
        stereoPlacement.setPhaseInversion(invertLeftButton.getToggleState(), invertRightButton.getToggleState());
    };
    addAndMakeVisible(invertLeftButton);
    
    invertRightButton.setButtonText("Invert Phase");
    invertRightButton.setColour(juce::ToggleButton::tickColourId, juce::Colours::orangered);
    invertRightButton.setColour(juce::ToggleButton::tickDisabledColourId, juce::Colours::darkgrey);
    invertRightButton.onClick = [this]() {
        // Update the stereo placement visualization when phase inversion changes
        stereoPlacement.setPhaseInversion(invertLeftButton.getToggleState(), invertRightButton.getToggleState());
    };
    addAndMakeVisible(invertRightButton);
    
    // Set up phase offset slider
    phaseOffsetSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
    phaseOffsetSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 15);
    phaseOffsetSlider.setRange(0.0, 360.0, 0.1);
    phaseOffsetSlider.setDoubleClickReturnValue(true, 0.0);
    phaseOffsetSlider.setTextValueSuffix("°");
    phaseOffsetSlider.setColour(juce::Slider::thumbColourId, juce::Colours::cyan);
    phaseOffsetSlider.setColour(juce::Slider::rotarySliderFillColourId, juce::Colours::lightblue);
    phaseOffsetSlider.setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colours::darkgrey);
    addAndMakeVisible(phaseOffsetSlider);
    
    phaseOffsetLabel.setText("Phase Offset", juce::dontSendNotification);
    phaseOffsetLabel.setJustificationType(juce::Justification::centred);
    phaseOffsetLabel.attachToComponent(&phaseOffsetSlider, false);
    addAndMakeVisible(phaseOffsetLabel);
    
    // Set up the stereo placement component
    addAndMakeVisible(stereoPlacement);
    
    stereoPlacementLabel.setText("Stereo Placement", juce::dontSendNotification);
    stereoPlacementLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(stereoPlacementLabel);
    
    // Connect controls to parameters
    masterGainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getAPVTS(), "master_gain", masterGainKnob);
    
    leftGainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getAPVTS(), "left_gain", leftGainKnob);
    
    rightGainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getAPVTS(), "right_gain", rightGainKnob);
    
    invertLeftAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        audioProcessor.getAPVTS(), "invert_left", invertLeftButton);
    
    invertRightAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        audioProcessor.getAPVTS(), "invert_right", invertRightButton);
    
    phaseOffsetAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getAPVTS(), "phase_offset", phaseOffsetSlider);
    
    midGainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getAPVTS(), "mid_gain", midGainKnob);
    
    sideGainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getAPVTS(), "side_gain", sideGainKnob);
    
    enableMidSideAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        audioProcessor.getAPVTS(), "use_mid_side", enableMidSideButton);
    
    // Start the timer for faster meter updates
    startTimerHz(60); // 60fps for smoother animation
    
    // Set editor size - increased height to ensure everything fits properly
    setSize (650, 600);
}

PluginV3AudioProcessorEditor::~PluginV3AudioProcessorEditor()
{
    stopTimer();
}

//==============================================================================
void PluginV3AudioProcessorEditor::paint (juce::Graphics& g)
{
    // Fill the background
    g.fillAll(backgroundColour);
    
    // Draw a border around the plugin
    g.setColour(juce::Colours::white.withAlpha(0.2f));
    g.drawRoundedRectangle(getLocalBounds().toFloat().reduced(5.0f), 5.0f, 2.0f);
    
    // Draw a title for the plugin
    g.setColour(juce::Colours::white);
    g.setFont(18.0f);
    g.drawText("Stereo Gain & Phase Plugin", getLocalBounds().removeFromTop(30), juce::Justification::centred, true);
}

void PluginV3AudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds().reduced(10);
    
    // Reserve space for the title
    bounds.removeFromTop(30);
    
    // Create sections for our layout
    auto meterSection = bounds.removeFromRight(120); // Wider to accommodate equal-sized meters
    
    // Position the meters with equal width
    auto leftMeterBounds = meterSection.removeFromLeft(meterSection.getWidth() / 2);
    auto rightMeterBounds = meterSection;
    
    leftMeterLabel.setBounds(leftMeterBounds.removeFromTop(20));
    rightMeterLabel.setBounds(rightMeterBounds.removeFromTop(20));
    
    // Give a bit more space around the meters for the labels
    leftMeter.setBounds(leftMeterBounds.reduced(5, 2));
    rightMeter.setBounds(rightMeterBounds.reduced(5, 2));
    
    // Position the controls
    auto controlSection = bounds;
    
    // Top row for stereo placement and master gain
    auto topRowHeight = controlSection.getHeight() * 0.35f;
    auto topRow = controlSection.removeFromTop(topRowHeight);
    
    // Left side of top row for stereo placement
    auto stereoPlacementSection = topRow.removeFromLeft(topRow.getWidth() * 0.6f);
    
    // Position the stereo placement visualization
    auto stereoPlacementBounds = stereoPlacementSection.reduced(15);
    stereoPlacementLabel.setBounds(stereoPlacementBounds.removeFromTop(20));
    stereoPlacement.setBounds(stereoPlacementBounds);
    
    // Right side of top row for master gain and phase offset
    auto masterAndPhaseSection = topRow;
    
    // Upper part for master gain
    auto masterSection = masterAndPhaseSection.removeFromTop(masterAndPhaseSection.getHeight() * 0.5f);
    masterGainKnob.setBounds(masterSection.reduced(15));
    
    // Lower part for phase offset
    auto phaseSection = masterAndPhaseSection;
    phaseOffsetSlider.setBounds(phaseSection.reduced(15));
    
    // Middle row for Mid/Side controls
    auto middleRowHeight = controlSection.getHeight() * 0.3f;
    auto middleRow = controlSection.removeFromTop(middleRowHeight);
    
    // Split middle row for Mid and Side knobs
    auto midSection = middleRow.removeFromLeft(middleRow.getWidth() / 2);
    auto sideSection = middleRow;
    
    // Position Mid/Side knobs with reduced width to make room for the enable button
    auto midKnobBounds = midSection.reduced(15);
    midKnobBounds.removeFromRight(midKnobBounds.getWidth() * 0.25f); // Make room on the right side
    midGainKnob.setBounds(midKnobBounds);
    
    auto sideKnobBounds = sideSection.reduced(15);
    sideKnobBounds.removeFromLeft(sideKnobBounds.getWidth() * 0.25f); // Make room on the left side
    sideGainKnob.setBounds(sideKnobBounds);
    
    // Position Enable Mid/Side button directly between Mid and Side knobs
    auto enableButtonWidth = 110;
    auto enableButtonHeight = 30;
    auto enableButtonBounds = juce::Rectangle<int>(
        midSection.getRight() - enableButtonWidth / 2,
        midSection.getY() + midSection.getHeight() / 2 - enableButtonHeight / 2,
        enableButtonWidth,
        enableButtonHeight
    );
    enableMidSideButton.setBounds(enableButtonBounds);
    
    // Bottom row for left and right channel controls
    auto bottomRow = controlSection;
    auto leftSection = bottomRow.removeFromLeft(bottomRow.getWidth() * 0.5f);
    auto rightSection = bottomRow;
    
    // Position the gain knobs in the upper part of each section
    auto leftGainSection = leftSection.removeFromTop(leftSection.getHeight() * 0.6f);
    leftGainKnob.setBounds(leftGainSection.reduced(15));
    
    auto rightGainSection = rightSection.removeFromTop(rightSection.getHeight() * 0.6f);
    rightGainKnob.setBounds(rightGainSection.reduced(15));
    
    // Create a dedicated section for the link button in the center
    int linkButtonWidth = 80;
    int linkButtonHeight = 30;
    auto linkButtonY = leftSection.getY() + 5; // Position it at the top of the lower section
    auto linkButtonBounds = juce::Rectangle<int>(
        (getWidth() / 2) - (linkButtonWidth / 2),
        linkButtonY,
        linkButtonWidth,
        linkButtonHeight
    );
    linkGainButton.setBounds(linkButtonBounds);
    
    // Clear space at the top of button sections for the link button
    auto linkButtonSpace = 40;
    auto leftButtonSection = leftSection.withTrimmedTop(linkButtonSpace);
    auto rightButtonSection = rightSection.withTrimmedTop(linkButtonSpace);
    
    // Position phase invert buttons below the link button
    invertLeftButton.setBounds(leftButtonSection.reduced(15, 5));
    invertRightButton.setBounds(rightButtonSection.reduced(15, 5));
}

void PluginV3AudioProcessorEditor::timerCallback()
{
    // Get the current level values from the processor
    auto leftLevel = audioProcessor.getLeftChannelLevel();
    auto rightLevel = audioProcessor.getRightChannelLevel();
    
    // Debug level values 
    // DBG("Left level: " + juce::String(leftLevel) + " Right level: " + juce::String(rightLevel));
    
    // Update the meters
    leftMeter.setLevel(leftLevel);
    rightMeter.setLevel(rightLevel);
    
    // Update the stereo placement visualization
    stereoPlacement.setLevels(leftLevel, rightLevel);
}
