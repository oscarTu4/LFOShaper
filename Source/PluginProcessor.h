/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "Modulator.h"
#include "ShapeGraph.h"

//==============================================================================


class RectanglesAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    RectanglesAudioProcessor();
    ~RectanglesAudioProcessor() override;

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
    
    bool hasSideChainInput();
    
    void updatePositionInfo();
    double getBpm();
    
    void setDepth(float depth);
    void setPanOffset(float offset);
    void setSCThreshold(float threshold);
    void setSCRelease(float release);
    void setLfoRate(float rate);
    void updateLfoData(const ShapeGraph& shapeGraph);
    double getPhase();
    void setScActivated(bool activated);
    
    void setShapeGraphXmlString(const juce::String& xmlString);
    
    juce::AudioPlayHead::PositionInfo positionInfo;
    juce::AudioProcessorValueTreeState parameters;
    juce::String shapeGraphXmlString;
    
    bool showWarningLabel;

private:
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RectanglesAudioProcessor)
    void processSample(int sample, juce::AudioBuffer<float>& buffer);
    
    juce::Random random;
    std::vector<std::pair<float, float>> lfoShape;
    
    float scThreshold = 0.0f;
    float scRelease = 0.01f;
    double curScRelease = scRelease;
    float lfoTriggered = false;
    bool scActivated;
    float previousRms = 0.0f;
    std::vector<float> lfoSmoothed;
    std::vector<float> scSmoothed;
    
    float sampleRate;
    float lfoRate;
    Modulator modulator;
    float depth = 1.0f;
    float panOffset = 0.0f;
    double phase = 0.0;
    float smoothing = 0.005f;
    float maxRelease = 8.0f;

};
