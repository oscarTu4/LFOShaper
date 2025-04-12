/*
 ==============================================================================
 
 ShapeGraph.cpp
 Created: 7 Mar 2025 5:10:41pm
 Author:  Oscar Eckhorst
 
 ==============================================================================
 */

#include "ShapeGraph.h"
#include <juce_core/juce_core.h>

ShapeGraph::ShapeGraph() {
    ///initialize nodeSize with a standard value
    ///add the corner rectangles to the graph, they won't be placed yet, since window size is unknown, hence the random values
    addNode(juce::Point<float>(0, 0), true);
    addNode(juce::Point<float>(1, 1), true);
    
    quantizeDepth = 8;
}



void ShapeGraph::addNode(juce::Point<float> position, bool isCornerNode) {
    //add new node
    float x = position.getX()-nodeSize/2;
    float y = position.getY()-nodeSize/2;
    
    ShapeNode* newNode = new ShapeNode(juce::Rectangle<float>(x, y, nodeSize, nodeSize), nodes.size());
    
    nodes.add(newNode);
    if(!isCornerNode) {
        nodes.sort(comparator);
        selectedIndex = nodes.indexOf(newNode);
        
        ///update the edges
        int nodeIndex = nodes.indexOf(newNode);
        //shouldn't nodeIndex be newFrom?
        int newFrom = nodeIndex-1;
        addEdge(newFrom);
        edges.sort(edgeComparator);
        if(nodeIndex < nodes.size()-1 && nodes.size() > 2) {
            shiftEdgeIndexes(nodeIndex);
        }
        resetEdgeCurve(newFrom);
        resetEdgeCurve(nodeIndex);
    }
    
}

void ShapeGraph::shiftEdgeIndexes(int leftAnchorNode) {
    ///move the edge to the correct position
    ShapeEdge& currEdge = *edges[leftAnchorNode];
    
    currEdge.from = leftAnchorNode;
    currEdge.to = leftAnchorNode+1;
    
    leftAnchorNode++;
    //do the same for the next edge
    if(leftAnchorNode < nodes.size()-1) {
        shiftEdgeIndexes(leftAnchorNode);
    }
}


void ShapeGraph::moveNode(int index, juce::Point<float> position) {
    ///move Node with index to parsed position, prevent overlap with other nodes
    
    int y = position.getY();
    int x = 0;
    
    if(y < topBound) y = topBound;
    if(y > bottomBound) y = bottomBound-nodeSize;
    
    //if node is corner node, only move in y direction
    if(index == nodes.size()-1) {
        nodes[index]->rect.setY(y);
        updateEdge(index-1);
    } else if (index == 0)  {
        nodes[index]->rect.setY(y);
        updateEdge(index);
    } else  {
        x = position.getX();
        int tempLeftBound = nodes[index - 1]->rect.getX();
        int tempRightBound = nodes[index + 1]->rect.getX();
        //if x and y are out of bounds, set them to the bounds
        if(x < tempLeftBound) x = tempLeftBound;
        if(x > tempRightBound) x = tempRightBound;
        
        nodes[index]->rect.setPosition(x, y);
        updateEdge(index);
        updateEdge(index-1);
    }
}


void ShapeGraph::moveNode(juce::Point<float> position) {
    ///move selected node to parsed position
    moveNode(selectedIndex, position);
}

void ShapeGraph::updateEdge(int edgeIndex) {
    ShapeEdge& edge = *edges[edgeIndex];

    float newEdgeX = calcEdgeMidX(edgeIndex)+edge.xDeviation;
    float newEdgeY = calcEdgeMidY(edgeIndex)+edge.yDeviation;

    if(newEdgeX < nodes[edge.from]->rect.getX()) newEdgeX = nodes[edge.from]->rect.getX();
    
    if(newEdgeX < nodes[edge.from]->rect.getX()) newEdgeX = nodes[edge.from]->rect.getX();
    if(newEdgeX > nodes[edge.to]->rect.getX()) newEdgeX = nodes[edge.to]->rect.getX();

    /// prevent the edge rect from exceeding topBound and bottomBound
    
    if(newEdgeY < topBound) newEdgeY = topBound;
    if(newEdgeY > bottomBound - nodeSize) newEdgeY = bottomBound - nodeSize;

    edge.rect.setPosition(newEdgeX, newEdgeY);
}


void ShapeGraph::removeNode(int nodeIndex) {
    ///remove node with index
    if(nodeIndex > 0 && nodeIndex < nodes.size()-1) {
        nodes.remove(nodeIndex);
        removeEdge(nodeIndex-1);
        shiftEdgeIndexes(nodeIndex-1);
        resetEdgeCurve(nodeIndex-1);
    }
}

void ShapeGraph::removeNode()   {
    ///remove selected node
    removeNode(selectedIndex);
}

///method to create edges between all nodes, should only be called at the beginning
void ShapeGraph::makeEdgesFromScratch()  {
    edges.clear();
    for (int i = 0; i < nodes.size()-1; ++i) {
        addEdge(i);
    }
}

void ShapeGraph::addEdge(int fromIndex) {
    ///add an edge in the middle of the two nodes with the indices from and to
    int midX = calcEdgeMidX(fromIndex);
    int midY = calcEdgeMidY(fromIndex);
    
    edges.add(new ShapeEdge(juce::Rectangle<float>(midX, midY, nodeSize, nodeSize), fromIndex, 0, 0));
}

void ShapeGraph::moveEdge(int index, juce::Point<float> position) {
    //std::cout << "Moving edge" << std::endl;
    ///move edge with index to parsed position
    /// Ensure the node stays within its allowed space
    ShapeEdge& edge = *edges[index];
    
    int from = edge.from;
    int to = edge.to;
    
    int tempLeftBound = nodes[from]->rect.getX();  // Prevent overlap
    int tempRightBound = nodes[to]->rect.getX(); // Prevent overlap
    int x = position.getX();
    int y = position.getY();
    //check if edge is out of bounds
    if (x < tempLeftBound) x = tempLeftBound;
    if (x > tempRightBound) x = tempRightBound;
    if (y < topBound) y = topBound;
    if (y > bottomBound) y = bottomBound-nodeSize;
    
    edge.xDeviation = x - calcEdgeMidX(from);
    edge.yDeviation = y - calcEdgeMidY(from);
    edge.rect.setPosition(x, y);
}

void ShapeGraph::moveEdge(juce::Point<float> position) {
    ///move selected edge to parsed position
    moveEdge(selectedIndex, position);
}

void ShapeGraph::removeEdge(int leftAnchorNode) {
    edges.remove(leftAnchorNode);
    shiftEdgeIndexes(leftAnchorNode);
}

void ShapeGraph::resetEdgeCurve(int leftAnchorNode) {
    ///reset edge with index leftAnchorNode
    ShapeEdge& edge = *edges[leftAnchorNode];
    int midX = calcEdgeMidX(edge.from);
    int midY = calcEdgeMidY(edge.from);
    edge.rect.setPosition(midX, midY);
    edge.xDeviation = 0;
    edge.yDeviation = 0;
}

float ShapeGraph::calcEdgeMidX(int from)   {
    return (nodes[from]->rect.getCentreX() + nodes[from+1]->rect.getCentreX() - nodeSize) / 2;
}

float ShapeGraph::calcEdgeMidY(int from)   {
    return (nodes[from]->rect.getCentreY() + nodes[from+1]->rect.getCentreY() - nodeSize) / 2;
}


std::pair<int, juce::Rectangle<float>*> ShapeGraph::containsPointOnNode(juce::Point<float> point)    {
    ///check if the point is inside a node
    ///return the index of the node and a pointer to the node
    ///if no node contains the point, return -1 and a nullptr
    for (int i = 0; i < nodes.size(); ++i) {
        if(nodes[i]->rect.contains(point)) {
            return {i, &nodes[i]->rect};
        }
    }
    
    return {-1, nullptr};
}

std::pair<int, juce::Rectangle<float>*> ShapeGraph::containsPointOnEdge(juce::Point<float> point)    {
    ///check if the point is inside an edge
    ///return the index of the edge and a pointer to the edge
    ///if no edge contains the point, return -1 and a nullptr
    for (int i = 0; i < edges.size(); ++i) {
        if(edges[i]->rect.contains(point)) {
            return {i, &edges[i]->rect};
        }
    }
    
    return {-1, nullptr};
}

void ShapeGraph::quantizeNode (int index)    {
    //if no rectangle is being edited, return, should never happen
    if(index < 0) return;
    
    ShapeNode& node = *nodes[index];
    float nodeX = node.rect.getX();
    
    auto closestStep = std::min_element(widthQuantizationSteps.begin(), widthQuantizationSteps.end(), [nodeX](float a, float b) {
        return std::abs(nodeX - a) < std::abs(nodeX - b);
    });
    
    if(closestStep != widthQuantizationSteps.end()) {
        node.rect.setX(*closestStep-nodeSize/2);
    }
    
    //update edges
    updateEdge(index);
    updateEdge(index-1);
}

void ShapeGraph::quantizeNode() {
    quantizeNode(selectedIndex);
}

void ShapeGraph::resizeNodeLayout()  {
    ///called when window size changes
    ///update the position of the corner nodes, later on the other nodes will be updated, priority very low
    nodes[0]->rect.setPosition(leftBound-nodeSize/2, bottomBound-nodeSize);
    nodes[nodes.size()-1]->rect.setPosition(rightBound-nodeSize/2, topBound);
    //recreate all edges
    for (int i = 0; i < nodes.size()-1; ++i) {
        addEdge(i);
    }
    
    //clear previous quantization steps
    widthQuantizationSteps.clear();
    
    //create quantization steps
    for(int i = 0; i <= quantizeDepth; ++i) {
        widthQuantizationSteps.add(leftBound + (i * width)/quantizeDepth);
    }
}

void ShapeGraph::paint(juce::Graphics& g)   {
    ///Iterate through all nodes and edges and draw them
    g.setColour (juce::Colours::orange);
    for (int i = 0; i < nodes.size(); i++) {
        g.fillRect(nodes[i]->rect);
    }
    //draw lines between rectangles and add curve point
    path.clear();
    for(int i = 0; i < edges.size(); ++i) {
        //draw edges
        path.startNewSubPath(nodes[edges[i]->from]->rect.getCentre().toFloat());
        path.quadraticTo(edges[i]->rect.getCentre().toFloat(), nodes[edges[i]->to]->rect.getCentre().toFloat());
        g.strokePath(path, juce::PathStrokeType(2.0f));
        g.drawEllipse(edges[i]->rect.toFloat(), 2.0f);
    }
    
    //draw lines for quantization steps
    g.setColour(juce::Colours::orange.withAlpha(0.5f));
    for(auto step : widthQuantizationSteps) {
        path.clear();
        path.startNewSubPath(step, topBound);
        path.lineTo(step, bottomBound);
        g.strokePath(path, juce::PathStrokeType(1.0f));
    }
}

void ShapeGraph::setQuantizeDepth(int depth)    {
    quantizeDepth = depth;
}

void ShapeGraph::selectNode(int index)   {
    selectedIndex = index;
    selectionType = SelectionType::Node;
}

void ShapeGraph::selectEdge(int index)   {
    selectedIndex = index;
    selectionType = SelectionType::Edge;
}

void ShapeGraph::clearSelection()    {
    selectedIndex = -1;
    selectionType = SelectionType::none;
}

int ShapeGraph::getSelectedIndex()  {
    return selectedIndex;
}

void ShapeGraph::setHeight(int frameHeight)  {
    height = frameHeight;
}
int ShapeGraph::getHeight() {
    return height;
}
void ShapeGraph::setWidth(int frameWidth)    {
    width = frameWidth;
}

int ShapeGraph::getWidth() {
    return width;
}

int ShapeGraph::getNodeSize()   {
    return nodeSize;
}

void ShapeGraph::setLeftBound(int left)  {
    leftBound = left;
}

int ShapeGraph::getLeftBound()  const {
    return leftBound;
}

void ShapeGraph::setRightBound(int right)    {
    rightBound = right;
}

int ShapeGraph::getRightBound() const {
    return rightBound;
}

void ShapeGraph::setTopBound(int top)    {
    topBound = top;
}

int ShapeGraph::getTopBound() const {
    return topBound;
}

void ShapeGraph::setBottomBound(int bottom)  {
    bottomBound = bottom;
}

int ShapeGraph::getBottomBound() const {
    return bottomBound;
}

std::unique_ptr<juce::XmlElement> ShapeGraph::createXML() {
    auto xml = std::make_unique<juce::XmlElement>("ShapeGraph");
    
    for (const auto& node : nodes)
    {
        auto* nodeXml = xml->createNewChildElement("Node");
        nodeXml->setAttribute("x", node->rect.getX());
        nodeXml->setAttribute("y", node->rect.getY());
    }
    
    for (const auto& edge : edges)
    {
        auto* edgeXml = xml->createNewChildElement("Edge");
        edgeXml->setAttribute("from", edge->from);
        edgeXml->setAttribute("to", edge->to);
        edgeXml->setAttribute("xDeviation", edge->xDeviation);
        edgeXml->setAttribute("yDeviation", edge->yDeviation);
    }
    
    return xml;
}

void ShapeGraph::loadXML(juce::XmlElement& xml)
{
    nodes.clear();
    edges.clear();
    
    // Load all nodes
    for (auto* child : xml.getChildIterator())
    {
        if (child->hasTagName("Node"))
        {
            float x = child->getDoubleAttribute("x");
            float y = child->getDoubleAttribute("y");
            addNode({ x + nodeSize / 2.0f, y + nodeSize / 2.0f }, true);
        }
    }
    
    
    // Set corner node positions correctly (you already reposition in resizeNodeLayout anyway)
    nodes.sort(comparator);
    
    // Load all edges
    for (auto* child : xml.getChildIterator())
    {
        if (child->hasTagName("Edge"))
        {
            int from = child->getIntAttribute("from");
            //int to = child->getIntAttribute("to");
            float xDev = child->getDoubleAttribute("xDeviation");
            float yDev = child->getDoubleAttribute("yDeviation");
            
            int midX = calcEdgeMidX(from);
            int midY = calcEdgeMidY(from);
            
            auto* edge = new ShapeEdge({ midX + xDev, midY + yDev, nodeSize, nodeSize}, from, xDev, yDev);
            edges.add(edge);
        }
    }

}


