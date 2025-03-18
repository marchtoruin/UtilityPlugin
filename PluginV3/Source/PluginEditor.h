#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "LevelMeter.h"
#include <cmath> // For sine function
#include <chrono> // For time-based calculations

//==============================================================================
// Custom look and feel for toggle buttons to make them look nicer
class CustomToggleLookAndFeel : public juce::LookAndFeel_V4
{
public:
    void drawToggleButton(juce::Graphics& g, juce::ToggleButton& button,
                         bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override
    {
        auto bounds = button.getLocalBounds().reduced(2);
        auto cornerSize = 3.0f;
        
        // Get button color
        auto buttonColor = button.findColour(juce::ToggleButton::tickColourId);
        
        // Draw background
        if (button.getToggleState()) {
            // When on, use a darker fill with neon border
            g.setColour(buttonColor.withAlpha(0.3f));
            g.fillRect(bounds.toFloat());
            
            // Neon border when on
            g.setColour(buttonColor);
            g.drawRect(bounds.toFloat(), 1.5f);
        } else {
            // When off, draw a dark background with subtle outline
            g.setColour(juce::Colour(0xFF0F0F1A));
            g.fillRect(bounds.toFloat());
            
            // Subtle border when off
            g.setColour(buttonColor.withAlpha(0.4f));
            g.drawRect(bounds.toFloat(), 1.0f);
        }
        
        // Draw highlight when hovered/pressed
        if (shouldDrawButtonAsHighlighted || shouldDrawButtonAsDown) {
            g.setColour(buttonColor.withAlpha(shouldDrawButtonAsDown ? 0.4f : 0.2f));
            g.fillRect(bounds.toFloat().reduced(1.0f));
        }
        
        // Draw text with 80s style
        if (button.getToggleState()) {
            // When toggled on, text gets a neon glow
            g.setColour(buttonColor.withAlpha(0.4f));
            g.setFont(juce::Font(juce::Font::FontStyleFlags::bold).withHeight(14.0f));
            g.drawText(button.getButtonText(), bounds.translated(0, 1), juce::Justification::centred, false);
            
            g.setColour(buttonColor.brighter(0.5f));
        } else {
            g.setColour(juce::Colours::white.withAlpha(0.7f));
        }
        
        g.setFont(juce::Font(juce::Font::FontStyleFlags::plain).withHeight(14.0f));
        g.drawText(button.getButtonText(), bounds, juce::Justification::centred, false);
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
        
        // Time-based pulsing effect using high-resolution timer
        float milliseconds = static_cast<float>(juce::Time::getMillisecondCounterHiRes());
        float pulsingFactor = 0.6f + 0.4f * (1.0f + std::sin(milliseconds * 0.001f)); // Adjusted for consistent brightness

        // Conditional coloring for mid-side gain
        juce::Colour outlineColour;
        if (slider.getName().containsIgnoreCase("mid") || slider.getName().containsIgnoreCase("side")) {
            outlineColour = juce::Colours::magenta.withAlpha(pulsingFactor);
        } else {
            outlineColour = juce::Colours::cyan.withAlpha(pulsingFactor);
        }

        juce::Colour pointerColour = juce::Colours::white.withAlpha(pulsingFactor);

        // Custom drawing code for rotary slider with fill and line indicator
        auto radius = juce::jmin(width / 2, height / 2) - 4.0f;
        auto centerX = x + width * 0.5f;
        auto centerY = y + height * 0.5f;
        auto rx = centerX - radius;
        auto ry = centerY - radius;
        auto rw = radius * 2.0f;
        auto angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
        
        // 80s-style background - darker version of the plugin background
        g.setColour(juce::Colour(0xFF0F0F1A));
        g.fillEllipse(rx, ry, rw, rw);
        
        // Draw outline with pulsing effect
        g.setColour(outlineColour);
        g.drawEllipse(rx, ry, rw, rw, 1.0f);
        
        // Draw tick marks with 80s neon look
        g.setColour(outlineColour.withAlpha(0.4f));
        for (int i = 0; i < 8; i++) {
            float tickAngle = rotaryStartAngle + (i / 8.0f) * (rotaryEndAngle - rotaryStartAngle);
            float innerRadius = radius * 0.7f;
            float outerRadius = radius * 0.9f;
            
            juce::Point<float> start(
                centerX + innerRadius * std::cos(tickAngle),
                centerY + innerRadius * std::sin(tickAngle)
            );
            
            juce::Point<float> end(
                centerX + outerRadius * std::cos(tickAngle),
                centerY + outerRadius * std::sin(tickAngle)
            );
            
            g.drawLine(start.x, start.y, end.x, end.y, 1.0f);
        }
        
        // Extra mark at 0dB position (noon) - more prominent
        if (!slider.getName().contains("Phase")) {
            g.setColour(outlineColour);
            float innerRadius = radius * 0.7f;
            float outerRadius = radius * 0.95f;
            
            juce::Point<float> start(
                centerX + innerRadius * std::cos(0.0f),
                centerY + innerRadius * std::sin(0.0f)
            );
            
            juce::Point<float> end(
                centerX + outerRadius * std::cos(0.0f),
                centerY + outerRadius * std::sin(0.0f)
            );
            
            g.drawLine(start.x, start.y, end.x, end.y, 1.5f);
        }
        
        // Draw the pointer with pulsing effect
        juce::Path pointer;
        float pointerLength = radius * 0.7f;
        float pointerThickness = 2.0f;
        pointer.addRectangle(-pointerThickness * 0.5f, -pointerLength, pointerThickness, pointerLength);
        g.setColour(pointerColour);
        g.fillPath(pointer, juce::AffineTransform::rotation(angle).translated(centerX, centerY));
        
        // Draw center dot with neon appearance
        g.setColour(outlineColour.darker(0.2f));
        g.fillEllipse(centerX - 3.0f, centerY - 3.0f, 6.0f, 6.0f);
        g.setColour(outlineColour.brighter(0.5f));
        g.fillEllipse(centerX - 1.5f, centerY - 1.5f, 3.0f, 3.0f);
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
        
        // Draw dark background
        g.setColour(juce::Colour(0xFF0F0F1A));
        g.fillEllipse(bounds);
        
        // Draw 80s grid circles
        g.setColour(juce::Colour(0xFF2A2A40));
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
        
        // Draw center crosshair with neon effect
        float crossSize = width * 0.5f;
        
        // Horizontal line
        g.setColour(juce::Colour(0xFF00DCDC).withAlpha(0.6f));
        g.drawLine(centerX - crossSize, centerY, centerX + crossSize, centerY, 1.0f);
        
        // Vertical line
        g.drawLine(centerX, centerY - crossSize, centerX, centerY + crossSize, 1.0f);
        
        // Draw left and right channel markings with neon effect
        g.setFont(juce::Font(juce::Font::FontStyleFlags::bold).withHeight(14.0f));
        
        // Draw L and R labels
        g.setColour(juce::Colour(0xFF00DCDC)); // Cyan
        g.drawText("L", static_cast<int>(bounds.getX()), static_cast<int>(centerY - 8), 20, 16, juce::Justification::centred);
        g.drawText("R", static_cast<int>(bounds.getRight() - 20), static_cast<int>(centerY - 8), 20, 16, juce::Justification::centred);
        
        // Draw the stereo position indicator
        float normalizedPosition = stereoPosition;
        float normalizedIntensity = std::min(std::max(leftLevel + rightLevel, 0.1f), 1.0f) * intensityMultiplier;
        
        float indicatorX = bounds.getX() + width * normalizedPosition;
        float indicatorY = centerY;
        
        // Draw a line from center to the position with neon effect
        g.setColour(juce::Colour(0xFFFF3B96)); // Magenta
        g.drawLine(centerX, centerY, indicatorX, indicatorY, 2.0f);
        
        // Draw neon glow around position dot
        float glowSize = 20.0f * normalizedIntensity;
        g.setColour(juce::Colour(0xFFFF3B96).withAlpha(0.3f));
        g.fillEllipse(indicatorX - (glowSize/2), indicatorY - (glowSize/2), glowSize, glowSize);
        
        // Draw the indicator dot with neon magenta
        float dotSize = 8.0f * normalizedIntensity;
        g.setColour(juce::Colour(0xFFFF3B96));
        g.fillEllipse(indicatorX - (dotSize/2), indicatorY - (dotSize/2), dotSize, dotSize);
        
        // Add highlight to the dot for a neon look
        g.setColour(juce::Colours::white);
        g.fillEllipse(
            indicatorX - (dotSize * 0.3f),
            indicatorY - (dotSize * 0.3f),
            dotSize * 0.6f, dotSize * 0.6f
        );
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
                                     public juce::Timer
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
    
    // Grid animation
    juce::LinearSmoothedValue<float> gridIntensity;     // For music response
    juce::LinearSmoothedValue<float> gridMasterScale;   // For master gain scaling
    float baseGridIntensity = 0.2f;                     // Minimum grid intensity
    float maxGridIntensity = 0.8f;                      // Maximum grid intensity when clipping
    
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
    juce::Label midSideGainLabel;
    
    // Stereo placement visualization
    StereoPlacementComponent stereoPlacement;
    juce::Label stereoPlacementLabel;
    
    // Left/Right section label
    juce::Label leftRightGainLabel;
    
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
    
    // Color scheme - 80s inspired
    juce::Colour backgroundColour { juce::Colour(0xFF181825) }; // Dark blue-black
    juce::Colour accentColour { juce::Colour(0xFF00DCDC) };     // Cyan
    juce::Colour secondaryAccentColour { juce::Colour(0xFFFF3B96) }; // Magenta/Pink
    juce::Colour gridColour { juce::Colour(0xFF2A2A40) };       // Subtle grid color
    juce::Colour glowingGridColour { juce::Colour(0xFF00DCDC).brighter(0.2f) }; // Brighter cyan for grid glow
    juce::Colour textColour { juce::Colours::white };
    
    // Custom Look and Feel objects
    ZeroDBAtNoonLookAndFeel zeroDBAtNoonLookAndFeel;
    CustomToggleLookAndFeel customToggleLookAndFeel;
    
    // Utility functions
    float calculateMasterGainScale() const;
    float calculateGridIntensity(float leftLevel, float rightLevel) const;
    
    // Bypass button
    juce::ToggleButton bypassButton { "Bypass" };
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginV3AudioProcessorEditor)
};
