#ifndef EXPANDABLEHASHMAP_INCLUDED //DO I HAVE TO INCLUDED THIS HEADER ???
#define EXPANDABLEHASHMAP_INCLUDED //MAKE SURE TO COMMENT CODE
// ExpandableHashMap.h

// Skeleton for the ExpandableHashMap class template.  You must implement the first six
// member functions.
template<typename KeyType, typename ValueType>
class ExpandableHashMap
{
public:
    ExpandableHashMap(double maximumLoadFactor = 0.5);
    ~ExpandableHashMap();
    void reset();
    int size() const;
    void associate(const KeyType& key, const ValueType& value);

      // for a map that can't be modified, return a pointer to const ValueType
    const ValueType* find(const KeyType& key) const;

      // for a modifiable map, return a pointer to modifiable ValueType
    ValueType* find(const KeyType& key)
    {
        return const_cast<ValueType*>(const_cast<const ExpandableHashMap*>(this)->find(key));
    }

      // C++11 syntax for preventing copying and assignment
    ExpandableHashMap(const ExpandableHashMap&) = delete;
    ExpandableHashMap& operator=(const ExpandableHashMap&) = delete;

private:
    struct Node
    {
        KeyType m_key;
        ValueType m_value;
        Node* next;
    };
    void clearMap();
    void initMap();
    int m_numBuckets;
    double m_numKeys;
    Node** arrBuckets;
    double m_maximumLoadFactor;
};
template<typename KeyType, typename ValueType>
ExpandableHashMap<KeyType,ValueType>::ExpandableHashMap(double maximumLoadFactor)
{
    if(maximumLoadFactor <= 0)
    {
        m_maximumLoadFactor = 0.5;
    }
    else
    {
        m_maximumLoadFactor = maximumLoadFactor;
    }
    initMap(); //initialize the map to default
}
template<typename KeyType, typename ValueType>
ExpandableHashMap<KeyType,ValueType>::~ExpandableHashMap()
{
    clearMap(); //deletes all dynamically allocated variables
}
template<typename KeyType, typename ValueType>
void ExpandableHashMap<KeyType,ValueType>::reset()
{
    clearMap();
    initMap();
    
}
template<typename KeyType, typename ValueType>
int ExpandableHashMap<KeyType,ValueType>::size() const //number of items in map
{
    return m_numKeys;
}
template<typename KeyType, typename ValueType>
void ExpandableHashMap<KeyType,ValueType>::associate(const KeyType& key, const ValueType& value) //add key to map
{
    unsigned int hasher(const KeyType& k);
    ValueType* found = find(key);
    if(found != nullptr) //if key in map already then update value
    {
        *found = value;
        return;
    }
    double newMaxLoad = (m_numKeys+1)/(m_numBuckets); //if load factor over limit with additional key then rehash
    if(newMaxLoad > m_maximumLoadFactor)
    {
        int m_numRehashBucket = m_numBuckets*2;
        Node** rehash = new Node*[m_numRehashBucket];
        for( int i = 0; i < m_numRehashBucket; i++)
        {
            rehash[i] = nullptr;
        }
        for(int i = 0; i < m_numBuckets; i++)
        {
            Node* currenBucket = arrBuckets[i];
            if(currenBucket != nullptr)
            {
                while(currenBucket != nullptr)
                {
                    Node* transferNode = currenBucket;
                    currenBucket = currenBucket->next;
                    KeyType relocateKey = transferNode->m_key;
                    unsigned int h = hasher(relocateKey);
                    int relocatedBucket  = h % m_numRehashBucket;
                    transferNode->next = rehash[relocatedBucket];
                    rehash[relocatedBucket] = transferNode;
                }
            }
        }
        for(int i = 0; i < m_numBuckets; i++)
        {
            arrBuckets[i] = nullptr;
        }
        delete [] arrBuckets;
        m_numBuckets = m_numRehashBucket;
        arrBuckets = rehash;
    }
    unsigned int h = hasher(key); //add key to the map
    int bucket  = h % m_numBuckets;
    Node* tempt = new Node;
    tempt->m_key = key;
    tempt->m_value = value;
    tempt->next = arrBuckets[bucket];
    arrBuckets[bucket] = tempt;
    m_numKeys++;
}
template<typename KeyType, typename ValueType>
const ValueType* ExpandableHashMap<KeyType,ValueType>::find(const KeyType& key) const //return pointer to key value
{
    unsigned int hasher(const KeyType& k);
    unsigned int h = hasher(key);
    int bucket  = h % m_numBuckets; //hash key to find bucket its in
    Node* tempt = arrBuckets[bucket];
    while(tempt != nullptr) //iterate through bucket till key found
    {
        if(tempt->m_key == key)
        {
            return &(tempt->m_value);
        }
        tempt = tempt->next;
    }
    return nullptr;
}
template<typename KeyType, typename ValueType>
void ExpandableHashMap<KeyType,ValueType>::clearMap()
{
    for(int i  = 0; i < m_numBuckets; i++)
    {
        Node* tempt = arrBuckets[i];
        while(tempt != nullptr)
        {
            Node* remove = tempt;
            tempt = tempt->next;
            delete remove;
        }
    }
    delete [] arrBuckets;
}
template<typename KeyType, typename ValueType>
void ExpandableHashMap<KeyType,ValueType>::initMap()
{
    m_numBuckets = 8;
    m_numKeys = 0;
    arrBuckets = new Node* [m_numBuckets];
    for( int i = 0; i < m_numBuckets; i++)
    {
        arrBuckets[i] = nullptr;
    }
}
#endif /* EXPANDABLEHASHMAP_INCLUDED */
