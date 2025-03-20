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
/**
*/

enum NoiseState {Playing = 0, Stopped = 1};


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
    
    NoiseState getState();
    void changeState(NoiseState newState);
    
    void setLfoRate(float rate);
    void updateLfoShape(const ShapeGraph& shapeGraph);
    

private:
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RectanglesAudioProcessor)

    juce::Random random;
    NoiseState state;
    std::vector<std::pair<float, float>> lfoShape;
    
    float sampleRate;
    float lfoRate;
    Modulator modulator;
    float phase = 0;
};
