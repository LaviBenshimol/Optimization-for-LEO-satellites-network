#include "L2Queue.h"

//L2Queue::simsignal_t L2Queue:: checkSignal= registerSignal("check");

L2Queue::L2Queue()
{
    endTransmissionEvent = nullptr;
}

L2Queue::~L2Queue()
{
    cancelAndDelete(endTransmissionEvent);
}
void L2Queue::finish()
{
    dropCountStats.recordAs("drop count");
    //throughput.recordAs("hop count");
    queue.clear();
   // dropCountStats.
}
//controlSignals
//void L2Queue::receiveSignal(cComponent *src, simsignal_t id, controlSignals *value,controlSignals *details){
int L2Queue::getQueueSize()
{
    return queue.getLength();
}
char* L2Queue::signalDeParser(int queuesize,int myAddress)
{
    int pivot=0;
    int queuesizetemp=queue.getLength();
    char idch[1],queuelench[10];
    sprintf(queuelench,"%d",queuesizetemp);
    sprintf(idch,"%d",queueid);
    char * str = (char*)malloc(sizeof(char)*20);

    int index=0;
    //insert id
    str[index]=idch[index];
    //insert space
    index++;str[index]=' ' ;index++;
    //insert queue size
    pivot=0;
    while ( queuelench[pivot] != '\0')
    {
        str[index]=queuelench[pivot];
        index++;
        pivot++;
    }
    //insert end of string
    str[index] = '\0';
    //clean rest of string
    index++;
    while(index < 20)
    {
        str[index] = '\0';
        index++;
    }
    return str;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

                                                                                // lllinitialize

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void L2Queue::initialize()
{
    LBS = cComponent:: registerSignal("load_balance_channel");
    //find queue id
    if(this == getParentModule()->getSubmodule("queue", 0))
        queueid = 0;
    if(this == getParentModule()->getSubmodule("queue", 1))
        queueid = 1;
    if(this == getParentModule()->getSubmodule("queue", 2))
        queueid = 2;
    if(this == getParentModule()->getSubmodule("queue", 3))
        queueid = 3;
    myAddress = getParentModule()->par("address");
    load_balance_mode = getParentModule()->par("load_balance_mode");
    load_balance_state = false;
    queue.setName("queue");
    myAddress = getParentModule()->par("address");
    endTransmissionEvent = new cMessage("endTxEvent");
    if (par("useCutThroughSwitching"))
        gate("line$i")->setDeliverOnReceptionStart(true);

    frameCapacity = getParentModule()->par("frameCapacity");
    datarate = check_and_cast<cDatarateChannel*>(gate("line$o")->getTransmissionChannel()) ->getDatarate();
    qlenSignal = registerSignal("qlen");

    busySignal = registerSignal("busy");
    queueingTimeSignal = registerSignal("queueingTime");


  //  throughput = registerSignal("throughput");
    dropSignal = registerSignal("drop");
    txBytesSignal = registerSignal("txBytes");
    rxBytesSignal = registerSignal("rxBytes");

//    EV<<"1: "<<this->getName()<<" 2: "<<this->getParentModule()->getSubmodule("routing")->getName()<<endl;
    emit(qlenSignal, queue.getLength());
    emit(busySignal, false);
    isBusy = false;
    queueSizeVector.setName("Size");
    dropCountVector.setName("Drop");
    throughput.setName("Throughput");
}


void L2Queue::startTransmitting(cMessage *msg)
{
    EV << "Starting transmission of " << msg << endl;
    isBusy = true;
    int64_t numBytes = check_and_cast<cPacket *>(msg)->getByteLength();


//    int64_t  c = check_and_cast<cPacket *>(msg) -> getBitLength();
//    int64_t  d = check_and_cast<cPacket *>(msg) -> getByteLength();
    send(msg, "line$o");
  //  simtime_t a =  gate("line$o")->getTransmissionChannel()->getTransmissionFinishTime();
    sent++;
    emit(txBytesSignal, (long)numBytes);
    throughput.record((double)sent/recived);
    //emit(throughput,(double)sent/recived);
       // Schedule an event for the time when last bit will leave the gate.
    simtime_t endTransmission = gate("line$o")->getTransmissionChannel()->getTransmissionFinishTime();
    simtime_t e = getSimulation()->getSimTime();
    simtime_t f = gate("line$o")->getTransmissionChannel()->getTransmissionFinishTime();
    scheduleAt(endTransmission, endTransmissionEvent);
}
void L2Queue::keepPing(cMessage *msg)
{
    int tempq = 0;
    cMessage *poorMessage = (cMessage *)queue.pop();
    while (check_and_cast<Packet *>(poorMessage) -> datadouble[24] == ping_ping && check_and_cast<Packet *>(poorMessage) -> datadouble[24] == ping_pong )
    {
        tempQueue.insert(poorMessage);
        tempq++;
        poorMessage = (cMessage *)queue.pop();
    }

    deleteMessageAndRecord(poorMessage);


    while(tempq>0)
    {
        poorMessage =(cMessage *)tempQueue.pop();
        queue.insert(poorMessage);
    }
    msg->setTimestamp();
    queue.insert(msg);
  //  queueSizeVector.record((long)queue.getLength());
}

void L2Queue::deleteMessageAndRecord(cMessage *msg,bool localAction )
{
    pkdropcounter++;
    dropCountVector.record((long)pkdropcounter);
    dropCountStats.collect((long)pkdropcounter);
    emit(dropSignal, (long)check_and_cast<cPacket *>(msg)->getByteLength());
    if( localAction )
        delete msg;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

                                                                                // lllhandlemessage

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void L2Queue::handleMessage(cMessage *msg)
{

    if (msg == endTransmissionEvent) {
        // Transmission finished, we can start next one.
        EV << "Transmission finished.\n";
        isBusy = false;
        if( load_balance_mode )
        {
            queuesizesignal = queue.getLength();
            char *sig  = signalDeParser(queuesizesignal,myAddress);
            if(hasListeners(LBS))
                emit(LBS,(const char*)sig);
            free(sig);
        }
        if (queue.isEmpty() && priorityQueue.isEmpty()) {
            queueSizeVector.record((long)queue.getLength());
            emit(busySignal, false);
        }
        else {
            if(priorityQueue.isEmpty() && !queue.isEmpty())
            {
                msg = (cMessage *)queue.pop();
                queueSizeVector.record((long)queue.getLength());
                if( load_balance_mode )
                {
                    queuesizesignal = queue.getLength();
                    char *sig  = signalDeParser(queuesizesignal,myAddress);
                    if(hasListeners(LBS))
                        emit(LBS,(const char*)sig);
                    free(sig);
                }
                emit(queueingTimeSignal, simTime() - msg->getTimestamp());
                emit(qlenSignal, queue.getLength());
            }
            else
            {
                msg = (cMessage *)priorityQueue.pop();
                emit(queueingTimeSignal, simTime() - msg->getTimestamp());
                emit(qlenSignal, queue.getLength());
            }

            startTransmitting(msg);
        }
    }

    else if (msg->arrivedOn("line$i")) {
        // pass up
        emit(rxBytesSignal, (long)check_and_cast<cPacket *>(msg)->getByteLength());
        send(msg, "out");
    }
    // arrived on gate "in"
    else {
        recived++;
      //  recivedVector.record(recived);
        // emit signal to routing
        if ( load_balance_mode )
        {
            check_and_cast<cPacket *>(msg) -> setBitLength((int64_t) (1500*8));
            queuesizesignal = queue.getLength() + 1;
            char *sig  = signalDeParser(queuesizesignal,myAddress);
            if(hasListeners(LBS))
                emit(LBS,(const char*)sig);
            free(sig);
        }
        // We are currently busy, so just queue up the packet.
        if (endTransmissionEvent->isScheduled()) {
            //drop the packet (unless its control plane message)
            if (frameCapacity && queue.getLength() >= frameCapacity) {
                EV << "Received " << msg << " but transmitter busy and queue full: discarding\n";
                if (check_and_cast<Packet *>(msg) -> getPacketType() == leader_to_roots ||  check_and_cast<Packet *>(msg) -> getPacketType() == route_from_root ||check_and_cast<Packet *>(msg) -> getPacketType() == phase2_link_info)
                {
                    msg->setTimestamp();
                    priorityQueue.insert(msg);
                }
                //keeping ping in the last stop in the queue if is full
                else if (check_and_cast<Packet *>(msg) -> datadouble[24] == ping_ping && check_and_cast<Packet *>(msg) -> datadouble[24] == ping_pong )
                {
                    keepPing(msg);

                }
                else
                {
                    deleteMessageAndRecord(msg); //TODO: drop count... delete proper
                }
            }
            //queue is not full. enqueue the message
            else {
                //control plane message
                if (check_and_cast<Packet *>(msg) -> getPacketType() == leader_to_roots ||  check_and_cast<Packet *>(msg) -> getPacketType() == route_from_root ||check_and_cast<Packet *>(msg) -> getPacketType() == phase2_link_info)
                {
                    msg->setTimestamp();
                    priorityQueue.insert(msg);
                }
                else
                {
                   EV<<endl<<endl << "Received " << msg << " but transmitter busy: queueing up.SIZE = "<<queue.getLength()<<endl<<endl;
                   msg->setTimestamp();
                   queue.insert(msg);
                   if ( load_balance_mode )
                   {
                       check_and_cast<cPacket *>(msg) -> setBitLength((int64_t) (1500*8));
                       queuesizesignal = queue.getLength() + 1;
                       char *sig  = signalDeParser(queuesizesignal,myAddress);
                       if(hasListeners(LBS))
                           emit(LBS,(const char*)sig);
                       free(sig);
                   }
                   queueSizeVector.record((long)queue.getLength());
                   emit(qlenSignal, queue.getLength());
                }

            }
        }

        else {
            // We are idle, so we can start transmitting right away.
            EV << "Received " << msg << endl;
            emit(queueingTimeSignal, SIMTIME_ZERO);
            startTransmitting(msg);
            emit(busySignal, true);
        }
    }
}

void L2Queue::refreshDisplay() const
{
    getDisplayString().setTagArg("t", 0, isBusy ? "transmitting" : "idle");
    getDisplayString().setTagArg("i", 1, isBusy ? (queue.getLength() >= 3 ? "red" : "yellow") : "");
}

