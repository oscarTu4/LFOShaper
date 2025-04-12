/*
  ==============================================================================

    Modulator.h
    Created: 11 Mar 2025 5:41:12pm
    Author:  Oscar Eckhorst

  ==============================================================================
*/

#pragma once
#include "juce_core/juce_core.h"
#include "juce_gui_basics/juce_gui_basics.h"
#include "ShapeGraph.h"
#include "juce_dsp/juce_dsp.h"


class Modulator {
    
private:
    
    int resolution;
    std::shared_ptr<std::vector<float>> modulationValues;
    
public:
    
    Modulator();
    
    void generateModulationValues(const ShapeGraph* shapeGraph);
    float getModulationValue(float phase);
    float getLastModulationValue();
};
