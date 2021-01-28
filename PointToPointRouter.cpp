#include "provided.h"
#include <map>
#include "ExpandableHashMap.h"
#include <stack>
using namespace std;

class PointToPointRouterImpl
{
public:
    PointToPointRouterImpl(const StreetMap* sm);
    ~PointToPointRouterImpl();
    DeliveryResult generatePointToPointRoute(
        const GeoCoord& start,
        const GeoCoord& end,
        list<StreetSegment>& route,
        double& totalDistanceTravelled) const;
private:
    const StreetMap* myStreetMap;
    struct Node
    {
        bool m_visted  = false; //
        double m_MilestoDes = 0;
        double m_milesTraveledSoFar = 0;
        GeoCoord m_Coord;
        Node* parentNode = nullptr;
    };
    ExpandableHashMap<GeoCoord, Node*> uniqueGeoCoords; //change to regular nodes? //each pointer points to a new node
};

PointToPointRouterImpl::PointToPointRouterImpl(const StreetMap* sm)
{
    myStreetMap = sm;
}

PointToPointRouterImpl::~PointToPointRouterImpl()
{
}

DeliveryResult PointToPointRouterImpl::generatePointToPointRoute(
        const GeoCoord& start,
        const GeoCoord& end,
        list<StreetSegment>& route,
        double& totalDistanceTravelled) const
{
    vector<StreetSegment> startSegVec;
    vector<StreetSegment> endSegVec;
    //check if start and end segments are valid
    if(!(myStreetMap->getSegmentsThatStartWith(start, startSegVec)) || !(myStreetMap->getSegmentsThatStartWith(end, endSegVec)))
    {
        return BAD_COORD;
    }
    if(start == end) //if end and start coords are the same delivery success
    {
        totalDistanceTravelled = 0;
        return DELIVERY_SUCCESS;
    }
    //create a dynamically allocated node for start coord and push to map and list
    Node* startingNode = new Node;
    Node* currentNode = startingNode;
    currentNode->m_Coord = start;
    currentNode->m_milesTraveledSoFar = 0;
    currentNode->m_MilestoDes = distanceEarthMiles(start, end);
    currentNode->parentNode = nullptr;
    list<Node*> nodesToTest;
    nodesToTest.push_back(startingNode);
    list<Node*> cleanup;
    cleanup.push_back(startingNode);
    const_cast<PointToPointRouterImpl*>(this)->uniqueGeoCoords.associate(currentNode->m_Coord, startingNode);
    
    //iterate until list empty
    while(!nodesToTest.empty())
    {
        //sort list based off global goal (heurisitc)
        nodesToTest.sort([](const Node* first, const Node* second){return first->m_MilestoDes < second->m_MilestoDes;});
        while(!nodesToTest.empty() && nodesToTest.front()->m_visted) //remove nodes that have been visted
        {
            nodesToTest.pop_front();
        }
        if(nodesToTest.empty())
            break;
        
        //get all the "neighboring" segments of the node(geocoord)
        currentNode = nodesToTest.front();
        currentNode->m_visted = true;
        vector<StreetSegment> neighborSegs;
        myStreetMap->getSegmentsThatStartWith(currentNode->m_Coord, neighborSegs);
        
        for(int i = 0; i <neighborSegs.size();i++) //iterate through every "neighboring segment
        {
            Node** neighborNodes = nullptr;
            //check if neighbor is already in the Map
            neighborNodes = const_cast<PointToPointRouterImpl*>(this)->uniqueGeoCoords.find(neighborSegs[i].end);
            if(neighborNodes != nullptr)
            {
                if((*neighborNodes)->m_visted == false && (*neighborNodes)->m_Coord != end) //test node if it hasnt been visited
                {
                    nodesToTest.push_back((*neighborNodes));
                }
                
                //calcuate if nodes current "parent" geocoord" has a better heuristic than if current node was its parent
                double potentialMilesTraveled = distanceEarthMiles(currentNode->m_Coord, (*neighborNodes)->m_Coord) + currentNode->m_milesTraveledSoFar;
                if(potentialMilesTraveled < (*neighborNodes)->m_milesTraveledSoFar)
                {
                    (*neighborNodes)->parentNode = currentNode; //make current node the parent
                    (*neighborNodes)->m_milesTraveledSoFar = potentialMilesTraveled; //update with better heursitic
                    (*neighborNodes)->m_MilestoDes = potentialMilesTraveled + distanceEarthMiles((*neighborNodes)->m_Coord, end);
                }
            }
            else
            {
                //if neighboring node undiscovered then add to list and map
                Node* tempt = new Node;
                tempt->m_Coord = neighborSegs[i].end;
                tempt->m_milesTraveledSoFar = 0;
                tempt->parentNode = currentNode;
                tempt->m_milesTraveledSoFar = distanceEarthMiles(tempt->parentNode->m_Coord,tempt->m_Coord) + tempt->parentNode->m_milesTraveledSoFar;
                tempt->m_MilestoDes = distanceEarthMiles(tempt->m_Coord, end)+tempt->m_milesTraveledSoFar;
                if(tempt->m_Coord != end)
                {
                    nodesToTest.push_back(tempt);
                }
                cleanup.push_back(tempt);
                const_cast<PointToPointRouterImpl*>(this)->uniqueGeoCoords.associate(tempt->m_Coord, tempt);
            }
        
        }
    }
    stack<StreetSegment> myStack;
    Node** neighborNodes = nullptr;
    neighborNodes = const_cast<PointToPointRouterImpl*>(this)->uniqueGeoCoords.find(end);
    DeliveryResult resultsOfDelivery;
    if((*neighborNodes) != nullptr) //find the parent of end coord and iterate backwords till you find start node
    {
        totalDistanceTravelled = (*neighborNodes)->m_MilestoDes;
        while((*neighborNodes)->parentNode != nullptr)
        {
            vector<StreetSegment> temptSegValues;
            myStreetMap->getSegmentsThatStartWith((*neighborNodes)->parentNode->m_Coord, temptSegValues);
            for(int i = 0; i < temptSegValues.size(); i++)
            {
                if(temptSegValues[i].end == (*neighborNodes)->m_Coord)
                {
                    myStack.push(temptSegValues[i]); //push the nodes iterated backwards onto the stack
                }
            }
            (*neighborNodes) = (*neighborNodes)->parentNode;
        }
        while(myStack.size() != 0)
        {
            route.push_back(myStack.top()); //push the stack of segments into the route of segments
            myStack.pop();
        }
        resultsOfDelivery = DELIVERY_SUCCESS;
    }
    else
    {
        resultsOfDelivery = NO_ROUTE;
    }
        list<Node*>::iterator it; //delete all dynamically allocated data 
        for(it = cleanup.begin(); it != cleanup.end();it++)
        {
            delete *it;
        }
        cleanup.clear();
        const_cast<PointToPointRouterImpl*>(this)->uniqueGeoCoords.reset();
    return resultsOfDelivery;
    //draw its path--iterate from last nodes parents all the way back to the start parent whose will eqyal nullptr
}

//******************** PointToPointRouter functions ***************************

// These functions simply delegate to PointToPointRouterImpl's functions.
// You probably don't want to change any of this code.

PointToPointRouter::PointToPointRouter(const StreetMap* sm)
{
    m_impl = new PointToPointRouterImpl(sm);
}

PointToPointRouter::~PointToPointRouter()
{
    delete m_impl;
}

DeliveryResult PointToPointRouter::generatePointToPointRoute(
        const GeoCoord& start,
        const GeoCoord& end,
        list<StreetSegment>& route,
        double& totalDistanceTravelled) const
{
    return m_impl->generatePointToPointRoute(start, end, route, totalDistanceTravelled);
}
