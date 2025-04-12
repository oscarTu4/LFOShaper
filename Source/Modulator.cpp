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
    resolution = 2048;
    modulationValues = std::make_shared<std::vector<float>>(resolution, 1.0f); // safe default
}

void Modulator::generateModulationValues(const ShapeGraph* shapeGraph) {
    if (shapeGraph == nullptr || shapeGraph->edges.size() == 0)
        return;

    auto newValues = std::make_shared<std::vector<float>>(resolution, 0.0f);

    float minX = shapeGraph->getLeftBound();
    float minY = shapeGraph->getTopBound();
    float normX = shapeGraph->getRightBound() - minX;
    float normY = shapeGraph->getBottomBound() - minY;

    // Precompute all segments (x0, x1, x2, y0, y1, y2)
    struct Segment {
        float x0, x1, x2;
        float y0, y1, y2;
    };
    std::vector<Segment> segments;

    for (int i = 0; i < shapeGraph->edges.size(); ++i) {
        auto* edge = shapeGraph->edges[i];
        auto rect1 = shapeGraph->nodes[edge->from]->rect;
        auto rect2 = edge->rect;
        auto rect3 = shapeGraph->nodes[edge->to]->rect;

        float x0 = (rect1.getCentreX() - minX) / normX;
        float x1 = (rect2.getCentreX() - minX) / normX;
        float x2 = (rect3.getCentreX() - minX) / normX;
        float y0 = 1.0f - (rect1.getCentreY() - minY) / normY;
        float y1 = 1.0f - (rect2.getCentreY() - minY) / normY;
        float y2 = 1.0f - (rect3.getCentreY() - minY) / normY;

        // avoid division-by-zero in alpha later
        if (std::abs(x2 - x0) < 1e-5f)
            x2 = x0 + 1e-5f;

        segments.push_back({ x0, x1, x2, y0, y1, y2 });
    }

    for (int i = 0; i < resolution; ++i) {
        float phase = (float)i / (resolution - 1);

        // find the segment that contains this phase
        for (int s = 0; s < segments.size(); ++s) {
            const auto& seg = segments[s];
            if (phase >= seg.x0 && phase <= seg.x2) {
                float alpha = (phase - seg.x0) / (seg.x2 - seg.x0);

                float y = (1 - alpha) * (1 - alpha) * seg.y0
                        + 2 * (1 - alpha) * alpha * seg.y1
                        + alpha * alpha * seg.y2;

                newValues->at(i) = juce::jlimit(0.0f, 1.0f, y);
                break;
            }
        }
    }

    std::atomic_store(&modulationValues, newValues);
}

///get the modulated value at phase point x on the curve

float Modulator::getModulationValue(float phase)
{
    auto values = std::atomic_load(&modulationValues);
    if (!values || values->empty())
        return 1.0f;

    int index = juce::jlimit(0, resolution - 1, static_cast<int>(std::fmod(phase, 1.0f) * resolution));
    return (*values)[index];
}

float Modulator::getLastModulationValue()   {
    auto values = std::atomic_load(&modulationValues);
    if (!values || values->empty())
            return 1.0f;
    return (*values)[resolution - 1];
}
