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
    //std::vector<float> modulationValues;
    std::atomic<std::shared_ptr<std::vector<float>>> modulationValues;
    
    float L0(float x, float x0, float x1, float x2);
    float L1(float x, float x0, float x1, float x2);
    float L2(float x, float x0, float x1, float x2);
    
public:
    
    Modulator();
    
    void generateModulationValues(const ShapeGraph* shapeGraph);
    float getModulationValue(float phase);
};
