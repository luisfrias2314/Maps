//
//  main.cpp
//  project4building
//
//  Created by Luis Frias on 3/9/20.
//  Copyright © 2020 Luis Frias. All rights reserved.
//

#include "provided.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
using namespace std;

/*int main() {
     //for all code make sure to check for empty list, vector, or other adt 
    StreetMap* myMap = new StreetMap;
    if(myMap->load("/Users/luisfrias/Desktop/project4/mapdata.txt"))
        cout << "PASSED TEST" <<endl;
    PointToPointRouter myRoute(myMap);
    list<StreetSegment> travel;
    double distanceTraveled = 0;
    vector<DeliveryRequest> DELIVERIES;
    vector<DeliveryCommand> myCommands;
    GeoCoord DEPOT("34.0651249", "-118.4406587");
    GeoCoord FIRSTSTOP("34.0635134", "-118.4389763");
    GeoCoord SECONDSTOP("34.0675095","-118.4364274");
    DeliveryRequest firstStop("hamburger", FIRSTSTOP);
    DeliveryRequest secondStop("pizza", SECONDSTOP);
    DeliveryPlanner myDeliveryPlan(myMap);
    DELIVERIES.push_back(firstStop);
    DELIVERIES.push_back(secondStop);
    if(myDeliveryPlan.generateDeliveryPlan(DEPOT, DELIVERIES,myCommands, distanceTraveled) == DELIVERY_SUCCESS)
    {
        cout << "test good so far";
    }
    
    
} */
#include "provided.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
using namespace std;

bool loadDeliveryRequests(string deliveriesFile, GeoCoord& depot, vector<DeliveryRequest>& v);
bool parseDelivery(string line, string& lat, string& lon, string& item);

int main(int argc, char *argv[])
{
    argc = 3;
    if (argc != 3)
    {
        cout << "Usage: " << argv[0] << " mapdata.txt deliveries.txt" << endl;
        return 1;
    }

    StreetMap sm;
        
    if (!sm.load("/Users/luisfrias/Desktop/project4building/project4building/mapdata.txt"))
    {
        cout << "Unable to load map data file " << argv[1] << endl;
        return 1;
    }

    GeoCoord depot;
    vector<DeliveryRequest> deliveries;
    if (!loadDeliveryRequests("/Users/luisfrias/Desktop/project4building/project4building/deliveries.txt", depot, deliveries))
    {
        cout << "Unable to load delivery request file " << argv[2] << endl;
        return 1;
    }

    cout << "Generating route...\n\n";

    DeliveryPlanner dp(&sm);
    vector<DeliveryCommand> dcs;
    double totalMiles;
    DeliveryResult result = dp.generateDeliveryPlan(depot, deliveries, dcs, totalMiles);
    if (result == BAD_COORD)
    {
        cout << "One or more depot or delivery coordinates are invalid." << endl;
        return 1;
    }
    if (result == NO_ROUTE)
    {
        cout << "No route can be found to deliver all items." << endl;
        return 1;
    }
    cout << "Starting at the depot...\n";
    for (const auto& dc : dcs)
        cout << dc.description() << endl;
    cout << "You are back at the depot and your deliveries are done!\n";
    cout.setf(ios::fixed);
    cout.precision(2);
    cout << totalMiles << " miles travelled for all deliveries." << endl;
}

bool loadDeliveryRequests(string deliveriesFile, GeoCoord& depot, vector<DeliveryRequest>& v)
{
    ifstream inf(deliveriesFile);
    if (!inf)
        return false;
    string lat;
    string lon;
    inf >> lat >> lon;
    inf.ignore(10000, '\n');
    depot = GeoCoord(lat, lon);
    string line;
    while (getline(inf, line))
    {
        string item;
        if (parseDelivery(line, lat, lon, item))
            v.push_back(DeliveryRequest(item, GeoCoord(lat, lon)));
    }
    return true;
}

bool parseDelivery(string line, string& lat, string& lon, string& item)
{
    const size_t colon = line.find(':');
    if (colon == string::npos)
    {
        cout << "Missing colon in deliveries file line: " << line << endl;
        return false;
    }
    istringstream iss(line.substr(0, colon));
    if (!(iss >> lat >> lon))
    {
        cout << "Bad format in deliveries file line: " << line << endl;
        return false;
    }
    item = line.substr(colon + 1);
    if (item.empty())
    {
        cout << "Missing item in deliveries file line: " << line << endl;
        return false;
    }
    return true;
}
