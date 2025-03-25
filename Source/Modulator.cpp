/*
  ==============================================================================

    Modulator.cpp
    Created: 11 Mar 2025 5:41:12pm
    Author:  Oscar Eckhorst

  ==============================================================================
*/

#include "Modulator.h"
#include <juce_core/juce_core.h>

Modulator::Modulator() {
    resolution = 256;
    //modulationValues.resize(resolution, 0.0f);
    modulationValues = std::make_shared<std::vector<float>>(resolution, 1.0f); // safe default
}

void Modulator::generateModulationValues(const ShapeGraph* shapeGraph) {
    if(shapeGraph == nullptr) return;
    ///generate a modulation curve from the ShapeGraph data
    
    auto newValues = std::make_shared<std::vector<float>>(resolution, 0.0f);
    //modulationValues.clear();
    //modulationValues.resize(resolution, 0.0f);
    juce::Rectangle<float> rect1;
    juce::Rectangle<float> rect2;
    juce::Rectangle<float> rect3;
    
    float minX = shapeGraph->getLeftBound();
    float minY = shapeGraph->getTopBound();
    float normX = shapeGraph->getRightBound() - minX;
    float normY = shapeGraph->getBottomBound() - minY;
    
    float y, y0, y1, y2;
    float x, x0, x1, x2;
    
    for(int i = 0; i < shapeGraph->edges.size(); ++i) {
        auto currEdge = shapeGraph->edges[i];
        rect1 = shapeGraph->nodes[currEdge->from]->rect;
        rect2 = shapeGraph->edges[i]->rect;
        rect3 = shapeGraph->nodes[currEdge->to]->rect;
        x0 = (rect1.getCentreX() - minX) / normX;
        x1 = (rect2.getCentreX() - minX) / normX;
        x2 = (rect3.getCentreX() - minX) / normX;
        y0 = 1 - (rect1.getCentreY() - minY) / normY;
        y1 = 1 - (rect2.getCentreY() - minY) / normY;
        y2 = 1 - (rect3.getCentreY() - minY) / normY;
        
        int segmentResolution = resolution / shapeGraph->edges.size();
        
        for (float t = 0; t < segmentResolution; t=t+1) {
            float alpha = t / (segmentResolution-1);
            x = (1-alpha)*x0 + alpha*x2;
            y = y0*L0(x, x0, x1, x2) + y1*L1(x, x0, x1, x2) + y2*L2(x, x0, x1, x2);
            //normalize y
            int index = juce::jlimit(0, resolution - 1, static_cast<int>(i * segmentResolution + t));
            //modulationValues[index] = y;
            (*newValues)[index] = y;
        }
    }
    modulationValues.store(newValues);
}

float Modulator::L0(float x, float x0, float x1, float x2) {
    return (x-x1)*(x-x2) / ((x0-x1)*(x0-x2));
}

float Modulator::L1(float x, float x0, float x1, float x2) {
    return (x-x0)*(x-x2) / ((x1-x0)*(x1-x2));
}

float Modulator::L2(float x, float x0, float x1, float x2) {
    return (x-x0)*(x-x1) / ((x2-x0)*(x2-x1));
}

///get the modulated value at phase point x on the curve
float Modulator::getModulationValue(float phase)    {
    auto values = modulationValues.load();
        if (!values || values->empty())
            return 0.0f;

        int index = static_cast<int>(phase * resolution) % resolution;
        return (*values)[index];
}
