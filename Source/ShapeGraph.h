/*
 ==============================================================================
 
 ShapeGraph.h
 Created: 7 Mar 2025 4:20:02pm
 Author:  Oscar Eckhorst
 
 ==============================================================================
 */

#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_data_structures/juce_data_structures.h> 

struct ShapeNode {
    juce::Rectangle<float> rect;
    
    ShapeNode(juce::Rectangle<float> rect, int index) : rect(rect) {}
};

struct ShapeEdge {
    juce::Rectangle<float> rect;
    int from;   //from also serves as index
    int to;
    float xDeviation;
    float yDeviation;
    
    ShapeEdge(juce::Rectangle<float> rect, int from, float x, float y) : rect(rect), from(from), to(from+1), xDeviation(x), yDeviation(y) {}
};

struct NodeComparator {
    static int compareElements(ShapeNode* node1, ShapeNode* node2) {
        return node1->rect.getX() - node2->rect.getX();
    }
};

struct EdgeComparator {
    static int compareElements(ShapeEdge* edge1, ShapeEdge* edge2) {
        return edge1->from - edge2->from;
    }
};


class ShapeGraph {
    
private:
    
    juce::Path path;
    
    const NodeComparator comparator;
    const EdgeComparator edgeComparator;
    
    int height;
    int width;
    const float nodeSize = 10;
    
    int leftBound;
    int rightBound;
    int topBound;
    int bottomBound;
    
    int selectedIndex = -1;
    
    //quantization variables
    int quantizeDepth;
    juce::Array<int> widthQuantizationSteps;
    
    void updateEdgesAroundNode(int nodeIndex);
    void updateEdge(int nodeIndex);
    void makeEdgesFromScratch();
    void addEdge(int from);
    void shiftEdgeIndexes(int leftAnchorNode);
    void removeEdge(int leftAnchorNode);
    float calcEdgeMidX(int from);
    float calcEdgeMidY(int from);
    
public:
    
    juce::OwnedArray<ShapeNode> nodes;
    juce::OwnedArray<ShapeEdge> edges;
    
    enum class SelectionType {Node, Edge, none};
    
    SelectionType selectionType = SelectionType::none;
    
    ShapeGraph();
    
    void addNode(juce::Point<float> position, bool isCornerNode);
    void moveNode(int index, juce::Point<float> position);
    void moveNode(juce::Point<float> position);
    void removeNode(int index);
    void removeNode();
    
    void quantizeNode(int index);
    void quantizeNode();
    
    void moveEdge(int index, juce::Point<float> position);
    void moveEdge(juce::Point<float> position);
    void resetEdgeCurve(int leftAnchorNode);
    
    std::pair<int, juce::Rectangle<float>*> containsPointOnNode(juce::Point<float> point);
    std::pair<int, juce::Rectangle<float>*> containsPointOnEdge(juce::Point<float> point);
    
    void paint(juce::Graphics& g);
    void resizeNodeLayout();
    void selectNode(int index);
    void selectEdge(int index);
    void clearSelection();
    int getSelectedIndex();
    
    void setHeight(int height);
    int getHeight();
    void setWidth(int width);
    int getWidth();
    int getNodeSize();
    void setQuantizeDepth(int depth);
    
    void setLeftBound(int left);
    int getLeftBound() const;
    void setRightBound(int right);
    int getRightBound() const;
    void setTopBound(int top);
    int getTopBound() const;
    void setBottomBound(int bottom);
    int getBottomBound() const;
    
    std::unique_ptr<juce::XmlElement> createXML();
    void loadXML(juce::XmlElement& xml);
    
};


