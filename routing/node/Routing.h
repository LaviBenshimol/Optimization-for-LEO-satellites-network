#include <map>
#include <omnetpp.h>
#include <string>
#include <math.h>
#include <algorithm>
#include <iterator>
#include <vector>
#include <iostream>
#include "L2Queue.h"


// #include "cobject.h"
//lll: max number of links availble
#define MAXAMOUNTOFLINKS 6
//lll: weight range. not in use
#define MAX_VALUE 10000
#define MIN_VALUE 0
//lll: does not appear in the code at all
#define PARAM 2
//lll: define the root node always with id 0
#define ROOTNODE 0
//lll: Broadcast address : 255
#define BROADCAST 255
//lll: debug mode (printing logs on the fly)
#define DEBUG 1
//MTU defined as 1 mb bytes
#define MTUMESSAGE 1048576

#define HISTORY_LENGTH 876          // 35s per topo. 5 topologies. 5 samples per second => 5*5*35=875. + 1
#define ALPHA_P 300                 //alpha predict
#define BETA_P 7000                 //total errors predict
#define PATTERN_LENGTH 35           //7 seconds. 5 samples per second
#define NUMOFNODES 25
#define LOAD_PREDICTION_INTERVAL 14  //search and predict the load every 5sec
#define BASELINE_MODE 0
using namespace omnetpp;
using namespace std;
enum direction_list {
    left_d =0,
    up_d = 1,
    right_d = 2,
    down_d = 3,
    diagonal_up = 4,
    diagonal_down = 5,
};
enum packet_type_list {
    initiale_packet = 1,
    regular_packet = 2,
    schedule_topo_packet = 3,
    send_node_info_packet = 4,
    update_app_packet = 10,
};



class Routing: public cSimpleModule , public cListener
{
public:
    int leader = -1;
    int rootID = -1;

    bool isRoot();
    int neighbors[MAXAMOUNTOFLINKS];
private:
    int myAddress;
    /*Flag to determine if a node has the updated topology. otherwise, it might send messages.
     *  through links that should not be exist.*/
    int hasInitTopo;
    int packetType;
    int currTopoID = 0;
    int rootID_current_topo_ID = 0;
    bool saveTempLinkFlag = false;
    bool keepAliveTempLinksFlag = false;
    int topocounter = 0;
    bool fiftLink = true;
    int packetCounter = 0;
    //lll: LAST variables predefined variable ( getParentModule()->par("variable"); )
    int alpha = 0;
    int m_var = 0;
    //lll: changing topology by predefined-time interval(getParentModule()->par("variable");
    int changeRate = 0;
    int firstNodeInfo = 0;
    int NodesInTopo = 0;
    int updateCounter = 0;
    bool fullTopoRun = true;
    //int num_of_hosts = getParentModule()->par("numofsatellite");
    int num_of_hosts = NUMOFNODES;

    int phase1Flag;
    int leader_table[NUMOFNODES];
    double leader_list[NUMOFNODES];

    int debugCounter=0;

    double n_table[NUMOFNODES][NUMOFNODES];
    int rootCounter=0;
    //int rootNodesArr[NUMOFNODES]={-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1};//25-5/5
    int rootNodesArr[NUMOFNODES]={-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1};//25-5/5
    int loadHistory[MAXAMOUNTOFLINKS][HISTORY_LENGTH];
    int loadTimerIndex;
    bool loadPredictionIsOn;
    bool loadPredictionFlag[MAXAMOUNTOFLINKS];
    int path[NUMOFNODES];
    int pathID;
    int pathWeight;
    int pathcounter;
    int tempActiveLinks [MAXAMOUNTOFLINKS];
    long queueSize[MAXAMOUNTOFLINKS] = {-1,-1,-1,-1,-1,-1};
    long frameCapacity;
    int packetLength;
    double rootNodesPathWeight[4] {-1,-1,-1,-1};
    cPar *_sendTime;  // volatile parameter

    int load_threshold;// = (600*12000);//600 out of 666.66667
    double loadCounter[MAXAMOUNTOFLINKS] = {0,0,0,0,0,0};
    int active_links_array[MAXAMOUNTOFLINKS] = {0,0,0,0,0,0};
    bool loadCounterFlag;
    bool load_balance_mode;
    bool load_balance_link_prediction;
    bool second_update = false;
    bool second_update_leader = false;
    bool app_is_on = false;
    double datarate;
    int active_links = 0;
//    // packetType 2 => Normal mode. These are regular packets generated from App.cc that will ride on the updated topology.
    Packet *_pk = NULL;
    int linkdatacounter = 0;
    typedef std::map<int, int> RoutingTable;  // destaddr -> gateindex
    RoutingTable rtable;
    int* linkCounter;
    typedef std::map<int, int> DiameterTable; // curAddr -> nextHop
    DiameterTable diameter;
    typedef std::list<cTopology::LinkIn*> LinkIns;
    LinkIns edges;
    typedef std::list<cTopology::Node*> NodesList;
    NodesList Nodes;

    cTopology::Node *thisNode;
    cTopology *topo;
    cTopology *baseSPT;
    cTopology *newTopo;


    simsignal_t dropSignal;
    simsignal_t outputIfSignal;
    simsignal_t TopoLinkCounter;
    simsignal_t hopCountSignal;
    simtime_t delayTime;

    cModule *links_control[MAXAMOUNTOFLINKS];
    simtime_t queueschannel;
    simsignal_t  LBS;//load balance signal


    cOutVector loadPrediction[4];

protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;

    void setLinksWeight(cTopology *topo, double wgt);
    void disableAllInLinks(cTopology *topo, bool debugging);
    void insertLinksToList(cTopology *topo);
    void checkVerticesConnectivity(cTopology *topo, int address);
    double findWeightOfShortestPath(cTopology *topo, cTopology::Node* src,
            cTopology::Node* dest);
    double getPuv(cTopology *topo, cTopology::LinkIn* edge);
    //lll: not in use
    bool checkIfInWeightRange(double wgt);
    int printEdgesList();
    cTopology::Node* findFurthestNodeInGraph(cTopology *topo,cTopology::Node *destNode);
    cTopology * primMST(cTopology *OrigTopo, int RootNodeInd);
    // int printMST(int parent[], cTopology *MSTTopo);
    int minKey(int key[], bool mstSet[], int NodeCount);
    int traverseDiameter(cTopology *topo, cTopology::Node *source,cTopology::Node *dest);
    void printNodesList();
    void paintRouteInColor(const char* color, int width);
    void copyTopology(cTopology* OrigTopo, cTopology* NewTopo,const char* colour);
    void PaintTopology(cTopology* OrigTopo, const char* colour);
    int getSatIndex(cTopology::Node *node);
    void LoadPacket(Packet *pk, cTopology *topo);
    void initRtable(cTopology* thisTopo, cTopology::Node *thisNode);
    int GetW(cTopology::Node *nodeU, cTopology::Node *nodeV);
    cTopology* CreateTopology(cTopology *Origtopo, const char* topoName);
    cTopology* BuildSPT(cTopology* origTopo, int RootIndex);
    void DFS(int NodeIndex, int RootIndex, cTopology* origTopo,
            cTopology* MSTTopo, cTopology* SPTTopo, int alpha, int parent[],int distance[],int color[]);
    void RelaxTopo(int ChildNodeIndex, int ParentNodeIndex, cTopology* origTopo,int parent[], int distance[]);
    int* FindLASTForRoot(cTopology* origTopo,int alpha, int RootIndex, int* parent);
    void AddPathToLAST(int NodeIndex, cTopology* origTopo, cTopology* SPTTopo,int parent[], int distance[]);
    cTopology* BuildTopoFromArray(int parent[], int nodesCount,cTopology* OrigTopo);
    cTopology* BuildTopoFromMultiArray(int** parentArrays, int ArrayCount,cTopology* OrigTopo);
    cTopology* BuildLASTtopo(cTopology* origTopo,int alpha,int m);
    int findDiameter(cTopology* currTopo, int startPoint);
    void changeConnectingLink(cTopology::Node* currNode, int otherNodeInd,bool state);
    cTopology::Node* nexNode(list<cTopology::Node*>::iterator it);
    void ScheduleNewTopoPacket();
    void setDualSideWeights(cTopology *MyTopo, int Node1ID, double wgt);
    void setDualSideWeightsByQueueSize(cTopology *MyTopo, int Node1ID);
    void verifyWeightsForTopo();
    void addWeightsToTopo(Packet* pk);
    void addSelfWeightsToTopo();
    void SendDuplicateTopology(Packet *pk, int destNodeInd);
    int getPathID();

    // dist leader choosing

    void scheduleChoosingLeaderPhaseOne();
    void scheduleSendLinkInfo();
    void schduleNewTopology();
    void schduleTopologyUpdate();
    void send_phase1_update();
    void initial_leader_table();
    void update_leader_table(Packet *pk);
    void send_phase2_update();
    void initial_leader_list();
    void setNeighborsIndex();
    int findDest(int direction,int index);
    void saveTempLinks();
    void keepAliveTempLinks();





    // Queue functions

    void flushQueue(int type);

    // LAST_Topology

    void update_weight_table();
    void send_link_info_to_leader();
    int getNumOfLinks();
    void send_link_information_to_leader();
  //  void send_build_LAST_to_choosen_roots(cTopology* topology, int root_table[], int arrayLength );
    void setMaxLeaderAmount(int amountofleaders);
    int getMaxLeaderAmount();

    bool isPathSet(Packet *pk);
    void setPathToRoot(Packet *pk);

    int getLinkIndex(int req);
    //int getlinkindex(int req);
    int getNeighboorIndex(int req);
    void setActiveLinks();
    void sendMessage(Packet *pk,int outGateIndex);
    int loadBalanceLinkFinder( int reqLink ,int incominglink,Packet *pk );
    int loadBalanceLinkPrediction(int reqLink,int incominglink);
    int loadBalance(int reqLink,int incominglink);
    int loadBalance(int reqLink);
    bool loadbalnceisbetterlink( int reqLink );
    void debughelp(); // help to stop in specifig id
    void signalStringParser(const char *s,int* queueid,uint32_t* load,int* connectedto,int *queuesize);
    virtual void receiveSignal(cComponent *src, simsignal_t id, cObject *value,cObject *details)override;
    virtual void receiveSignal(cComponent *source, simsignal_t signalID, const char *s, cObject *details) override;
    void updateLoadHistory();
    void link_weight_update();
    void reset_ntable();
    void LinkLoadPrediction();
    void updateLoadBalancePrediction(int index,int loadPredicted);


    void schduleBaseLine();
    int totalErrors(int *pattern,int *history,int offset,int size_P);
    void pi_func(int pattern[], int alpha,int size,int pi[]);
    double search(int *T,int *P,int alpha,int beta, int pattern_size, int History_size);

    int* getNeighborsArr(int index,int* arr);
    //void finish() override;
};

Define_Module(Routing);


