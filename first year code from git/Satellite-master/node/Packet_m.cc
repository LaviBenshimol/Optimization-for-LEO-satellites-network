//
// Generated file, do not edit! Created by nedtool 5.4 from node/Packet.msg.
//

// Disable warnings about unused variables, empty switch stmts, etc:
#ifdef _MSC_VER
#  pragma warning(disable:4101)
#  pragma warning(disable:4065)
#endif

#if defined(__clang__)
#  pragma clang diagnostic ignored "-Wshadow"
#  pragma clang diagnostic ignored "-Wconversion"
#  pragma clang diagnostic ignored "-Wunused-parameter"
#  pragma clang diagnostic ignored "-Wc++98-compat"
#  pragma clang diagnostic ignored "-Wunreachable-code-break"
#  pragma clang diagnostic ignored "-Wold-style-cast"
#elif defined(__GNUC__)
#  pragma GCC diagnostic ignored "-Wshadow"
#  pragma GCC diagnostic ignored "-Wconversion"
#  pragma GCC diagnostic ignored "-Wunused-parameter"
#  pragma GCC diagnostic ignored "-Wold-style-cast"
#  pragma GCC diagnostic ignored "-Wsuggest-attribute=noreturn"
#  pragma GCC diagnostic ignored "-Wfloat-conversion"
#endif

#include <iostream>
#include <sstream>
#include <memory>
#include "Packet_m.h"

namespace omnetpp {

// Template pack/unpack rules. They are declared *after* a1l type-specific pack functions for multiple reasons.
// They are in the omnetpp namespace, to allow them to be found by argument-dependent lookup via the cCommBuffer argument

// Packing/unpacking an std::vector
template<typename T, typename A>
void doParsimPacking(omnetpp::cCommBuffer *buffer, const std::vector<T,A>& v)
{
    int n = v.size();
    doParsimPacking(buffer, n);
    for (int i = 0; i < n; i++)
        doParsimPacking(buffer, v[i]);
}

template<typename T, typename A>
void doParsimUnpacking(omnetpp::cCommBuffer *buffer, std::vector<T,A>& v)
{
    int n;
    doParsimUnpacking(buffer, n);
    v.resize(n);
    for (int i = 0; i < n; i++)
        doParsimUnpacking(buffer, v[i]);
}

// Packing/unpacking an std::list
template<typename T, typename A>
void doParsimPacking(omnetpp::cCommBuffer *buffer, const std::list<T,A>& l)
{
    doParsimPacking(buffer, (int)l.size());
    for (typename std::list<T,A>::const_iterator it = l.begin(); it != l.end(); ++it)
        doParsimPacking(buffer, (T&)*it);
}

template<typename T, typename A>
void doParsimUnpacking(omnetpp::cCommBuffer *buffer, std::list<T,A>& l)
{
    int n;
    doParsimUnpacking(buffer, n);
    for (int i = 0; i < n; i++) {
        l.push_back(T());
        doParsimUnpacking(buffer, l.back());
    }
}

// Packing/unpacking an std::set
template<typename T, typename Tr, typename A>
void doParsimPacking(omnetpp::cCommBuffer *buffer, const std::set<T,Tr,A>& s)
{
    doParsimPacking(buffer, (int)s.size());
    for (typename std::set<T,Tr,A>::const_iterator it = s.begin(); it != s.end(); ++it)
        doParsimPacking(buffer, *it);
}

template<typename T, typename Tr, typename A>
void doParsimUnpacking(omnetpp::cCommBuffer *buffer, std::set<T,Tr,A>& s)
{
    int n;
    doParsimUnpacking(buffer, n);
    for (int i = 0; i < n; i++) {
        T x;
        doParsimUnpacking(buffer, x);
        s.insert(x);
    }
}

// Packing/unpacking an std::map
template<typename K, typename V, typename Tr, typename A>
void doParsimPacking(omnetpp::cCommBuffer *buffer, const std::map<K,V,Tr,A>& m)
{
    doParsimPacking(buffer, (int)m.size());
    for (typename std::map<K,V,Tr,A>::const_iterator it = m.begin(); it != m.end(); ++it) {
        doParsimPacking(buffer, it->first);
        doParsimPacking(buffer, it->second);
    }
}

template<typename K, typename V, typename Tr, typename A>
void doParsimUnpacking(omnetpp::cCommBuffer *buffer, std::map<K,V,Tr,A>& m)
{
    int n;
    doParsimUnpacking(buffer, n);
    for (int i = 0; i < n; i++) {
        K k; V v;
        doParsimUnpacking(buffer, k);
        doParsimUnpacking(buffer, v);
        m[k] = v;
    }
}

// Default pack/unpack function for arrays
template<typename T>
void doParsimArrayPacking(omnetpp::cCommBuffer *b, const T *t, int n)
{
    for (int i = 0; i < n; i++)
        doParsimPacking(b, t[i]);
}

template<typename T>
void doParsimArrayUnpacking(omnetpp::cCommBuffer *b, T *t, int n)
{
    for (int i = 0; i < n; i++)
        doParsimUnpacking(b, t[i]);
}

// Default rule to prevent compiler from choosing base class' doParsimPacking() function
template<typename T>
void doParsimPacking(omnetpp::cCommBuffer *, const T& t)
{
    throw omnetpp::cRuntimeError("Parsim error: No doParsimPacking() function for type %s", omnetpp::opp_typename(typeid(t)));
}

template<typename T>
void doParsimUnpacking(omnetpp::cCommBuffer *, T& t)
{
    throw omnetpp::cRuntimeError("Parsim error: No doParsimUnpacking() function for type %s", omnetpp::opp_typename(typeid(t)));
}

}  // namespace omnetpp

namespace {
template <class T> inline
typename std::enable_if<std::is_polymorphic<T>::value && std::is_base_of<omnetpp::cObject,T>::value, void *>::type
toVoidPtr(T* t)
{
    return (void *)(static_cast<const omnetpp::cObject *>(t));
}

template <class T> inline
typename std::enable_if<std::is_polymorphic<T>::value && !std::is_base_of<omnetpp::cObject,T>::value, void *>::type
toVoidPtr(T* t)
{
    return (void *)dynamic_cast<const void *>(t);
}

template <class T> inline
typename std::enable_if<!std::is_polymorphic<T>::value, void *>::type
toVoidPtr(T* t)
{
    return (void *)static_cast<const void *>(t);
}

}


// forward
template<typename T, typename A>
std::ostream& operator<<(std::ostream& out, const std::vector<T,A>& vec);

// Template rule to generate operator<< for shared_ptr<T>
template<typename T>
inline std::ostream& operator<<(std::ostream& out,const std::shared_ptr<T>& t) { return out << t.get(); }

// Template rule which fires if a struct or class doesn't have operator<<
template<typename T>
inline std::ostream& operator<<(std::ostream& out,const T&) {return out;}

// operator<< for std::vector<T>
template<typename T, typename A>
inline std::ostream& operator<<(std::ostream& out, const std::vector<T,A>& vec)
{
    out.put('{');
    for(typename std::vector<T,A>::const_iterator it = vec.begin(); it != vec.end(); ++it)
    {
        if (it != vec.begin()) {
            out.put(','); out.put(' ');
        }
        out << *it;
    }
    out.put('}');

    char buf[32];
    sprintf(buf, " (size=%u)", (unsigned int)vec.size());
    out.write(buf, strlen(buf));
    return out;
}

Register_Class(Packet)

Packet::Packet(const char *name, short kind) : ::omnetpp::cPacket(name, kind)
{
}

Packet::Packet(const Packet& other) : ::omnetpp::cPacket(other)
{
    copy(other);
}

Packet::~Packet()
{
    delete [] this->gateIDList;
    delete [] this->packetList;
}

Packet& Packet::operator=(const Packet& other)
{
    if (this == &other) return *this;
    ::omnetpp::cPacket::operator=(other);
    copy(other);
    return *this;
}

void Packet::copy(const Packet& other)
{
    this->srcAddr = other.srcAddr;
    this->destAddr = other.destAddr;
    this->hopCount = other.hopCount;
    this->hasInitTopo = other.hasInitTopo;
    this->packetType = other.packetType;
    this->TopologyVar = other.TopologyVar;
    this->TopologyID = other.TopologyID;
    delete [] this->gateIDList;
    this->gateIDList = (other.gateIDList_arraysize==0) ? nullptr : new int[other.gateIDList_arraysize];
    gateIDList_arraysize = other.gateIDList_arraysize;
    for (size_t i = 0; i < gateIDList_arraysize; i++) {
        this->gateIDList[i] = other.gateIDList[i];
    }
    delete [] this->packetList;
    this->packetList = (other.packetList_arraysize==0) ? nullptr : new int[other.packetList_arraysize];
    packetList_arraysize = other.packetList_arraysize;
    for (size_t i = 0; i < packetList_arraysize; i++) {
        this->packetList[i] = other.packetList[i];
    }
}

void Packet::parsimPack(omnetpp::cCommBuffer *b) const
{
    ::omnetpp::cPacket::parsimPack(b);
    doParsimPacking(b,this->srcAddr);
    doParsimPacking(b,this->destAddr);
    doParsimPacking(b,this->hopCount);
    doParsimPacking(b,this->hasInitTopo);
    doParsimPacking(b,this->packetType);
    doParsimPacking(b,this->TopologyVar);
    doParsimPacking(b,this->TopologyID);
    b->pack(gateIDList_arraysize);
    doParsimArrayPacking(b,this->gateIDList,gateIDList_arraysize);
    b->pack(packetList_arraysize);
    doParsimArrayPacking(b,this->packetList,packetList_arraysize);
}

void Packet::parsimUnpack(omnetpp::cCommBuffer *b)
{
    ::omnetpp::cPacket::parsimUnpack(b);
    doParsimUnpacking(b,this->srcAddr);
    doParsimUnpacking(b,this->destAddr);
    doParsimUnpacking(b,this->hopCount);
    doParsimUnpacking(b,this->hasInitTopo);
    doParsimUnpacking(b,this->packetType);
    doParsimUnpacking(b,this->TopologyVar);
    doParsimUnpacking(b,this->TopologyID);
    delete [] this->gateIDList;
    b->unpack(gateIDList_arraysize);
    if (gateIDList_arraysize == 0) {
        this->gateIDList = nullptr;
    } else {
        this->gateIDList = new int[gateIDList_arraysize];
        doParsimArrayUnpacking(b,this->gateIDList,gateIDList_arraysize);
    }
    delete [] this->packetList;
    b->unpack(packetList_arraysize);
    if (packetList_arraysize == 0) {
        this->packetList = nullptr;
    } else {
        this->packetList = new int[packetList_arraysize];
        doParsimArrayUnpacking(b,this->packetList,packetList_arraysize);
    }
}

int Packet::getSrcAddr() const
{
    return this->srcAddr;
}

void Packet::setSrcAddr(int srcAddr)
{
    this->srcAddr = srcAddr;
}

int Packet::getDestAddr() const
{
    return this->destAddr;
}

void Packet::setDestAddr(int destAddr)
{
    this->destAddr = destAddr;
}

int Packet::getHopCount() const
{
    return this->hopCount;
}

void Packet::setHopCount(int hopCount)
{
    this->hopCount = hopCount;
}

int Packet::getHasInitTopo() const
{
    return this->hasInitTopo;
}

void Packet::setHasInitTopo(int hasInitTopo)
{
    this->hasInitTopo = hasInitTopo;
}

int Packet::getPacketType() const
{
    return this->packetType;
}

void Packet::setPacketType(int packetType)
{
    this->packetType = packetType;
}

const omnetpp::cTopology * Packet::getTopologyVar() const
{
    return this->TopologyVar;
}

void Packet::setTopologyVar(omnetpp::cTopology * TopologyVar)
{
    this->TopologyVar = TopologyVar;
}

int Packet::getTopologyID() const
{
    return this->TopologyID;
}

void Packet::setTopologyID(int TopologyID)
{
    this->TopologyID = TopologyID;
}

size_t Packet::getGateIDListArraySize() const
{
    return gateIDList_arraysize;
}

int Packet::getGateIDList(size_t k) const
{
    if (k >= gateIDList_arraysize) throw omnetpp::cRuntimeError("Array of size gateIDList_arraysize indexed by %lu", (unsigned long)k);
    return this->gateIDList[k];
}

void Packet::setGateIDListArraySize(size_t newSize)
{
    int *gateIDList2 = (newSize==0) ? nullptr : new int[newSize];
    size_t minSize = gateIDList_arraysize < newSize ? gateIDList_arraysize : newSize;
    for (size_t i = 0; i < minSize; i++)
        gateIDList2[i] = this->gateIDList[i];
    for (size_t i = minSize; i < newSize; i++)
        gateIDList2[i] = 0;
    delete [] this->gateIDList;
    this->gateIDList = gateIDList2;
    gateIDList_arraysize = newSize;
}

void Packet::setGateIDList(size_t k, int gateIDList)
{
    if (k >= gateIDList_arraysize) throw omnetpp::cRuntimeError("Array of size  indexed by %lu", (unsigned long)k);
    this->gateIDList[k] = gateIDList;
}

void Packet::insertGateIDList(size_t k, int gateIDList)
{
    if (k > gateIDList_arraysize) throw omnetpp::cRuntimeError("Array of size  indexed by %lu", (unsigned long)k);
    size_t newSize = gateIDList_arraysize + 1;
    int *gateIDList2 = new int[newSize];
    size_t i;
    for (i = 0; i < k; i++)
        gateIDList2[i] = this->gateIDList[i];
    gateIDList2[k] = gateIDList;
    for (i = k + 1; i < newSize; i++)
        gateIDList2[i] = this->gateIDList[i-1];
    delete [] this->gateIDList;
    this->gateIDList = gateIDList2;
    gateIDList_arraysize = newSize;
}

void Packet::insertGateIDList(int gateIDList)
{
    insertGateIDList(gateIDList_arraysize, gateIDList);
}

void Packet::eraseGateIDList(size_t k)
{
    if (k >= gateIDList_arraysize) throw omnetpp::cRuntimeError("Array of size  indexed by %lu", (unsigned long)k);
    size_t newSize = gateIDList_arraysize - 1;
    int *gateIDList2 = (newSize == 0) ? nullptr : new int[newSize];
    size_t i;
    for (i = 0; i < k; i++)
        gateIDList2[i] = this->gateIDList[i];
    for (i = k; i < newSize; i++)
        gateIDList2[i] = this->gateIDList[i+1];
    delete [] this->gateIDList;
    this->gateIDList = gateIDList2;
    gateIDList_arraysize = newSize;
}

size_t Packet::getPacketListArraySize() const
{
    return packetList_arraysize;
}

int Packet::getPacketList(size_t k) const
{
    if (k >= packetList_arraysize) throw omnetpp::cRuntimeError("Array of size packetList_arraysize indexed by %lu", (unsigned long)k);
    return this->packetList[k];
}

void Packet::setPacketListArraySize(size_t newSize)
{
    int *packetList2 = (newSize==0) ? nullptr : new int[newSize];
    size_t minSize = packetList_arraysize < newSize ? packetList_arraysize : newSize;
    for (size_t i = 0; i < minSize; i++)
        packetList2[i] = this->packetList[i];
    for (size_t i = minSize; i < newSize; i++)
        packetList2[i] = 0;
    delete [] this->packetList;
    this->packetList = packetList2;
    packetList_arraysize = newSize;
}

void Packet::setPacketList(size_t k, int packetList)
{
    if (k >= packetList_arraysize) throw omnetpp::cRuntimeError("Array of size  indexed by %lu", (unsigned long)k);
    this->packetList[k] = packetList;
}

void Packet::insertPacketList(size_t k, int packetList)
{
    if (k > packetList_arraysize) throw omnetpp::cRuntimeError("Array of size  indexed by %lu", (unsigned long)k);
    size_t newSize = packetList_arraysize + 1;
    int *packetList2 = new int[newSize];
    size_t i;
    for (i = 0; i < k; i++)
        packetList2[i] = this->packetList[i];
    packetList2[k] = packetList;
    for (i = k + 1; i < newSize; i++)
        packetList2[i] = this->packetList[i-1];
    delete [] this->packetList;
    this->packetList = packetList2;
    packetList_arraysize = newSize;
}

void Packet::insertPacketList(int packetList)
{
    insertPacketList(packetList_arraysize, packetList);
}

void Packet::erasePacketList(size_t k)
{
    if (k >= packetList_arraysize) throw omnetpp::cRuntimeError("Array of size  indexed by %lu", (unsigned long)k);
    size_t newSize = packetList_arraysize - 1;
    int *packetList2 = (newSize == 0) ? nullptr : new int[newSize];
    size_t i;
    for (i = 0; i < k; i++)
        packetList2[i] = this->packetList[i];
    for (i = k; i < newSize; i++)
        packetList2[i] = this->packetList[i+1];
    delete [] this->packetList;
    this->packetList = packetList2;
    packetList_arraysize = newSize;
}

class PacketDescriptor : public omnetpp::cClassDescriptor
{
  private:
    mutable const char **propertynames;
    enum FieldConstants {
        FIELD_srcAddr,
        FIELD_destAddr,
        FIELD_hopCount,
        FIELD_hasInitTopo,
        FIELD_packetType,
        FIELD_TopologyVar,
        FIELD_TopologyID,
        FIELD_gateIDList,
        FIELD_packetList,
    };
  public:
    PacketDescriptor();
    virtual ~PacketDescriptor();

    virtual bool doesSupport(omnetpp::cObject *obj) const override;
    virtual const char **getPropertyNames() const override;
    virtual const char *getProperty(const char *propertyname) const override;
    virtual int getFieldCount() const override;
    virtual const char *getFieldName(int field) const override;
    virtual int findField(const char *fieldName) const override;
    virtual unsigned int getFieldTypeFlags(int field) const override;
    virtual const char *getFieldTypeString(int field) const override;
    virtual const char **getFieldPropertyNames(int field) const override;
    virtual const char *getFieldProperty(int field, const char *propertyname) const override;
    virtual int getFieldArraySize(void *object, int field) const override;

    virtual const char *getFieldDynamicTypeString(void *object, int field, int i) const override;
    virtual std::string getFieldValueAsString(void *object, int field, int i) const override;
    virtual bool setFieldValueAsString(void *object, int field, int i, const char *value) const override;

    virtual const char *getFieldStructName(int field) const override;
    virtual void *getFieldStructValuePointer(void *object, int field, int i) const override;
};

Register_ClassDescriptor(PacketDescriptor)

PacketDescriptor::PacketDescriptor() : omnetpp::cClassDescriptor(omnetpp::opp_typename(typeid(Packet)), "omnetpp::cPacket")
{
    propertynames = nullptr;
}

PacketDescriptor::~PacketDescriptor()
{
    delete[] propertynames;
}

bool PacketDescriptor::doesSupport(omnetpp::cObject *obj) const
{
    return dynamic_cast<Packet *>(obj)!=nullptr;
}

const char **PacketDescriptor::getPropertyNames() const
{
    if (!propertynames) {
        static const char *names[] = {  nullptr };
        omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
        const char **basenames = basedesc ? basedesc->getPropertyNames() : nullptr;
        propertynames = mergeLists(basenames, names);
    }
    return propertynames;
}

const char *PacketDescriptor::getProperty(const char *propertyname) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? basedesc->getProperty(propertyname) : nullptr;
}

int PacketDescriptor::getFieldCount() const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? 9+basedesc->getFieldCount() : 9;
}

unsigned int PacketDescriptor::getFieldTypeFlags(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldTypeFlags(field);
        field -= basedesc->getFieldCount();
    }
    static unsigned int fieldTypeFlags[] = {
        FD_ISEDITABLE,    // FIELD_srcAddr
        FD_ISEDITABLE,    // FIELD_destAddr
        FD_ISEDITABLE,    // FIELD_hopCount
        FD_ISEDITABLE,    // FIELD_hasInitTopo
        FD_ISEDITABLE,    // FIELD_packetType
        FD_ISCOMPOUND | FD_ISPOINTER | FD_ISCOBJECT | FD_ISCOWNEDOBJECT,    // FIELD_TopologyVar
        FD_ISEDITABLE,    // FIELD_TopologyID
        FD_ISARRAY | FD_ISEDITABLE,    // FIELD_gateIDList
        FD_ISARRAY | FD_ISEDITABLE,    // FIELD_packetList
    };
    return (field >= 0 && field < 9) ? fieldTypeFlags[field] : 0;
}

const char *PacketDescriptor::getFieldName(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldName(field);
        field -= basedesc->getFieldCount();
    }
    static const char *fieldNames[] = {
        "srcAddr",
        "destAddr",
        "hopCount",
        "hasInitTopo",
        "packetType",
        "TopologyVar",
        "TopologyID",
        "gateIDList",
        "packetList",
    };
    return (field >= 0 && field < 9) ? fieldNames[field] : nullptr;
}

int PacketDescriptor::findField(const char *fieldName) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    int base = basedesc ? basedesc->getFieldCount() : 0;
    if (fieldName[0] == 's' && strcmp(fieldName, "srcAddr") == 0) return base+0;
    if (fieldName[0] == 'd' && strcmp(fieldName, "destAddr") == 0) return base+1;
    if (fieldName[0] == 'h' && strcmp(fieldName, "hopCount") == 0) return base+2;
    if (fieldName[0] == 'h' && strcmp(fieldName, "hasInitTopo") == 0) return base+3;
    if (fieldName[0] == 'p' && strcmp(fieldName, "packetType") == 0) return base+4;
    if (fieldName[0] == 'T' && strcmp(fieldName, "TopologyVar") == 0) return base+5;
    if (fieldName[0] == 'T' && strcmp(fieldName, "TopologyID") == 0) return base+6;
    if (fieldName[0] == 'g' && strcmp(fieldName, "gateIDList") == 0) return base+7;
    if (fieldName[0] == 'p' && strcmp(fieldName, "packetList") == 0) return base+8;
    return basedesc ? basedesc->findField(fieldName) : -1;
}

const char *PacketDescriptor::getFieldTypeString(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldTypeString(field);
        field -= basedesc->getFieldCount();
    }
    static const char *fieldTypeStrings[] = {
        "int",    // FIELD_srcAddr
        "int",    // FIELD_destAddr
        "int",    // FIELD_hopCount
        "int",    // FIELD_hasInitTopo
        "int",    // FIELD_packetType
        "omnetpp::cTopology",    // FIELD_TopologyVar
        "int",    // FIELD_TopologyID
        "int",    // FIELD_gateIDList
        "int",    // FIELD_packetList
    };
    return (field >= 0 && field < 9) ? fieldTypeStrings[field] : nullptr;
}

const char **PacketDescriptor::getFieldPropertyNames(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldPropertyNames(field);
        field -= basedesc->getFieldCount();
    }
    switch (field) {
        case FIELD_srcAddr: {
            static const char *names[] = { "packetData",  nullptr };
            return names;
        }
        case FIELD_destAddr: {
            static const char *names[] = { "packetData",  nullptr };
            return names;
        }
        case FIELD_hopCount: {
            static const char *names[] = { "packetData",  nullptr };
            return names;
        }
        case FIELD_hasInitTopo: {
            static const char *names[] = { "packetData",  nullptr };
            return names;
        }
        case FIELD_packetType: {
            static const char *names[] = { "packetData",  nullptr };
            return names;
        }
        case FIELD_TopologyVar: {
            static const char *names[] = { "packetData",  nullptr };
            return names;
        }
        case FIELD_TopologyID: {
            static const char *names[] = { "packetData",  nullptr };
            return names;
        }
        case FIELD_gateIDList: {
            static const char *names[] = { "packetData",  nullptr };
            return names;
        }
        case FIELD_packetList: {
            static const char *names[] = { "packetData",  nullptr };
            return names;
        }
        default: return nullptr;
    }
}

const char *PacketDescriptor::getFieldProperty(int field, const char *propertyname) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldProperty(field, propertyname);
        field -= basedesc->getFieldCount();
    }
    switch (field) {
        case FIELD_srcAddr:
            if (!strcmp(propertyname, "packetData")) return "";
            return nullptr;
        case FIELD_destAddr:
            if (!strcmp(propertyname, "packetData")) return "";
            return nullptr;
        case FIELD_hopCount:
            if (!strcmp(propertyname, "packetData")) return "";
            return nullptr;
        case FIELD_hasInitTopo:
            if (!strcmp(propertyname, "packetData")) return "";
            return nullptr;
        case FIELD_packetType:
            if (!strcmp(propertyname, "packetData")) return "";
            return nullptr;
        case FIELD_TopologyVar:
            if (!strcmp(propertyname, "packetData")) return "";
            return nullptr;
        case FIELD_TopologyID:
            if (!strcmp(propertyname, "packetData")) return "";
            return nullptr;
        case FIELD_gateIDList:
            if (!strcmp(propertyname, "packetData")) return "";
            return nullptr;
        case FIELD_packetList:
            if (!strcmp(propertyname, "packetData")) return "";
            return nullptr;
        default: return nullptr;
    }
}

int PacketDescriptor::getFieldArraySize(void *object, int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldArraySize(object, field);
        field -= basedesc->getFieldCount();
    }
    Packet *pp = (Packet *)object; (void)pp;
    switch (field) {
        case FIELD_gateIDList: return pp->getGateIDListArraySize();
        case FIELD_packetList: return pp->getPacketListArraySize();
        default: return 0;
    }
}

const char *PacketDescriptor::getFieldDynamicTypeString(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldDynamicTypeString(object,field,i);
        field -= basedesc->getFieldCount();
    }
    Packet *pp = (Packet *)object; (void)pp;
    switch (field) {
        case FIELD_TopologyVar: { const omnetpp::cTopology * value = pp->getTopologyVar(); return omnetpp::opp_typename(typeid(*value)); }
        default: return nullptr;
    }
}

std::string PacketDescriptor::getFieldValueAsString(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldValueAsString(object,field,i);
        field -= basedesc->getFieldCount();
    }
    Packet *pp = (Packet *)object; (void)pp;
    switch (field) {
        case FIELD_srcAddr: return long2string(pp->getSrcAddr());
        case FIELD_destAddr: return long2string(pp->getDestAddr());
        case FIELD_hopCount: return long2string(pp->getHopCount());
        case FIELD_hasInitTopo: return long2string(pp->getHasInitTopo());
        case FIELD_packetType: return long2string(pp->getPacketType());
        case FIELD_TopologyVar: {std::stringstream out; out << pp->getTopologyVar(); return out.str();}
        case FIELD_TopologyID: return long2string(pp->getTopologyID());
        case FIELD_gateIDList: return long2string(pp->getGateIDList(i));
        case FIELD_packetList: return long2string(pp->getPacketList(i));
        default: return "";
    }
}

bool PacketDescriptor::setFieldValueAsString(void *object, int field, int i, const char *value) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->setFieldValueAsString(object,field,i,value);
        field -= basedesc->getFieldCount();
    }
    Packet *pp = (Packet *)object; (void)pp;
    switch (field) {
        case FIELD_srcAddr: pp->setSrcAddr(string2long(value)); return true;
        case FIELD_destAddr: pp->setDestAddr(string2long(value)); return true;
        case FIELD_hopCount: pp->setHopCount(string2long(value)); return true;
        case FIELD_hasInitTopo: pp->setHasInitTopo(string2long(value)); return true;
        case FIELD_packetType: pp->setPacketType(string2long(value)); return true;
        case FIELD_TopologyID: pp->setTopologyID(string2long(value)); return true;
        case FIELD_gateIDList: pp->setGateIDList(i,string2long(value)); return true;
        case FIELD_packetList: pp->setPacketList(i,string2long(value)); return true;
        default: return false;
    }
}

const char *PacketDescriptor::getFieldStructName(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldStructName(field);
        field -= basedesc->getFieldCount();
    }
    switch (field) {
        case FIELD_TopologyVar: return omnetpp::opp_typename(typeid(omnetpp::cTopology));
        default: return nullptr;
    };
}

void *PacketDescriptor::getFieldStructValuePointer(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldStructValuePointer(object, field, i);
        field -= basedesc->getFieldCount();
    }
    Packet *pp = (Packet *)object; (void)pp;
    switch (field) {
        case FIELD_TopologyVar: return toVoidPtr(pp->getTopologyVar()); break;
        default: return nullptr;
    }
}

