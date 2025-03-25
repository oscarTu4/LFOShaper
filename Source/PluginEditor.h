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
    std::vector<double> rhythmValues { 0.125, 0.25, 0.5, 1.0, 2.0, 4.0, 8.0 };
    juce::StringArray rhythmLabels { "1/32", "1/16", "1/8", "1/4", "1/2", "1", "2" };
    float lastUnsyncedRate;
    float lastSyncedRate;
    juce::ToggleButton syncButton;
    
    bool lfoChangePending = false;
    bool mouseDragPending = false;
    
    void noiseButtonClicked();
    void lfoRateSliderValueChanged();
    
    void timerCallback() override;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RectanglesAudioProcessorEditor)
};
