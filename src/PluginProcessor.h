#pragma once

#include <JuceHeader.h>

class RzrAudioProcessor  : public juce::AudioProcessor
                            #if JucePlugin_Enable_ARA
                             , public juce::AudioProcessorARAExtension
                            #endif
{
public:
    //==============================================================================
    RzrAudioProcessor();
    ~RzrAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

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

    juce::AudioProcessorValueTreeState apvts;

private:
    juce::UndoManager undoManager;

    juce::AudioProcessorValueTreeState::ParameterLayout createParams();

    // Chebyshev
    float chebyshevPolynomial(float x, int order);
    float chebyshevDistortion(float input, int order, float gain);

    // Quantization
    float maxQuantizedVal = 0.f;
    int32_t quantizedSample = 0;
    
    // Foldback
    float folded = 0.f;
    
    // Tattered
    int32_t scaledSample = 0;

    // Fractal
    float fractalFunction(float x);
    
    // L-System
    int maxLsysIters = 10;
    int lsysIters = 4;
    float lsysOffset = 0.2f;
    float lsysAmnt = 0.5f;
    float lsysDist = 0.f;
    float lsysSample = 0.f;
    
    // Resonant
    float filteredSample = 0.f;
    float resCutoff = 1000.f;
    float resonance = 0.5f;
    juce::dsp::IIR::Filter<float> resFilter;
    void updateResFilter();

    // Master
    float masterQLevels = 0.f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RzrAudioProcessor)
};
