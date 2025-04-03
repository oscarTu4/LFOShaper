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
                   .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                   .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                   .withInput  ("Aux Input", juce::AudioChannelSet::stereo(), true)
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
    layout.add(std::make_unique<AudioParameterFloat>(
                                                     ParameterID{"sc threshold", 1}, "SC Threshold", NormalisableRange<float>(0.0, 1.0f), 0.0f));
    
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
    return false;
}

bool RectanglesAudioProcessor::producesMidi() const
{
    return false;
}

bool RectanglesAudioProcessor::isMidiEffect() const
{
    return false;
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
    auto mainInput  = layouts.getMainInputChannelSet();
    if(mainInput != juce::AudioChannelSet::stereo()
       && mainInput != juce::AudioChannelSet::mono())
        return false;
    
    if(layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
       return false;
    
    if(layouts.getNumChannels(true, 1) > 0) {
        auto sideIn = layouts.getChannelSet(true, 1);
        if(sideIn != juce::AudioChannelSet::stereo() && sideIn != juce::AudioChannelSet::mono())
            return false;
    }
    
    return true;
}
#endif

void RectanglesAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();
    
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());
    
    updatePositionInfo();
    
    juce::AudioBuffer<float> scBuffer;
    if (getBusCount(true) > 1 && getChannelCountOfBus(true, 1) > 0)
    {
        sideChainActive = true;
        scBuffer = getBusBuffer(buffer, true, 1);
        float meanRms = 0;
        for (int channel = 0; channel < scBuffer.getNumChannels(); ++channel) {
            meanRms += scBuffer.getRMSLevel(channel, 0, scBuffer.getNumSamples());
        }
        meanRms = meanRms / scBuffer.getNumChannels();
        if(meanRms > scThreshold && !lfoTriggered) {
            lfoTriggered = true;
            phase = 0.0f;
        }
    }
    else
    {
        sideChainActive = false;
    }

    if (lfoTriggered || !sideChainActive) {
        float delta_f = lfoRate / sampleRate;
        
        float lfoPhase = phase;
        float modulatorValue;
        for (int channel = 0; channel < totalNumInputChannels; ++channel)
        {
            auto* channelData = buffer.getWritePointer (channel);
            for (int sample = 0; sample < buffer.getNumSamples(); ++sample) {
                modulatorValue = modulator.getModulationValue(lfoPhase)*depth;
                channelData[sample] = channelData[sample] + channelData[sample] * modulatorValue;
                
                lfoPhase += delta_f;
                if (lfoPhase >= 1.0f) {
                    lfoTriggered = false;
                    break;
                }
            }
        }
        phase = lfoPhase;
    }
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
    auto stateXml = parameters.copyState().createXml();
    
    if (stateXml != nullptr)
    {
        if (shapeGraphXmlString.isNotEmpty())
        {
            auto shapeXml = juce::XmlDocument::parse(shapeGraphXmlString);
            if (shapeXml != nullptr)
                stateXml->addChildElement(shapeXml.release());
        }
        
        copyXmlToBinary(*stateXml, destData);
    }
}


void RectanglesAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    
    if (xmlState != nullptr)
    {
        parameters.replaceState(juce::ValueTree::fromXml(*xmlState));
        
        if (auto* shapeXml = xmlState->getChildByName("ShapeGraph"))
        {
            shapeGraphXmlString = shapeXml->toString();
        }
    }
}

bool RectanglesAudioProcessor::hasSideChainInput() {
    return sideChainActive;
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

void RectanglesAudioProcessor::setSCThreshold(float threshold) {
    scThreshold = threshold;
}

void RectanglesAudioProcessor::setLfoRate(float rate) {
    lfoRate = rate;
}

float RectanglesAudioProcessor::getPhase()  {
    return phase;
}

void RectanglesAudioProcessor::updateLfoData(const ShapeGraph& shapeGraph) {
    modulator.generateModulationValues(&shapeGraph);
}

void RectanglesAudioProcessor::setShapeGraphXmlString(const juce::String& xmlString)
{
    shapeGraphXmlString = xmlString;
}
