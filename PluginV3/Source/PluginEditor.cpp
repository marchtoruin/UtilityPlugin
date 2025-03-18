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
    
    // Set up a common function for gain knob setup
    auto setupGainKnob = [this](juce::Slider& knob, juce::Label& label, const juce::String& text, bool isMaster = false) {
        knob.setSliderStyle(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
        knob.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        knob.setDoubleClickReturnValue(true, 1.0f); // Reset to 0dB (unity gain) on double click
        knob.setName(text + " Gain"); // Set a name that includes 'gain' for our look and feel
        
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
        // Use standard angles: start at 7 o'clock (-135°), end at 5 o'clock (135°), with 0dB at 12 o'clock (0°)
        knob.setRotaryParameters(juce::MathConstants<float>::pi * -0.75f,  // -135 degrees
                               juce::MathConstants<float>::pi * 0.75f,    // 135 degrees
                               true);  // stops at end
        
        // Custom value to text conversion for dB display
        knob.textFromValueFunction = [](double value) {
            // For values close to 1.0 (0.95 to 1.05), display as "0.0 dB"
            if (value <= 0.001)
                return juce::String("-inf dB");
            else if (value >= 0.95 && value <= 1.05) // Much wider tolerance to catch all values near 1.0
                return juce::String("0.0 dB");
            else
                return juce::String(20.0 * std::log10(value), 1) + " dB";
        };
        
        // Label for the knob - don't attach directly to avoid overlap problems
        label.setText(text, juce::dontSendNotification);
        label.setJustificationType(juce::Justification::centred);
        
        // Apply our custom look and feel
        knob.setLookAndFeel(&zeroDBAtNoonLookAndFeel);
        
        addAndMakeVisible(knob);
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
    midGainKnob.setDoubleClickReturnValue(true, 1.0f); // Reset to 0dB (unity gain) on double click
    midGainKnob.setTextValueSuffix(" dB");
    midGainKnob.setName("Mid Gain");
    midGainKnob.setColour(juce::Slider::thumbColourId, juce::Colours::purple);
    midGainKnob.setColour(juce::Slider::rotarySliderFillColourId, juce::Colours::mediumpurple);
    midGainKnob.setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colours::darkgrey);
    
    // Use standard rotary parameters for consistency, with 0° at 12 o'clock
    midGainKnob.setRotaryParameters(juce::MathConstants<float>::pi * -0.75f,  // -135 degrees
                                    juce::MathConstants<float>::pi * 0.75f,    // 135 degrees
                                    true);
    
    // Apply the custom look and feel to ensure 0dB is at noon
    midGainKnob.setLookAndFeel(&zeroDBAtNoonLookAndFeel);
    
    midGainKnob.textFromValueFunction = [](double value) {
        if (value <= 0.001)
            return juce::String("-inf dB");
        else if (value >= 0.95 && value <= 1.05) // Much wider tolerance to catch all values near 1.0
            return juce::String("0.0 dB");
        else
            return juce::String(20.0 * std::log10(value), 1) + " dB";
    };
    
    // Match the parameter's normalized range
    midGainKnob.setNormalisableRange(juce::NormalisableRange<double>(0.0, 3.16227766017, 0.001, 0.3f));
    
    addAndMakeVisible(midGainKnob);
    
    midGainLabel.setText("Mid", juce::dontSendNotification);
    midGainLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(midGainLabel);
    
    // Set up the side gain knob with unique color
    sideGainKnob.setSliderStyle(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
    sideGainKnob.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 15);
    sideGainKnob.setDoubleClickReturnValue(true, 1.0f); // Reset to 0dB (unity gain) on double click
    sideGainKnob.setTextValueSuffix(" dB");
    sideGainKnob.setName("Side Gain");
    sideGainKnob.setColour(juce::Slider::thumbColourId, juce::Colours::magenta);
    sideGainKnob.setColour(juce::Slider::rotarySliderFillColourId, juce::Colours::hotpink);
    sideGainKnob.setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colours::darkgrey);
    
    // Use standard rotary parameters for consistency, with 0° at 12 o'clock
    sideGainKnob.setRotaryParameters(juce::MathConstants<float>::pi * -0.75f,  // -135 degrees
                                     juce::MathConstants<float>::pi * 0.75f,    // 135 degrees
                                     true);
    
    // Apply the custom look and feel to ensure 0dB is at noon
    sideGainKnob.setLookAndFeel(&zeroDBAtNoonLookAndFeel);
    
    sideGainKnob.textFromValueFunction = [](double value) {
        if (value <= 0.001)
            return juce::String("-inf dB");
        else if (value >= 0.95 && value <= 1.05) // Much wider tolerance to catch all values near 1.0
            return juce::String("0.0 dB");
        else
            return juce::String(20.0 * std::log10(value), 1) + " dB";
    };
    
    // Match the parameter's normalized range
    sideGainKnob.setNormalisableRange(juce::NormalisableRange<double>(0.0, 3.16227766017, 0.001, 0.3f));
    
    addAndMakeVisible(sideGainKnob);
    
    sideGainLabel.setText("Side", juce::dontSendNotification);
    sideGainLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(sideGainLabel);
    
    // Set up enable Mid/Side toggle button
    enableMidSideButton.setButtonText("Enable Mid/Side");
    enableMidSideButton.setColour(juce::ToggleButton::tickColourId, juce::Colours::hotpink);
    enableMidSideButton.setColour(juce::ToggleButton::tickDisabledColourId, juce::Colours::darkgrey);
    enableMidSideButton.setColour(juce::ToggleButton::textColourId, juce::Colours::white);
    enableMidSideButton.onClick = [this]() {
        // Update the stereo placement visualization when Mid/Side is toggled
        stereoPlacement.repaint();
    };
    enableMidSideButton.setLookAndFeel(&customToggleLookAndFeel);
    addAndMakeVisible(enableMidSideButton);
    
    // Set up the link gain button
    linkGainButton.setButtonText("Link L/R");
    linkGainButton.setColour(juce::ToggleButton::tickColourId, juce::Colours::orangered);
    linkGainButton.setColour(juce::ToggleButton::tickDisabledColourId, juce::Colours::darkgrey);
    linkGainButton.setColour(juce::ToggleButton::textColourId, juce::Colours::white);
    linkGainButton.setLookAndFeel(&customToggleLookAndFeel);
    linkGainButton.onClick = [this]() {
        gainKnobsLinked = linkGainButton.getToggleState();
        
        // If they're now linked, sync the right gain to match left
        if (gainKnobsLinked) {
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
    invertLeftButton.setButtonText("Invert L Phase");
    invertLeftButton.setColour(juce::ToggleButton::tickColourId, juce::Colours::orangered);
    invertLeftButton.setColour(juce::ToggleButton::tickDisabledColourId, juce::Colours::darkgrey);
    invertLeftButton.setColour(juce::ToggleButton::textColourId, juce::Colours::white);
    invertLeftButton.setLookAndFeel(&customToggleLookAndFeel);
    invertLeftButton.onClick = [this]() {
        // Update the stereo placement visualization when phase inversion changes
        stereoPlacement.setPhaseInversion(invertLeftButton.getToggleState(), invertRightButton.getToggleState());
    };
    addAndMakeVisible(invertLeftButton);
    
    invertRightButton.setButtonText("Invert R Phase");
    invertRightButton.setColour(juce::ToggleButton::tickColourId, juce::Colours::orangered);
    invertRightButton.setColour(juce::ToggleButton::tickDisabledColourId, juce::Colours::darkgrey);
    invertRightButton.setColour(juce::ToggleButton::textColourId, juce::Colours::white);
    invertRightButton.setLookAndFeel(&customToggleLookAndFeel);
    invertRightButton.onClick = [this]() {
        // Update the stereo placement visualization when phase inversion changes
        stereoPlacement.setPhaseInversion(invertLeftButton.getToggleState(), invertRightButton.getToggleState());
    };
    addAndMakeVisible(invertRightButton);
    
    // Set up phase offset slider
    phaseOffsetSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
    phaseOffsetSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    phaseOffsetSlider.setRange(0.0, 360.0, 0.1);
    phaseOffsetSlider.setDoubleClickReturnValue(true, 0.0);
    phaseOffsetSlider.setName("Phase Offset");
    phaseOffsetSlider.setColour(juce::Slider::thumbColourId, juce::Colours::cyan);
    phaseOffsetSlider.setColour(juce::Slider::rotarySliderFillColourId, juce::Colours::lightblue);
    phaseOffsetSlider.setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colours::darkgrey);
    
    // Use standard rotary parameters for consistency, with 0° at 12 o'clock
    phaseOffsetSlider.setRotaryParameters(juce::MathConstants<float>::pi * -0.75f,  // -135 degrees
                                         juce::MathConstants<float>::pi * 0.75f,    // 135 degrees
                                         true);
    
    // Apply the custom look and feel to ensure consistent styling
    phaseOffsetSlider.setLookAndFeel(&zeroDBAtNoonLookAndFeel);
    
    // Make sure normalized range matches parameter range (0-360 degrees)
    phaseOffsetSlider.setNormalisableRange(juce::NormalisableRange<double>(0.0, 360.0, 0.1));
    
    addAndMakeVisible(phaseOffsetSlider);
    
    phaseOffsetLabel.setText("Phase Offset", juce::dontSendNotification);
    phaseOffsetLabel.setJustificationType(juce::Justification::centred);
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
    
    // Set editor size - increased width and height to ensure everything fits properly
    setSize (700, 620);
    
    // Set up sliders with explicit label attachments for the gain display
    setupGainKnob(masterGainKnob, masterGainLabel, "Master", true);
    setupGainKnob(leftGainKnob, leftGainLabel, "Left");
    setupGainKnob(rightGainKnob, rightGainLabel, "Right");
    
    // Force display labels to show "0.0 dB" at startup
    masterGainDisplay.setFont(juce::Font(juce::Font::getDefaultSansSerifFontName(), 14.0f, juce::Font::plain));
    masterGainDisplay.setText("0.0 dB", juce::dontSendNotification);
    masterGainDisplay.setJustificationType(juce::Justification::centred);
    masterGainDisplay.setColour(juce::Label::textColourId, juce::Colours::white);
    masterGainDisplay.setColour(juce::Label::backgroundColourId, juce::Colours::transparentBlack);
    addAndMakeVisible(masterGainDisplay);
    
    // Add a display for phase offset
    phaseOffsetDisplay.setFont(juce::Font(juce::Font::getDefaultSansSerifFontName(), 14.0f, juce::Font::plain));
    phaseOffsetDisplay.setText("0.0°", juce::dontSendNotification);
    phaseOffsetDisplay.setJustificationType(juce::Justification::centred);
    phaseOffsetDisplay.setColour(juce::Label::textColourId, juce::Colours::white);
    phaseOffsetDisplay.setColour(juce::Label::backgroundColourId, juce::Colours::transparentBlack);
    addAndMakeVisible(phaseOffsetDisplay);
    
    leftGainDisplay.setFont(juce::Font(juce::Font::getDefaultSansSerifFontName(), 14.0f, juce::Font::plain));
    leftGainDisplay.setText("0.0 dB", juce::dontSendNotification);
    leftGainDisplay.setJustificationType(juce::Justification::centred);
    leftGainDisplay.setColour(juce::Label::textColourId, juce::Colours::white);
    leftGainDisplay.setColour(juce::Label::backgroundColourId, juce::Colours::transparentBlack);
    addAndMakeVisible(leftGainDisplay);
    
    rightGainDisplay.setFont(juce::Font(juce::Font::getDefaultSansSerifFontName(), 14.0f, juce::Font::plain));
    rightGainDisplay.setText("0.0 dB", juce::dontSendNotification);
    rightGainDisplay.setJustificationType(juce::Justification::centred);
    rightGainDisplay.setColour(juce::Label::textColourId, juce::Colours::white);
    rightGainDisplay.setColour(juce::Label::backgroundColourId, juce::Colours::transparentBlack);
    addAndMakeVisible(rightGainDisplay);
    
    midGainDisplay.setFont(juce::Font(juce::Font::getDefaultSansSerifFontName(), 14.0f, juce::Font::plain));
    midGainDisplay.setText("0.0 dB", juce::dontSendNotification);
    midGainDisplay.setJustificationType(juce::Justification::centred);
    midGainDisplay.setColour(juce::Label::textColourId, juce::Colours::white);
    midGainDisplay.setColour(juce::Label::backgroundColourId, juce::Colours::transparentBlack);
    addAndMakeVisible(midGainDisplay);
    
    sideGainDisplay.setFont(juce::Font(juce::Font::getDefaultSansSerifFontName(), 14.0f, juce::Font::plain));
    sideGainDisplay.setText("0.0 dB", juce::dontSendNotification);
    sideGainDisplay.setJustificationType(juce::Justification::centred);
    sideGainDisplay.setColour(juce::Label::textColourId, juce::Colours::white);
    sideGainDisplay.setColour(juce::Label::backgroundColourId, juce::Colours::transparentBlack);
    addAndMakeVisible(sideGainDisplay);
    
    // Hide the original text boxes
    masterGainKnob.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    leftGainKnob.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    rightGainKnob.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    midGainKnob.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    sideGainKnob.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
}

PluginV3AudioProcessorEditor::~PluginV3AudioProcessorEditor()
{
    // Clean up timer
    stopTimer();
    
    // Remove custom look and feel from all knobs to prevent dangling pointers
    masterGainKnob.setLookAndFeel(nullptr);
    leftGainKnob.setLookAndFeel(nullptr);
    rightGainKnob.setLookAndFeel(nullptr);
    midGainKnob.setLookAndFeel(nullptr);
    sideGainKnob.setLookAndFeel(nullptr);
    
    // Remove custom look and feel from toggle buttons
    invertLeftButton.setLookAndFeel(nullptr);
    invertRightButton.setLookAndFeel(nullptr);
    linkGainButton.setLookAndFeel(nullptr);
    enableMidSideButton.setLookAndFeel(nullptr);
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
    g.setFont(juce::Font(juce::Font::getDefaultSansSerifFontName(), 18.0f, juce::Font::plain));
    g.drawText("Justin's Stereo Sculptor", getLocalBounds().removeFromTop(30), juce::Justification::centred, true);
}

void PluginV3AudioProcessorEditor::resized()
{
    // This is generally called when the component is resized.
    // Update the bounds of our child components.
    auto bounds = getLocalBounds().reduced(20);
    
    // Main area width/height
    auto width = bounds.getWidth();
    auto height = bounds.getHeight();
    
    // Divide the area into sections
    // Top section: 25% of height, used for title and meters
    // Middle section: 50% of height, used for main controls and stereo placement
    // Bottom section: 25% of height, used for additional controls like phase inversion
    
    auto titleHeight = height * 0.1f; // 10% of height for title
    auto meterWidth = width * 0.08f; // Reduced width for meters (was 0.1f)
    auto knobSize = static_cast<int>(height * 0.12f); // Standard knob size
    auto masterKnobSize = static_cast<int>(height * 0.14f); // Larger knob size for master
    auto labelHeight = 20;
    auto buttonHeight = 24; // Define button height
    
    // Title section (top 10%)
    auto titleBounds = bounds.removeFromTop(static_cast<int>(titleHeight));
    
    // Position meters on the right side, aligning bottom with bottom row knobs
    auto metersArea = bounds.removeFromRight(static_cast<int>(meterWidth * 2.2f));
    
    // Main area (remaining after title and meters)
    auto mainArea = bounds;
    auto topSection = mainArea.removeFromTop(static_cast<int>(mainArea.getHeight() * 0.40f));
    
    // Split the bottom area into left (Mid/Side) and right (standard L/R) sections
    auto bottomSection = mainArea;
    auto leftBottomSection = bottomSection.removeFromLeft(static_cast<int>(bottomSection.getWidth() * 0.5f));
    auto rightBottomSection = bottomSection;
    
    // Further divide the left bottom area for Mid/Side controls
    auto midSideArea = leftBottomSection;
    auto midArea = midSideArea.removeFromLeft(static_cast<int>(midSideArea.getWidth() * 0.5f));
    auto sideArea = midSideArea;
    
    // Further divide the right bottom area for Left/Right controls
    auto leftRightArea = rightBottomSection;
    auto leftArea = leftRightArea.removeFromLeft(static_cast<int>(leftRightArea.getWidth() * 0.5f));
    auto rightArea = leftRightArea;
    
    // Add spacing between sections
    const int knobVerticalOffset = static_cast<int>(height * 0.05f);
    
    // Position left/right knobs
    auto leftKnobBounds = juce::Rectangle<int>(
        leftArea.getCentreX() - knobSize / 2,
        leftArea.getCentreY() - knobSize / 2 - knobVerticalOffset,
        knobSize,
        knobSize
    );
    leftGainKnob.setBounds(leftKnobBounds);
    
    // Position the label above the knob
    leftGainLabel.setBounds(
        leftKnobBounds.getX(),
        leftKnobBounds.getY() - labelHeight - 5,
        leftKnobBounds.getWidth(),
        labelHeight
    );
    
    // Position custom display label under knob
    auto leftDisplayBounds = juce::Rectangle<int>(
        leftKnobBounds.getX(), 
        leftKnobBounds.getBottom() + 5, 
        leftKnobBounds.getWidth(), 
        20
    );
    leftGainDisplay.setBounds(leftDisplayBounds);
    
    auto rightKnobBounds = juce::Rectangle<int>(
        rightArea.getCentreX() - knobSize / 2,
        rightArea.getCentreY() - knobSize / 2 - knobVerticalOffset,
        knobSize,
        knobSize
    );
    rightGainKnob.setBounds(rightKnobBounds);
    
    // Position the label above the knob
    rightGainLabel.setBounds(
        rightKnobBounds.getX(),
        rightKnobBounds.getY() - labelHeight - 5,
        rightKnobBounds.getWidth(),
        labelHeight
    );
    
    // Position custom display label under knob
    auto rightDisplayBounds = juce::Rectangle<int>(
        rightKnobBounds.getX(), 
        rightKnobBounds.getBottom() + 5, 
        rightKnobBounds.getWidth(), 
        20
    );
    rightGainDisplay.setBounds(rightDisplayBounds);
    
    // Position Mid/Side knobs
    auto midKnobBounds = juce::Rectangle<int>(
        midArea.getCentreX() - knobSize / 2,
        midArea.getCentreY() - knobSize / 2 - knobVerticalOffset,
        knobSize,
        knobSize
    );
    midGainKnob.setBounds(midKnobBounds);
    
    // Position the label above the knob
    midGainLabel.setBounds(
        midKnobBounds.getX(),
        midKnobBounds.getY() - labelHeight - 5,
        midKnobBounds.getWidth(),
        labelHeight
    );
    
    // Position custom display label under knob
    auto midDisplayBounds = juce::Rectangle<int>(
        midKnobBounds.getX(), 
        midKnobBounds.getBottom() + 5, 
        midKnobBounds.getWidth(), 
        20
    );
    midGainDisplay.setBounds(midDisplayBounds);
    
    auto sideKnobBounds = juce::Rectangle<int>(
        sideArea.getCentreX() - knobSize / 2,
        sideArea.getCentreY() - knobSize / 2 - knobVerticalOffset,
        knobSize,
        knobSize
    );
    sideGainKnob.setBounds(sideKnobBounds);
    
    // Position the label above the knob
    sideGainLabel.setBounds(
        sideKnobBounds.getX(),
        sideKnobBounds.getY() - labelHeight - 5,
        sideKnobBounds.getWidth(),
        labelHeight
    );
    
    // Position custom display label under knob
    auto sideDisplayBounds = juce::Rectangle<int>(
        sideKnobBounds.getX(), 
        sideKnobBounds.getBottom() + 5, 
        sideKnobBounds.getWidth(), 
        20
    );
    sideGainDisplay.setBounds(sideDisplayBounds);
    
    // Calculate meters height and position to align the bottom with the bottom row knobs
    auto metersHeight = static_cast<int>(height * 0.6f);
    auto metersBottom = std::max(leftKnobBounds.getBottom(), midKnobBounds.getBottom());
    auto metersYPos = metersBottom - metersHeight;
    
    // Split meters area exactly in half to ensure equal width
    auto leftMeterBounds = metersArea.removeFromLeft(static_cast<int>(metersArea.getWidth() / 2))
                           .withHeight(metersHeight).withY(metersYPos);
    auto rightMeterBounds = metersArea.withHeight(metersHeight).withY(metersYPos);
    
    leftMeter.setBounds(leftMeterBounds);
    rightMeter.setBounds(rightMeterBounds);
    
    // Position meter labels
    auto leftMeterLabelBounds = juce::Rectangle<int>(leftMeterBounds.getX(), leftMeterBounds.getY() - labelHeight - 5, leftMeterBounds.getWidth(), labelHeight);
    leftMeterLabel.setBounds(leftMeterLabelBounds);
    
    auto rightMeterLabelBounds = juce::Rectangle<int>(rightMeterBounds.getX(), rightMeterBounds.getY() - labelHeight - 5, rightMeterBounds.getWidth(), labelHeight);
    rightMeterLabel.setBounds(rightMeterLabelBounds);
    
    // Placement visualization in top section
    auto placementHeight = static_cast<int>(topSection.getHeight() * 0.8f);
    auto placementWidth = placementHeight; // Make it square
    
    auto placementBounds = juce::Rectangle<int>(
        topSection.getX() + static_cast<int>((topSection.getWidth()) * 0.05f),
        topSection.getCentreY() - placementHeight / 2,
        placementWidth,
        placementHeight
    );
    stereoPlacement.setBounds(placementBounds);
    
    // Position the stereo placement label
    auto placementLabelBounds = juce::Rectangle<int>(
        placementBounds.getX(),
        placementBounds.getY() - labelHeight - 5,
        placementBounds.getWidth(),
        labelHeight
    );
    stereoPlacementLabel.setBounds(placementLabelBounds);
    
    // Control area to the right of the stereo placement visualization
    auto controlsArea = topSection.withTrimmedLeft(placementBounds.getRight() + 20);
    
    // Divide controls area: left for phase offset, right for master gain
    auto controlsLeftArea = controlsArea.removeFromLeft(static_cast<int>(controlsArea.getWidth() * 0.45f));
    auto controlsRightArea = controlsArea;
    
    // Move phase offset and master knobs up to align with stereo placement text
    auto topOffset = placementLabelBounds.getY() - controlsLeftArea.getY() + knobSize/2;
    
    // Position phase offset slider in the left control area, moved up to align with stereo placement text
    auto phaseOffsetBounds = juce::Rectangle<int>(
        controlsLeftArea.getCentreX() - masterKnobSize / 2,
        controlsLeftArea.getY() + topOffset,
        masterKnobSize,
        masterKnobSize
    );
    phaseOffsetSlider.setBounds(phaseOffsetBounds);
    
    // Position the phase offset label above the slider
    phaseOffsetLabel.setBounds(
        phaseOffsetBounds.getX(),
        placementLabelBounds.getY(),
        phaseOffsetBounds.getWidth(),
        labelHeight
    );
    
    // Position custom display label under knob
    auto phaseOffsetDisplayBounds = juce::Rectangle<int>(
        phaseOffsetBounds.getX(), 
        phaseOffsetBounds.getBottom() + 5, 
        phaseOffsetBounds.getWidth(), 
        20
    );
    phaseOffsetDisplay.setBounds(phaseOffsetDisplayBounds);
    
    // Position master gain knob, also moved up to align with stereo placement text
    auto masterKnobBounds = juce::Rectangle<int>(
        controlsRightArea.getCentreX() - masterKnobSize / 2,
        controlsRightArea.getY() + topOffset,
        masterKnobSize,
        masterKnobSize
    );
    masterGainKnob.setBounds(masterKnobBounds);
    
    // Position the label above the knob, aligned with stereo placement
    masterGainLabel.setBounds(
        masterKnobBounds.getX(),
        placementLabelBounds.getY(),
        masterKnobBounds.getWidth(),
        labelHeight
    );
    
    // Position custom display label under knob
    auto masterDisplayBounds = juce::Rectangle<int>(
        masterKnobBounds.getX(), 
        masterKnobBounds.getBottom() + 5, 
        masterKnobBounds.getWidth(), 
        20
    );
    masterGainDisplay.setBounds(masterDisplayBounds);
    
    // Position buttons with better spacing
    const int buttonVerticalSpacing = 15;
    
    // Calculate the exact centers for button positioning
    auto leftKnobCenter = leftKnobBounds.getCentreX();
    auto rightKnobCenter = rightKnobBounds.getCentreX();
    auto midKnobCenter = midKnobBounds.getCentreX();
    auto sideKnobCenter = sideKnobBounds.getCentreX();
    
    // Position link L/R button precisely centered between left and right knobs
    auto linkButtonWidth = static_cast<int>((leftArea.getWidth() + rightArea.getWidth()) * 0.8f);
    auto linkButtonX = (leftKnobCenter + rightKnobCenter) / 2 - linkButtonWidth / 2;
    
    auto linkButtonBounds = juce::Rectangle<int>(
        linkButtonX,
        leftDisplayBounds.getBottom() + buttonVerticalSpacing,
        linkButtonWidth,
        buttonHeight
    );
    linkGainButton.setBounds(linkButtonBounds);
    
    // Position phase invert buttons centered under their respective knobs
    auto invertButtonWidth = static_cast<int>(leftArea.getWidth() * 0.8f);
    
    auto invertLeftBounds = juce::Rectangle<int>(
        leftKnobCenter - invertButtonWidth / 2,
        linkButtonBounds.getBottom() + buttonVerticalSpacing,
        invertButtonWidth,
        buttonHeight
    );
    invertLeftButton.setBounds(invertLeftBounds);
    
    auto invertRightBounds = juce::Rectangle<int>(
        rightKnobCenter - invertButtonWidth / 2,
        linkButtonBounds.getBottom() + buttonVerticalSpacing,
        invertButtonWidth,
        buttonHeight
    );
    invertRightButton.setBounds(invertRightBounds);
    
    // Position enable mid/side button precisely centered between mid and side knobs
    auto midSideButtonWidth = static_cast<int>((midArea.getWidth() + sideArea.getWidth()) * 0.8f);
    auto midSideButtonX = (midKnobCenter + sideKnobCenter) / 2 - midSideButtonWidth / 2;
    
    auto midSideButtonBounds = juce::Rectangle<int>(
        midSideButtonX,
        midDisplayBounds.getBottom() + buttonVerticalSpacing,
        midSideButtonWidth,
        buttonHeight
    );
    enableMidSideButton.setBounds(midSideButtonBounds);
}

void PluginV3AudioProcessorEditor::timerCallback()
{
    // Get the current level values from the processor
    auto leftLevel = audioProcessor.getLeftChannelLevel();
    auto rightLevel = audioProcessor.getRightChannelLevel();
    
    // Update the meters
    leftMeter.setLevel(leftLevel);
    rightMeter.setLevel(rightLevel);
    
    // Update custom display labels based on slider values
    auto updateGainDisplay = [](juce::Slider& slider, juce::Label& display) {
        auto value = slider.getValue();
        if (std::abs(value - 1.0) < 0.01)
            display.setText("0.0 dB", juce::dontSendNotification);
        else if (value <= 0.001)
            display.setText("-inf dB", juce::dontSendNotification);
        else
            display.setText(juce::String(20.0 * std::log10(value), 1) + " dB", juce::dontSendNotification);
    };
    
    updateGainDisplay(masterGainKnob, masterGainDisplay);
    updateGainDisplay(leftGainKnob, leftGainDisplay);
    updateGainDisplay(rightGainKnob, rightGainDisplay);
    updateGainDisplay(midGainKnob, midGainDisplay);
    updateGainDisplay(sideGainKnob, sideGainDisplay);
    
    // Update phase offset display
    phaseOffsetDisplay.setText(juce::String(phaseOffsetSlider.getValue(), 1) + "°", juce::dontSendNotification);
    
    // Update stereo placement visualization
    stereoPlacement.setLevels(leftLevel, rightLevel);
    stereoPlacement.repaint();
}
