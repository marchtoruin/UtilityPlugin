#pragma once

#include <JuceHeader.h>

//==============================================================================
/**
 * A customizable level meter component for displaying audio levels
 */
class LevelMeter : public juce::Component
{
public:
    //==============================================================================
    enum ColourIds
    {
        backgroundColourId = 0x2001200,
        foregroundColourId = 0x2001201,
        outlineColourId = 0x2001202
    };
    
    //==============================================================================
    LevelMeter();
    ~LevelMeter() override;

    //==============================================================================
    void paint(juce::Graphics& g) override;
    void resized() override;
    
    //==============================================================================
    /** Sets the level to display (0.0 to 1.0) */
    void setLevel(float level);
    
    /** Gets the current level being displayed */
    float getLevel() const;
    
    /** Sets whether the meter is vertical (true) or horizontal (false) */
    void setVertical(bool vertical);
    
    /** Sets whether the meter should show a peak marker */
    void showPeakMarker(bool shouldShowPeakMarker);
    
    /** Sets the decay rates for the level and peak in dB/second */
    void setDecayRates(float levelDecayRate, float peakDecayRate);
    
    /** Sets the meter's color based on the level value */
    void setMeterColour(juce::Colour lowColour, juce::Colour midColour, juce::Colour highColour);
    
private:
    float level = 0.0f;
    bool isVertical = true;
    bool showingPeakMarker = true;
    bool hasBeenUpdatedSinceLastDecay = false;
    
    float peakLevel = 0.0f;
    float levelDecayRate = 12.0f;  // dB per second
    float peakDecayRate = 3.0f;    // dB per second
    
    float lastUpdateTime = 0.0f;   // Changed from juce::Time to float to match implementation
    
    juce::Colour lowColour = juce::Colours::green;
    juce::Colour midColour = juce::Colours::yellow;
    juce::Colour highColour = juce::Colours::red;
    
    juce::Colour getColourForLevel(float level);
    void updatePeakAndDecay();
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LevelMeter)
}; 