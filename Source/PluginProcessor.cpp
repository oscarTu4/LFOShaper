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
                                                     ParameterID{"lfoRate", 1}, "LFO Rate", NormalisableRange<float>(0.01f, 20.0f, 0.01f), 1.0f));
    
    layout.add(std::make_unique<AudioParameterBool>(
                                                    ParameterID{"sync", 1}, "Sync", false));
    layout.add(std::make_unique<AudioParameterBool>(
                                                    ParameterID{"quantize", 1}, "Quantize", false));
    
    layout.add(std::make_unique<AudioParameterFloat>(
                                                     ParameterID{"depth", 1}, "Depth", NormalisableRange<float>(0.0f, 1.0f), 0.5f));
    layout.add(std::make_unique<AudioParameterBool>(
                                                     ParameterID{"pan offset", 1}, "Pan Offset", false));
    layout.add(std::make_unique<AudioParameterFloat>(
                                                     ParameterID{"sc threshold", 1}, "SC Threshold", NormalisableRange<float>(0.0f, 0.5f), 0.2f));
    layout.add(std::make_unique<AudioParameterFloat>(
                                                     ParameterID{"sc release", 1}, "SC Release", NormalisableRange<float>(0.01f, 1.0f), 0.01f));
    layout.add(std::make_unique<AudioParameterBool>(
                                                     ParameterID{"sc", 1}, "SC", false));
    
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
    lfoSmoothed.resize(getTotalNumInputChannels(), 1.0f);
    scSmoothed.resize(getTotalNumInputChannels(), 1.0f);
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

    for (auto i = getTotalNumInputChannels(); i < getTotalNumOutputChannels(); ++i)
        buffer.clear(i, 0, buffer.getNumSamples());
    
    updatePositionInfo();

    const int numSamples = buffer.getNumSamples();
    float delta_f = lfoRate / sampleRate;
    
    if(scActivated)    {
        juce::AudioBuffer<float> scBuffer = getBusBuffer(buffer, true, 1);
        float meanRms = 0.0f;
        const int numScChannels = scBuffer.getNumChannels();
        if(numScChannels > 0)    {
            showWarningLabel = false;
            for (int channel = 0; channel < scBuffer.getNumChannels(); ++channel)
                meanRms += scBuffer.getRMSLevel(channel, 0, numSamples);
            meanRms /= scBuffer.getNumChannels();
        } else  showWarningLabel = true;
        
        if (meanRms > scThreshold)
        {
            lfoTriggered = true;
            phase = 0.0;
            //curScRelease = 4.0f * scRelease * (lfoRate / sampleRate);
            curScRelease = scRelease;
        }
        previousRms = meanRms;
        
        if (lfoTriggered)    {
            if (parameters.getRawParameterValue("sync")->load())
            {
                if (auto ppq = positionInfo.getPpqPosition())
                {
                    float secondsPerCycle = 60.0f / (getBpm() * lfoRate);
                    delta_f = 1.0f / (secondsPerCycle * sampleRate);
                }
                
            }
            for (int sample = 0; sample < numSamples; ++sample)
            {
                processSample(sample, buffer);
                phase += delta_f;
                
                if(phase >= 1.0)
                {
                    //do release update here
                    /*if(curScRelease >= 0.01f)    {
                        //decrease the release time
                        curScRelease -= delta_f;
                    }*/
                    lfoTriggered = false;
                    break;
                }
            }
        }
        else    {
            //keep the last phase
            curScRelease = scRelease;
            phase = modulator.getLastModulationValue();
            for (int sample = 0; sample < numSamples; ++sample) {
                processSample(sample, buffer);
            }
        }
    }
        

    else { //if not sidechaining
        curScRelease = scRelease;
        if (parameters.getRawParameterValue("sync")->load())    {
            if (auto ppq = positionInfo.getPpqPosition())
            {
                for (int sample = 0; sample < numSamples; ++sample)
                {
                    double samplePpq = *ppq + (sample / sampleRate) * getBpm() / 60.0f;
                    double continuousPhase = samplePpq * lfoRate;
                    phase = (continuousPhase - std::floor(continuousPhase));
                    processSample(sample, buffer);
                }
            }
        }
        else    {
            for (int sample = 0; sample < numSamples; ++sample) {
                processSample(sample, buffer);
                phase = std::fmod(phase + delta_f, 1.0f);
            }
        }
    }
}

void RectanglesAudioProcessor::processSample(int sample, juce::AudioBuffer<float>& buffer) {
    
    float rawMod;
    const float effectiveDepth = depth * (curScRelease / scRelease);
    for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
    {
        auto* channelData = buffer.getWritePointer(channel);
        if(channel % 2 == 0)    {
            rawMod = modulator.getModulationValue(phase) * effectiveDepth;
        }
        else    {
            float wrappedPhase = std::fmod(std::fmod(phase+panOffset, 1.0f) + 1.0f, 1.0f);
            rawMod = modulator.getModulationValue(wrappedPhase) * effectiveDepth;
        }
        float& smoothed = lfoSmoothed[channel];
        smoothed += smoothing * (rawMod - smoothed);
        //find good value for smoothing to get absolute 0 when no modulation
        if (std::abs(smoothed) < 0.001f)
            smoothed = 0.0f;
        channelData[sample] *= (1.0f - depth) + smoothed;
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
        // Restore parameter state
        parameters.replaceState(juce::ValueTree::fromXml(*xmlState));

        // Restore and apply shape graph
        if (auto* shapeXml = xmlState->getChildByName("ShapeGraph"))
        {
            shapeGraphXmlString = shapeXml->toString();

            // Rebuild and apply shape graph to modulator
            ShapeGraph restoredGraph;
            restoredGraph.loadXML(*shapeXml);
            updateLfoData(restoredGraph); // Ensure modulation values match
        }
    }
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

void RectanglesAudioProcessor::setPanOffset(float offset) {
    panOffset = offset;
}

void RectanglesAudioProcessor::setSCThreshold(float threshold) {
    scThreshold = threshold;
}

void RectanglesAudioProcessor::setSCRelease(float release) {
    scRelease = release;
}

void RectanglesAudioProcessor::setScActivated(bool activated) {
    scActivated = activated;
}

void RectanglesAudioProcessor::setLfoRate(float rate) {
    lfoRate = rate;
}

double RectanglesAudioProcessor::getPhase()
{
    if(!scActivated && parameters.getRawParameterValue("sync")->load())   {
            if (auto ppq = positionInfo.getPpqPosition())
            {
                double continuousPhase = *ppq * lfoRate;
                return std::fmod(continuousPhase, 1.0);
            }
        }
    return phase;
}
    


void RectanglesAudioProcessor::updateLfoData(const ShapeGraph& shapeGraph) {
    modulator.generateModulationValues(&shapeGraph);
}

void RectanglesAudioProcessor::setShapeGraphXmlString(const juce::String& xmlString)
{
    shapeGraphXmlString = xmlString;
}
