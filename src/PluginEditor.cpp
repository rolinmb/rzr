#include "PluginProcessor.h"
#include "PluginEditor.h"

RzrAudioProcessorEditor::RzrAudioProcessorEditor(RzrAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    buildSliderWithAttachment(p.apvts, mGainSlider, gainAttachment, mGainLabel, 0.f, 1.5f, 0.01f, 1.f, "GAIN", "Input Gain");
    buildSliderWithAttachment(p.apvts, mRangeSlider, rangeAttachment, mRangeLabel, 0.f, 3000.f, 1.f, 1.f, "RANGE", "Range");
    buildSliderWithAttachment(p.apvts, mBlendSlider, blendAttachment, mBlendLabel, 0.f, 1.f, 0.01f, 1.f, "BLEND", "Blend");
    buildSliderWithAttachment(p.apvts, mVolumeSlider, volumeAttachment, mVolumeLabel, 0.f, 3.f, 0.01f, 1.f, "VOLUME", "Output Volume");
    buildSliderWithAttachment(p.apvts, mBitsSlider, bitsAttachment, mBitsLabel, 3.f, 32.f, 1.f, 32.f, "BITS", "Master Bit Depth");
    buildSliderWithAttachment(p.apvts, mQuantSlider, quantAttachment, mQuantLabel, 3.f, 24.f, 1.f, 8.f, "QUANTIZATION", "Quantization Bit Depth");
    buildSliderWithAttachment(p.apvts, mChebySlider, chebyAttachment, mChebyLabel, 1.f, 10.f, 1.f, 1.f, "CHEBYORDER", "Chebyshev Order");
    buildSliderWithAttachment(p.apvts, mFoldbackSlider, foldbackAttachment, mFoldbackLabel, 0.1f, 1.f, .05f, 1.f, "FOLDBACK", "Fold Threshold");
    buildSliderWithAttachment(p.apvts, mFractalSlider, fractalAttachment, mFractalLabel, 0.1f, 2.5f, 0.1f, 0.1f, "FRACTAL", "Fractal Scale");
    buildSliderWithAttachment(p.apvts, mFractalIterSlider, fractalIterAttachment, mFractalIterLabel, 1.f, 10.f, 1.f, 4.f, "FRACTALITER", "Fractal Iterations");
    buildSliderWithAttachment(p.apvts, mResonanceSlider, resonanceAttachment, mResonanceLabel, 0.01f, 1.f, 0.01f, 0.5f, "RESONANCE", "Resonance");

    fxSelector.addItemList(selectorOptions, 1);

    fxSelector.onChange = [this]() {
        mQuantSlider.setVisible(false);
        mQuantLabel.setVisible(false);

        mChebySlider.setVisible(false);
        mChebyLabel.setVisible(false);

        mFoldbackSlider.setVisible(false);
        mFoldbackLabel.setVisible(false);

        mFractalSlider.setVisible(false);
        mFractalLabel.setVisible(false);
        mFractalIterSlider.setVisible(false);
        mFractalIterLabel.setVisible(false);

        mResonanceSlider.setVisible(false);
        mResonanceLabel.setVisible(false);

        if (fxSelector.getSelectedItemIndex() == 12) // Quantize Selection
        {
            mQuantSlider.setVisible(true);
            mQuantLabel.setVisible(true);
        }
        else if (fxSelector.getSelectedItemIndex() == 13) // Chebyshev Selection
        {
            mChebySlider.setVisible(true);
            mChebyLabel.setVisible(true);
        }
        else if (fxSelector.getSelectedItemIndex() == 14) // Foldback Selection
        {
            mFoldbackSlider.setVisible(true);
            mFoldbackLabel.setVisible(true);
        }
        else if (fxSelector.getSelectedItemIndex() == 23) // Fractal Selection
        {
            mFractalSlider.setVisible(true);
            mFractalLabel.setVisible(true);
            mFractalIterSlider.setVisible(true);
            mFractalIterLabel.setVisible(true);
        }
        else if (fxSelector.getSelectedItemIndex() == 25) // Resonant Selection
        {
            mResonanceSlider.setVisible(true);
            mResonanceLabel.setVisible(true);
        }
        this->resized();
    };

    addAndMakeVisible(fxSelector);

    selectorAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(p.apvts, "FX TYPE", fxSelector);

    background = juce::ImageCache::getFromMemory(BinaryData::lazrBackground_png, BinaryData::lazrBackground_pngSize);
    logo = juce::ImageCache::getFromMemory(BinaryData::rzrLogo_png, BinaryData::rzrLogo_pngSize);

    setSize(1200, 400);
}

RzrAudioProcessorEditor::~RzrAudioProcessorEditor()
{
}

void RzrAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.drawImageWithin(background, 0, 0, getWidth(), getHeight(), juce::RectanglePlacement::stretchToFit);
    g.drawImageAt(logo, 50, 200, false);
}

void RzrAudioProcessorEditor::resized()
{
    mGainSlider.setBounds(getWidth() - 100, getHeight() / 2 - 75, 100, 150);
    mRangeSlider.setBounds(getWidth() - 250, getHeight() / 2 - 75, 100, 150);
    mBlendSlider.setBounds(getWidth() - 400, getHeight() / 2 - 75, 100, 150);
    mBitsSlider.setBounds(getWidth() - 550, getHeight() / 2 - 75, 100, 150);
    mVolumeSlider.setBounds(getWidth() - 700, getHeight() / 2 - 75, 100, 150);

    mQuantSlider.setBounds(getWidth() - 850, getHeight() / 2 - 75, 100, 150);

    mChebySlider.setBounds(getWidth() - 850, getHeight() / 2 - 75, 100, 150);

    mFoldbackSlider.setBounds(getWidth() - 850, getHeight() / 2 - 75, 100, 150);

    mFractalSlider.setBounds(getWidth() - 850, getHeight() / 2 - 75, 100, 150);
    mFractalIterSlider.setBounds(getWidth() - 1000, getHeight() / 2 - 75, 100, 150);

    mResonanceSlider.setBounds(getWidth() - 850, getHeight() / 2 - 75, 100, 150);

    mGainLabel.setBounds(getWidth() - 90, 75, 100, 50);
    mRangeLabel.setBounds(getWidth() - 223, 75, 100, 50);
    mBlendLabel.setBounds(getWidth() - 370.5, 75, 100, 50);
    mBitsLabel.setBounds(getWidth() - 525, 75, 100, 50);
    mVolumeLabel.setBounds(getWidth() - 700, 75, 100, 50);

    mQuantLabel.setBounds(getWidth() - 865, 75, 100, 50);

    mChebyLabel.setBounds(getWidth() - 865, 75, 100, 50);

    mFoldbackLabel.setBounds(getWidth() - 865, 75, 100, 50);

    mFractalLabel.setBounds(getWidth() - 865, 75, 100, 50);
    mFractalIterLabel.setBounds(getWidth() - 1020, 75, 100, 50);

    mResonanceLabel.setBounds(getWidth() - 865, 75, 100, 50);

    fxSelector.setBounds(50, 20, 150, 42.5);
}

void RzrAudioProcessorEditor::buildSliderWithAttachment(juce::AudioProcessorValueTreeState& apvts, juce::Slider& slider,
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>& attachment, juce::Label& label,
    float minVal, float maxVal, float interVal, float defaultVal, juce::String name, juce::String labelText)
{
    slider.setSliderStyle(juce::Slider::SliderStyle::LinearVertical);
    slider.setTextBoxStyle(juce::Slider::TextBoxBelow, true, 50, 20);
    slider.setRange(minVal, maxVal, interVal);
    slider.setValue(defaultVal);
    label.setText(labelText, juce::dontSendNotification);
    label.attachToComponent(&slider, false);
    addAndMakeVisible(slider);
    addAndMakeVisible(label);

    attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, name, slider);
}