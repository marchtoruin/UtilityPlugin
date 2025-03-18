#pragma once

#include <JuceHeader.h>

//==============================================================================
/**
*/
class PluginV3AudioProcessor  : public juce::AudioProcessor,
                                public juce::AudioProcessorValueTreeState::Listener
{
public:
    //==============================================================================
    PluginV3AudioProcessor();
    ~PluginV3AudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    void parameterChanged(const juce::String& parameterID, float newValue) override;

    juce::AudioProcessorValueTreeState& getAPVTS() { return apvts; }
    
    // Level meter values - return target values for immediate response
    float getLeftChannelLevel() const { return leftChannelLevel.getTargetValue(); }
    float getRightChannelLevel() const { return rightChannelLevel.getTargetValue(); }

    // Method to toggle bypass state
    void setBypass(bool shouldBypass) { isBypassed = shouldBypass; }
    bool getBypass() const { return isBypassed; }

private:
    juce::AudioProcessorValueTreeState apvts;
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    
    // Gain values for separate L/R control
    float masterGain { 1.0f }; 
    float leftGain { 1.0f };
    float rightGain { 1.0f };
    
    // Phase invert flags
    bool invertLeftPhase { false };
    bool invertRightPhase { false };
    
    // Phase offset (in degrees, 0-360)
    float phaseOffset { 0.0f }; 
    
    // Mid/Side gain values
    float midGain { 1.0f };
    float sideGain { 1.0f };
    bool useMidSideProcessing { false };
    
    // For phase offset delay buffer
    std::unique_ptr<juce::AudioBuffer<float>> delayBuffer;
    int delayBufferPos { 0 };
    int delayBufferLength { 0 };
    float sampleRate { 44100.0f };
    
    // Level meters for display - smoothed with ballistics to look natural
    juce::LinearSmoothedValue<float> leftChannelLevel { 0.0f };
    juce::LinearSmoothedValue<float> rightChannelLevel { 0.0f };
    
    // Helper methods for phase processing
    void updateDelayBufferSize(int samplesPerBlock);
    float getPhaseOffsetDelaySamples() const;
    
    // Helper method for Mid/Side processing
    void processMidSide(juce::AudioBuffer<float>& buffer, int numSamples);
    
    // Bypass state
    bool isBypassed { false };
    
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginV3AudioProcessor)
};
