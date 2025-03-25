/*
 ==============================================================================
 
 This file contains the basic framework code for a JUCE plugin editor.
 
 ==============================================================================
 */

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "ShapeGraph.h"
#include "Modulator.h"

//==============================================================================
RectanglesAudioProcessorEditor::RectanglesAudioProcessorEditor (RectanglesAudioProcessor& p)
    : AudioProcessorEditor(p), audioProcessor(p)
{
    
    lfoRateSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
    lfoRateSlider.setRange(0.01, 5, 0.01);
    lfoRateSlider.setColour(juce::Slider::rotarySliderFillColourId, juce::Colours::orange);
    lfoRateSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, true, 100, 20);
    lfoRateSlider.setHasFocusOutline(true);
    lfoRateSlider.setTextValueSuffix("Hz");
    lfoRateSlider.setColour(juce::Slider::thumbColourId, juce::Colours::transparentBlack);
    lfoRateSlider.setValue(1);
    lfoRateSlider.onValueChange = [this] {
        lfoRateSliderValueChanged();
    };
    addAndMakeVisible(lfoRateSlider);
    
    syncButton.setButtonText("Sync");
    addAndMakeVisible(syncButton);
    
    setSize (600, 400);
    
    shapeGraph.setQuantizeDepth(8);
    shapeGraph.setLeftBound(10);
    shapeGraph.setTopBound(10);
    shapeGraph.setWidth(getWidth()-20);
    shapeGraph.setHeight(getHeight()-100);
    shapeGraph.setRightBound(shapeGraph.getLeftBound() + shapeGraph.getWidth());
    shapeGraph.setBottomBound(shapeGraph.getTopBound() + shapeGraph.getHeight());
    
    audioProcessor.setLfoRate(lfoRateSlider.getValue());
    audioProcessor.updateLfoShape(shapeGraph);
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
    
    //paintCounter++;
    //std::cout << paintCounter << std::endl;
}

void RectanglesAudioProcessorEditor::resized()
{
    std::cout << "Resized" << std::endl;
    shapeGraph.setHeight(getHeight()-100);
    shapeGraph.setWidth(getWidth()-20);
    shapeGraph.setLeftBound(10);
    shapeGraph.setRightBound(shapeGraph.getLeftBound()+shapeGraph.getWidth());
    shapeGraph.setTopBound(10);
    shapeGraph.setBottomBound(shapeGraph.getTopBound()+shapeGraph.getHeight());
    
    shapeGraph.resizeNodeLayout();
    lfoRateSlider.setBounds(getWidth()/2, getHeight()-100, 100, 80);
    //place syncButton right next to lfoRateSlider
    syncButton.setBounds(lfoRateSlider.getX()+lfoRateSlider.getWidth(), lfoRateSlider.getY(), 80, 80);
    
    repaint();
    
    audioProcessor.updateLfoShape(shapeGraph);
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
            //if the control key is pressed, quantize the node
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

//TODO check if position has actually changed, might reduce CPU usage
void RectanglesAudioProcessorEditor::mouseDrag(const juce::MouseEvent& event)   {
    //std::cout << "Mouse drag" << std::endl;
    bool grabbedSomething = false;
    if(shapeGraph.selectionType == ShapeGraph::SelectionType::Node) {
        shapeGraph.moveNode(event.getPosition().toFloat()-draggedShapeOffset);
        grabbedSomething = true;
    }
    
    else if(shapeGraph.selectionType == ShapeGraph::SelectionType::Edge) {
        shapeGraph.moveEdge(event.getPosition().toFloat()-draggedShapeOffset);
        grabbedSomething = true;
    }
    if(grabbedSomething) {
        startTimer(10);
        mouseDragPending = true;
    }
}

void RectanglesAudioProcessorEditor::mouseDoubleClick(const juce::MouseEvent& event)    {
    //std::cout << "Mouse double click" << std::endl;
    auto [nodeIndex, node] = shapeGraph.containsPointOnNode(event.getPosition().toFloat());
    //auto [edgeIndex, edge] = shapeGraph.containsPointOnEdge(event.getPosition().toFloat());
    
    //check wether double click was on node and delete, otherwise create new node
    if(node)    {
        shapeGraph.removeNode(nodeIndex);
        std::cout << "Removed node" << std::endl;
    }
    else    {
        shapeGraph.addNode(event.getPosition().toFloat(), false);
        std::cout << "Added node" << std::endl;
    }
    audioProcessor.updateLfoShape(shapeGraph);
    repaint();
}

void RectanglesAudioProcessorEditor::mouseUp(const juce::MouseEvent& event) {
    //only do repaint() here for efficiency?
    //repaint();
    shapeGraph.clearSelection();
}

void RectanglesAudioProcessorEditor::lfoRateSliderValueChanged() {
    if(!lfoChangePending) {
        startTimer(100);
        lfoChangePending = true;
    }
}

void RectanglesAudioProcessorEditor::timerCallback() {
    if(lfoChangePending) {
        audioProcessor.setLfoRate(lfoRateSlider.getValue());
        lfoChangePending = false;
        stopTimer();
    }
    
    if(mouseDragPending) {
        repaint();
        audioProcessor.updateLfoShape(shapeGraph);
        mouseDragPending = false;
        stopTimer();
    }
}
