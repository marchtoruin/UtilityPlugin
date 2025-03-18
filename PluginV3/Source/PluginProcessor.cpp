#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
PluginV3AudioProcessor::PluginV3AudioProcessor()
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ),
       apvts (*this, nullptr, "Parameters", createParameterLayout())
{
    // Add parameter listeners
    apvts.addParameterListener("master_gain", this);
    apvts.addParameterListener("left_gain", this);
    apvts.addParameterListener("right_gain", this);
    apvts.addParameterListener("invert_left", this);
    apvts.addParameterListener("invert_right", this);
    apvts.addParameterListener("phase_offset", this);
    apvts.addParameterListener("mid_gain", this);
    apvts.addParameterListener("side_gain", this);
    apvts.addParameterListener("use_mid_side", this);
}

PluginV3AudioProcessor::~PluginV3AudioProcessor()
{
    // Remove parameter listeners
    apvts.removeParameterListener("master_gain", this);
    apvts.removeParameterListener("left_gain", this);
    apvts.removeParameterListener("right_gain", this);
    apvts.removeParameterListener("invert_left", this);
    apvts.removeParameterListener("invert_right", this);
    apvts.removeParameterListener("phase_offset", this);
    apvts.removeParameterListener("mid_gain", this);
    apvts.removeParameterListener("side_gain", this);
    apvts.removeParameterListener("use_mid_side", this);
}

juce::AudioProcessorValueTreeState::ParameterLayout PluginV3AudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    
    // Create master gain parameter
    auto masterGainParam = std::make_unique<juce::AudioParameterFloat>(
        "master_gain",                             // Parameter ID
        "Master Gain",                             // Parameter name
        juce::NormalisableRange<float>(0.0f, 3.16227766017f, 0.001f, 0.3f), // min, max, step, skew
        1.0f);                                     // Default value (0dB, no gain change)
    
    // Create left channel gain parameter
    auto leftGainParam = std::make_unique<juce::AudioParameterFloat>(
        "left_gain",                               // Parameter ID
        "Left Gain",                               // Parameter name
        juce::NormalisableRange<float>(0.0f, 3.16227766017f, 0.001f, 0.3f), // min, max, step, skew
        1.0f);                                     // Default value (0dB, no gain change)
    
    // Create right channel gain parameter
    auto rightGainParam = std::make_unique<juce::AudioParameterFloat>(
        "right_gain",                              // Parameter ID
        "Right Gain",                              // Parameter name
        juce::NormalisableRange<float>(0.0f, 3.16227766017f, 0.001f, 0.3f), // min, max, step, skew
        1.0f);                                     // Default value (0dB, no gain change)
    
    // Create left channel phase invert parameter
    auto invertLeftParam = std::make_unique<juce::AudioParameterBool>(
        "invert_left",                             // Parameter ID
        "Invert Left Phase",                       // Parameter name
        false);                                    // Default value (not inverted)
    
    // Create right channel phase invert parameter
    auto invertRightParam = std::make_unique<juce::AudioParameterBool>(
        "invert_right",                            // Parameter ID
        "Invert Right Phase",                      // Parameter name
        false);                                    // Default value (not inverted)
    
    // Create phase offset parameter (0-360 degrees)
    auto phaseOffsetParam = std::make_unique<juce::AudioParameterFloat>(
        "phase_offset",                            // Parameter ID
        "Phase Offset",                            // Parameter name
        juce::NormalisableRange<float>(0.0f, 360.0f, 0.1f), // min, max, step
        0.0f);                                     // Default value (0 degrees)
    
    // Create Mid/Side gain parameters
    auto midGainParam = std::make_unique<juce::AudioParameterFloat>(
        "mid_gain",                                // Parameter ID
        "Mid Gain",                                // Parameter name
        juce::NormalisableRange<float>(0.0f, 3.16227766017f, 0.001f, 0.3f), // min, max, step, skew
        1.0f);                                     // Default value (0dB, no gain change)
    
    auto sideGainParam = std::make_unique<juce::AudioParameterFloat>(
        "side_gain",                               // Parameter ID
        "Side Gain",                               // Parameter name
        juce::NormalisableRange<float>(0.0f, 3.16227766017f, 0.001f, 0.3f), // min, max, step, skew
        1.0f);                                     // Default value (0dB, no gain change)
    
    // Toggle to enable/disable Mid/Side processing
    auto useMidSideParam = std::make_unique<juce::AudioParameterBool>(
        "use_mid_side",                            // Parameter ID
        "Enable Mid/Side",                         // Parameter name
        false);                                    // Default value (disabled)
    
    layout.add(std::move(masterGainParam));
    layout.add(std::move(leftGainParam));
    layout.add(std::move(rightGainParam));
    layout.add(std::move(invertLeftParam));
    layout.add(std::move(invertRightParam));
    layout.add(std::move(phaseOffsetParam));
    layout.add(std::move(midGainParam));
    layout.add(std::move(sideGainParam));
    layout.add(std::move(useMidSideParam));
    
    return layout;
}

void PluginV3AudioProcessor::parameterChanged(const juce::String& parameterID, float newValue)
{
    if (parameterID == "master_gain")
        masterGain = newValue;
    else if (parameterID == "left_gain")
        leftGain = newValue;
    else if (parameterID == "right_gain")
        rightGain = newValue;
    else if (parameterID == "invert_left")
        invertLeftPhase = newValue > 0.5f;
    else if (parameterID == "invert_right")
        invertRightPhase = newValue > 0.5f;
    else if (parameterID == "phase_offset")
        phaseOffset = newValue;
    else if (parameterID == "mid_gain")
        midGain = newValue;
    else if (parameterID == "side_gain")
        sideGain = newValue;
    else if (parameterID == "use_mid_side")
        useMidSideProcessing = newValue > 0.5f;
}

void PluginV3AudioProcessor::processMidSide(juce::AudioBuffer<float>& buffer, int numSamples)
{
    // This only works with stereo audio
    if (buffer.getNumChannels() < 2)
        return;
    
    auto* leftChannel = buffer.getWritePointer(0);
    auto* rightChannel = buffer.getWritePointer(1);
    
    // Process each sample
    for (int sample = 0; sample < numSamples; ++sample)
    {
        // Convert L/R to Mid/Side
        const float mid = (leftChannel[sample] + rightChannel[sample]) * 0.5f;
        const float side = (rightChannel[sample] - leftChannel[sample]) * 0.5f;
        
        // Apply Mid/Side gain
        const float processedMid = mid * midGain;
        const float processedSide = side * sideGain;
        
        // Convert back to L/R
        leftChannel[sample] = processedMid - processedSide;
        rightChannel[sample] = processedMid + processedSide;
    }
}

float PluginV3AudioProcessor::getPhaseOffsetDelaySamples() const
{
    // Convert phase offset from degrees to samples
    // 360 degrees = 1 cycle
    return (phaseOffset / 360.0f) * (sampleRate / 100.0f); // Limit to max 10ms at 360 degrees
}

void PluginV3AudioProcessor::updateDelayBufferSize(int samplesPerBlock)
{
    // Calculate max delay needed (10ms max)
    int maxDelaySamples = juce::roundToInt(sampleRate * 0.01f); // 10ms
    
    // Ensure buffer is at least twice the block size + maximum delay
    delayBufferLength = 2 * samplesPerBlock + maxDelaySamples;
    
    // Create or resize the delay buffer if needed
    if (delayBuffer == nullptr || delayBuffer->getNumSamples() < delayBufferLength)
    {
        delayBuffer.reset(new juce::AudioBuffer<float>(2, delayBufferLength));
        delayBuffer->clear();
        delayBufferPos = 0;
    }
}

//==============================================================================
const juce::String PluginV3AudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool PluginV3AudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool PluginV3AudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool PluginV3AudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double PluginV3AudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int PluginV3AudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int PluginV3AudioProcessor::getCurrentProgram()
{
    return 0;
}

void PluginV3AudioProcessor::setCurrentProgram (int index)
{
    juce::ignoreUnused(index);
}

const juce::String PluginV3AudioProcessor::getProgramName (int index)
{
    juce::ignoreUnused(index);
    return {};
}

void PluginV3AudioProcessor::changeProgramName (int index, const juce::String& newName)
{
    juce::ignoreUnused(index, newName);
}

//==============================================================================
void PluginV3AudioProcessor::prepareToPlay (double newSampleRate, int samplesPerBlock)
{
    // Use juce::ignoreUnused for parameters we don't use directly
    
    // Store sample rate for phase offset calculations
    sampleRate = static_cast<float>(newSampleRate);
    
    // Initialize level smoothing with appropriate ramp length
    leftChannelLevel.reset(sampleRate, 0.5);  // 500ms smoothing
    rightChannelLevel.reset(sampleRate, 0.5); // 500ms smoothing
    
    leftChannelLevel.setCurrentAndTargetValue(0.0f);
    rightChannelLevel.setCurrentAndTargetValue(0.0f);
    
    // Initialize delay buffer for phase offset
    updateDelayBufferSize(samplesPerBlock);
}

void PluginV3AudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

bool PluginV3AudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}

void PluginV3AudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ignoreUnused(midiMessages);

    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // In case we have more outputs than inputs, clear any output
    // channels that didn't contain input data
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());
    
    // Actual processing of audio
    const int numSamples = buffer.getNumSamples();
    
    // Apply Mid/Side processing if enabled (before other processing)
    if (useMidSideProcessing && totalNumInputChannels > 1)
    {
        processMidSide(buffer, numSamples);
    }
    
    // Check if we need to apply phase offset
    bool applyPhaseOffset = phaseOffset > 0.001f;
    
    if (applyPhaseOffset)
    {
        // Ensure the delay buffer is large enough
        updateDelayBufferSize(numSamples);
        
        // Calculate delay in samples for current phase offset
        float delaySamples = getPhaseOffsetDelaySamples();
        int delayIntegerSamples = static_cast<int>(delaySamples);
        float delayFraction = delaySamples - delayIntegerSamples;
        
        // Copy input to delay buffer (we'll process directly from there)
        for (int channel = 0; channel < totalNumInputChannels; ++channel)
        {
            // Only delay the right channel (assuming stereo)
            if (channel == 1)
            {
                auto* inData = buffer.getReadPointer(channel);
                
                // Copy current input to delay buffer
                for (int i = 0; i < numSamples; ++i)
                {
                    int bufIndex = (delayBufferPos + i) % delayBufferLength;
                    delayBuffer->setSample(channel, bufIndex, inData[i]);
                }
            }
        }
    }
    
    // Process left channel (0)
    if (totalNumInputChannels > 0)
    {
        auto* channelData = buffer.getWritePointer(0);
        
        // Apply phase inversion if needed
        if (invertLeftPhase)
        {
            for (int sample = 0; sample < numSamples; ++sample)
            {
                channelData[sample] = -channelData[sample];
            }
        }
        
        // Apply gain
        float combinedGain = leftGain * masterGain;
        for (int sample = 0; sample < numSamples; ++sample)
        {
            channelData[sample] *= combinedGain;
        }
        
        // Find peak level AFTER applying gain
        float leftPeak = 0.0f;
        for (int sample = 0; sample < numSamples; ++sample)
        {
            leftPeak = std::max(leftPeak, std::abs(channelData[sample]));
        }
        
        // Convert peak to a dB value
        float leftDb = 0.0f;
        if (leftPeak > 0.0f)
        {
            leftDb = juce::Decibels::gainToDecibels(leftPeak, -60.0f);
            // Map to 0-1 range for meter display (-60dB to 0dB)
            float leftMeterValue = juce::jmap(leftDb, -60.0f, 0.0f, 0.0f, 1.0f);
            leftChannelLevel.setTargetValue(leftMeterValue);
        }
        else
        {
            // Set to minimum when no signal is present
            leftChannelLevel.setTargetValue(0.0f);
        }
    }
    
    // Process right channel (1)
    if (totalNumInputChannels > 1)
    {
        auto* channelData = buffer.getWritePointer(1);
        
        // Apply phase offset if needed
        if (applyPhaseOffset)
        {
            // Calculate delay in samples for current phase offset
            float delaySamples = getPhaseOffsetDelaySamples();
            int delayIntegerSamples = static_cast<int>(delaySamples);
            float delayFraction = delaySamples - delayIntegerSamples;
            
            // Read delayed samples
            for (int i = 0; i < numSamples; ++i)
            {
                int readPos = delayBufferPos - delayIntegerSamples + i;
                // Wrap around if negative
                if (readPos < 0)
                    readPos += delayBufferLength;
                // Wrap around if past end
                readPos = readPos % delayBufferLength;
                
                // Get delayed sample (simple linear interpolation for fractional delays)
                int readPos2 = (readPos + 1) % delayBufferLength;
                float sample1 = delayBuffer->getSample(1, readPos);
                float sample2 = delayBuffer->getSample(1, readPos2);
                
                // Apply linear interpolation for fractional delay
                channelData[i] = sample1 + delayFraction * (sample2 - sample1);
            }
            
            // Update delay buffer position
            delayBufferPos = (delayBufferPos + numSamples) % delayBufferLength;
        }
        
        // Apply phase inversion if needed
        if (invertRightPhase)
        {
            for (int sample = 0; sample < numSamples; ++sample)
            {
                channelData[sample] = -channelData[sample];
            }
        }
        
        // Apply gain
        float combinedGain = rightGain * masterGain;
        for (int sample = 0; sample < numSamples; ++sample)
        {
            channelData[sample] *= combinedGain;
        }
        
        // Find peak level AFTER applying gain
        float rightPeak = 0.0f;
        for (int sample = 0; sample < numSamples; ++sample)
        {
            rightPeak = std::max(rightPeak, std::abs(channelData[sample]));
        }
        
        // Convert peak to a dB value
        float rightDb = 0.0f;
        if (rightPeak > 0.0f)
        {
            rightDb = juce::Decibels::gainToDecibels(rightPeak, -60.0f);
            // Map to 0-1 range for meter display (-60dB to 0dB)
            float rightMeterValue = juce::jmap(rightDb, -60.0f, 0.0f, 0.0f, 1.0f);
            rightChannelLevel.setTargetValue(rightMeterValue);
        }
        else
        {
            // Set to minimum when no signal is present
            rightChannelLevel.setTargetValue(0.0f);
        }
    }
}

//==============================================================================
bool PluginV3AudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* PluginV3AudioProcessor::createEditor()
{
    return new PluginV3AudioProcessorEditor (*this);
}

//==============================================================================
void PluginV3AudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void PluginV3AudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    
    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName(apvts.state.getType()))
            apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new PluginV3AudioProcessor();
}
