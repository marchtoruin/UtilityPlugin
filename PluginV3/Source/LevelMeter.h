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
    /** Sets whether the meter is vertical or horizontal */
    void setVertical(bool shouldBeVertical);
    
    /** Gets whether the meter is vertical */
    bool isVerticalMeter() const { return isVertical; }
    
    /** Sets whether to show the peak marker */
    void showPeakMarker(bool shouldShowPeakMarker);
    
    /** Gets whether the peak marker is shown */
    bool isPeakMarkerShown() const { return showPeak; }
    
    /** Sets the current level for the meter */
    void setLevel(float newLevel);
    
    /** Gets the current level */
    float getLevel() const { return level; }
    
    /** Completely resets the meter and peak to zero */
    void reset();
    
    /** Sets the decay rates for the level and peak in dB/second */
    void setDecayRates(float meterDecayRate, float peakDecayRate);
    
    /** Sets the meter's color based on the level value */
    void setMeterColour(juce::Colour low, juce::Colour mid, juce::Colour high);
    
private:
    bool isVertical = false;
    float level = 0.0f;
    float peak = 0.0f;
    bool showPeak = false;
    float lastUpdateTime = 0.0f;
    bool hasBeenUpdatedSinceLastDecay = false;
    bool wasClipping = false;
    
    // Decay rates in dB/s
    float meterDecayRate = 36.0f;    // 36dB/second decay for main level
    float peakDecayRate = 12.0f;     // Slower decay for peak marker
    
    juce::Colour meterColourLow;     // Color for low levels (green)
    juce::Colour meterColourMid;     // Color for mid levels (yellow)
    juce::Colour meterColourHigh;    // Color for high levels (red)
    
    float lowThreshold = 0.25f;      // Below this uses the low color
    float highThreshold = 0.9f;      // Above this uses the high color
    
    juce::Colour getColourForLevel(float level);
    void updatePeakAndDecay();
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LevelMeter)
}; 