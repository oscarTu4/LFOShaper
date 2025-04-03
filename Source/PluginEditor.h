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
    juce::Slider depthSlider;
    juce::Label depthLabel;
    juce::Slider scThresholdSlider;
    juce::Label scThresholdLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> lfoRateSliderAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> syncButtonAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> depthSliderAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> scThresholdSliderAttachment;
    
    std::vector<float> rhythmValues { 0.125f, 0.16667f, 0.25f, 0.333f, 0.5f, 0.667f, 1.0f, 1.5f, 2.0f, 3.0f, 4.0f, 6.0f, 8.0f, 12.0f, 16.0f };
    juce::StringArray rhythmLabels
            { "1/32", "1/32T", "1/16", "1/16T", "1/8", "1/8T", "1/4", "1/4T", "1/2", "1/2T", "1", "1T", "2", "2T", "4" };
    std::vector<float> rhythmValuesHz;
    float bpm = 120.0f;
    
    bool lfoChangePending = false;
    bool mouseDragPending = false;
    bool sideChainActive = false;
    
    void noiseButtonClicked();
    void lfoRateSliderValueChanged();
    
    void timerCallback() override;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RectanglesAudioProcessorEditor)
};
