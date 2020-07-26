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

#define CONTROL_NODE 99

using namespace omnetpp;

/**
 * Generates traffic for the network.
 */
class App : public cSimpleModule
{
private:
    // configuration
    int myAddress;
    int currTopoID =0;
    std::vector<int> destAddresses;
    cPar *sendIATime;
    cPar *packetLengthBytes;
    int FirstTimeGenerate =0;
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
    void GenerateSchedulePacket();
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

void App::GenerateSchedulePacket(){
    generatePacket = new Packet("nextPacket");
    generatePacket->setPacketType(5);
    if(FirstTimeGenerate==0){
        FirstTimeGenerate =1;
        scheduleAt(simTime() + sendIATime->doubleValue(), generatePacket);
    }
}

void App::initialize()
{
    myAddress = par("address");
    packetLengthBytes = &par("packetLength");
    sendIATime = &par("sendIaTime");  // volatile parameter
    fullTopoRun = getParentModule()->par("fullTopoRun");
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

    if(fullTopoRun && (myAddress==CONTROL_NODE || CONTROL_NODE ==999)){  //Only when testing full topology run
        GenerateSchedulePacket();
    }

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

        if(myAddress ==CONTROL_NODE || CONTROL_NODE ==999){
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
        GenerateSchedulePacket();
        EV << "Got first packet, node - " << myAddress << endl;
        currTopoID = pk->getTopologyID();
    }
    break;
    }
}

