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
    addAndMakeVisible(scThresholdSlider);
    addAndMakeVisible(scThresholdLabel);
    addAndMakeVisible(syncButton);
    addAndMakeVisible(quantizeButton);
    addAndMakeVisible(scButton);
    addAndMakeVisible(scReleaseSlider);
    addAndMakeVisible(panOffsetSlider);
    addAndMakeVisible(scWarningLabel);
    
    lfoRateSliderAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, "lfoRate", lfoRateSlider);
    syncButtonAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(audioProcessor.parameters, "sync", syncButton);
    quantizeButtonAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(audioProcessor.parameters, "quantize", quantizeButton);
    depthSliderAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, "depth", depthSlider);
    scThresholdSliderAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, "sc threshold", scThresholdSlider);
    scButtonAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(audioProcessor.parameters, "sc", scButton);
    scReleaseSliderAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, "sc release", scReleaseSlider);
    panOffsetSliderAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, "pan offset", panOffsetSlider);
    
    lfoRateSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
    lfoRateSlider.setColour(juce::Slider::rotarySliderFillColourId, juce::Colours::orange);
    lfoRateSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 100, 20); // false = no outline
    lfoRateSlider.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    lfoRateSlider.setColour(juce::Slider::thumbColourId, juce::Colours::transparentBlack);
    lfoRateSlider.onValueChange = [this] {
        lfoRateSliderValueChanged();
    };
    
    syncButton.setButtonText("Sync");
    syncButton.onClick = [this] { syncButtonClicked(); };
    
    quantizeButton.setButtonText("Grid Snap");
    
    depthSlider.setSliderStyle(juce::Slider::SliderStyle::LinearHorizontal);
    depthSlider.setColour(juce::Slider::thumbColourId, juce::Colours::orange);
    depthSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    depthSlider.setRange(0.0f, 1.0f, 0.001f);
    depthSlider.setVelocityBasedMode(true);
    depthSlider.setScrollWheelEnabled(true);
    depthSlider.setDoubleClickReturnValue(true, 1.0);
    //depthSlider.setVelocityModeParameters(1.0, 0.5, 0.09, true);
    depthSlider.setSkewFactor(0.3f);
    depthSlider.onValueChange = [this] {
        audioProcessor.setDepth(depthSlider.getValue());
    };
    
    depthLabel.setText("Depth", juce::dontSendNotification);
    depthLabel.attachToComponent(&depthSlider, true);
    
    panOffsetSlider.setSliderStyle(juce::Slider::SliderStyle::LinearHorizontal);
    panOffsetSlider.setColour(juce::Slider::thumbColourId, juce::Colours::orange);
    panOffsetSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    panOffsetSlider.setRange(-0.5f, 0.5f, 0.01f);
    panOffsetSlider.setVelocityBasedMode(true);
    panOffsetSlider.setScrollWheelEnabled(true);
    panOffsetSlider.setDoubleClickReturnValue(true, 0.0);
    //panOffsetSlider.setVelocityModeParameters(1.0, 0.5, 0.09, true);
    panOffsetSlider.setSkewFactorFromMidPoint(0.0f);
    panOffsetSlider.onValueChange = [this] {
        audioProcessor.setPanOffset(panOffsetSlider.getValue());
    };
    
    panOffsetLabel.setText("Pan Offset", juce::dontSendNotification);
    panOffsetLabel.attachToComponent(&panOffsetSlider, true);
    
    scThresholdSlider.setSliderStyle(juce::Slider::SliderStyle::LinearHorizontal);
    scThresholdSlider.setColour(juce::Slider::thumbColourId, juce::Colours::orange);
    scThresholdSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    scThresholdSlider.setRange(0.0f, 0.5f, 0.001f);
    scThresholdSlider.setVelocityBasedMode(true);
    scThresholdSlider.setScrollWheelEnabled(true);
    scThresholdSlider.setDoubleClickReturnValue(true, 0.2f);
    scThresholdSlider.setVelocityModeParameters(1.0, 0.5, 0.09, true);
    scThresholdSlider.setSkewFactor(0.3f);
    scThresholdSlider.setSliderSnapsToMousePosition(false);
    scThresholdSlider.onValueChange = [this] {
        audioProcessor.setSCThreshold(scThresholdSlider.getValue());
    };
    
    scThresholdLabel.setText("Threshold", juce::dontSendNotification);
    scThresholdLabel.attachToComponent(&scThresholdSlider, true);
    
    scButton.setButtonText("sc");
    scButton.onClick = [this] {
        scButtonClicked();
    };
    
    scReleaseSlider.setSliderStyle(juce::Slider::SliderStyle::LinearHorizontal);
    scReleaseSlider.setColour(juce::Slider::thumbColourId, juce::Colours::orange);
    scReleaseSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    scReleaseSlider.setRange(0.01f, 1.0f, 0.01f);
    scReleaseSlider.setVelocityBasedMode(true);
    scReleaseSlider.setScrollWheelEnabled(true);
    scReleaseSlider.setDoubleClickReturnValue(true, 0.2f);
    scReleaseSlider.setVelocityModeParameters(1.0, 0.5, 0.09, true);
    scReleaseSlider.setSliderSnapsToMousePosition(false);
    scReleaseSlider.onValueChange = [this] {
        audioProcessor.setSCRelease(scReleaseSlider.getValue());
    };
    scReleaseLabel.setText("Release", juce::dontSendNotification);
    scReleaseLabel.attachToComponent(&scReleaseSlider, true);
    
    scWarningLabel.setText("no sc input", juce::dontSendNotification);
    scWarningLabel.setColour(juce::Label::textColourId, juce::Colours::red);
    scWarningLabel.setVisible(false);

    setSize (600, 500);
    
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
    
    scButtonClicked();
    syncButtonClicked();
    bpm = audioProcessor.getBpm();
    startTimerHz(30);
}

RectanglesAudioProcessorEditor::~RectanglesAudioProcessorEditor()
{
}

//==============================================================================
void RectanglesAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colours::black);
    
    //draw shapeGraph
    shapeGraph.paint(g);
    
    //draw playhead
    g.setColour(juce::Colours::grey.withAlpha(0.5f));
    juce::Path path;
    double xPos = shapeGraph.getLeftBound()+audioProcessor.getPhase()*shapeGraph.getWidth();
    path.startNewSubPath(xPos, shapeGraph.getTopBound());
    path.lineTo(xPos, shapeGraph.getBottomBound());
    g.strokePath(path, juce::PathStrokeType(2.0f));
    path.closeSubPath();
}

void RectanglesAudioProcessorEditor::resized()
{
    int xMargin = 10;
    int yMargin = 10;
    shapeGraph.setHeight(getHeight()-100);
    shapeGraph.setWidth(getWidth()-xMargin*2);
    shapeGraph.setLeftBound(xMargin);
    shapeGraph.setRightBound(shapeGraph.getLeftBound()+shapeGraph.getWidth());
    shapeGraph.setTopBound(yMargin);
    shapeGraph.setBottomBound(shapeGraph.getTopBound()+shapeGraph.getHeight());
    
    shapeGraph.resizeNodeLayout();
    
    int itemMargin = 80;
    int buttonSize = 30;
    
    syncButton.setBounds(getWidth()-itemMargin-5, getHeight()-itemMargin+5, itemMargin, buttonSize);
    quantizeButton.setBounds(getWidth()-itemMargin-5, getHeight()-itemMargin+buttonSize+5, itemMargin, buttonSize+5);
    
    lfoRateSlider.setBounds(syncButton.getX()-itemMargin-5, getHeight()-itemMargin, itemMargin, itemMargin);
    depthSlider.setBounds(lfoRateSlider.getX()-itemMargin, getHeight()-70, itemMargin, buttonSize);
    panOffsetSlider.setBounds(lfoRateSlider.getX()-itemMargin, getHeight()-40, itemMargin, buttonSize);
    
    scButton.setBounds(xMargin, getHeight()-70, 100, buttonSize);
    scThresholdSlider.setBounds(scButton.getX()+scButton.getWidth()+itemMargin/2+scThresholdLabel.getWidth(), getHeight()-70, itemMargin, buttonSize);
    scReleaseSlider.setBounds(scButton.getX()+scButton.getWidth()+itemMargin/2+scReleaseLabel.getWidth(), getHeight()-40, itemMargin, buttonSize);
    //scWarningLabel.setBounds(scButton.getX(), getHeight()-40, itemMargin, 30);
    
    repaint();
    
    audioProcessor.updateLfoData(shapeGraph);
}

void RectanglesAudioProcessorEditor::mouseDown(const juce::MouseEvent& event)   {
    //std::cout << "Mouse down" << std::endl;
    shapeGraph.clearSelection();
    auto [nodeIndex, node] = shapeGraph.containsPointOnNode(event.getPosition().toFloat());
    auto [edgeIndex, edge] = shapeGraph.containsPointOnEdge(event.getPosition().toFloat());
    
    if(node) {
        //mouse click is on a node
        shapeGraph.selectNode(nodeIndex);
        draggedShapeOffset = event.getPosition().toFloat() - node->getPosition();
        
        //quantize if control key is down
        if(event.mods.isCtrlDown()) {
            shapeGraph.quantizeNode();
        }
    }
    if(edge)    {
        shapeGraph.selectEdge(edgeIndex);
        draggedShapeOffset = event.getPosition().toFloat() - edge->getPosition();
        if(event.mods.isCtrlDown()) {
            shapeGraph.resetEdgeCurve(edgeIndex);
        }
    }
    
}


void RectanglesAudioProcessorEditor::mouseDrag(const juce::MouseEvent& event)   {
    //std::cout << "Mouse drag" << std::endl;
    mouseDragPending = false;
    if(shapeGraph.selectionType == ShapeGraph::SelectionType::Node) {
        shapeGraph.moveNode(event.getPosition().toFloat()-draggedShapeOffset);
        if(quantizeButton.getToggleState()) {
            shapeGraph.quantizeNode();
        }
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
        //quantize newly created node
        if(quantizeButton.getToggleState()) {
            shapeGraph.quantizeNode();
        }
        std::cout << "Added node" << std::endl;
    }
    audioProcessor.updateLfoData(shapeGraph);
    //repaint();
}

void RectanglesAudioProcessorEditor::mouseUp(const juce::MouseEvent& event) {
    shapeGraph.clearSelection();
}

void RectanglesAudioProcessorEditor::lfoRateSliderValueChanged() {
    if (syncButton.getToggleState()) {
        int index = (int) lfoRateSlider.getValue();
        float value = rhythmValues[index];
        audioProcessor.setLfoRate(value);
        *audioProcessor.parameters.getRawParameterValue("lfoRate") = value; // <-- this ensures persistence
        lastSyncedValue = index;
    }
    else    {
        audioProcessor.setLfoRate(lfoRateSlider.getValue());
        lastFreeValue = lfoRateSlider.getValue();
    }
}

void RectanglesAudioProcessorEditor::syncButtonClicked()    {
    if(syncButton.getToggleState())    {
        enableSyncMode();
    }
    else enableFreeMode();
    lfoRateSlider.updateText();
}

void RectanglesAudioProcessorEditor::enableFreeMode()
{
    lfoRateSlider.setRange(0.01, 20.0, 0.01);
    lfoRateSlider.setSkewFactorFromMidPoint(4.0f);
    audioProcessor.setLfoRate(lfoRateSlider.getValue());
    if(lastFreeValue > 0.0f)  {
        lfoRateSlider.setValue(lastFreeValue);
    }

    lfoRateSlider.textFromValueFunction = [](double value)
    {
        return juce::String(value, 2) + " Hz";
    };

    
    lfoRateSlider.valueFromTextFunction = nullptr;
}

void RectanglesAudioProcessorEditor::enableSyncMode()   {
    lfoRateSlider.setRange(0, rhythmValues.size() - 1, 1.0);
    lfoRateSlider.setSkewFactorFromMidPoint(rhythmValues.size() / 2.0f);
    if(lastSyncedValue > 0.0f)  {
        lfoRateSlider.setValue(lastSyncedValue);
    }
    lfoRateSlider.textFromValueFunction = [this](double index)
    {
        int i = juce::jlimit(0, (int)rhythmLabels.size() - 1, (int)index);
        return rhythmLabels[i];
    };

    lfoRateSlider.valueFromTextFunction = [this](const juce::String& text)
    {
        return rhythmLabels.indexOf(text);
    };
    audioProcessor.setLfoRate(rhythmValues[(int) lfoRateSlider.getValue()]);
    *audioProcessor.parameters.getRawParameterValue("lfoRate") = rhythmValues[(int) lfoRateSlider.getValue()];
}



void RectanglesAudioProcessorEditor::scButtonClicked() {
    bool scActivated = scButton.getToggleState();
    scThresholdSlider.setVisible(scActivated);
    scReleaseSlider.setVisible(scActivated);
    /*if(scActivated) {
        scWarningLabel.setVisible(audioProcessor.showWarningLabel);
    }*/
    audioProcessor.setScActivated(scActivated);
}


void RectanglesAudioProcessorEditor::timerCallback() {
    if(mouseDragPending) {
        audioProcessor.updateLfoData(shapeGraph);
        mouseDragPending = false;
    }
    if (auto xml = shapeGraph.createXML())
        audioProcessor.setShapeGraphXmlString(xml->toString());
    
    bpm = audioProcessor.getBpm();
    
    if(scButton.getToggleState()) {
        scWarningLabel.setVisible(audioProcessor.showWarningLabel);
    }
    repaint();
}
