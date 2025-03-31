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
    std::vector<double> rhythmValues { 0.125, 0.1667, 0.25, 0.3333, 0.5, 0.6667, 1.0, 1.5, 2.0, 3.0, 4.0, 6.0, 8.0, 12.0 };
    juce::StringArray rhythmLabels 
            { "1/32", "1/32T", "1/16", "1/16T", "1/8", "1/8T", "1/4", "1/4T", "1/2", "1/2T", "1", "1T", "2", "2T" };
    float lastUnsyncedRate;
    float lastSyncedRate;
    juce::ToggleButton syncButton;
    
    bool lfoChangePending = false;
    bool mouseDragPending = false;
    
    void noiseButtonClicked();
    void lfoRateSliderValueChanged();
    
    void timerCallback() override;
    
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> lfoRateSliderAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> syncButtonAttachment;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RectanglesAudioProcessorEditor)
};
