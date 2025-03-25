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

private:
    
    int paintCounter = 0;
    
    ShapeGraph shapeGraph;
    
    juce::Point<float> draggedShapeOffset;
    
    RectanglesAudioProcessor& audioProcessor;
    
    juce::Slider lfoRateSlider;
    juce::ToggleButton syncButton;
    
    bool lfoChangePending = false;
    bool mouseDragPending = false;
    
    void noiseButtonClicked();
    void lfoRateSliderValueChanged();
    
    void timerCallback() override;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RectanglesAudioProcessorEditor)
};
