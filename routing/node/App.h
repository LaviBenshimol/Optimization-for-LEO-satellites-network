#ifdef _MSC_VER
#pragma warning(disable:4786)
#endif

#include <vector>
#include <omnetpp.h>
#include "Packet_m.h"
#include "Routing.h"
#define CONTROL_NODE    99
#define LAVI            1
#define BURSTSIZE       300
//lll New Code

//lll New Code

/**
 * Generates traffic for the network.
 */
class App : public cSimpleModule
{
private:
    //record
    double numSent;
    double numReceived;
    // configuration
    int myAddress;
    int num_of_hosts;
    int currTopoID = 0;
   // std::vector<int> destAddresses;
    cPar *sendIATime = nullptr;
    cPar *packetLengthBytes = nullptr;
    cPar *burstNextInterval;
    cPar *burstSize;
    cPar *burstNextEvent;
    int burstCounter = 0;

    int FirstTimeGenerate = 0;
    bool fullTopoRun = false;
    double pingIndex = 0;
    simtime_t currentPingStartTime;
    int ping_table[NUMOFNODES] ;
    cHistogram hopCountStats;
    cOutVector hopCountVector;
    cOutVector pingAll[NUMOFNODES];

    // state
    Packet *generatePacket = nullptr;
    cMessage *firstPacket = nullptr;
    long pkCounter;


    //burst app
    int pivotRoundRobin = 0;
    int sendflow[NUMOFNODES];
    bool flowincoming[NUMOFNODES];

    // signals
    simsignal_t endToEndDelaySignal;
    simsignal_t hopCountSignal;
    simsignal_t sourceAddressSignal;

public:
    App();
    virtual ~App();

protected:
    virtual void initialize() override;
    void sendMessage();
    void sendPing();
    void sendFlowMessage(int pivotRoundRobin);
    void createFlow( int pivotRoundRobin );
    void sendPong( int src , int pingFlag );
    void recordping( int src ,simtime_t_cref creationTime );
    virtual void handleMessage(cMessage *msg) override;
    virtual void finish() override;
};

Define_Module(App);


