#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "LevelMeter.h"

//==============================================================================
// Custom look and feel for toggle buttons to make them look nicer
class CustomToggleLookAndFeel : public juce::LookAndFeel_V4
{
public:
    void drawToggleButton(juce::Graphics& g, juce::ToggleButton& button,
                         bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override
    {
        auto bounds = button.getLocalBounds().reduced(2);
        auto cornerSize = 6.0f;
        
        // Draw the background with rounded corners - make it more prominent when on
        g.setColour(button.findColour(juce::ToggleButton::tickDisabledColourId)
                   .withAlpha(button.getToggleState() ? 0.2f : 0.1f));
        g.fillRoundedRectangle(bounds.toFloat(), cornerSize);
        
        // Draw the border
        g.setColour(button.findColour(juce::ToggleButton::tickColourId)
                   .withAlpha(button.getToggleState() ? 0.8f : 0.3f));
        g.drawRoundedRectangle(bounds.toFloat().reduced(0.5f), cornerSize, 1.0f);
        
        // Draw text
        g.setColour(button.findColour(juce::ToggleButton::textColourId)
                   .withMultipliedAlpha(button.isEnabled() ? 1.0f : 0.6f));
        g.setFont(juce::Font(juce::Font::getDefaultSansSerifFontName(), 14.0f, juce::Font::plain));
        
        if (button.getToggleState())
        {
            // When toggled, use the tick color instead of changing the font
            g.setColour(button.findColour(juce::ToggleButton::tickColourId));
        }
        
        // Draw the text, accounting for the toggle box
        auto textBounds = bounds;
        g.drawText(button.getButtonText(), textBounds, juce::Justification::centred, true);
    }
};

//==============================================================================
// Custom look and feel that maps 0dB to noon position for gain knobs
class ZeroDBAtNoonLookAndFeel : public juce::LookAndFeel_V4 
{
public:
    // Override to adjust the rotation position for 0dB at noon and draw a line indicator instead of a ball
    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height, 
                         float sliderPosProportional, const float rotaryStartAngle, 
                         const float rotaryEndAngle, juce::Slider& slider) override
    {
        // Calculate custom position where 0dB (value 1.0) appears at noon
        float sliderPos = sliderPosProportional;
        bool atUnityGain = false;
        bool belowUnity = false;
        
        // If it's a gain knob (our specific use case), modify the position
        auto value = slider.getValue();
        
        // Standardize the mapping for all gain parameters (checking text suffix OR parameter ID)
        if (value > 0.0 && (slider.getTextValueSuffix().contains("dB") || 
                           slider.getName().containsIgnoreCase("gain")))
        {
            // Check if we're at unity gain (0.0 dB)
            atUnityGain = std::abs(value - 1.0) < 0.01;
            belowUnity = value < 1.0;
            
            // Convert to dB
            double dBValue = 20.0 * std::log10(value);
            
            // Map -infinity to -60dB for visualization
            double minDB = -60.0;
            double maxDB = 20.0 * std::log10(slider.getMaximum());
            
            // Normalize to 0-1 with 0dB at 0.5
            if (dBValue <= 0.0) {
                // Values below 0dB map to 0.0-0.5 range
                sliderPos = static_cast<float>(0.5 * (dBValue - minDB) / (0.0 - minDB));
            } else {
                // Values above 0dB map to 0.5-1.0 range
                sliderPos = static_cast<float>(0.5 + 0.5 * dBValue / maxDB);
            }
        }
        
        // Custom drawing code for rotary slider with fill and line indicator
        auto radius = juce::jmin(width / 2, height / 2) - 4.0f;
        auto centerX = x + width * 0.5f;
        auto centerY = y + height * 0.5f;
        auto rx = centerX - radius;
        auto ry = centerY - radius;
        auto rw = radius * 2.0f;
        auto angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
        
        // Background - use dark grey that matches the plugin background instead of outline color
        // For Phase Offset, use black background
        if (slider.getName().contains("Phase"))
            g.setColour(juce::Colours::black.withAlpha(0.8f));
        else
            g.setColour(juce::Colours::darkgrey.darker(0.8f));
            
        g.fillEllipse(rx, ry, rw, rw);
        
        // Add thin black outline to all knobs
        g.setColour(juce::Colours::black);
        g.drawEllipse(rx, ry, rw, rw, 1.0f);
        
        // Calculate the noon position (Unity Gain / 0dB position)
        const float noonAngle = 0.0f; // 12 o'clock is at 0 radians
        
        // Fill arc - only if not at unity gain and not the phase knob
        if (slider.isEnabled() && (!atUnityGain || slider.getName().contains("Phase"))) {
            // Create a path for the value arc
            juce::Path valueArc;
            
            if (slider.getName().contains("Phase")) {
                // For phase, keep the normal left-to-right fill
                valueArc.addPieSegment(rx, ry, rw, rw, rotaryStartAngle, angle, 0.0f);
            } else {
                // For gain knobs: Arc starts at Unity (noon) and extends in the direction of rotation
                if (belowUnity) {
                    // If below unity, arc from current position to noon (counterclockwise)
                    valueArc.addPieSegment(rx, ry, rw, rw, angle, noonAngle, 0.0f);
                } else {
                    // If above unity, arc from noon to current position (clockwise)
                    valueArc.addPieSegment(rx, ry, rw, rw, noonAngle, angle, 0.0f);
                }
            }
            
            g.setColour(slider.findColour(juce::Slider::rotarySliderFillColourId));
            g.fillPath(valueArc);
        }
        
        // Draw line indicator
        juce::Path p;
        auto lineLength = radius * 0.8f;     // Length of the indicator line
        auto lineThickness = 2.5f;           // Thickness of the indicator line
        
        // Draw indicator line
        p.addRectangle(-lineThickness * 0.5f, -radius, lineThickness, lineLength);
        
        // Rotate indicator line to the correct angle
        p.applyTransform(juce::AffineTransform::rotation(angle).translated(centerX, centerY));
        
        // Use the fill color for the indicator instead of thumb color
        if (atUnityGain && !slider.getName().contains("Phase"))
            g.setColour(juce::Colours::white.withAlpha(0.7f)); // White indicator at unity gain
        else
            g.setColour(slider.findColour(juce::Slider::rotarySliderFillColourId));
        
        g.fillPath(p);
        
        // Draw center dot
        g.setColour(juce::Colours::white.withAlpha(0.5f));
        g.fillEllipse(centerX - 2.0f, centerY - 2.0f, 4.0f, 4.0f);
    }
};

//==============================================================================
// Stereo Placement Visualization Component
class StereoPlacementComponent : public juce::Component
{
public:
    StereoPlacementComponent()
    {
        setOpaque(false);
    }
    
    void setLevels(float newLeftLevel, float newRightLevel)
    {
        this->leftLevel = newLeftLevel;
        this->rightLevel = newRightLevel;
        
        // Calculate the stereo position based on the levels
        float sum = leftLevel + rightLevel;
        if (sum > 0.0f)
        {
            // Normalize to get the position between left (0.0) and right (1.0) channels
            stereoPosition = rightLevel / sum;
            
            // Adjust for phase inversion
            if (invertLeftPhase && !invertRightPhase)
                stereoPosition = 1.0f - (stereoPosition * 0.5f); // Bias more towards the right
            else if (!invertLeftPhase && invertRightPhase)
                stereoPosition = stereoPosition * 0.5f; // Bias more towards the left
            
            // Account for phase cancellation when both channels are inverted
            if (invertLeftPhase && invertRightPhase)
                intensityMultiplier = 0.8f; // Slight reduction in intensity to indicate both channels inverted
            else
                intensityMultiplier = 1.0f;
        }
        else
        {
            stereoPosition = 0.5f; // Center if no signal
            intensityMultiplier = 0.2f; // Very low intensity for no signal
        }
        
        repaint();
    }
    
    void setPhaseInversion(bool leftInverted, bool rightInverted)
    {
        invertLeftPhase = leftInverted;
        invertRightPhase = rightInverted;
        repaint();
    }
    
    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat().reduced(10.0f);
        auto width = bounds.getWidth();
        auto height = bounds.getHeight();
        auto centerX = bounds.getCentreX();
        auto centerY = bounds.getCentreY();
        
        // Draw background ellipse
        g.setColour(juce::Colours::darkgrey.withAlpha(0.3f));
        g.fillEllipse(bounds);
        
        // Draw concentric rings
        g.setColour(juce::Colours::lightgrey.withAlpha(0.2f));
        for (float radius = 0.2f; radius <= 1.0f; radius += 0.2f)
        {
            g.drawEllipse(
                centerX - (width * 0.5f * radius),
                centerY - (height * 0.5f * radius),
                width * radius,
                height * radius,
                1.0f
            );
        }
        
        // Draw left and right channel markings
        g.setFont(12.0f);
        g.drawText("L", static_cast<int>(bounds.getX()), static_cast<int>(centerY - 6.0f), 20, 12, juce::Justification::centred);
        g.drawText("R", static_cast<int>(bounds.getRight() - 20.0f), static_cast<int>(centerY - 6.0f), 20, 12, juce::Justification::centred);
        
        // Draw the center line
        g.setColour(juce::Colours::lightgrey.withAlpha(0.5f));
        g.drawLine(centerX, bounds.getY(), centerX, bounds.getBottom(), 1.0f);
        
        // Draw the stereo position indicator
        float normalizedPosition = stereoPosition;
        float normalizedIntensity = std::min(std::max(leftLevel + rightLevel, 0.1f), 1.0f) * intensityMultiplier;
        
        float indicatorX = bounds.getX() + width * normalizedPosition;
        float indicatorY = centerY;
        
        // Draw a line from center to the position
        g.setColour(juce::Colours::orange.withAlpha(0.7f));
        g.drawLine(centerX, centerY, indicatorX, indicatorY, 2.0f);
        
        // Draw the indicator dot
        float dotSize = 10.0f * normalizedIntensity;
        g.setColour(juce::Colours::orange);
        g.fillEllipse(indicatorX - (dotSize / 2.0f), indicatorY - (dotSize / 2.0f), dotSize, dotSize);
    }
    
private:
    float leftLevel = 0.0f;
    float rightLevel = 0.0f;
    float stereoPosition = 0.5f; // 0.0 = full left, 1.0 = full right
    float intensityMultiplier = 1.0f; // Reduced for phase cancellation or weak signals
    bool invertLeftPhase = false;
    bool invertRightPhase = false;
};

//==============================================================================
/**
*/
class PluginV3AudioProcessorEditor  : public juce::AudioProcessorEditor,
                                     private juce::Timer
{
public:
    PluginV3AudioProcessorEditor (PluginV3AudioProcessor&);
    ~PluginV3AudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    
    void timerCallback() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    PluginV3AudioProcessor& audioProcessor;
    
    // Level meters
    LevelMeter leftMeter;
    LevelMeter rightMeter;
    juce::Label leftMeterLabel;
    juce::Label rightMeterLabel;
    
    // Gain controls
    juce::Slider masterGainKnob;
    juce::Slider leftGainKnob;
    juce::Slider rightGainKnob;
    
    juce::Label masterGainLabel;
    juce::Label leftGainLabel;
    juce::Label rightGainLabel;
    
    // Link button for left/right gain knobs
    juce::ToggleButton linkGainButton;
    bool gainKnobsLinked = false;
    
    // Phase controls
    juce::ToggleButton invertLeftButton;
    juce::ToggleButton invertRightButton;
    juce::Slider phaseOffsetSlider;
    juce::Label phaseOffsetLabel;
    
    // Mid/Side controls
    juce::Slider midGainKnob;
    juce::Slider sideGainKnob;
    juce::ToggleButton enableMidSideButton;
    juce::Label midGainLabel;
    juce::Label sideGainLabel;
    
    // Stereo placement visualization
    StereoPlacementComponent stereoPlacement;
    juce::Label stereoPlacementLabel;
    
    // Custom gain display labels that we control
    juce::Label masterGainDisplay;
    juce::Label leftGainDisplay;
    juce::Label rightGainDisplay;
    juce::Label midGainDisplay;
    juce::Label sideGainDisplay;
    juce::Label phaseOffsetDisplay;
    
    // Parameter attachments
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> masterGainAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> leftGainAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> rightGainAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> invertLeftAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> invertRightAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> phaseOffsetAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> midGainAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> sideGainAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> enableMidSideAttachment;
    
    // UI customization
    juce::Colour backgroundColour { juce::Colours::darkgrey.darker(0.8f) };
    
    // Custom look and feel for gain knobs that will position 0dB at noon
    ZeroDBAtNoonLookAndFeel zeroDBAtNoonLookAndFeel;
    
    // Custom look and feel for toggle buttons
    CustomToggleLookAndFeel customToggleLookAndFeel;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginV3AudioProcessorEditor)
};
