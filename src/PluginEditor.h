#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

class RzrAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    RzrAudioProcessorEditor(RzrAudioProcessor&);
    ~RzrAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    juce::Slider mGainSlider;
    juce::Slider mRangeSlider;
    juce::Slider mBlendSlider;
    juce::Slider mVolumeSlider;
    juce::Slider mBitsSlider;
    juce::Slider mQuantSlider;
    juce::Slider mChebySlider;
    juce::Slider mFoldbackSlider;
    juce::Slider mFractalSlider;
    juce::Slider mFractalIterSlider;
    juce::Slider mResonanceSlider;

    juce::Label mGainLabel;
    juce::Label mRangeLabel;
    juce::Label mBlendLabel;
    juce::Label mVolumeLabel;
    juce::Label mBitsLabel;
    juce::Label mQuantLabel;
    juce::Label mChebyLabel;
    juce::Label mFoldbackLabel;
    juce::Label mFractalLabel;
    juce::Label mFractalIterLabel;
    juce::Label mResonanceLabel;

    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> gainAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> rangeAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> blendAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> volumeAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> bitsAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> quantAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> chebyAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> foldbackAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> fractalAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> fractalIterAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> resonanceAttachment;

    void buildSliderWithAttachment(juce::AudioProcessorValueTreeState& apvts, juce::Slider& slider,
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>& attachment, juce::Label& label,
        float minVal, float maxVal, float interVal, float defaultVal, juce::String name, juce::String labelText);

    juce::ComboBox fxSelector;

    juce::StringArray selectorOptions = {
        "None/Bypass",
        "Dusty", "Dirty", "Power",
        "Arctan", "Tan Hyperbolic", "Sine",
        "Sigmoid", "Parabolic", "Logarithmic",
        "EvenOdd", "JustOdd",
        "Quantized", "Chebyshev", "Foldback", "Tattered", "Rectified",
        "Orbit", "Xorbit", "Norbit", "Xnorbit", "Andbit", "Nandbit",
        "Fractal", "Systemic", "Resonant" };

    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> selectorAttachment;

    juce::Image background;

    RzrAudioProcessor& audioProcessor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RzrAudioProcessorEditor)
};