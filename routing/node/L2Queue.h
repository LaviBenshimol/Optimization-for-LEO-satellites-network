#ifndef NODE_L2QUEUE_H_
#define NODE_L2QUEUE_H_

#include <stdio.h>
#include <string.h>
#include <omnetpp.h>
#include "Packet_m.h"
#include <sstream>
//#include "Routing.h"

using namespace omnetpp;
enum lables{
    hot_potato_lable = -5,

};
//signaling object (used to notify router module on current queue status)
enum last_routin {
    leader_to_roots = 21,
    route_from_root = 22,
    app_initial = 23,
    create_message = 24,
    message = 25,
    load_balance_reset = 26,
    //load balancer
    hot_potato = 27,
    ping_app = 28,
    ping_ping = 29,
    ping_pong = 30,
    load_prediction = 31,
    load_prediction_update = 32,
    update_topology_leader = 33,
    update_topology = 34,
    turn_off_link = 35,
    burst_app = 36,
    burst_flow = 37,
};

//lll: choosing leader enum
enum phase_list {
    phase1_update = 11,             //Every node keep updates the id table untill he learns all the nodes (given const number)
    phase2_link_info = 12,          //when finish the second phase, all the nodes wait for the topology  update to send packets
    phase3_building_LAST_tree = 13, //Only the leader build the LAST_Tree and advertise it to all nodes
  //  phase4_root_broadcast = 14,          //Only last root's will be recieving this message to build LAST tree
};


class  controlSignals : public cObject
{
  public:
  //  Packet *pk;
    int queueID;    //to whom i connected
    int ID;         //node address
    int loadOnLink;  //load on the link in the current moment of signal
    cModule *module;
    const char *SignalName;
    //cGate::Type gateType;
    //ool isVector;
    //friend class cComponent;
};
/**
 * Point-to-point interface module. While one frame is transmitted,
 * additional frames get queued up; see NED file for more info.
 */
class L2Queue : public cSimpleModule, public cListener
{
public:

  private:
    long frameCapacity;
    long sent=1;
    long recived=1;
    //cIListener queue_monitor;
   //cModule *links_control;
    cQueue queue;
    cQueue priorityQueue;
    cQueue tempQueue;
    cMessage *endTransmissionEvent;
    bool isBusy;
    int queueid;
    int myAddress;
    bool load_balance_mode;
    bool load_balance_state;
    double datarate;
    int linkusage = 0;
    int pkdropcounter = 0;
    cHistogram dropCountStats;
    cOutVector dropCountVector;
    cOutVector queueSizeVector;
    cOutVector throughput;

//    cHistogram throughput;
    simsignal_t qlenSignal;
    simsignal_t busySignal;
    simsignal_t queueingTimeSignal;
    simsignal_t dropSignal;

    simsignal_t txBytesSignal;
    simsignal_t rxBytesSignal;

    simsignal_t LBS;//load balance signal
    int queuesizesignal=0;
   // controlSignals test;
//    friend class cIListener;


  public:
    L2Queue();
    virtual ~L2Queue();
    void deleteMessageAndRecord(cMessage *msg,bool localAction = true);
  //  cSimpleModule *links_control;
   // Routing *links_control;

    int getQueueSize();

  protected:
   // friend class cIListener;
    virtual void initialize() override;

    virtual void handleMessage(cMessage *msg) override;
    virtual void refreshDisplay() const override;
    virtual void startTransmitting(cMessage *msg);

    virtual void finish() override;


    //inter module connection
    //virtual void recieveSignal();
    simsignal_t  checkSignal;
    //cModule *rotuing_module;
    char* signalDeParser(int queuesize,int dst);
    void keepPing(cMessage *msg);
//    virtual void receiveSignal(cComponent *src, simsignal_t id, controlSignals *value,controlSignals *details);
//
//
 //   virtual void receiveSignal(cComponent *src, simsignal_t id, cObject *value,cObject *details)override;
    //virtual void receiveSignal(cComponent *, simsignal_t signalID, long l, cObject *);

};

Define_Module(L2Queue);




#endif /* NODE_L2QUEUE_H_ */
