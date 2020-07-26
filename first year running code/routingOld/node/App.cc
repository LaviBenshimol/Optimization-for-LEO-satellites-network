//
// This file is part of an OMNeT++/OMNEST simulation example.
//
// Copyright (C) 1992-2015 Andras Varga
//
// This file is distributed WITHOUT ANY WARRANTY. See the file
// `license' for details on this and other legal matters.
//

#ifdef _MSC_VER
#pragma warning(disable:4786)
#endif

#include <vector>
#include <omnetpp.h>
#include "Packet_m.h"

#define CONTROL_NODE    99
#define LAVI            1
//lll New Code

using namespace omnetpp;
        enum packet_type_list
        {
            initiale_packet = 1,
            regular_packet = 2,
            schedule_topo_packet = 3 ,
            send_node_info_packet = 4,
            update_app_packet = 10,
            lavi_initialize_packet = 7,
        };
//lll New Code

/**
 * Generates traffic for the network.
 */
class App : public cSimpleModule
{
private:
    // configuration
    int myAddress;
    int currTopoID = 0;
    std::vector<int> destAddresses;
    cPar *sendIATime;
    cPar *packetLengthBytes;
    int FirstTimeGenerate = 0;
    bool fullTopoRun = false;

    // state
    Packet *generatePacket;
    cMessage *firstPacket;
    long pkCounter;

    // signals
    simsignal_t endToEndDelaySignal;
    simsignal_t hopCountSignal;
    simsignal_t sourceAddressSignal;

public:
    App();
    virtual ~App();

protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;
    void GenerateSchedulePacket(int typeOfPacket);
};

Define_Module(App);

App::App()
{
    generatePacket = nullptr;
    firstPacket = nullptr;
}

App::~App()
{
    cancelAndDelete(generatePacket);
}
//lll New Code
void App::GenerateSchedulePacket(int typeOfPacket){
    if(FirstTimeGenerate == 0){
        FirstTimeGenerate = 1;
        generatePacket = new Packet("nextPacket");
        generatePacket->setPacketType(typeOfPacket);
        scheduleAt(simTime() + sendIATime->doubleValue(), generatePacket);
    }
}
//lll New Code
void App::initialize()
{
    myAddress = par("address");
    packetLengthBytes = &par("packetLength");
    sendIATime = &par("sendIaTime");  // volatile parameter
    fullTopoRun = true;// getParentModule()->par("fullTopoRun");
    pkCounter = 0;

    WATCH(pkCounter);
    WATCH(myAddress);

    const char *destAddressesPar = par("destAddresses");
    cStringTokenizer tokenizer(destAddressesPar);
    const char *token;
    while ((token = tokenizer.nextToken()) != nullptr)
        destAddresses.push_back(atoi(token));

    if (destAddresses.size() == 0)
        throw cRuntimeError("At least one address must be specified in the destAddresses parameter!");

//lll New Code LAVI = 1 to activate lavi's code
//
//    if(LAVI){
//        EV << "ENTERED LAVI SECTION" << endl;
//        GenerateSchedulePacket(lavi_initialize_packet);
//    }
//    if( fullTopoRun && (myAddress ==CONTROL_NODE || CONTROL_NODE ==999|| myAddress==1 || myAddress==10 || myAddress== 55 || myAddress==90 || myAddress==99)){  //Only when testing full topology run
//         GenerateSchedulePacket(5); //currently not in packet type enum (only in Routing.cc)
//     }

//    if(!LAVI && fullTopoRun && (myAddress ==CONTROL_NODE || CONTROL_NODE ==999|| myAddress==1 || myAddress==10 || myAddress== 55 || myAddress==90 || myAddress==99)){  //Only when testing full topology run
//        GenerateSchedulePacket(5); //currently not in packet type enum (only in Routing.cc)
//    }
//lll New Code
    endToEndDelaySignal = registerSignal("endToEndDelay");
    hopCountSignal = registerSignal("hopCount");

    sourceAddressSignal = registerSignal("sourceAddress");
}

void App::handleMessage(cMessage *msg)
{
    Packet *pk = check_and_cast<Packet *>(msg);
    int packetType =pk->getPacketType();
    switch (packetType)
    {
    case 2:
    {
        EV << "received packet " << pk->getName() << " after " << pk->getHopCount() << "hops" << endl;
        emit(endToEndDelaySignal, simTime() - pk->getCreationTime());
        emit(hopCountSignal, pk->getHopCount());
        emit(sourceAddressSignal, pk->getSrcAddr());
        delete pk;

        if (hasGUI())
            getParentModule()->bubble("Arrived!");
    }
    break;

    case 5:
    {
        // Sending packet
        int destAddress = destAddresses[intuniform(0, destAddresses.size()-1)];

        if(myAddress ==CONTROL_NODE || CONTROL_NODE ==999|| myAddress==1 || myAddress==10 || myAddress== 55 || myAddress==90 || myAddress==99){
            char pkname[40];
            sprintf(pkname, "pk-%d-to-%d-#%ld", myAddress, destAddress, pkCounter++);
            EV << "generating packet " << pkname << endl;

            Packet *Genpk = new Packet(pkname);
            Genpk->setByteLength(packetLengthBytes->intValue());
            Genpk->setKind(intuniform(0, 7));
            Genpk->setSrcAddr(myAddress);
            Genpk->setDestAddr(destAddress);
            Genpk->setPacketType(2);
            Genpk->setTopologyID(currTopoID);
            send(Genpk, "out");

            scheduleAt(simTime() + sendIATime->doubleValue(), generatePacket);
            if (hasGUI())
                getParentModule()->bubble("Generating packet...");
        }
    }
    break;

    case 10:
    {
        GenerateSchedulePacket(5);
        EV << "Got first packet, node - " << myAddress << endl;
        currTopoID = pk->getTopologyID();
    }
    break;

//lll New Code

    case lavi_initialize_packet :
    {
        //test. to send msg from 25 to 1
        int destAddress = 0;
        if(myAddress == 99){
            char pkname[40];
            sprintf(pkname, "lavi_initialize_packet pk-%d-to-%d-#%ld", myAddress, destAddress, pkCounter++);
            EV << "lll generating packet " << pkname << endl;
            Packet *Genpk = new Packet(pkname);
            Genpk->setByteLength(packetLengthBytes->intValue());
            Genpk->setKind(intuniform(0, 7));                       // Need to decide what value to do what....
            Genpk->setSrcAddr(myAddress);
            Genpk->setDestAddr(destAddress);
            Genpk->setPacketType(2);                                // simple messages type.
            Genpk->setTopologyID(currTopoID);                       //CurrTopoID ??????
            send(Genpk, "out");

            scheduleAt(simTime() + sendIATime->doubleValue(), generatePacket);
            if (hasGUI())
                getParentModule()->bubble("lll Generating packet...");
        }
    }//case lavi_initialize_packet
    break;

    } //switch (packetType)

//lll New Code
    }//handle message

