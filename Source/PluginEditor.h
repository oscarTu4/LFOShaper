/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "ShapeGraph.h"

//==============================================================================
/**
*/

class RectanglesAudioProcessorEditor  : public juce::AudioProcessorEditor, public juce::Timer
{
public:
    RectanglesAudioProcessorEditor (RectanglesAudioProcessor&);
    ~RectanglesAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    void mouseDown(const juce::MouseEvent&) override;
    void mouseDrag(const juce::MouseEvent&) override;
    void mouseDoubleClick(const juce::MouseEvent&) override;
    void mouseUp(const juce::MouseEvent&) override;
    
    void syncButtonClicked();

private:
    
    RectanglesAudioProcessor& audioProcessor;
    int paintCounter = 0;
    
    ShapeGraph shapeGraph;
    juce::Point<float> draggedShapeOffset;
    
    juce::Slider lfoRateSlider;
    juce::ToggleButton syncButton;
    juce::ToggleButton quantizeButton;
    juce::Slider depthSlider;
    juce::Label depthLabel;
    juce::Slider panOffsetSlider;
    juce::Label panOffsetLabel;
    juce::Slider scThresholdSlider;
    juce::Label scThresholdLabel;
    juce::ToggleButton scButton;
    //juce::Slider scReleaseSlider;
    //juce::Label scReleaseLabel;
    //juce::Label scWarningLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> lfoRateSliderAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> syncButtonAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> depthSliderAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> scThresholdSliderAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> scButtonAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> scReleaseSliderAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> panOffsetSliderAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> quantizeButtonAttachment;
    
    
    std::vector<float> rhythmValues {
        1.0f/16.0f, 1.0f/12.0f, 0.125f,     // 4, 3, 2
        0.25f, 1.0f/1.5f, 0.5f,             // 1, 3/4, 1/2
        2.0f / 3.0f, 1.0f, 1.5f,        // 1/4T, 1/4, 1/4.
        2.0f, 8.0f / 3.0f, 3.0f,        // 1/8, 1/8T, 1/8.
        4.0f, 16.0f / 3.0f, 6.0f,       // 1/16, 1/16T, 1/16.
        8.0f, 24.0f / 3.0f, 12.0f       // 1/32, 1/32T, 1/32.
    };

    juce::StringArray rhythmLabels {
        "4", "3", "2",
        "1", "3/4", "1/2",
        "1/4T", "1/4", "1/4.",
        "1/8", "1/8T", "1/8.",
        "1/16", "1/16T", "1/16.",
        "1/32", "1/32T", "1/32."
    };

    
    float bpm = 120.0f;
    
    bool lfoChangePending = false;
    bool mouseDragPending = false;
    bool sideChainActive = false;
    float lastSyncedValue = 0.0f;
    float lastFreeValue = 0.0f;
    
    void noiseButtonClicked();
    void lfoRateSliderValueChanged();
    void scButtonClicked();
    void enableSyncMode();
    void enableFreeMode();
    
    void timerCallback() override;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RectanglesAudioProcessorEditor)
};
