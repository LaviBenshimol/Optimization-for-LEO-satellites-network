#include "App.h"
App::App()
{

}

App::~App()
{
    cancelAndDelete(generatePacket);
}

void App::sendPong( int src, int pingFlag )
{
    Packet * pongMessage = new Packet("regular message");
    pongMessage -> setSrcAddr(myAddress);
    pongMessage -> setPacketType( message );
    pongMessage -> setBitLength((int64_t) 12000); //1500Bytes as Ethernet protocol MTU suggest
    pongMessage -> setHopCount(0);
    pongMessage->setDestAddr(src);
    pongMessage->setTopologyID(currTopoID);
    int k = 0;
    while( k <  num_of_hosts )
    {
       pongMessage -> data[k] = -1;
       pongMessage -> datadouble[k] = -1;
       k++;
    }
    pongMessage -> datadouble[0] = -2;
    pongMessage -> datadouble[num_of_hosts-1] = ping_pong;
    pongMessage -> datadouble[num_of_hosts-2] = pingFlag;
    numSent++;//
   // int timeInterval = 0;
   send(pongMessage, "out");
}
void App::sendPing()
{
    for( int i = 0; i < num_of_hosts; i++ )
    {
        if ( i != myAddress  )
        {
            Packet * pingMessage = new Packet("regular message");
            pingMessage -> setSrcAddr(myAddress);
            pingMessage -> setPacketType( message );
            pingMessage -> setBitLength((int64_t) 12000);//1500Bytes as Ethernet protocol MTU suggest
            pingMessage -> setHopCount(0);
            pingMessage->setDestAddr(i);
            pingMessage->setTopologyID(currTopoID);
            int k = 0;
            while( k <  num_of_hosts )
            {
               pingMessage -> data[k] = -1;
               pingMessage -> datadouble[k] = -1;
               k++;
            }
            pingMessage -> datadouble[0] = -2;
            pingMessage -> datadouble[num_of_hosts-1] = ping_ping;
            pingMessage -> datadouble[num_of_hosts-2] = pingIndex;
            numSent++;//
            // int timeInterval = 0;
            send(pingMessage, "out");
        }
   }
}

void App::recordping( int src ,simtime_t_cref creationTime )
{
    simtime_t latancy = simTime() - creationTime;

    pingAll[src].record(latancy);
  //  int b;
}
void App::sendMessage()
{
    volatile int j=intuniform(0,num_of_hosts-1,0);//(int)simTime().dbl()*getSimulation()->getEventNumber()%117);
    while ( j == myAddress )
        j = intuniform(0,num_of_hosts -1,0);//(int)simTime().dbl()*getSimulation()->getEventNumber()%117);
    Packet * regularmessage = new Packet("regular message");
    regularmessage -> setSrcAddr(myAddress);
    regularmessage->setDestAddr(j);
    regularmessage -> setPacketType(message);
    regularmessage -> setBitLength((int64_t) 12000);//1500Bytes as Ethernet protocol MTU suggest
    regularmessage -> setHopCount(0);

    regularmessage->setTopologyID(currTopoID);
    int i=0;
    while( i <  num_of_hosts )
    {
      regularmessage->data[i] = -1;
      regularmessage->datadouble[i] = -1;
      i++;
    }
    regularmessage -> datadouble[0] = -2;

    send(regularmessage, "out");
}
void App::sendFlowMessage(int pivotRoundRobin)
{
    Packet * regularmessage = new Packet("regular message - flow");
    regularmessage -> setSrcAddr(myAddress);
    regularmessage->setDestAddr(pivotRoundRobin);
    regularmessage -> setPacketType(message);
    regularmessage -> setBitLength((int64_t) 12000);//1500Bytes as Ethernet protocol MTU suggest
    regularmessage -> setHopCount(0);

    regularmessage->setTopologyID(currTopoID);
    int i=0;
    while( i <  num_of_hosts )
    {
     regularmessage->data[i] = -1;
     regularmessage->datadouble[i] = -1;
     i++;
    }
    regularmessage -> datadouble[0] = -2;

    send(regularmessage, "out");
}
void App::createFlow( int pivotRoundRobin )
{
    Packet * createBurst = new Packet("Burst App");
    createBurst -> setPacketType( burst_app );
    createBurst -> data[0] = pivotRoundRobin;
    createBurst -> datadouble[0] = burstSize -> operator int();
    scheduleAt(simTime() + burstNextEvent->doubleValue() ,createBurst);
}

//lll New Code
///////////////////////////////////////////////////////

                // lllinitialize

///////////////////////////////////////////////////////
void App::initialize()
{
    myAddress = getParentModule()->par("address");//par("address");
    num_of_hosts = getParentModule()->par("num_of_satellite");
    packetLengthBytes = &par("packetLength");
    sendIATime = &par("sendIaTime");  // volatile parameter
    fullTopoRun = true;// getParentModule()->par("fullTopoRun");
    burstNextInterval = &par("burst_next_interval");
    burstNextEvent = &par("burst_next_event");
    burstSize = &par("burst_size");
    pkCounter = 0;
    WATCH(pkCounter);
    WATCH(myAddress);
    //record
    numSent=0;
    numReceived=0;
    WATCH(numSent);
    WATCH(numReceived);
    hopCountVector.setName("hop count");
    num_of_hosts = 25;
    for(int i=0;i<num_of_hosts;i++){
        char pingdest[80];
        sprintf(pingdest, "Latency with %d", i);
        pingAll[i].setName(pingdest);
        pingAll[i].setUnit("ms");
        ping_table[i] = 0;
        sendflow[i]=0;
        flowincoming[i] = false;
    }

    endToEndDelaySignal = registerSignal("endToEndDelay");
    hopCountSignal = registerSignal("hopCount");
    sourceAddressSignal = registerSignal("sourceAddress");
}
///////////////////////////////////////////////////////

                // lllhandlemessage

///////////////////////////////////////////////////////
void App::handleMessage(cMessage *msg)
{

    Packet *pk = check_and_cast<Packet *>(msg);
    int packetType =pk->getPacketType();
    //delete pk;
    switch( packetType )
    {
        case app_initial:
        {
//            int a= myAddress;
//            Routing *rootisrouter = check_and_cast<Routing*>(getParentModule()->getSubmodule("routing"));
//            if(rootisrouter -> isRoot())
//            {
//                delete pk;
//                break;
//            }
            Packet * createMessage = new Packet("self message. create more messages");
            createMessage -> setPacketType(create_message);
            scheduleAt(simTime() + sendIATime -> doubleValue() ,createMessage);

            //activate ping_app
            Packet * createPing = new Packet("Ping App");
            createPing -> setPacketType(ping_app);
            createPing -> setHopCount(0);
            scheduleAt(simTime() + exponential(0.1) ,createPing);

            //activate burst_flow
            Packet * createBurst = new Packet("burst_flow");
            createBurst -> setPacketType( burst_flow );
            scheduleAt(simTime() + exponential(0.1) ,createBurst);
            delete pk;
            //TODO : timeInterval ?
                //int timeInterval = 1;
            //dropAndDelete(pk);
        }break;
        case create_message :
        {
            pkCounter++;
            sendMessage();
            numSent++;//
           // int timeInterval = 0;
            Packet * createMessage = new Packet("self message. create more messages");
            createMessage -> setPacketType(create_message);
            scheduleAt(simTime() + sendIATime->doubleValue() ,createMessage);
            delete pk;

        }break;
        case message:
        {
//            if(hasGUI())
//                this->getParentModule()->bubble("recived from ");
         //   simtime_t check = simTime();
            int a =1,b=2;
            if( pk -> datadouble[num_of_hosts-1] == ping_ping )
            {

              //  sendPong( pk -> getSrcAddr() , pk ->datadouble[23] );
                a =b;
                recordping( pk -> getSrcAddr() , pk -> getCreationTime() );
                a++;
                if(hasGUI())
                {
                    this->getParentModule() -> bubble("Ping!!!");
                }
            }
            int hopcount = pk->getHopCount();
            //TODO: message recived - record ?
            hopCountVector.record(hopcount);
            hopCountStats.collect(hopcount);
            numReceived++;
            delete pk;
        }break;
        case ping_app:
        {
            //first time
            int a=0;

            if( pk->getHopCount() == 0 )
            {
                a=1;
                currentPingStartTime = simTime();
                sendPing();
                pk->setHopCount(pk->getHopCount()+1);
                a=1;
                delete pk;
                Packet * createPing = new Packet("Ping App");
                createPing -> setPacketType(ping_app);
                createPing -> setHopCount(pingIndex);
                scheduleAt(simTime() + truncnormal(3,0.1) ,createPing);
                a=1;
            }
            else
            {
                //recordping( pk -> getSrcAddr() );
                //clean
                for(int i=0;i<num_of_hosts;i++)ping_table[i]=0;
                pingIndex ++;
                a=1;
                currentPingStartTime = simTime();
                sendPing();
                delete pk;
                Packet * createPing = new Packet("Ping App");
                createPing -> setPacketType(ping_app);
                createPing -> setHopCount(pingIndex);
                scheduleAt(simTime() + exponential(3) ,createPing);
                a=1;
            }



        }break;
        case burst_app:
        {
            sendflow[pk->data[0]] = (int)pk->datadouble[0];
            flowincoming[pk->data[0]] = true;
            delete pk;
        }break;
        case burst_flow:
        {
            int loop = 0;
            while ( loop < NUMOFNODES )
            {
                if(sendflow[pivotRoundRobin] == 0 && !flowincoming[pivotRoundRobin])
                {
                  createFlow(pivotRoundRobin);
                }
                else if(sendflow[pivotRoundRobin] > 0)
                {
                   sendFlowMessage(pivotRoundRobin);
                   sendflow[pivotRoundRobin]--;
                   if(sendflow[pivotRoundRobin] == 0)
                       flowincoming[pivotRoundRobin] = false;
                   break;
                }
                pivotRoundRobin++;
                pivotRoundRobin = pivotRoundRobin % (NUMOFNODES-1);
                loop++;
            }
            pivotRoundRobin++;
            pivotRoundRobin = pivotRoundRobin % (NUMOFNODES-1);
            scheduleAt( simTime() + burstNextInterval->doubleValue(),pk);
        }break;
    }

//lll New Code
    }//handle message

void App:: finish()
{
    recordScalar("#sent", numSent);
    recordScalar("#received", numReceived);

    hopCountStats.recordAs("hop count");
//    for(int i = 0; i < 25; i++)
//    {
//        std::string s = "ping to " + std::to_string(i);
//        pingAll[i].recordAs(s);
//    }
}
