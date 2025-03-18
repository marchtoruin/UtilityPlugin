#include "LevelMeter.h"

//==============================================================================
LevelMeter::LevelMeter()
{
    lastUpdateTime = static_cast<float>(juce::Time::getMillisecondCounterHiRes());
    
    // Set default colors - update to cyan and magenta theme
    meterColourLow = juce::Colour(0xFF00DCDC);   // Cyan
    meterColourMid = juce::Colour(0xFF9EFFFF);   // Light cyan
    meterColourHigh = juce::Colour(0xFFFF3B96);  // Magenta
    
    // Set default colors for the component
    setColour(backgroundColourId, juce::Colours::black);
    setColour(foregroundColourId, juce::Colours::green);
    setColour(outlineColourId, juce::Colours::white.withAlpha(0.5f));
    
    // Set faster decay rates for better responsiveness
    meterDecayRate = 36.0f;    // 36dB/second decay for main level
    peakDecayRate = 24.0f;     // Faster decay for peak marker (was 12.0f)
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
    g.setColour(juce::Colour(0xFF0F0F1A));
    g.fillRoundedRectangle(bounds, 2.0f);
    
    // Draw grid lines - 80s style
    g.setColour(juce::Colour(0xFF2A2A40));
    float gridSpacing;
    
    if (isVertical) {
        gridSpacing = bounds.getHeight() / 10.0f;
        for (int i = 1; i < 10; i++) {
            float y = bounds.getY() + i * gridSpacing;
            g.drawLine(bounds.getX(), y, bounds.getRight(), y, 1.0f);
        }
    } else {
        gridSpacing = bounds.getWidth() / 10.0f;
        for (int i = 1; i < 10; i++) {
            float x = bounds.getX() + i * gridSpacing;
            g.drawLine(x, bounds.getY(), x, bounds.getBottom(), 1.0f);
        }
    }
    
    // Draw border with neon effect
    g.setColour(meterColourLow.withAlpha(0.5f));
    g.drawRoundedRectangle(bounds, 2.0f, 1.0f);
    
    // Draw level meter
    if (level > 0.0f)
    {
        if (isVertical)
        {
            auto levelHeight = bounds.getHeight() * level;
            auto meterBounds = bounds.removeFromBottom(levelHeight);
            
            // Create a gradient from cyan to magenta with translucent effect
            juce::ColourGradient gradient;
            gradient.point1 = meterBounds.getBottomLeft();
            gradient.point2 = meterBounds.getTopLeft();
            
            // Add color stops based on level with translucency
            gradient.addColour(0.0, meterColourLow.withAlpha(0.65f));                  // Translucent cyan at bottom
            gradient.addColour(0.5, meterColourLow.interpolatedWith(meterColourHigh, 0.3f).withAlpha(0.7f));  // Blended in middle
            gradient.addColour(0.85, meterColourLow.interpolatedWith(meterColourHigh, 0.7f).withAlpha(0.8f)); // More magenta
            gradient.addColour(1.0, meterColourHigh.withAlpha(0.85f));                 // Translucent magenta at top
            
            g.setGradientFill(gradient);
            g.fillRoundedRectangle(meterBounds, 2.0f);
            
            // Add an inner highlight to create glass effect
            auto innerHighlightBounds = meterBounds.reduced(1.0f, 2.0f);
            innerHighlightBounds.setBottom(innerHighlightBounds.getBottom() - 2.0f);
            g.setColour(juce::Colours::white.withAlpha(0.15f));
            g.fillRoundedRectangle(innerHighlightBounds, 1.5f);
            
            // Draw neon edges to make it pop
            g.setColour(juce::Colours::white.withAlpha(0.3f));
            g.drawRoundedRectangle(meterBounds, 2.0f, 1.0f);
            
            // Draw neon highlight along top edge
            g.setColour(juce::Colours::white.withAlpha(0.6f));
            g.fillRect(meterBounds.getX() + 2.0f, meterBounds.getY(), meterBounds.getWidth() - 4.0f, 1.5f);
        }
        else
        {
            auto levelWidth = bounds.getWidth() * level;
            auto meterBounds = bounds.removeFromLeft(levelWidth);
            
            // Create a gradient from cyan to magenta with translucent effect
            juce::ColourGradient gradient;
            gradient.point1 = meterBounds.getTopLeft();
            gradient.point2 = meterBounds.getTopRight();
            
            // Add color stops based on level with translucency
            gradient.addColour(0.0, meterColourLow.withAlpha(0.65f));                  // Translucent cyan at left
            gradient.addColour(0.5, meterColourLow.interpolatedWith(meterColourHigh, 0.3f).withAlpha(0.7f));  // Blended in middle
            gradient.addColour(0.85, meterColourLow.interpolatedWith(meterColourHigh, 0.7f).withAlpha(0.8f)); // More magenta
            gradient.addColour(1.0, meterColourHigh.withAlpha(0.85f));                 // Translucent magenta at right
            
            g.setGradientFill(gradient);
            g.fillRoundedRectangle(meterBounds, 2.0f);
            
            // Add an inner highlight to create glass effect
            auto innerHighlightBounds = meterBounds.reduced(2.0f, 1.0f);
            innerHighlightBounds.setRight(innerHighlightBounds.getRight() - 2.0f);
            g.setColour(juce::Colours::white.withAlpha(0.15f));
            g.fillRoundedRectangle(innerHighlightBounds, 1.5f);
            
            // Draw neon edges to make it pop
            g.setColour(juce::Colours::white.withAlpha(0.3f));
            g.drawRoundedRectangle(meterBounds, 2.0f, 1.0f);
            
            // Draw neon highlight along right edge
            g.setColour(juce::Colours::white.withAlpha(0.6f));
            g.fillRect(meterBounds.getRight() - 1.5f, meterBounds.getY() + 2.0f, 1.5f, meterBounds.getHeight() - 4.0f);
        }
    }
    
    // Draw peak marker with neon glow effect
    if (showPeak && peak > 0.0f)
    {
        // Use a more prominent color for peak indicator
        juce::Colour peakColour = meterColourHigh.brighter(0.2f);
        
        if (isVertical)
        {
            auto peakY = bounds.getBottom() - bounds.getHeight() * peak;
            
            // Draw glow
            g.setColour(peakColour.withAlpha(0.5f));
            g.fillRect(bounds.getX(), peakY - 2.0f, bounds.getWidth(), 4.0f);
            
            // Draw peak line
            g.setColour(juce::Colours::white);
            g.fillRect(bounds.getX() + 1.0f, peakY, bounds.getWidth() - 2.0f, 1.0f);
        }
        else
        {
            auto peakX = bounds.getX() + bounds.getWidth() * peak;
            
            // Draw glow
            g.setColour(peakColour.withAlpha(0.5f));
            g.fillRect(peakX - 2.0f, bounds.getY(), 4.0f, bounds.getHeight());
            
            // Draw peak line
            g.setColour(juce::Colours::white);
            g.fillRect(peakX, bounds.getY() + 1.0f, 1.0f, bounds.getHeight() - 2.0f);
        }
    }
    
    // Draw dB markings with fixed font size and positioning
    g.setColour(findColour(outlineColourId));
    
    // Create font with modern approach
    g.setFont(juce::Font(juce::Font::FontStyleFlags::plain).withHeight(10.0f));
    
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
    // Clamp level to 0.0-1.0 range
    newLevel = juce::jlimit(0.0f, 1.0f, newLevel);
    
    // Detect significant level drop (like when gain is turned down)
    bool significantDrop = (newLevel < level * 0.5f) && (level - newLevel > 0.1f);
    
    // Special case for silence or extremely low levels (approaching -inf dB)
    bool nearSilence = newLevel < 0.0001f; // ~-80dB
    
    // Special case for clipping - we're at or very near max level
    bool isClipping = newLevel > 0.99f;
    
    // Check if clipping stopped (using member variable instead of static)
    bool clippingStopped = wasClipping && !isClipping;
    wasClipping = isClipping;
    
    if (level != newLevel || clippingStopped) {
        // Store previous level to detect drops
        float prevLevel = level;
        level = newLevel;
        
        // Update peak level if new level is higher
        if (level > peak)
            peak = level;
        
        // Important: When clipping stops, force peak to start decaying
        if (clippingStopped) {
            // Set peak to a slightly lower value to unstick it from the top
            peak = 0.98f;
        }
        
        // Force peak to follow level when level drops significantly
        if (significantDrop) {
            // Move peak down with the level but keep a bit of visual delay
            peak = juce::jmax(level * 1.2f, peak * 0.5f);
        }
        
        // Force peak to exactly match level for near-silence case (master turned down to -inf)
        if (nearSilence) {
            // Immediately drop peak to match level for -inf scenarios
            peak = level;
        }
            
        hasBeenUpdatedSinceLastDecay = true;
        repaint();
    }
}

void LevelMeter::setVertical(bool shouldBeVertical)
{
    if (isVertical != shouldBeVertical) {
        isVertical = shouldBeVertical;
        repaint();
    }
}

void LevelMeter::showPeakMarker(bool shouldShowPeakMarker)
{
    if (showPeak != shouldShowPeakMarker)
    {
        showPeak = shouldShowPeakMarker;
        repaint();
    }
}

void LevelMeter::setDecayRates(float newLevelDecayRate, float newPeakDecayRate)
{
    meterDecayRate = newLevelDecayRate;
    peakDecayRate = newPeakDecayRate;
}

void LevelMeter::setMeterColour(juce::Colour low, juce::Colour mid, juce::Colour high)
{
    meterColourLow = low;
    meterColourMid = mid;
    meterColourHigh = high;
    repaint();
}

//==============================================================================
juce::Colour LevelMeter::getColourForLevel(float levelValue)
{
    if (levelValue < lowThreshold)
        return meterColourLow;
    else if (levelValue < highThreshold)
        return meterColourMid.interpolatedWith(meterColourLow, (highThreshold - levelValue) / (highThreshold - lowThreshold));
    else
        return meterColourHigh.interpolatedWith(meterColourMid, (1.0f - levelValue) / (1.0f - highThreshold));
}

void LevelMeter::updatePeakAndDecay()
{
    // Calculate elapsed time since last update
    auto now = static_cast<float>(juce::Time::getMillisecondCounterHiRes());
    float elapsedMs = now - lastUpdateTime;
    lastUpdateTime = now;
    
    // Convert to seconds for decay calculations
    float elapsedSec = elapsedMs / 1000.0f;
    
    if (hasBeenUpdatedSinceLastDecay)
    {
        hasBeenUpdatedSinceLastDecay = false;
    }
    else
    {
        // Apply level decay when no recent updates
        if (level > 0.0f)
        {
            // Apply logarithmic decay to the level (in dB)
            float dbLevel = juce::Decibels::gainToDecibels(level);
            float dbDecayAmount = meterDecayRate * elapsedSec;
            dbLevel -= dbDecayAmount;
            
            // Ensure level goes to zero when it gets very low
            if (dbLevel < -70.0f)
                level = 0.0f;
            else
                level = juce::Decibels::decibelsToGain(dbLevel);
        }
        
        // Always apply peak decay - make sure peaks fall even after clipping
        if (peak > 0.0f)
        {
            // Always decay peak regardless of level
            float peakDecayAmount = peakDecayRate * elapsedSec;
            
            // If peak is at max or near max (clipping), use a MUCH faster decay rate
            if (peak >= 0.95f) {
                // Use an even faster decay rate for peaks at max
                peakDecayAmount *= 3.0f;
                
                // Apply direct reduction to help unstick from top
                peak *= 0.99f;
            }
                
            float dbPeakLevel = juce::Decibels::gainToDecibels(peak);
            dbPeakLevel -= peakDecayAmount;
            
            // Apply faster decay when level is very low
            if (level < 0.01f)
                dbPeakLevel -= peakDecayAmount * 2.0f; // Double decay rate for low levels
            
            // Ensure peak goes to zero when it gets very low - use higher threshold
            if (dbPeakLevel < -60.0f || (level < 0.01f && peak < 0.03f))
                peak = 0.0f;
            else
                peak = juce::Decibels::decibelsToGain(dbPeakLevel);
                
            // Ensure peak is never less than level UNLESS we're at max level 
            // This allows peak to fall from the top even if we're still clipping
            if (peak < level && level < 0.9f)
                peak = level;
        }
    }
}

// Add a method to completely reset the meter (for when master is turned to -inf)
void LevelMeter::reset()
{
    level = 0.0f;
    peak = 0.0f;
    hasBeenUpdatedSinceLastDecay = false;
    repaint();
} 