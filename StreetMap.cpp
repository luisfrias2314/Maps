#include "provided.h"
#include <string>
#include <vector>
#include <fstream>
#include <functional>
#include "ExpandableHashMap.h"
using namespace std;

unsigned int hasher(const GeoCoord& g)
{
    return std::hash<string>()(g.latitudeText + g.longitudeText);
}

class StreetMapImpl
{
public:
    StreetMapImpl();
    ~StreetMapImpl();
    bool load(string mapFile);
    bool getSegmentsThatStartWith(const GeoCoord& gc, vector<StreetSegment>& segs) const;
private:
    ExpandableHashMap<GeoCoord, vector<StreetSegment>> myMap;
};

StreetMapImpl::StreetMapImpl()
{
}

StreetMapImpl::~StreetMapImpl()
{
}

bool StreetMapImpl::load(string mapFile)
{
    ifstream inf(mapFile);
    if(!inf) //invalid file
        return false;
    string line;
    while (getline(inf, line)) //iterate through every streetname line
    {
        
        string streetName = line; //streetname
        int amt;
        inf >> amt; //get number of streets
        inf.ignore(10000, '\n');;
        for(int i = 0; i < amt;i++) //iterate through every segment line
        {
            getline(inf, line);
            istringstream sec(line);
            string startLat;
            string startLong;
            string endLat;
            string endLong;
            if((sec >> startLat >> startLong >> endLat >> endLong)) //get start and end segment from line
            {
                GeoCoord start(startLat,startLong);
                GeoCoord end(endLat,endLong);
                StreetSegment front(start, end, streetName);
                vector<StreetSegment>* tempt;
                tempt = myMap.find(start);
                StreetSegment record(start, end, streetName);//push forward version of segment to the map of geocords 
                if(tempt != nullptr)
                {
                    (*tempt).push_back(record);
                }
                else
                {
                    vector<StreetSegment> temptRecord;
                    temptRecord.push_back(record);
                    myMap.associate(start, temptRecord);
                }
                tempt = myMap.find(end);
                StreetSegment record1(end, start, streetName); //push back version of segment to the map of geocords
                if(tempt != nullptr)
                {
                    (*tempt).push_back(record1);
                }
                else
                {
                    vector<StreetSegment> temptRecord;
                    temptRecord.push_back(record1);
                    myMap.associate(end, temptRecord);
                }
            }
        }
    }
    return true;
}

bool StreetMapImpl::getSegmentsThatStartWith(const GeoCoord& gc, vector<StreetSegment>& segs) const
{
    const vector<StreetSegment>* tempt;
    tempt = myMap.find(gc); //get all segments that start with geocord
    if(tempt != nullptr)
    {
        segs = (*tempt);
        return true;
    }
    return false; //return false if geocord is not in the map
}

//******************** StreetMap functions ************************************

// These functions simply delegate to StreetMapImpl's functions.
// You probably don't want to change any of this code.

StreetMap::StreetMap()
{
    m_impl = new StreetMapImpl;
}

StreetMap::~StreetMap()
{
    delete m_impl;
}

bool StreetMap::load(string mapFile)
{
    return m_impl->load(mapFile);
}

bool StreetMap::getSegmentsThatStartWith(const GeoCoord& gc, vector<StreetSegment>& segs) const
{
   return m_impl->getSegmentsThatStartWith(gc, segs);
}

