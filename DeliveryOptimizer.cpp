#include "provided.h"
#include <vector>
using namespace std;

class DeliveryOptimizerImpl
{
public:
    DeliveryOptimizerImpl(const StreetMap* sm);
    ~DeliveryOptimizerImpl();
    void optimizeDeliveryOrder(
        const GeoCoord& depot,
        vector<DeliveryRequest>& deliveries,
        double& oldCrowDistance,
        double& newCrowDistance) const;
private:
    const StreetMap* myStreetMap;
};

DeliveryOptimizerImpl::DeliveryOptimizerImpl(const StreetMap* sm)
{
    myStreetMap = sm;
}

DeliveryOptimizerImpl::~DeliveryOptimizerImpl()
{
}

void DeliveryOptimizerImpl::optimizeDeliveryOrder(
    const GeoCoord& depot,
    vector<DeliveryRequest>& deliveries,
    double& oldCrowDistance,
    double& newCrowDistance) const
{
    vector<DeliveryRequest>  oldDelivery;
    oldCrowDistance = 0;
    newCrowDistance = 0;
    oldDelivery = deliveries;
    if(deliveries.size() >0 )
    {
        oldCrowDistance = distanceEarthMiles(depot, deliveries.front().location);
        for(int i = 0; i < deliveries.size()-1; i++)
        {
            oldCrowDistance += distanceEarthMiles(deliveries[i].location, deliveries[i+1].location);
        }
        oldCrowDistance += distanceEarthMiles(deliveries.back().location, depot);
        for(int i = 0; i < deliveries.size();i++) //calculate oldcrowDistnace based on distance from depot
        {
            for(int k = i+1; k < deliveries.size();k++)
            {
                if((distanceEarthMiles(depot, deliveries[i].location)) > (distanceEarthMiles(depot, deliveries[k].location)))
                {
                    DeliveryRequest tempt = deliveries[i];
                    deliveries[i] = deliveries[k];
                    deliveries[k] = tempt;
                }
            }
        }
        for(int i = 0; i < deliveries.size()-1;i++) //reorder deliveries that are close to each other
        {
            double distanceCal =distanceEarthMiles(deliveries[i].location, deliveries[i+1].location);
            for(int k = i+1; k < deliveries.size()-1;k++)
            {
                if(distanceEarthMiles(deliveries[i].location,deliveries[k].location) < distanceCal)
                {
                    DeliveryRequest tempt = deliveries[i];
                    deliveries[i] = deliveries[k];
                    deliveries[k] = tempt;
                }
                
            }
        }
        for(int i = 0; i < deliveries.size()-1; i++) //optomize based on distance from depot
        {
            newCrowDistance += distanceEarthMiles(deliveries[i].location, deliveries[i+1].location);
        }
        if(newCrowDistance > oldCrowDistance) //if optimizing failed then revert to previous order of delivery
        {
            deliveries = oldDelivery;
            newCrowDistance = oldCrowDistance;
        }
    }
}

//******************** DeliveryOptimizer functions ****************************

// These functions simply delegate to DeliveryOptimizerImpl's functions.
// You probably don't want to change any of this code.

DeliveryOptimizer::DeliveryOptimizer(const StreetMap* sm)
{
    m_impl = new DeliveryOptimizerImpl(sm);
}

DeliveryOptimizer::~DeliveryOptimizer()
{
    delete m_impl;
}

void DeliveryOptimizer::optimizeDeliveryOrder(
        const GeoCoord& depot,
        vector<DeliveryRequest>& deliveries,
        double& oldCrowDistance,
        double& newCrowDistance) const
{
    return m_impl->optimizeDeliveryOrder(depot, deliveries, oldCrowDistance, newCrowDistance);
}
