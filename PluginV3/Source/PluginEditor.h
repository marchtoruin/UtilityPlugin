#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "LevelMeter.h"

//==============================================================================
// Stereo Placement Visualization Component
class StereoPlacementComponent : public juce::Component
{
public:
    StereoPlacementComponent()
    {
        setOpaque(false);
    }
    
    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat().reduced(4.0f);
        auto centerX = bounds.getCentreX();
        auto centerY = bounds.getCentreY();
        
        // Draw the circular background
        g.setColour(juce::Colours::darkgrey.darker(0.2f));
        g.fillEllipse(bounds);
        
        // Draw grid lines
        g.setColour(juce::Colours::grey.withAlpha(0.4f));
        
        // Draw the horizontal and vertical center lines
        g.drawLine(bounds.getX(), centerY, bounds.getRight(), centerY, 1.0f);
        g.drawLine(centerX, bounds.getY(), centerX, bounds.getBottom(), 1.0f);
        
        // Draw concentric circles
        float maxRadius = juce::jmin(bounds.getWidth(), bounds.getHeight()) / 2.0f;
        for (float radiusPercentage : {0.33f, 0.67f, 1.0f})
        {
            float radius = maxRadius * radiusPercentage;
            g.drawEllipse(centerX - radius, centerY - radius, radius * 2.0f, radius * 2.0f, 1.0f);
        }
        
        // Draw labels
        g.setColour(juce::Colours::white);
        g.setFont(12.0f);
        g.drawText("L", bounds.getX(), centerY - 10.0f, 20.0f, 20.0f, juce::Justification::centred);
        g.drawText("R", bounds.getRight() - 20.0f, centerY - 10.0f, 20.0f, 20.0f, juce::Justification::centred);
        
        // Draw the stereo placement dot based on current values
        if (leftLevel > 0.0f || rightLevel > 0.0f)
        {
            // Use logarithmic scaling for levels to make visualization more useful
            float leftLevelDb = juce::Decibels::gainToDecibels(leftLevel, -60.0f);
            float rightLevelDb = juce::Decibels::gainToDecibels(rightLevel, -60.0f);
            
            // Normalize to 0.0 - 1.0 range
            float leftNormalized = juce::jmap(leftLevelDb, -60.0f, 0.0f, 0.0f, 1.0f);
            float rightNormalized = juce::jmap(rightLevelDb, -60.0f, 0.0f, 0.0f, 1.0f);
            
            // Apply phase inversion if needed
            if (invertLeft)
                leftNormalized *= -1.0f;
            if (invertRight)
                rightNormalized *= -1.0f;
            
            // Calculate dot position
            float dotX = centerX + (rightNormalized - leftNormalized) * maxRadius * 0.5f;
            float dotY = centerY - ((leftNormalized + rightNormalized) * 0.5f) * maxRadius * 0.5f;
            
            // Draw the stereo position dot
            float dotSize = 10.0f;
            g.setColour(juce::Colours::orange);
            g.fillEllipse(dotX - dotSize * 0.5f, dotY - dotSize * 0.5f, dotSize, dotSize);
            
            // Draw a trail line from center to the dot
            g.setColour(juce::Colours::orange.withAlpha(0.4f));
            g.drawLine(centerX, centerY, dotX, dotY, 1.0f);
        }
    }
    
    void setLevels(float leftLevelIn, float rightLevelIn)
    {
        leftLevel = leftLevelIn;
        rightLevel = rightLevelIn;
        repaint();
    }
    
    void setPhaseInversion(bool invertLeftIn, bool invertRightIn)
    {
        invertLeft = invertLeftIn;
        invertRight = invertRightIn;
        repaint();
    }
    
private:
    float leftLevel = 0.0f;
    float rightLevel = 0.0f;
    bool invertLeft = false;
    bool invertRight = false;
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
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginV3AudioProcessorEditor)
};
