/*
 ==============================================================================
 
 This file contains the basic framework code for a JUCE plugin processor.
 
 ==============================================================================
 */

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "Modulator.h"
#include <optional>
#include <juce_data_structures/juce_data_structures.h>

//==============================================================================
RectanglesAudioProcessor::RectanglesAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
:  AudioProcessor (BusesProperties()
#if ! JucePlugin_IsMidiEffect
#if ! JucePlugin_IsSynth
                   .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
#endif
                   .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
#endif
                   ),
parameters (*this, nullptr, "PARAMETERS", [] {
    using namespace juce;
    AudioProcessorValueTreeState::ParameterLayout layout;
    
    layout.add(std::make_unique<AudioParameterFloat>(
                                                     ParameterID{"lfoRate", 1}, "LFO Rate", NormalisableRange<float>(0.125f, 16.0f, 0.01f), 1.0f));
    
    layout.add(std::make_unique<AudioParameterBool>(
                                                    ParameterID{"sync", 1}, "Sync", false));
    
    layout.add(std::make_unique<AudioParameterFloat>(
                                                     ParameterID{"depth", 1}, "Depth", NormalisableRange<float>(-1.0f, 1.0f), 0.0f));
    
    return layout;
}())
#endif
{
}

RectanglesAudioProcessor::~RectanglesAudioProcessor()
{
}

//==============================================================================
const juce::String RectanglesAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool RectanglesAudioProcessor::acceptsMidi() const
{
#if JucePlugin_WantsMidiInput
    return true;
#else
    return false;
#endif
}

bool RectanglesAudioProcessor::producesMidi() const
{
#if JucePlugin_ProducesMidiOutput
    return true;
#else
    return false;
#endif
}

bool RectanglesAudioProcessor::isMidiEffect() const
{
#if JucePlugin_IsMidiEffect
    return true;
#else
    return false;
#endif
}

double RectanglesAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int RectanglesAudioProcessor::getNumPrograms()
{
    return 1;
}

int RectanglesAudioProcessor::getCurrentProgram()
{
    return 0;
}

void RectanglesAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String RectanglesAudioProcessor::getProgramName (int index)
{
    return {};
}

void RectanglesAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void RectanglesAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    this->sampleRate = (float) sampleRate;
    phase = 0.0f;
}

void RectanglesAudioProcessor::releaseResources()
{
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool RectanglesAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
#if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
#else
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
        && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;
    
#if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
#endif
    
    return true;
#endif
}
#endif

void RectanglesAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();
    
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());
    
    updatePositionInfo();
    float delta_f = lfoRate / sampleRate;
    
    float lfoPhase = phase;
    float modulatorValue;
    float noiseGain = 0.25;
    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        auto* channelData = buffer.getWritePointer (channel);
        for (int sample = 0; sample < buffer.getNumSamples(); ++sample) {
            modulatorValue = modulator.getModulationValue(lfoPhase)*depth;
            //float noise = noiseGain * (2.0f * random.nextFloat() - 1.0f);
            channelData[sample] = channelData[sample] + channelData[sample] * modulatorValue;
            //channelData[sample] = noise + noise * modulatorValue;
            
            lfoPhase += delta_f;
            if (lfoPhase >= 1.0f) lfoPhase -= 1.0f;
        }
    }
    phase = lfoPhase;
}

//==============================================================================
bool RectanglesAudioProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* RectanglesAudioProcessor::createEditor()
{
    return new RectanglesAudioProcessorEditor (*this);
}

//==============================================================================
void RectanglesAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    /*auto stateXml = parameters.copyState().createXml();
    
    if (stateXml != nullptr)
    {
        if (shapeGraphXmlString.isNotEmpty())
        {
            auto shapeXml = juce::XmlDocument::parse(shapeGraphXmlString);
            if (shapeXml != nullptr)
                stateXml->addChildElement(shapeXml.release());
        }
        
        copyXmlToBinary(*stateXml, destData);
    }*/
}


void RectanglesAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    /*std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    
    if (xmlState != nullptr)
    {
        parameters.replaceState(juce::ValueTree::fromXml(*xmlState));
        
        if (auto* shapeXml = xmlState->getChildByName("ShapeGraph"))
        {
            shapeGraphXmlString = shapeXml->toString();
        }
    }*/
}


//==============================================================================
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new RectanglesAudioProcessor();
}

void RectanglesAudioProcessor::updatePositionInfo() {
    if(auto* playHead = getPlayHead()) {
        if(auto info = playHead->getPosition()) {
            positionInfo = *info;
        }
    }
}

double RectanglesAudioProcessor::getBpm() {
    juce::Optional<double> bpm = positionInfo.getBpm();
    return bpm ? *bpm : 120.0;
}

void RectanglesAudioProcessor::setDepth(float depth) {
    this->depth = depth;
}

void RectanglesAudioProcessor::setLfoRate(float rate) {
    lfoRate = rate;
}

void RectanglesAudioProcessor::updateLfoShape(const ShapeGraph& shapeGraph) {
    modulator.generateModulationValues(&shapeGraph);
}

void RectanglesAudioProcessor::setShapeGraphXmlString(const juce::String& xmlString)
{
    shapeGraphXmlString = xmlString;
}
