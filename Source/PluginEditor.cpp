/*
 ==============================================================================
 
 This file contains the basic framework code for a JUCE plugin editor.
 
 ==============================================================================
 */

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "ShapeGraph.h"
#include "Modulator.h"
#include <juce_data_structures/juce_data_structures.h>

//==============================================================================
RectanglesAudioProcessorEditor::RectanglesAudioProcessorEditor (RectanglesAudioProcessor& p)
    : AudioProcessorEditor(p), audioProcessor(p)
{
    addAndMakeVisible(lfoRateSlider);
    addAndMakeVisible(depthSlider);
    addAndMakeVisible(depthLabel);
    
    lfoRateSliderAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, "lfoRate", lfoRateSlider);
    syncButtonAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(audioProcessor.parameters, "sync", syncButton);
    depthSliderAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, "depth", depthSlider);
    
    lfoRateSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
    lfoRateSlider.setRange(0.125, 16.0, 0.01);
    lfoRateSlider.setColour(juce::Slider::rotarySliderFillColourId, juce::Colours::orange);
    lfoRateSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 100, 20); // false = no outline
    lfoRateSlider.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    lfoRateSlider.setHasFocusOutline(true);
    
    lfoRateSlider.setColour(juce::Slider::thumbColourId, juce::Colours::transparentBlack);
    lfoRateSlider.setValue(1);
    lfoRateSlider.onValueChange = [this] {
        lfoRateSliderValueChanged();
    };
    
    syncButton.setButtonText("Sync");
    syncButton.onClick = [this] { syncButtonClicked(); };
    addAndMakeVisible(syncButton);
    
    depthSlider.setSliderStyle(juce::Slider::SliderStyle::LinearHorizontal);
    depthSlider.setColour(juce::Slider::thumbColourId, juce::Colours::orange);
    depthSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    depthSlider.setRange(-1.0, 1.0, 0.01);
    depthSlider.setValue(0);
    depthSlider.setVelocityBasedMode(true);
    depthSlider.setScrollWheelEnabled(true);
    depthSlider.setDoubleClickReturnValue(true, 0.0);
    depthSlider.setVelocityModeParameters(
        1.0,   // sensitivity (higher = faster value change)
        0.5,   // threshold from click point (before drag starts affecting value)
        0.09,  // offset (prevents small accidental changes)
        true   // user can fine-adjust with a modifier (like Shift)
    );
    depthSlider.onValueChange = [this] {
        audioProcessor.setDepth(depthSlider.getValue());
    };
    
    depthLabel.setText("Depth", juce::dontSendNotification);
    depthLabel.attachToComponent(&depthSlider, true);
    
    if (!syncButton.getToggleState())
    {
        lfoRateSlider.textFromValueFunction = [](double value) {
            return juce::String(value, 2) + " Hz";
        };
        lfoRateSlider.updateText();
    }

    setSize (600, 400);
    
    shapeGraph.setQuantizeDepth(8);
    shapeGraph.setLeftBound(10);
    shapeGraph.setTopBound(10);
    shapeGraph.setWidth(getWidth()-20);
    shapeGraph.setHeight(getHeight()-100);
    shapeGraph.setRightBound(shapeGraph.getLeftBound() + shapeGraph.getWidth());
    shapeGraph.setBottomBound(shapeGraph.getTopBound() + shapeGraph.getHeight());
    
    if (audioProcessor.shapeGraphXmlString.isNotEmpty()) {
        juce::XmlDocument doc(audioProcessor.shapeGraphXmlString);
        std::unique_ptr<juce::XmlElement> xml(doc.getDocumentElement());

        if (xml != nullptr)
            shapeGraph.loadXML(*xml);
    }
    
    audioProcessor.setLfoRate(lfoRateSlider.getValue());
    audioProcessor.updateLfoData(shapeGraph);
    
    rhythmValuesHz.clear();
    bpm = audioProcessor.getBpm();
    for (auto beat : rhythmValues)   {
        rhythmValuesHz.push_back(bpm/(60*beat));
        std::cout << "beat: " << beat << " Hz: " << bpm/(60*beat) << std::endl;
    }
    startTimerHz(30);
}

RectanglesAudioProcessorEditor::~RectanglesAudioProcessorEditor()
{
}

//==============================================================================
void RectanglesAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colours::black);
    
    //draw all rectangles
    shapeGraph.paint(g);
    
    //draw playhead
    g.setColour(juce::Colours::grey.withAlpha(0.5f));
    juce::Path path;
    float xPos = shapeGraph.getLeftBound()+audioProcessor.getPhase()*shapeGraph.getWidth();
    path.startNewSubPath(xPos, shapeGraph.getTopBound());
    path.lineTo(xPos, shapeGraph.getBottomBound());
    g.strokePath(path, juce::PathStrokeType(2.0f));
    path.closeSubPath();
}

void RectanglesAudioProcessorEditor::resized()
{
    //std::cout << "Resized" << std::endl;
    shapeGraph.setHeight(getHeight()-100);
    shapeGraph.setWidth(getWidth()-20);
    shapeGraph.setLeftBound(10);
    shapeGraph.setRightBound(shapeGraph.getLeftBound()+shapeGraph.getWidth());
    shapeGraph.setTopBound(10);
    shapeGraph.setBottomBound(shapeGraph.getTopBound()+shapeGraph.getHeight());
    
    shapeGraph.resizeNodeLayout();
    lfoRateSlider.setBounds(getWidth()/2, getHeight()-80, 100, 80);
    syncButton.setBounds(lfoRateSlider.getX()+lfoRateSlider.getWidth(), lfoRateSlider.getY(), 80, 80);
    depthSlider.setBounds(lfoRateSlider.getX()-100, getHeight()-80, 100, 80);
    repaint();
    
    audioProcessor.updateLfoData(shapeGraph);
}

void RectanglesAudioProcessorEditor::mouseDown(const juce::MouseEvent& event)   {
    //std::cout << "Mouse down" << std::endl;
    shapeGraph.clearSelection();
    auto [nodeIndex, node] = shapeGraph.containsPointOnNode(event.getPosition().toFloat());
    auto [edgeIndex, edge] = shapeGraph.containsPointOnEdge(event.getPosition().toFloat());
    bool clickedSomething = false;
    
    if(node) {
        //mouse click is on a node
        shapeGraph.selectNode(nodeIndex);
        draggedShapeOffset = event.getPosition().toFloat() - node->getPosition();
        
        //quantize if control key is down
        if(event.mods.isCtrlDown()) {
            shapeGraph.quantizeNode();
            clickedSomething = true;
        }
    }
    if(edge)    {
        shapeGraph.selectEdge(edgeIndex);
        draggedShapeOffset = event.getPosition().toFloat() - edge->getPosition();
        if(event.mods.isCtrlDown()) {
            shapeGraph.resetEdgeCurve(edgeIndex);
            clickedSomething = true;
        }
    }
    if(clickedSomething) repaint();
    
}


void RectanglesAudioProcessorEditor::mouseDrag(const juce::MouseEvent& event)   {
    //std::cout << "Mouse drag" << std::endl;
    mouseDragPending = false;
    if(shapeGraph.selectionType == ShapeGraph::SelectionType::Node) {
        shapeGraph.moveNode(event.getPosition().toFloat()-draggedShapeOffset);
        mouseDragPending = true;
    }
    
    else if(shapeGraph.selectionType == ShapeGraph::SelectionType::Edge) {
        shapeGraph.moveEdge(event.getPosition().toFloat()-draggedShapeOffset);
        mouseDragPending = true;
    }
}

void RectanglesAudioProcessorEditor::mouseDoubleClick(const juce::MouseEvent& event)    {
    //std::cout << "Mouse double click" << std::endl;
    auto [nodeIndex, node] = shapeGraph.containsPointOnNode(event.getPosition().toFloat());
    
    //check wether double click was on node and delete, otherwise create new node
    if(node)    {
        shapeGraph.removeNode(nodeIndex);
        std::cout << "Removed node" << std::endl;
    }
    else    {
        shapeGraph.addNode(event.getPosition().toFloat(), false);
        std::cout << "Added node" << std::endl;
    }
    audioProcessor.updateLfoData(shapeGraph);
    repaint();
}

void RectanglesAudioProcessorEditor::mouseUp(const juce::MouseEvent& event) {
    //repaint();
    shapeGraph.clearSelection();
}

void RectanglesAudioProcessorEditor::lfoRateSliderValueChanged() {
    if(syncButton.getToggleState()) {
        float freq = lfoRateSlider.getValue();
        // Snap to nearest rhythm value
        auto it = std::min_element(rhythmValuesHz.begin(), rhythmValuesHz.end(),
                                   [=](float a, float b) {
            return std::abs(a - freq) < std::abs(b -  freq);
        });
        
        if (it != rhythmValuesHz.end())
            lfoRateSlider.setValue(*it, juce::dontSendNotification);
        else std::cout << "No match found" << std::endl;
    }
    audioProcessor.setLfoRate(lfoRateSlider.getValue());
    std::cout << "lfoRateSliderValueChanged lfoRateSlider value: " << lfoRateSlider.getValue() << std::endl;
}

void RectanglesAudioProcessorEditor::syncButtonClicked() {
    bpm = audioProcessor.getBpm();
    if (syncButton.getToggleState()) {
        auto [minHzIt, maxHzIt] = std::minmax_element(rhythmValuesHz.begin(), rhythmValuesHz.end());
        std::cout << "minHz: " << *minHzIt << " maxHz: " << *maxHzIt << std::endl;
        lfoRateSlider.setRange(*minHzIt, *maxHzIt, 0.01f);
        
        //float freq = juce::jlimit<float>(*minHzIt, *maxHzIt, lfoRateSlider.getValue());
        float freq = lfoRateSlider.getValue();
        // Snap to nearest rhythm value
        auto it = std::min_element(rhythmValuesHz.begin(), rhythmValuesHz.end(),
                                   [=](float a, float b) {
            return std::abs(a - freq) < std::abs(b -  freq);
        });
        
        if (it != rhythmValuesHz.end()) {
            lfoRateSlider.setValue(*it, juce::dontSendNotification);
        } else std::cout << "No match found" << std::endl;
        audioProcessor.setLfoRate(lfoRateSlider.getValue());
        
        // Set text to rhythm labels like "1/4"
        lfoRateSlider.textFromValueFunction = [this](float value) -> juce::String {
            float note = bpm / (60.0f * value);

            auto it = std::min_element(rhythmValues.begin(), rhythmValues.end(),
                                       [=](float a, float b) {
                return std::abs(a - note) < std::abs(b - note);
            });

            if (it != rhythmValues.end())
            {
                auto index = std::distance(rhythmValues.begin(), it);
                return rhythmLabels[(int)index] + " Bars";
            }

            return juce::String(note, 2) + " Bars"; // fallback
        };


    }
    else {
        audioProcessor.setLfoRate(juce::jlimit<float>(0.125, 16.0, lfoRateSlider.getValue()));
        lfoRateSlider.setRange(0.125, 16.0, 0.01);
        
        lfoRateSlider.textFromValueFunction = [](double value) {
            return juce::String(value, 2) + " Hz";
        };
    }
    lfoRateSlider.updateText();
    std::cout << "syncButtonClicked lfoRateSlider value: " << lfoRateSlider.getValue() << std::endl;
}


void RectanglesAudioProcessorEditor::timerCallback() {
    if(mouseDragPending) {
        //repaint();
        audioProcessor.updateLfoData(shapeGraph);
        mouseDragPending = false;
    }
    if (auto xml = shapeGraph.createXML())
        audioProcessor.setShapeGraphXmlString(xml->toString());
    
    // check if bpm has changed
    float newBpm = audioProcessor.getBpm();
    if(newBpm != bpm)    {
        rhythmValuesHz.clear();
        bpm = audioProcessor.getBpm();
        for (auto beat : rhythmValues)   {
            rhythmValuesHz.push_back(bpm/(60*beat));
            std::cout << "beat: " << beat << " Hz: " << bpm/(60*beat) << std::endl;
        }
    }
    
    //update position of playhead done in repaint
    repaint();
}
