#include "PluginProcessor.h"
#include "PluginEditor.h"

RzrAudioProcessor::RzrAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
    : AudioProcessor(BusesProperties()
#if ! JucePlugin_IsMidiEffect
#if ! JucePlugin_IsSynth
        .withInput("Input", juce::AudioChannelSet::stereo(), true)
#endif
        .withOutput("Output", juce::AudioChannelSet::stereo(), true)
#endif
    ), apvts(*this, &undoManager, "curState", createParams())
#endif
{
}

RzrAudioProcessor::~RzrAudioProcessor()
{
}

const juce::String RzrAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool RzrAudioProcessor::acceptsMidi() const
{
#if JucePlugin_WantsMidiInput
    return true;
#else
    return false;
#endif
}

bool RzrAudioProcessor::producesMidi() const
{
#if JucePlugin_ProducesMidiOutput
    return true;
#else
    return false;
#endif
}

bool RzrAudioProcessor::isMidiEffect() const
{
#if JucePlugin_IsMidiEffect
    return true;
#else
    return false;
#endif
}

double RzrAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int RzrAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
    // so this should be at least 1, even if you're not really implementing programs.
}

int RzrAudioProcessor::getCurrentProgram()
{
    return 0;
}

void RzrAudioProcessor::setCurrentProgram(int index)
{
}

const juce::String RzrAudioProcessor::getProgramName(int index)
{
    return {};
}

void RzrAudioProcessor::changeProgramName(int index, const juce::String& newName)
{
}

//==============================================================================
void RzrAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    resFilter.reset();
    resFilter.coefficients = juce::dsp::IIR::Coefficients<float>::makeLowPass(getSampleRate(), resCutoff, resonance);
}

void RzrAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool RzrAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
#if JucePlugin_IsMidiEffect
    juce::ignoreUnused(layouts);
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
#endif

void RzrAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    float gainVal = *apvts.getRawParameterValue("GAIN");
    float rangeVal = *apvts.getRawParameterValue("RANGE");
    float blendVal = *apvts.getRawParameterValue("BLEND");
    float volumeVal = *apvts.getRawParameterValue("VOLUME");
    float bitDepth = *apvts.getRawParameterValue("BITS");
    int quantDepth = (int)*apvts.getRawParameterValue("QUANTIZATION");
    int chebyOrder = (int)*apvts.getRawParameterValue("CHEBYORDER");
    float foldThreshold = *apvts.getRawParameterValue("FOLDBACK");
    float fractalScale = *apvts.getRawParameterValue("FRACTAL");
    int fractalIters = (int)*apvts.getRawParameterValue("FRACTALITER");
    int fxType = (int)*apvts.getRawParameterValue("FX TYPE");
    resonance = *apvts.getRawParameterValue("RESONANCE");
    updateResFilter();
    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        float* channelData = buffer.getWritePointer(channel);
        for (int s = 0; s < buffer.getNumSamples(); ++s)
        {
            float cleanOut = *channelData;

            // Applying distortion/fuzz
            switch (fxType)
            {
            case 0: // Bypass
                break;
            case 1: // Dusty
                *channelData *= (gainVal * rangeVal);
                *channelData = *channelData / (1 + fabs(*channelData));
                break;
            case 2: // Dirty
                *channelData *= (gainVal * rangeVal);
                break;
            case 3: // Power
                *channelData *= (gainVal * rangeVal);
                *channelData = (*channelData - (1.f / 3.f)) * powf(*channelData, 3.f);
                break;
            case 4: // Arctan (Soft Clipping)
                *channelData = (2.f / juce::MathConstants<float>::pi) * atanf(*channelData * gainVal * rangeVal);
                break;
            case 5: // Tan Hyperbolic
                *channelData = tanhf(*channelData * gainVal * rangeVal);
                break;
            case 6: // Sine
                *channelData = sinf(*channelData * gainVal * rangeVal);
                break;
            case 7: //Sigmoid
                *channelData *= (gainVal * rangeVal);
                *channelData = 2.f / (1 + expf(-*channelData)) - 1.f;
                break;
            case 8: // Parabolic
                *channelData *= (gainVal * rangeVal);
                *channelData = *channelData - gainVal * powf(*channelData, 2.f);
                break;
            case 9: // Logarithmic
                *channelData *= rangeVal;
                *channelData = logf(1.f + gainVal * fabs(*channelData)) / logf(1.f + gainVal);
                break;
            case 10: // EvenOdd (Even and Odd Harmonics)
                *channelData *= (gainVal * rangeVal);
                *channelData = *channelData - (gainVal * powf(*channelData, 2.f)) / 2.f;
                break;
            case 11: // JustOdd (Odd Harmonics)
                *channelData *= rangeVal;
                *channelData = *channelData + (gainVal * powf(*channelData, 3.f)) / 3.f;
                break;
            case 12: // Quantized (Bitcrushing)
                maxQuantizedVal = static_cast<float>((1 << quantDepth) - 1);
                quantizedSample = static_cast<int32_t>(*channelData * gainVal * rangeVal * maxQuantizedVal);
                *channelData = static_cast<float>(quantizedSample) / maxQuantizedVal;
                break;
            case 13: // Chebyshev
                *channelData *= rangeVal;
                *channelData = chebyshevDistortion(*channelData, chebyOrder, gainVal);
                break;
            case 14: // Foldback
                *channelData *= (gainVal * rangeVal);
                folded = fmodf(fabs(*channelData), foldThreshold);
                if (*channelData > foldThreshold)
                {
                    *channelData = folded;
                }
                else if (*channelData < -foldThreshold)
                {
                    *channelData = -folded;
                }
                break;
            case 15: // Tattered (Piecewise Linear)
                scaledSample = static_cast<int32_t>(*channelData * INT32_MAX);
                if (scaledSample > 0)
                {
                    scaledSample = (scaledSample << 1) - (scaledSample * scaledSample >> 31);
                }
                else
                {
                    scaledSample = (scaledSample << 1) + (scaledSample * scaledSample >> 31);
                }
                *channelData = gainVal * rangeVal * (static_cast<float>(scaledSample) / INT32_MAX);
                break;
            case 16: // Rectified
                *channelData = fabs(*channelData) * gainVal * rangeVal;
                break;
            case 17: // Orbit (Bitwise OR)
                scaledSample = static_cast<int32_t>(*channelData * INT32_MAX);
                scaledSample = scaledSample | 0x55555555;
                *channelData = gainVal * rangeVal * (static_cast<float>(scaledSample) / INT32_MAX);
                break;
            case 18: // Xorbit (Bitwise XOR)
                scaledSample = static_cast<int32_t>(*channelData * INT32_MAX);
                scaledSample = scaledSample ^ 0xAAAAAAAA;
                *channelData = gainVal * rangeVal * (static_cast<float>(scaledSample) / INT32_MAX);
                break;
            case 19: // Norbit (Bitwise NOR)
                scaledSample = static_cast<int32_t>(*channelData * INT32_MAX);
                scaledSample = ~(scaledSample | 0xAAAAAAAA);
                *channelData = gainVal * rangeVal * (static_cast<float>(scaledSample) / INT32_MAX);
                break;
            case 20: // Xnorbit (Bitwise XNOR)
                scaledSample = static_cast<int32_t>(*channelData * INT32_MAX);
                scaledSample = ~(scaledSample ^ 0xAAAAAAAA);
                *channelData = gainVal * rangeVal * (static_cast<float>(scaledSample) / INT32_MAX);
                break;
            case 21: // Andbit (Bitwise AND)
                scaledSample = static_cast<int32_t>(*channelData * INT32_MAX);
                scaledSample = scaledSample & 0xAAAAAAAA;
                *channelData = gainVal * rangeVal * (static_cast<float>(scaledSample) / INT32_MAX);
                break;
            case 22: // Nandbit (Bitwise NAND)
                scaledSample = static_cast<int32_t>(*channelData * INT32_MAX);
                scaledSample = ~(scaledSample & 0xAAAAAAAA);
                *channelData = gainVal * rangeVal * (static_cast<float>(scaledSample) / INT32_MAX);
                break;
            case 23: // Fractal
                *channelData *= (gainVal * rangeVal);
                for (int i = 0; i < fractalIters; ++i)
                {
                    *channelData = fractalFunction(*channelData) * fractalScale;
                }
                break;
            case 24: // Systemic (L-System)
                lsysDist = lsysAmnt * (static_cast<float>(lsysIters) / maxLsysIters);
                lsysSample = (*channelData * gainVal * rangeVal) + lsysDist + lsysOffset;
                *channelData = juce::jlimit(-1.f, 1.f, lsysSample);
                break;
            case 25: // Resonant
                *channelData *= (gainVal * rangeVal);
                filteredSample = resFilter.processSample(*channelData);
                *channelData = *channelData + (filteredSample - *channelData) * resonance;
                break;
            default:
                break;
            }

            // Apply Bitcrushing after fxSelector effect applied
            masterQLevels = powf(2, bitDepth);
            *channelData -= fmodf(*channelData, 1 / masterQLevels);

            // Apply Blend and Volume after fuzz and bitcrushing
            *channelData = ((*channelData * blendVal) + (cleanOut * (1.f / blendVal)) / 2) * volumeVal;

            if (*channelData > 0.8)
            {
                *channelData = 0.8;
            }
            if (*channelData < -0.8)
            {
                *channelData = -0.8;
            }
            channelData++; // Increment channelData pointer to go to next float sample
        }
    }
}

//==============================================================================
bool RzrAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* RzrAudioProcessor::createEditor()
{
    return new RzrAudioProcessorEditor(*this);
}

//==============================================================================
void RzrAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
    std::unique_ptr<juce::XmlElement> xml(apvts.state.createXml());
    copyXmlToBinary(*xml, destData);
}

void RzrAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
    std::unique_ptr<juce::XmlElement> presetXml(getXmlFromBinary(data, sizeInBytes));
    if (presetXml != nullptr)
    {
        if (presetXml -> hasTagName(apvts.state.getType()))
        {
            apvts.replaceState(juce::ValueTree::fromXml(*presetXml));
        }
    }
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new RzrAudioProcessor();
}

juce::AudioProcessorValueTreeState::ParameterLayout RzrAudioProcessor::createParams()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    params.push_back(std::make_unique<juce::AudioParameterFloat>("GAIN", "Gain", juce::NormalisableRange<float>(0.f, 1.5f, 0.01f), 1.f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("RANGE", "Range", juce::NormalisableRange<float>(0.f, 3000.f, 1.f), 1.f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("BLEND", "Blend", juce::NormalisableRange<float>(0.01f, 1.f, 0.01f), 1.f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("VOLUME", "Volume", juce::NormalisableRange<float>(0.f, 3.f, 0.01f), 1.f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("BITS", "Bits", juce::NormalisableRange<float>(3.f, 32.f, 1.f), 32.f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("QUANTIZATION", "Quantization", juce::NormalisableRange<float>(3.f, 24.f, 1.f), 8.f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("CHEBYORDER", "Chebyorder", juce::NormalisableRange<float>(1.f, 10.f, 1.f), 1.f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("FOLDBACK", "Foldback", juce::NormalisableRange<float>(0.1f, 1.f, .05f), 1.f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("FRACTAL", "Fractal", juce::NormalisableRange<float>(0.1f, 2.5f, 0.1f), 0.1f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("FRACTALITER", "Fractaliter", juce::NormalisableRange<float>(1.f, 10.f, 1.f), 4.f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("RESONANCE", "Resonance", juce::NormalisableRange<float>(0.01f, 1.f, 0.01f), 0.5f));
    params.push_back(std::make_unique<juce::AudioParameterChoice>("FX TYPE", "Fx Type", juce::StringArray({
        "None/Bypass",
        "Dusty", "Dirty", "Power",
        "Arctan", "Tan Hyperbolic", "Sine",
        "Sigmoid", "Parabolic", "Logarithmic",
        "EvenOdd", "JustOdd",
        "Quantized", "Chebyshev", "Foldback", "Tattered", "Rectified",
        "Orbit", "Xorbit", "Norbit", "Xnorbit", "Andbit", "Nandbit",
        "Fractal", "Systemic", "Resonant" }), 0));
    return { params.begin(), params.end() };
}

float RzrAudioProcessor::chebyshevPolynomial(float x, int order)
{
    if (order == 0)
    {
        return 1.f;
    }
    else if (order == 1)
    {
        return x;
    }
    else
    {
        return 2.f * x * chebyshevPolynomial(x, order - 1) - chebyshevPolynomial(x, order - 2);
    }
}

float RzrAudioProcessor::chebyshevDistortion(float input, int order, float gain)
{
    float distorted = input * gain * chebyshevPolynomial(input, order);
    return distorted;
}

float RzrAudioProcessor::fractalFunction(float x)
{
    return (x < 0.5) ? x * 2.f : 1.f - (x - 0.5f) * 2.f;
}

void RzrAudioProcessor::updateResFilter()
{
    resFilter.reset();
    resFilter.coefficients = juce::dsp::IIR::Coefficients<float>::makeLowPass(getSampleRate(), resCutoff, resonance);
}