#include "LevelMeter.h"

//==============================================================================
LevelMeter::LevelMeter()
{
    lastUpdateTime = static_cast<float>(juce::Time::getMillisecondCounterHiRes());
    
    // Set default colors
    setColour(backgroundColourId, juce::Colours::black);
    setColour(foregroundColourId, juce::Colours::green);
    setColour(outlineColourId, juce::Colours::white.withAlpha(0.5f));
}

LevelMeter::~LevelMeter()
{
}

//==============================================================================
void LevelMeter::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat().reduced(1.0f);
    
    // Update peak and apply decay
    updatePeakAndDecay();
    
    // Draw background
    g.setColour(findColour(backgroundColourId));
    g.fillRoundedRectangle(bounds, 2.0f);
    
    // Draw level meter
    if (level > 0.0f)
    {
        g.setColour(getColourForLevel(level));
        
        if (isVertical)
        {
            auto levelHeight = bounds.getHeight() * level;
            auto meterBounds = bounds.removeFromBottom(levelHeight);
            g.fillRoundedRectangle(meterBounds, 2.0f);
        }
        else
        {
            auto levelWidth = bounds.getWidth() * level;
            auto meterBounds = bounds.removeFromLeft(levelWidth);
            g.fillRoundedRectangle(meterBounds, 2.0f);
        }
    }
    
    // Draw peak marker
    if (showingPeakMarker && peakLevel > 0.0f)
    {
        g.setColour(getColourForLevel(peakLevel).brighter(0.5f));
        
        if (isVertical)
        {
            auto peakY = bounds.getBottom() - (bounds.getHeight() * peakLevel);
            g.fillRect(bounds.getX(), peakY - 1.0f, bounds.getWidth(), 2.0f);
        }
        else
        {
            auto peakX = bounds.getX() + (bounds.getWidth() * peakLevel);
            g.fillRect(peakX - 1.0f, bounds.getY(), 2.0f, bounds.getHeight());
        }
    }
    
    // Draw outline
    g.setColour(findColour(outlineColourId));
    g.drawRoundedRectangle(bounds, 2.0f, 1.0f);
    
    // Draw dB markings with fixed font size and positioning
    g.setColour(findColour(outlineColourId));
    
    // Create font with simple approach that avoids deprecation warnings
    auto font = 10.0f; // Default font size
    g.setFont(font);
    
    // Fixed width for labels to prevent scrunching
    const int labelWidth = 30;
    const int labelHeight = 12;
    
    auto drawDbMarking = [&](float db, const juce::String& text)
    {
        auto normLevel = juce::Decibels::decibelsToGain(db);
        
        if (isVertical)
        {
            auto y = bounds.getBottom() - (bounds.getHeight() * normLevel);
            g.drawLine(bounds.getX(), y, bounds.getX() + 3.0f, y, 1.0f);
            
            if (!text.isEmpty())
            {
                auto yPos = static_cast<int>(y) - labelHeight / 2;
                // Draw text outside the meter but aligned with the tick mark
                g.drawText(text, 
                           static_cast<int>(bounds.getRight()) + 2, 
                           yPos, 
                           labelWidth, 
                           labelHeight, 
                           juce::Justification::left, 
                           false);
            }
        }
        else
        {
            auto x = bounds.getX() + (bounds.getWidth() * normLevel);
            g.drawLine(x, bounds.getBottom(), x, bounds.getBottom() - 3.0f, 1.0f);
            
            if (!text.isEmpty())
            {
                auto xPos = static_cast<int>(x) - labelWidth / 2;
                // Position text below the meter with consistent width
                g.drawText(text, 
                           xPos, 
                           static_cast<int>(bounds.getBottom()) + 2, 
                           labelWidth, 
                           labelHeight,
                           juce::Justification::centred, 
                           false);
            }
        }
    };
    
    // Draw common dB markings
    drawDbMarking(0.0f, "0");
    drawDbMarking(-6.0f, "-6");
    drawDbMarking(-12.0f, "-12");
    drawDbMarking(-24.0f, "-24");
    drawDbMarking(-36.0f, "-36");
    drawDbMarking(-48.0f, "-48");
}

void LevelMeter::resized()
{
    repaint();
}

//==============================================================================
void LevelMeter::setLevel(float newLevel)
{
    // Ensure level is between 0 and 1
    newLevel = juce::jlimit(0.0f, 1.0f, newLevel);
    
    // Always update level to ensure we get fresh values
    level = newLevel;
    
    // Update peak level if new level is higher
    if (level > peakLevel)
        peakLevel = level;
    
    hasBeenUpdatedSinceLastDecay = true;
    
    // Always repaint to ensure smooth updates
    repaint();
}

float LevelMeter::getLevel() const
{
    return level;
}

void LevelMeter::setVertical(bool vertical)
{
    if (isVertical != vertical)
    {
        isVertical = vertical;
        repaint();
    }
}

void LevelMeter::showPeakMarker(bool shouldShowPeakMarker)
{
    if (showingPeakMarker != shouldShowPeakMarker)
    {
        showingPeakMarker = shouldShowPeakMarker;
        repaint();
    }
}

void LevelMeter::setDecayRates(float newLevelDecayRate, float newPeakDecayRate)
{
    levelDecayRate = newLevelDecayRate;
    peakDecayRate = newPeakDecayRate;
}

void LevelMeter::setMeterColour(juce::Colour newLowColour, juce::Colour newMidColour, juce::Colour newHighColour)
{
    lowColour = newLowColour;
    midColour = newMidColour;
    highColour = newHighColour;
    repaint();
}

//==============================================================================
juce::Colour LevelMeter::getColourForLevel(float lvl)
{
    // Determine color based on level:
    // 0.0 to 0.6: gradient from lowColour to midColour
    // 0.6 to 1.0: gradient from midColour to highColour
    
    if (lvl < 0.6f)
    {
        auto ratio = lvl / 0.6f;
        return lowColour.interpolatedWith(midColour, ratio);
    }
    else
    {
        auto ratio = (lvl - 0.6f) / 0.4f;
        return midColour.interpolatedWith(highColour, ratio);
    }
}

void LevelMeter::updatePeakAndDecay()
{
    auto currentTime = static_cast<float>(juce::Time::getMillisecondCounterHiRes());
    float elapsedSec = (currentTime - lastUpdateTime) / 1000.0f;
    lastUpdateTime = currentTime;
    
    // Limit elapsed time to avoid huge jumps if app was suspended
    elapsedSec = juce::jmin(elapsedSec, 0.05f);
    
    if (elapsedSec > 0.0f)
    {
        // Apply level decay (only if no new level has been set)
        if (!hasBeenUpdatedSinceLastDecay)
        {
            float decayAmount = levelDecayRate * elapsedSec;
            float dbLevel = juce::Decibels::gainToDecibels(level);
            dbLevel -= decayAmount;
            level = juce::Decibels::decibelsToGain(dbLevel);
        }
        
        // Always apply peak decay if peak is higher than level
        if (peakLevel > level)
        {
            float peakDecayAmount = peakDecayRate * elapsedSec;
            float dbPeakLevel = juce::Decibels::gainToDecibels(peakLevel);
            dbPeakLevel -= peakDecayAmount;
            peakLevel = juce::Decibels::decibelsToGain(dbPeakLevel);
        }
        else
        {
            peakLevel = level;
        }
    }
    
    hasBeenUpdatedSinceLastDecay = false;
} 