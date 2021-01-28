#include "provided.h"
#include <vector>
using namespace std;

class DeliveryPlannerImpl
{
public:
    DeliveryPlannerImpl(const StreetMap* sm);
    ~DeliveryPlannerImpl();
    DeliveryResult generateDeliveryPlan(
        const GeoCoord& depot,
        const vector<DeliveryRequest>& deliveries,
        vector<DeliveryCommand>& commands,
        double& totalDistanceTravelled) const;
private:
    DeliveryOptimizer myDeliveryOptimizer;
    PointToPointRouter RoutingPerDelivery;
    const StreetMap* myStreetMap;
    void deliveryCommandPerRoute(list<StreetSegment>routing,vector<DeliveryCommand>& commands,DeliveryRequest desiredDelivery, bool backToDepot) const;
    string dirNumToString(double angle) const;
};

DeliveryPlannerImpl::DeliveryPlannerImpl(const StreetMap* sm): myDeliveryOptimizer(sm), RoutingPerDelivery(sm)
{
    myStreetMap = sm;
    
}

DeliveryPlannerImpl::~DeliveryPlannerImpl()
{
}
string DeliveryPlannerImpl::dirNumToString(double angle) const //generate direction for angle parameter 
{
        if(angle <22.5 && angle >= 0)
        {
            return "east";
        }
        else if(angle < 67.5 && angle >= 22.5)
        {
            return "northeast";
            
        }
        else if(angle <112.5 && angle >= 67.5)
        {
            return "north";
        }
        else if (angle <157.5 && angle >= 112.5)
        {
            return "northwest";
        }
        else if (angle <202.5 && angle >= 157.5)
        {
            return "west";
        }
        else if (angle <247.5 && angle >= 202.5)
        {
            return "southwest";
        }
        else if (angle <292.5 && angle >= 247.5)
        {
            return "south";
        }
        else if (angle <337.5 && angle >= 292.5)
        {
            return "southeast";
        }
        else
        {
            return "east";
        }
}
void DeliveryPlannerImpl::deliveryCommandPerRoute(list<StreetSegment> routing, vector<DeliveryCommand> &commands, DeliveryRequest desiredDelivery, bool backToDepot) const
{
    string streetName;
    bool newDirFlag = true;
    double dirDouble;
    string dirString;
    DeliveryCommand myCommand;
    if(routing.size() == 0) //checks if depot is delivery
    {
        if(backToDepot != true)
        {
            myCommand.initAsDeliverCommand(desiredDelivery.item);
            commands.push_back(myCommand);
        }
        return;
    }
    for (std::list<StreetSegment>::iterator it=routing.begin(); it != routing.end(); ++it) //iterates every segment in delivery route
    {
        if(newDirFlag == true) //initializes proceed command
        {
            streetName = it->name;
            newDirFlag = false;
            dirDouble = angleOfLine(*it);
            dirString = dirNumToString(dirDouble);
            myCommand.initAsProceedCommand(dirString, streetName, 0);
        }
        if(it->name == streetName) //adds distance to proceed command
        {
            myCommand.increaseDistance(distanceEarthMiles(it->start, it->end));
        }
        else
        {
            //if a change in street name is made proceed command is pushed to list of command
            commands.push_back(myCommand);
            StreetSegment current = (*it);
            --it;
            double AngleBetween = angleBetween2Lines((*it),current); //check for a turn
            newDirFlag = true; //flag to create a new proceed object
            
            if(AngleBetween >= 1 && AngleBetween < 180) //check for turn and push to list of commands
            {
                myCommand.initAsTurnCommand("left", current.name);
                commands.push_back(myCommand);
                
            }
            else if(AngleBetween >= 180 &&AngleBetween < 359)
            {
                myCommand.initAsTurnCommand("right", current.name);
                commands.push_back(myCommand);
            }
        }
        if((*it) == routing.back()) //make delivery if needed
        {
            commands.push_back(myCommand);
            if(backToDepot != true)
            {
                myCommand.initAsDeliverCommand(desiredDelivery.item);
                commands.push_back(myCommand);
            }
            return;

        }
    }
    
}

DeliveryResult DeliveryPlannerImpl::generateDeliveryPlan(
    const GeoCoord& depot,
    const vector<DeliveryRequest>& deliveries,
    vector<DeliveryCommand>& commands,
    double& totalDistanceTravelled) const
{
    double oldCrowDistance;
    double newCrowDistance;
    double shortDistanceTrav;
    totalDistanceTravelled = 0;
    list<StreetSegment> routingPerDelivery;
    vector<DeliveryRequest> reOderedDeliveries = deliveries;
    DeliveryResult resultofDelivery;
    if(deliveries.size() == 0)
       {
           return NO_ROUTE;
       }
   
    //optimize deliveries with optimizer class
    myDeliveryOptimizer.optimizeDeliveryOrder(depot, reOderedDeliveries, oldCrowDistance, newCrowDistance);
    
    //route between depot and first delivery
    resultofDelivery = RoutingPerDelivery.generatePointToPointRoute(depot, reOderedDeliveries.front().location,routingPerDelivery, shortDistanceTrav);
    
    if(resultofDelivery == BAD_COORD || resultofDelivery == NO_ROUTE)
    {
        return resultofDelivery;
    }
    else
    {
        deliveryCommandPerRoute(routingPerDelivery, commands, reOderedDeliveries.front(),false);
        totalDistanceTravelled += shortDistanceTrav;
    }
    for(int i = 0; i < deliveries.size()-1;i++)
    {
        //route between two deliveries
        list<StreetSegment> routingPerDelivery;
        resultofDelivery = RoutingPerDelivery.generatePointToPointRoute(reOderedDeliveries[i].location, reOderedDeliveries[i+1].location,routingPerDelivery, shortDistanceTrav);
        if(resultofDelivery == BAD_COORD || resultofDelivery == NO_ROUTE) //check if route valid
        {
            return resultofDelivery;
        }
        else
        {
            deliveryCommandPerRoute(routingPerDelivery, commands, reOderedDeliveries[i+1],false);
            totalDistanceTravelled += shortDistanceTrav; //add route distance to overall distance
            
        }
    }
    
    //get route for getting back to depot
    list<StreetSegment> routingPerDeliveryLast;
    resultofDelivery = RoutingPerDelivery.generatePointToPointRoute(reOderedDeliveries.back().location, depot ,routingPerDeliveryLast, shortDistanceTrav);
    
    if(resultofDelivery == BAD_COORD || resultofDelivery == NO_ROUTE)
    {
        return resultofDelivery;
    }
    else
    {
        //if valid generate commands for route and update command list
        deliveryCommandPerRoute(routingPerDeliveryLast, commands, reOderedDeliveries.back(),true);
        totalDistanceTravelled += shortDistanceTrav;
        
    }
    
    return DELIVERY_SUCCESS;
}

//******************** DeliveryPlanner functions ******************************

// These functions simply delegate to DeliveryPlannerImpl's functions.
// You probably don't want to change any of this code.

DeliveryPlanner::DeliveryPlanner(const StreetMap* sm)
{
    m_impl = new DeliveryPlannerImpl(sm);
}

DeliveryPlanner::~DeliveryPlanner()
{
    delete m_impl;
}

DeliveryResult DeliveryPlanner::generateDeliveryPlan(
    const GeoCoord& depot,
    const vector<DeliveryRequest>& deliveries,
    vector<DeliveryCommand>& commands,
    double& totalDistanceTravelled) const
{
    return m_impl->generateDeliveryPlan(depot, deliveries, commands, totalDistanceTravelled);
}
