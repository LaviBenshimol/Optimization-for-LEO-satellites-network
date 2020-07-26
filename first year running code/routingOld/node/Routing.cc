//
// This file is part of an OMNeT++/OMNEST simulation example.
//
// Copyright (C) 1992-2015 Andras Varga
//
// This file is distributed WITHOUT ANY WARRANTY. See the file
// `license' for details on this and other legal matters.
//

//
//
///
            //OLD
//
////
//
//
//
//
//
//
//
#ifdef _MSC_VER
#pragma warning(disable:4786)
#endif

#include <map>
#include <omnetpp.h>
#include "Packet_m.h"

#include <algorithm>
#include <iterator>
#include <vector>
#include <iostream>

//lll: weight range. not in use
#define MAX_VALUE 10000
#define MIN_VALUE 0
//lll: does not appear in the code at all
#define PARAM 2

//lll: define the root node always with id 0
#define ROOTNODE 0

//lll: debug mode (printing logs on the fly)
#define DEBUG 0

using namespace omnetpp;
using namespace std;

enum packet_type_list
{
    initiale_packet = 1,
    regular_packet = 2,
    schedule_topo_packet = 3 ,
    send_node_info_packet = 4,
    update_app_packet = 10,
};

/**
 * Demonstrates static routing, utilizing the cTopology class.
 */

class Routing : public cSimpleModule
{
private:
    int myAddress;
    /*Flag to determine if a node has the updated topology. otherwise, it might send messages.
     *  through links that should not be exist.*/
    int hasInitTopo;
    //packetType 1 => Init mode. A command to distribute the topology among all nodes.
    int packetType;
    int currTopoID = 0;
    //lll: not in use
    int firstTime = 0;
    int packetCounter = 0;
    //lll: LAST variables predefined variable ( getParentModule()->par("variable"); )
    int alpha = 0;
    int m_var = 0;
    //lll: changing topology by predefined-time interval(getParentModule()->par("variable");
    int changeRate = 0;
    int firstNodeInfo = 0;
    int NodesInTopo = 0;
    int updateCounter = 0;
    bool fullTopoRun=true;


    cPar *_sendTime;  // volatile parameter
    // packetType 2 => Normal mode. These are regular packets generated from App.cc that will ride on the updated topology.
    Packet *_pk = NULL;

    typedef std::map<int, int> RoutingTable;  // destaddr -> gateindex
    RoutingTable rtable;
    int* linkCounter;
    typedef std::map<int,int> DiameterTable; // curAddr -> nextHop
    DiameterTable diameter;
    typedef std::list<cTopology::LinkIn*> LinkIns;
    LinkIns edges;
    typedef std::list<cTopology::Node*> NodesList;
    NodesList Nodes;

    cTopology::Node *thisNode;
    cTopology *topo;
    cTopology *newTopo;
    simsignal_t dropSignal;
    simsignal_t outputIfSignal;
    simsignal_t TopoLinkCounter;
    simsignal_t hopCountSignal;

protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;


    void setLinksWeight(cTopology *topo, double wgt);
    void disableAllInLinks(cTopology *topo, bool debugging);
    void insertLinksToList(cTopology *topo);
    void checkVerticesConnectivity(cTopology *topo, int address);
    double findWeightOfShortestPath(cTopology *topo,cTopology::Node* src, cTopology::Node* dest);
    double getPuv(cTopology *topo, cTopology::LinkIn* edge);
    //lll: not in use
    bool checkIfInWeightRange(double wgt);
    int printEdgesList();
    cTopology::Node* findFurthestNodeInGraph(cTopology *topo, cTopology::Node *destNode);
    cTopology * primMST(cTopology *OrigTopo, int RootNodeInd);
    // int printMST(int parent[], cTopology *MSTTopo);
    int minKey(int key[], bool mstSet[],int NodeCount);
    int traverseDiameter(cTopology *topo, cTopology::Node *source, cTopology::Node *dest);
    void printNodesList();
    void paintRouteInColor(const char* color, int width);
    void copyTopology(cTopology* OrigTopo,cTopology* NewTopo, const char* colour);
    void PaintTopology(cTopology* OrigTopo, const char* colour);
    int getSatIndex(cTopology::Node *node);
    void LoadPacket(Packet *pk, cTopology *topo);
    void initRtable(cTopology* thisTopo, cTopology::Node *thisNode);
    int GetW(cTopology::Node *nodeU, cTopology::Node *nodeV);
    cTopology* CreateTopology(cTopology *Origtopo);
    cTopology* BuildSPT(cTopology* origTopo, int RootIndex);
    void DFS(int NodeIndex,int RootIndex,  cTopology* origTopo, cTopology* MSTTopo, cTopology* SPTTopo, int alpha, int parent[], int distance[]);
    void RelaxTopo(int ChildNodeIndex,int ParentNodeIndex,cTopology* origTopo,  int parent[], int distance[]);
    int* FindLASTForRoot(cTopology* origTopo,int alpha, int RootIndex, int* parent);
    void AddPathToLAST(int NodeIndex, cTopology* origTopo,cTopology* SPTTopo, int parent[], int distance[]);
    cTopology* BuildTopoFromArray(int parent[], int nodesCount,cTopology* OrigTopo);
    cTopology* BuildTopoFromMultiArray(int** parentArrays, int ArrayCount,cTopology* OrigTopo);
    cTopology* BuildLASTtopo(cTopology* origTopo,int alpha,int m);
    int findDiameter(cTopology* currTopo, int startPoint);
    void changeConnectingLink(cTopology::Node* currNode, int otherNodeInd,bool state);
    cTopology::Node* nexNode(list<cTopology::Node*>::iterator it);
    void ScheduleNewTopoPacket();
    void ScheduleChangeTopo();
    void CreateInitTopology(cTopology *myTopo);
    void setDualSideWeights(cTopology *MyTopo, int Node1ID, double wgt);
    void setDualSideWeightsBySize(cTopology *MyTopo, int Node1ID);
    void verifyWeightsForTopo();
    void addWeightsToTopo(Packet* pk);
    void addSelfWeightsToTopo();
    void SendReportToRoot();
    void ScheduleReportTrigger();
    void SendDuplicateTopology(Packet *pk, int destNodeInd);
};

Define_Module(Routing);


//This function is not working correctly. The problem seems to be with the method getNode(int i)
void Routing::insertLinksToList(cTopology *topo)
{
    int numInLinks = 0;

    for (int i=0; i<topo->getNumNodes(); i++)
    {
        numInLinks = 0;
        numInLinks = topo->getNode(i)->getNumInLinks();
        for(int j=0; j<numInLinks; j++)
        {
            edges.push_back(topo->getNode(i)->getLinkIn(j));
        }
    }
}


/*
 * Args: None
 * Return: Void
 * Description: Prints out the source and destination of each LinkIn and its weight.
 */
int Routing::printEdgesList()
{
    std::list<cTopology::LinkIn*>::iterator it;
    int source_node = 0;
    int dest_node = 0;
    int enabledCounter = 0;
    double wgt = 0;

    for(it = edges.begin() ; it != edges.end() ; it++)
    {
        source_node = (*it)->getRemoteNode()->getModule()->par("address");
        dest_node = (*it)->getLocalNode()->getModule()->par("address");
        wgt = (*it)->getWeight();
        if ((*it)->isEnabled() )
        {
            enabledCounter +=1;
        }
        if (DEBUG){
            EV << "[+] printEdgesList:: " << "Source Node: " << source_node << " Dest Node: " << dest_node <<
                    " Weight: " << wgt << " Enabled:" << (*it)->isEnabled() << endl;
        }
    }
    return enabledCounter ;
}


/*
 * Pass it pk = new cPacket();
 */
void Routing::LoadPacket(Packet *pk, cTopology *topo)
{
    // Create a new topology, extracted from Ned types (e.g. Net60)
    cTopology *tempTopo = new cTopology("topo");
    std::vector<std::string> nedTypes;
    nedTypes.push_back(getParentModule()->getNedTypeName());
    tempTopo->extractByNedTypeName(nedTypes);

    // Disable all links to have a fresh start
    Routing::disableAllInLinks(tempTopo, false);

    // Copy topology to send via packet (topo) to a new instance (tempTopo)
    Routing::copyTopology(tempTopo, topo, "green");

    // Packets for dispatching the topology will be identified as INIT packets
    pk->setName("INIT");
    // pk->setByteLength(l) // TODO
    pk->setTopologyVar(tempTopo);
}


void Routing::printNodesList()
{
    std::list<cTopology::Node*>::iterator it;
    int temp;

    for(it = Nodes.begin() ; it != Nodes.end() ; it++)
    {
        //temp = (*it)->getModule()->par("address");
        temp = (*it)->getModule()->getIndex();

        EV << "[+] printNodesList:: " << temp << endl;
    }
}


/*
 * Args: cTopology variable
 * Return: Void
 * Description: Iterates through each LinkIn in each Node and disable it. For 'calculateWeightedSingleShortestPathsTo', when
 * a LinkIn is disabled, it is "not connected". By doing that, we achieve a graph G'={}, although, all vertices and all
 * edges are still there. Later on, each edge that will be added, will be simply turned back with "enable()" method.
 */
void Routing::disableAllInLinks(cTopology *topo, bool debugging)
{
    int numInLinks = 0;

    for (int i=0; i<topo->getNumNodes(); i++)
    {
        numInLinks = 0;
        numInLinks = topo->getNode(i)->getNumInLinks();
        for(int j=0; j<numInLinks; j++)
        {
            if (debugging)
                topo->getNode(i)->getLinkIn(j)->setWeight(-1);
            else{
                topo->getNode(i)->getLinkIn(j)->disable();
                cDisplayString& dispStr = topo->getNode(i)->getLinkIn(j)->getRemoteGate()->getDisplayString();
                dispStr.setTagArg("ls", 0, "red");
                dispStr.setTagArg("ls",1,"0");
            }

        }
    }
}


/*
 * Args: cTopology variable, address of a node
 * Return: Void
 * Description: It verifies that disabling all LinkIns (edges) with 'disableAllInLinks' actually causes Nodes to be disconnected.
 * Verification is using the Dijkstra function 'calculateWeightedSingleShortestPathsTo'.
 * When a node is not connected, its 'getNumPaths' method will not be set, therefore will be NULL.
 */
void Routing::checkVerticesConnectivity(cTopology *topo, int address)
{
    int adrs = 0;

    EV << "[+] checkVerticesConnectivity:: Address is " << address << endl;

    for (int i=0; i<topo->getNumNodes(); i++)
    {
        if(i==address) continue;

        topo->calculateWeightedSingleShortestPathsTo(topo->getNode(i));

        adrs = topo->getNode(i)->getModule()->par("address");

        if (topo->getNode(i)->getNumPaths()==0)
            EV << "[+] checkVerticesConnectivity:: Node " << adrs << " is not connected" << endl;
        else
            EV << "[+] checkVerticesConnectivity:: Node " << adrs << " is connected" << endl;
    }
}


/*
 * Args: cTopology topo, source Node src and destination Node dest
 * Return: distance of shortest path from source to destination
 * Description: Find P(u,v) where u= Node* src and v= Node* dest.
 *                              Using calculateWeightedSingleShortestPathsTo function
 */
double Routing::findWeightOfShortestPath(cTopology *topo,cTopology::Node* src, cTopology::Node* dest)
{

    double source_dist = 0;

    topo->calculateWeightedSingleShortestPathsTo(dest);

    source_dist = src->getDistanceToTarget();

    if (DEBUG) {
        EV << "[+] findWeightOfShortestPath:: Distance from source to dest is: " << source_dist << endl;
    }

    return source_dist;
}


void Routing::setDualSideWeights(cTopology *MyTopo, int Node1ID, double wgt){
    int Node2ID =0;
    int numOfLinks, numInLinks, N2numInLinks;
    numInLinks = MyTopo->getNode(Node1ID)->getNumInLinks();
    cTopology::Node* tempNode = MyTopo->getNode(Node1ID);
    cTopology::Node*  N2tempNode ;

    for(int j=0; j<numInLinks; j++)
    {
        Node2ID = MyTopo->getNode(Node1ID)->getLinkIn(j)->getRemoteNode()->getModule()->getIndex();
        if (Node2ID >= 0){
            if(MyTopo->getNode(Node1ID)->getLinkIn(j)->getWeight() == -1)
                MyTopo->getNode(Node1ID)->getLinkIn(j)->setWeight(wgt);
        }

        N2tempNode = MyTopo->getNode(Node2ID);
        N2numInLinks = N2tempNode ->getNumInLinks();
        for (int v=0; v< N2numInLinks; v++)
        {
            if(N2tempNode->getLinkIn(v)->getRemoteNode()->getModule()->getIndex() == Node1ID
                    && MyTopo->getNode(Node2ID)->getLinkIn(v)->getWeight() == -1)
                MyTopo->getNode(Node2ID)->getLinkIn(v)->setWeight(wgt);
        }
    }
}

void Routing::setDualSideWeightsBySize(cTopology *MyTopo, int Node1ID){
    int Node2ID =0;
    int N1numInLinks, N2numInLinks;
    int wgt, Node1Weight, Node2Weight;

    cTopology::Node* tempNode;
    cTopology::Node* N2tempNode;

    tempNode = MyTopo->getNode(Node1ID);

    N1numInLinks = tempNode->getNumInLinks();

    for(int j=0; j<N1numInLinks; j++)
    {
        Node1Weight = tempNode->getLinkIn(j)->getWeight();
        N2tempNode = MyTopo->getNode(Node1ID)->getLinkIn(j)->getRemoteNode();
        Node2ID = N2tempNode->getModule()->getIndex();

        if (Node2ID >= 0){
            for (int i=0; i< N2tempNode->getNumInLinks(); i++){
                if(N2tempNode->getLinkIn(i)->getRemoteNode()->getModule()->getIndex() == Node1ID){
                    Node2Weight = N2tempNode->getLinkIn(i)->getWeight();

                    if(Node1Weight > Node2Weight ){
                        N2tempNode->getLinkIn(i)->setWeight(Node1Weight);
                    }
                    else{
                        tempNode->getLinkIn(j)->setWeight(Node2Weight);
                    }
                }
            }
        }
    }
}

void Routing::verifyWeightsForTopo(){
    int Node2ID =0;
    int numOflinks;
    cTopology::Node * tempNode;
    for( int i=0; i <newTopo->getNumNodes(); i++)
    {
        tempNode = newTopo->getNode(i);
        setDualSideWeightsBySize(newTopo, tempNode->getModule()->getIndex());
    }
}

/*
 * Args: cTopology variable, weight
 * Return: Void
 * Description: Set weights to LinkIn objects (edges), since this is the actual object that counts when computing
 * 'calculateWeightedSingleShortestPathsTo' (Disjktra Algorithm)
 */
void Routing::setLinksWeight(cTopology *MyTopo, double wgt)
{
    int numInLinks = 0, Node2ID, Node1ID, numOfLinks;
    cTopology::Node* tempNode;
    srand(time(NULL));
    switch((int)wgt)
    {
    case 0: // random

        for (int i=0; i<MyTopo->getNumNodes(); i++)
        {
            wgt = rand() % 100 + 1; // 1 to 100

            Node1ID = i;
            setDualSideWeights(MyTopo, Node1ID, wgt);

        }
        break;

    case 1: //all ones - distributed
        for (int i=0; i<MyTopo->getNumNodes(); i++)
        {
            numInLinks = 0;
            numInLinks = MyTopo->getNode(i)->getNumInLinks();
            for(int j=0; j<numInLinks; j++)
            {
                MyTopo->getNode(i)->getLinkIn(j)->setWeight(1);
            }
        }
        break;

    case 2: //predefined const values - for debugging

        for (int i=0; i<MyTopo->getNumNodes(); i++)
        {
            numInLinks = 0;
            numInLinks = MyTopo->getNode(i)->getNumInLinks();
            Node1ID = i;
            for(int j=0; j<numInLinks; j++)
            {
                Node2ID = MyTopo->getNode(i)->getLinkIn(j)->getRemoteNode()->getModule()->getIndex();
                if (Node2ID >= 0){
                    tempNode = MyTopo->getNode(Node1ID);
                }
                wgt = i*j;
                if(wgt==0)
                    wgt = 1;
                MyTopo->getNode(i)->getLinkIn(j)->setWeight(wgt);
            }
        }
        break;

    case 8:

        for (int i=0; i<topo->getNumNodes(); i++)
        {
            numInLinks = 0;
            numInLinks = topo->getNode(i)->getNumInLinks();

            Node1ID = i;
            for(int j=0; j<numInLinks; j++)
            {
                Node2ID = topo->getNode(i)->getLinkIn(j)->getRemoteNode()->getModule()->getIndex();
                switch(Node1ID)
                {
                case 0:
                    switch (Node2ID)
                    {
                    case 1:
                        wgt=10;
                        break;
                    case 3:
                        wgt=15;
                        break;
                    case 4:
                        wgt=15;
                        break;
                    case 5:
                        wgt=15;
                        break;
                    case 7:
                        wgt=5;
                        break;
                    }
                    break;
                    case 1:
                        if (Node2ID == 2)
                            wgt=10;
                        break;
                    case 2:
                        if (Node2ID == 3)
                            wgt=10;
                        break;
                    case 3:
                        if (Node2ID == 4)
                            wgt=10;
                        else if (Node2ID == 5)
                            wgt=5;
                        break;
                    case 5:
                        if (Node2ID == 6)
                            wgt=10;
                        break;
                    case 6:
                        if (Node2ID == 7)
                            wgt=10;
                        break;
                }
                if (Node2ID >= 0){

                    tempNode = topo->getNode(Node1ID);
                    numOfLinks = tempNode->getNumInLinks();
                    for (int v=0; v< numOfLinks; v++)
                    {
                        if(tempNode->getLinkIn(v)->getRemoteNode()->getModule()->getIndex() == Node2ID
                                && topo->getNode(Node1ID)->getLinkIn(v)->getWeight() == -1)
                            topo->getNode(Node1ID)->getLinkIn(v)->setWeight(wgt);
                    }
                    tempNode = topo->getNode(Node2ID);
                    numOfLinks = tempNode->getNumInLinks();

                    for (int v=0; v< numOfLinks; v++)
                    {
                        if(tempNode->getLinkIn(v)->getRemoteNode()->getModule()->getIndex() == Node1ID
                                && topo->getNode(Node2ID)->getLinkIn(v)->getWeight() == -1)
                            topo->getNode(Node2ID)->getLinkIn(v)->setWeight(wgt);
                    }
                }
            }
        }
        break;

    default:

        for (int i=0; i<topo->getNumNodes(); i++)
        {
            numInLinks = 0;
            numInLinks = topo->getNode(i)->getNumInLinks();
            for(int j=0; j<numInLinks; j++)
            {
                topo->getNode(i)->getLinkIn(j)->setWeight(wgt);
            }
        }
        break;
    }

}


/*
 * Args: cTopology topo, LinkIn* edge
 * Return: distance of shortest path from source to destination
 * Description: Find P(u,v) where u= Node* src and v= Node* dest.
 *                          Using calculateWeightedSingleShortestPathsTo function
 */
double Routing::getPuv(cTopology *topo, cTopology::LinkIn* edge)
{
    double Puv = 0;

    cTopology::Node* u = edge->getRemoteNode(); // From Node
    cTopology::Node* v = edge->getLocalNode(); // To Node

    Puv = Routing::findWeightOfShortestPath(topo,u,v); // Get P(u,v) - the shortest path weight from u to v

    return Puv;
}


bool Routing::checkIfInWeightRange(double wgt)
{
    return (wgt > MIN_VALUE && wgt < MAX_VALUE);
}



cTopology::Node* Routing::findFurthestNodeInGraph(cTopology *topo, cTopology::Node *destNode){
    cTopology::Node *tempNode =NULL, *resNode = NULL;
    int nodesCount = topo->getNumNodes(), dist=0;
    int currDist = 0;
    topo->calculateWeightedSingleShortestPathsTo(destNode);
    for(int i=0;i<nodesCount;i++)
    {
        tempNode = topo->getNode(i);
        if (destNode != tempNode)
        {
            currDist = tempNode->getDistanceToTarget();
            if (currDist > 0 && dist < currDist)
            {
                dist = currDist;
                resNode = tempNode;
            }
        }
    }


    return resNode;
}


// A utility function to find the vertex with
// minimum key value, from the set of vertices
// not yet included in MST
int Routing::minKey(int key[], bool mstSet[], int NodeCount)
{
    // Initialize min value
    int min = INT_MAX, min_index;

    for (int v = 0; v < NodeCount; v++)
        if (mstSet[v] == false && key[v] < min)
            min = key[v], min_index = v;

    return min_index;
}


// Function to construct and print MST for
// a graph represented using adjacency
// matrix representation
cTopology * Routing::primMST(cTopology *OrigTopo, int RootNodeInd)
{
    // Array to store constructed MST


    int nodesCount = OrigTopo->getNumNodes();

    int parent[nodesCount];

    // Key values used to pick minimum weight edge in cut
    int key[nodesCount];
    // To represent set of vertices not yet included in MST
    bool MstSet[nodesCount];
    int tempNodeNieghbor;
    cTopology::Node* tempNode;

    int numOfLinks, edgeWeight;

    // Initialize all keys as INFINITE
    for (int i = 0; i < nodesCount; i++){
        MstSet[i] = false;
        key[i] = INT_MAX;
        parent[i] = -2;
    }
    // Always include first 1st vertex in MST.
    // Make key 0 so that this vertex is picked as first vertex.
    key[RootNodeInd] = 0;
    parent[RootNodeInd] = -1; // First node is always root of MST
    int u;
    for (int count = 0; count < nodesCount; count++)
    {

        u = minKey(key, MstSet, nodesCount);

        tempNode = OrigTopo->getNode(u);
        MstSet[u] = true;
        numOfLinks = tempNode->getNumInLinks();

        for (int v = 0; v < numOfLinks; v++){
            tempNodeNieghbor = tempNode->getLinkIn(v)->getRemoteNode()->getModule()->getIndex();
            edgeWeight = (int)tempNode->getLinkIn(v)->getWeight();
            if(MstSet[tempNodeNieghbor] == false && edgeWeight < key[tempNodeNieghbor]){
                parent[tempNodeNieghbor] = u;
                key[tempNodeNieghbor] = edgeWeight;
            }
        }
    }
    cTopology *MSTtopology = BuildTopoFromArray(parent, nodesCount, OrigTopo);

    MSTtopology->calculateWeightedSingleShortestPathsTo(MSTtopology->getNode(RootNodeInd));

    return MSTtopology;
}


/*
 * Parameters: Node* source, dest, two edges of the graph's diameter
 * Return: int length of diameter
 * Description:
 * This method will traverse the diameter using the Node->outPath field (type LinkIn).
 * The last time we ran the calculateWeightedSingleShortestPathsTo was on *dest. Therefore,
 * every other node in the topology holds the outPath field directed to dest.
 * So, if we follow each outPath, it is guarenteed that 1. It is the shortest path
 * 2. The path will lead us to dest.
 */
int Routing::traverseDiameter(cTopology *topo, cTopology::Node *source, cTopology::Node *dest){
    cTopology::Node *tempNode;
    cTopology::LinkOut *tempLinkOut;
    int hopCount = 0;
    int sourceIndex = 0, destIndex = 0,  tempNodeIndex = 0;

    Nodes.clear();

    sourceIndex = source->getModule()->getIndex();
    destIndex = dest->getModule()->getIndex();

    if (DEBUG){
        EV << "[+] Routing::traverseDiameter:: Source Index: " << sourceIndex << endl;
        EV << "[+] Routing::traverseDiameter:: Dest Index: " << destIndex << endl;
  }

    Nodes.push_back(source);
    tempLinkOut = source->cTopology::Node::getPath(0);
    tempNode = tempLinkOut->getRemoteNode();
    if (DEBUG){
        EV << "[+] Routing::traverseDiameter:: Before while" << endl;
    }
    while(tempNode != dest)
    {
        Nodes.push_back(tempNode);
        tempNodeIndex = tempNode->getModule()->getIndex();
        EV << "[+] tempNode Index: " << tempNodeIndex << endl;
        hopCount = hopCount + 1;
        tempNode = tempNode->cTopology::Node::getPath(0)->getRemoteNode(); //If not it should be local node
    }

    hopCount = hopCount + 1;
    Nodes.push_back(dest);
    if (DEBUG){
        EV << "[+] Routing::traverseDiameter:: After while" << endl;
    }

    return hopCount;
}


// Input: Global variable List if nodes (Nodes) - Doesn't pass that but uses it,
//color(ex. "red", "yellow", "green", width (ex. "1", "4")
// Return: void
void Routing::paintRouteInColor(const char* color, int width)
{
    std::list<cTopology::Node*>::iterator it;
    cTopology::LinkOut *tempLinkOut;
    int currNodeIndex = 0;
    int numPath = 0;

    for(it= Nodes.begin(); it!= Nodes.end(); it++)
    {
        numPath  = (*it)->getNumPaths();
        if(numPath != 0)
        {
            tempLinkOut = (*it)->cTopology::Node::getPath(0);
            cDisplayString& dspString = (cDisplayString&)tempLinkOut->getLocalGate()->getDisplayString();
            currNodeIndex = (*it)->getModule()->getIndex();
            if (DEBUG){
                EV << "[+] Routing::paintRouteInColoer:: Coloring Node with Index: " << currNodeIndex << " in " <<
                        color << endl;
            }
            dspString.setTagArg("ls",0,color);
            dspString.setTagArg("ls",1,width);
        }
    }
}

/*
 * void Routing::copyTopology(cTopology* OrigTopo,cTopology* NewTopo)
 * a function to copy 1 topology to the other
 * the function receives 2 Topologies - The Original topology and the New topology.
 * the function changes disable all of the links in the original topology and afterwards
 * copy the links enabled in the new topology to the Original, in order to display the topology
 */
void Routing::copyTopology(cTopology* OrigTopo,cTopology* NewTopo, const char* colour)
{
    //disable all the links in the original topology
    disableAllInLinks(OrigTopo, false);

    int nodesCount = OrigTopo->getNumNodes();
    cTopology::Node *tempOrigNode, *tempNewNode;
    int edgeCount;
    double edgeWeight;
    //run over all of the nodes in the topology
    for (int i=0;i< nodesCount; i++)
    {
        //get the nodes from both topologies
        tempOrigNode = OrigTopo->getNode(i);
        tempNewNode = NewTopo->getNode(i);
        edgeCount = tempOrigNode->getNumInLinks();

        //run over all of the links of the node
        for (int u=0; u< edgeCount; u++)
        {
            //if the link is enabled in the new topology, update the original topology and paint the connection
            if(tempNewNode->getLinkIn(u)->isEnabled())
            {
                tempOrigNode->getLinkIn(u)->enable();
                edgeWeight = tempNewNode->getLinkIn(u)->getWeight();
                tempOrigNode->getLinkIn(u)->setWeight(edgeWeight);
                if (DEBUG){
                    EV << "[+] copyTopology - Adding link From Node " <<
                            tempOrigNode->getModule()->getIndex()<< " To Node " <<
                            tempOrigNode->getLinkIn(u)->getRemoteNode()->getModule()->getIndex()<< endl;
                }
            }
        }
    }
    if (DEBUG){
        EV << "[+] copyTopology - Done!" << endl;
    }
    PaintTopology(OrigTopo, colour);
    if (hasGUI())
        getParentModule()->bubble("Topology Updated!");
}


cTopology* Routing::CreateTopology(cTopology *Origtopo)
{
    cTopology *NewTopology = new cTopology("topo");
    std::vector<std::string> nedTypes;

    nedTypes.push_back(getParentModule()->getNedTypeName());
    NewTopology->extractByNedTypeName(nedTypes);
    if (DEBUG){
        EV << "[+] CreateTopology- calling copyTopology " << endl;
    }
    copyTopology(NewTopology, Origtopo, "green");
    return NewTopology;
}


void Routing::PaintTopology(cTopology* OrigTopo, const char* colour)
{
    int nodesCount = OrigTopo->getNumNodes();
    cTopology::Node *tempOrigNode;
    int edgeCount;
    //run over all of the nodes in the topology
    for (int i=0;i< nodesCount; i++)
    {
        //get the nodes from both topologies
        tempOrigNode = OrigTopo->getNode(i);
        edgeCount = tempOrigNode->getNumInLinks();

        //run over all of the links of the node
        for (int u=0; u< edgeCount; u++)
        {
            //if the link is enabled in the new topology, update the original topology and paint the connection
            if(tempOrigNode->getLinkIn(u)->isEnabled())
            {
                cDisplayString& dspString = (cDisplayString&)tempOrigNode->getLinkIn(u)->getRemoteGate()->getDisplayString();
                dspString.setTagArg("ls",0,colour);
                dspString.setTagArg("ls",1,4);
            }
        }
    }
}


// Returns the index of satellite i
int getSatIndex(cTopology::Node *node)
{
    return node->getModule()->getIndex();
}


void Routing:: initRtable(cTopology* thisTopo, cTopology::Node *thisNode)
{
    // find and store next hops
    for (int i = 0; i < thisTopo->getNumNodes(); i++) {
        if (thisTopo->getNode(i) == thisNode)
            continue;  // skip ourselves
        thisTopo->calculateWeightedSingleShortestPathsTo(thisTopo->getNode(i));

        if (thisNode->getNumPaths() == 0)
            continue;  // not connected

        cGate *parentModuleGate = thisNode->getPath(0)->getLocalGate();
        int gateIndex = parentModuleGate->getIndex();
        int address = thisTopo->getNode(i)->getModule()->par("address");
        rtable[address] = gateIndex;
        if (DEBUG){
            EV << "  towards address " << address << " gateIndex is " << gateIndex << endl;
        }
    }

}


int Routing::GetW(cTopology::Node *nodeU, cTopology::Node *nodeV)
{

    int numOfLinks = nodeV->getNumInLinks();
    cTopology::LinkIn* checkedLink;
    for(int i=0;i<numOfLinks;i++){
        checkedLink = nodeV->getLinkIn(i);
        if( checkedLink->getRemoteNode()->getModule()->getIndex() == nodeU->getModule()->getIndex() &&
                checkedLink->isEnabled())
        {
            return checkedLink->getWeight();
        }
    }
    return 0;
}


cTopology* Routing::BuildSPT(cTopology* origTopo, int RootIndex)
{
    if (DEBUG){
        EV << "[+] BuildSPT- calling CreateTopology " << endl;
    }
    cTopology* SPTtopo = CreateTopology(origTopo);

    SPTtopo->calculateWeightedSingleShortestPathsTo(SPTtopo->getNode(RootIndex));
    return SPTtopo;

}


void Routing::RelaxTopo(int ChildNodeIndex,int ParentNodeIndex,cTopology* origTopo,  int parent[], int distance[])
{
    int LinkWeight = GetW(origTopo->getNode(ChildNodeIndex), origTopo->getNode(ParentNodeIndex));
    int compareWeight = distance[ParentNodeIndex] + LinkWeight;
    if (distance[ChildNodeIndex] > compareWeight){
        distance[ChildNodeIndex] = compareWeight;
        parent[ChildNodeIndex] = ParentNodeIndex;
    }
}


void Routing::AddPathToLAST(int NodeIndex, cTopology* origTopo,cTopology* SPTTopo, int parent[], int distance[])
{
    if (distance[NodeIndex] > (int)SPTTopo->getNode(NodeIndex)->getDistanceToTarget()){
        cTopology::LinkOut* tempLink = SPTTopo->getNode(NodeIndex)->cTopology::Node::getPath(0);
        if (tempLink != NULL)
        {
            int ParentNodeIndex = tempLink->getRemoteNode()->getModule()->getIndex();
            AddPathToLAST(ParentNodeIndex,origTopo,SPTTopo,parent, distance);
            RelaxTopo(NodeIndex, ParentNodeIndex, origTopo, parent, distance);
        }
    }
}


void Routing::DFS(int NodeIndex,int RootIndex,  cTopology* origTopo, cTopology* MSTTopo, cTopology* SPTTopo, int alpha, int parent[], int distance[])
{
    int ParentNodeIndex, ChildNodeIndex;
    int multiplySPT = alpha * SPTTopo->getNode(NodeIndex)->getDistanceToTarget();
    if (distance[NodeIndex] > multiplySPT){
        AddPathToLAST(NodeIndex, origTopo,SPTTopo,parent, distance);
    }
    cTopology::Node* MSTNode = MSTTopo->getNode(NodeIndex);
    int NumOfLinks = MSTNode->getNumInLinks();
    if (NodeIndex == RootIndex)
        ParentNodeIndex = -1;
    else
        ParentNodeIndex = MSTNode->cTopology::Node::getPath(0)->getRemoteNode()->getModule()->getIndex();
    for (int i=0; i< NumOfLinks ;i++)
    {
        if(MSTNode->getLinkIn(i)->isEnabled()){
            ChildNodeIndex = MSTNode->getLinkIn(i)->getRemoteNode()->getModule()->getIndex();
            if (ChildNodeIndex != ParentNodeIndex){
                RelaxTopo(ChildNodeIndex, NodeIndex, origTopo, parent, distance);
                DFS(ChildNodeIndex,RootIndex, origTopo, MSTTopo, SPTTopo, alpha , parent, distance);
                RelaxTopo(NodeIndex, ChildNodeIndex, origTopo, parent, distance);
            }
        }
    }
}


cTopology* Routing::BuildTopoFromArray(int parent[], int nodesCount,cTopology* OrigTopo)
{
    if (DEBUG){
        EV << "[+] BuildTopoFromArray- calling CreateTopology " << endl;
    }
    cTopology* newTopo = CreateTopology(OrigTopo);
    cTopology::Node* tempNode;
    disableAllInLinks(newTopo, false);

    int Node1ID, Node2ID;
    for (int count = 0; count < nodesCount; count++)
    {
        Node1ID = count;
        Node2ID = parent[count];
        if (Node2ID >= 0){
            tempNode = newTopo->getNode(Node1ID);
            changeConnectingLink(tempNode, Node2ID, true);

            tempNode = newTopo->getNode(Node2ID);
            changeConnectingLink(tempNode, Node1ID, true);
        }
    }
    return newTopo;
}


cTopology* Routing::BuildTopoFromMultiArray(int** parentArrays,int ArrayCount,cTopology* OrigTopo)
{
    if (DEBUG){
        EV << "[+] BuildTopoFromMultiArray- calling CreateTopology " << endl;
    }
    cTopology* newTopoFromArr = CreateTopology(OrigTopo);
    cTopology::Node* tempNode;
    disableAllInLinks(newTopoFromArr, false);
    int *parent;
    int Node1ID, Node2ID, nodesCount = OrigTopo->getNumNodes();
    for (int arrCtr= 0; arrCtr <ArrayCount; arrCtr++)
    {
        parent = parentArrays[arrCtr];
        for (int count = 0; count < nodesCount; count++)
        {
            Node1ID = count;
            Node2ID = parent[count];
            if (Node2ID >= 0){
                tempNode = newTopoFromArr->getNode(Node1ID);
                changeConnectingLink(tempNode, Node2ID, true);

                tempNode = newTopoFromArr->getNode(Node2ID);
                changeConnectingLink(tempNode, Node1ID, true);

            }
        }
    }
    return newTopoFromArr;
}


int* Routing::FindLASTForRoot(cTopology* origTopo,int alpha, int RootIndex,int* parent)
{
    cTopology* MSTTopo = primMST(origTopo, RootIndex);
    cTopology* SPTTopo = BuildSPT(origTopo, RootIndex);

    int nodesCount = origTopo->getNumNodes();

    // Key values used to pick minimum weight edge in cut
    int distance[nodesCount];


    // Initialize all keys as INFINITE
    for (int i = 0; i < nodesCount; i++){
        distance[i] = INT_MAX;
        parent[i] = -2;
    }
    parent[RootIndex] = 0;
    distance[RootIndex] = 0;

    DFS(RootIndex,RootIndex, origTopo, MSTTopo, SPTTopo, alpha, parent, distance);
    return parent;
}

int Routing::findDiameter(cTopology* currTopo, int startPoint)
{
    //TraverseDiameter
    cTopology::Node* furthest_node_1 = findFurthestNodeInGraph(currTopo,currTopo->getNode(startPoint));
    cTopology::Node* furthest_node_2 = findFurthestNodeInGraph(currTopo,furthest_node_1);
    int diameterLength = traverseDiameter(currTopo, furthest_node_2,furthest_node_1);
    if (DEBUG){
        EV << "[+] Diameter length is: " << diameterLength << endl;
    }
    return diameterLength;
}


void Routing::changeConnectingLink(cTopology::Node* currNode, int otherNodeInd,bool state){
    int numOfLinks = currNode->getNumInLinks();

    for (int v=0; v< numOfLinks; v++)
    {
        if(currNode->getLinkIn(v)->getRemoteNode()->getModule()->getIndex()== otherNodeInd)
        {
            if (state == true)
            {
                currNode->getLinkIn(v)->enable();
                currNode->getLinkOut(v)->enable();
            }
            else
            {
                currNode->getLinkIn(v)->disable();
                currNode->getLinkOut(v)->disable();
            }
        }
    }
}

void Routing::ScheduleChangeTopo(){

    //create a  timer to change the topology every X minutes
    Packet * timerPacket = new Packet("ChangeTopology");
    //timerPacket->setpacketType(schedule_topo_packet);
    scheduleAt(simTime() + changeRate*60, timerPacket);

}

void Routing::CreateInitTopology(cTopology *myTopo){
    myTopo = new cTopology("topo");
    std::vector<std::string> nedTypes;

    nedTypes.push_back(getParentModule()->getNedTypeName());
    myTopo->extractByNedTypeName(nedTypes);
}

void Routing::addWeightsToTopo(Packet* pk){
    int nodeIndex = pk->getSrcAddr();
    int linkWeight =0;
    cTopology::Node* tempNode = newTopo->getNode(nodeIndex);
    for (int i=0; i< (int)pk->getPacketListArraySize(); i++)
    {
        linkWeight = (pk->getPacketList(i) >0) ? pk->getPacketList(i) : 1;
        if( tempNode->getLinkIn(i)->getWeight() < linkWeight)
            tempNode->getLinkIn(i)->setWeight(linkWeight);
    }
}

void Routing::addSelfWeightsToTopo(){
    cTopology::Node* tempNode = newTopo->getNode(ROOTNODE);
    int linkWeight;
    for (int i=0; i< thisNode->getNumInLinks(); i++)
    {
        linkWeight = (linkCounter[i] >0) ? linkCounter[i] : 1;
        if( tempNode->getLinkIn(i)->getWeight() < linkWeight)
            tempNode->getLinkIn(i)->setWeight(linkWeight);
    }
}


cTopology* Routing::BuildLASTtopo(cTopology* origTopo,int alpha,int m)
{
    std::list<cTopology::Node*>::iterator it;
    cTopology::Node* nextNode = NULL;
    cTopology::Node* currNode= NULL;
    cTopology *newMSTTopo = primMST(origTopo, 1);
    int nodeCtr = 0,numPath = 0,  n=newMSTTopo->getNumNodes();
    int rootCounter=0, hops = (n-m)/m;
    int nextNodeInd = 0, currNodeInd=0;

    int rootNodesArr[n];
    int diameterLen = 0;
    bool Found = false, doneSearch= false;
    EV << "in buildLAst Topo"<< endl;
    while(!doneSearch){
        EV << "in buildLAst Topo donesearch is true"<< endl;
        if(diameterLen/2 > hops)

            diameterLen = findDiameter(newMSTTopo, nextNodeInd);
        else
            diameterLen = findDiameter(newMSTTopo, currNodeInd);

        if (diameterLen > hops)
        {
            EV << "in build last Topo diameterLen > hops "<< endl;
            it= Nodes.begin();
            nodeCtr=0;
            Found = false;
            while(!Found && it!= Nodes.end())
            {
                // a flag from the calculateWeightedSingleShortestPathsTofunction that
                // indicate that there is a SP route to that node
                numPath  = (*it)->getNumPaths();
                if(numPath != 0 && hops == nodeCtr)
                {
                    Found = true;
                    currNode = (*it);
                    rootNodesArr[rootCounter] = (*it)->getModule()->getIndex(); //save root index
                    rootCounter++;
                    nextNode = (*next(it,1));
                    currNodeInd = (*it)->getModule()->getIndex();
                    nextNodeInd = nextNode->getModule()->getIndex();
                    //disconnect the root from topology
                    if (nextNode!= NULL && nextNodeInd != currNodeInd)
                    {
                        changeConnectingLink(currNode, nextNodeInd, false);
                        changeConnectingLink(nextNode, currNodeInd, false);
                    }

                }

                nodeCtr++;
                it++;
            }
        }
        else
        {
            doneSearch = true;
        }
    }
    int* LASTparentArr [rootCounter];
    for(int i=0;i<rootCounter;i++)
    {
        int* parent = new int[n];
        LASTparentArr[i] = FindLASTForRoot(origTopo, alpha, rootNodesArr[i], parent);
    }

    cTopology* completeTopo = BuildTopoFromMultiArray(LASTparentArr, rootCounter, origTopo);
    return completeTopo;
    // int* parent = new int[n];
    // int *LastARr = FindLASTForRoot(origTopo, alpha, 54, parent);
    // return BuildTopoFromArray(LastARr, origTopo->getNumNodes(), origTopo);
}

void Routing::ScheduleNewTopoPacket()
{
    char pkname[40];
    char *packetName = (char*)"pk-Initialize-topo";
    sprintf(pkname, "pk-%s", packetName);
    if (DEBUG){
        EV << "generating packet " << pkname << endl;
    }

    _pk = new Packet(pkname);

    //set a unique ID for the topology created so that only the packets that are related to this topology will pass on
    currTopoID = rand()% 10000 + 1;
    _pk->setHasInitTopo(1); // RTE 0 finished with topology
    hasInitTopo = 0;
    _pk->setSrcAddr(myAddress);
    _pk->setDestAddr(myAddress);
    _pk->setPacketType(initiale_packet); // At the beginning of the world set the type of packet to 1 (=Initialize packet)
    _pk->setTopologyID(currTopoID);
    Routing::LoadPacket(_pk, topo); // Copy topology to the field in the packet
    scheduleAt(simTime() + _sendTime->doubleValue(), _pk);
}

void Routing::SendReportToRoot()
{
    //send the packet with the info to the root node
    RoutingTable::iterator iter = rtable.find(0);
        int rootGateIndex = (*iter).second;

    Packet *sendReport = new Packet("Send Info");
    sendReport->setSrcAddr(myAddress);
    sendReport->setDestAddr(ROOTNODE);
    sendReport->setPacketType(send_node_info_packet);
    sendReport->setTopologyID(currTopoID);
    sendReport->setPacketListArraySize(thisNode->getNumInLinks());


    for (int i=0; i< thisNode->getNumInLinks(); i++)
    {
        sendReport->setPacketList(i, linkCounter[i]);
        linkCounter[i] = 0;
    }
    send(sendReport, "out", rootGateIndex);
}

void Routing::ScheduleReportTrigger(){
    //send a delayed message to redo the proccess
    Packet *sendInitReport = new Packet("Init Info");
    sendInitReport->setDestAddr(myAddress);
    sendInitReport->setPacketType(send_node_info_packet);
    sendInitReport->setTopologyID(currTopoID);
    scheduleAt(simTime() + changeRate, sendInitReport);
}

void Routing::SendDuplicateTopology(Packet *pk, int destNodeInd)
{
    Packet *dupPk = pk->dup();

    dupPk->setDestAddr(destNodeInd);
    dupPk->setSrcAddr(myAddress);
    dupPk->setTopologyVar(pk->getTopologyVarForUpdate());
    dupPk->setTopologyID(currTopoID);
    dupPk->setPacketType(initiale_packet);
    RoutingTable::iterator it = rtable.find(destNodeInd);

    int outGateIndex = (*it).second;
    send(dupPk, "out", outGateIndex);
}

void Routing::initialize()
{
    myAddress = getParentModule()->par("address");
    hasInitTopo = 0;
    packetType = 1;
    dropSignal = registerSignal("drop");
    outputIfSignal = registerSignal("outputIf");
    TopoLinkCounter = registerSignal("TopoLinkCounter");
    hopCountSignal= registerSignal("hopCount");
    alpha = getParentModule()->par("alpha");
    fullTopoRun = false;//getParentModule()->par("fullTopoRun");
    m_var = getParentModule()->par("m_var");
    changeRate =getParentModule()->par("changeRate");
    topo = new cTopology("topo");
    std::vector<std::string> nedTypes;

    nedTypes.push_back(getParentModule()->getNedTypeName());
    topo->extractByNedTypeName(nedTypes);
    NodesInTopo = topo->getNumNodes() ;
    EV << "cTopology found " << NodesInTopo << " nodes\n";

    thisNode = topo->getNodeFor(getParentModule());
    linkCounter = new int[thisNode->getNumInLinks()];
    for (int j=0;j< thisNode->getNumInLinks();j++)
    {
        linkCounter[j]=0;
    }
    Routing::disableAllInLinks(topo, true); // set G'= {}

    //only for full topology simulation
    if (fullTopoRun){
        setLinksWeight(topo,1); // Set random weights to edges in the graph
        initRtable(topo, thisNode);
        PaintTopology(topo, "green");
        insertLinksToList(topo);
        int enabledCtr = printEdgesList();
        emit(TopoLinkCounter,enabledCtr);

    }
    else if(this->myAddress == ROOTNODE)
    {
        setLinksWeight(topo,0); // Set random weights to edges in the graph
        cTopology* LASTtopo = BuildLASTtopo(topo,alpha,m_var);
        PaintTopology(LASTtopo,"blue");
        EV << "[+] disableAllInLinks- calling copyTopology " << endl;
        copyTopology(topo, LASTtopo, "green");
        if (DEBUG){
            printNodesList();
        }

        initRtable(topo, thisNode);

        if (hasGUI())
            getParentModule()->bubble("Generated Topology");
        _sendTime = &par("sendTime");

        ScheduleNewTopoPacket();
    }
    else
    {
     //   setLinksWeight(topo,0);
        ScheduleReportTrigger();
    }

}


void Routing::handleMessage(cMessage *msg)
{
    Packet *pk = check_and_cast<Packet *>(msg);

    int i=0;
    int arrivingGate = pk->getArrivalGateId();
    int destAddr = pk->getDestAddr();
    int srcAddr = pk->getSrcAddr();
    // Self message
    if (DEBUG){
        EV << "Got packet, source - "<< srcAddr << " Dest - " <<destAddr <<"  Type -" <<
                pk->getPacketType() <<endl;
    }
    if(destAddr == myAddress)
    {
        //new topology has arrived, update the topology
        switch(pk->getPacketType() )
        {
        case 1: //init_topology_packet
        {
            EV << " case 1 update topo routing init_topology_packet" << endl;
            if(currTopoID !=pk->getTopologyID())
            {
                hasInitTopo =0;
            }

            if (hasInitTopo ==0 )
            {
                // copyTopology
                if (DEBUG){
                    EV << "[+] handleMessage, Case1 - calling copyTopology " << endl;
                }
                copyTopology(topo, pk->getTopologyVarForUpdate(), "yellow");
                currTopoID = pk->getTopologyID();
                thisNode = topo->getNodeFor(getParentModule());
                initRtable(topo, thisNode);
                hasInitTopo = 1;
                insertLinksToList(topo);
                int enabledCtr = printEdgesList();
                emit(TopoLinkCounter,enabledCtr);
                edges.clear();

                cTopology::Node *tempNode = topo->getNode(myAddress);
                int numInLinks = tempNode->getNumInLinks(), destNodeInd;

                for (i=0 ; i<numInLinks ; i++)
                {
                    if(tempNode->getLinkIn(i)->isEnabled())
                    {
                        destNodeInd = tempNode->getLinkIn(i)->getRemoteNode()->getModule()->getIndex();
                        if (destNodeInd != pk->getSrcAddr()){
                            SendDuplicateTopology(pk, destNodeInd);
                        }
                    }
                }
                //send a massage to App to start sending packets
                Packet* firstPacket = new Packet("StartSending");
                firstPacket->setPacketType(update_app_packet);
                firstPacket->setTopologyID(currTopoID);
                if (DEBUG){
                    EV << "Sending Start messsage to App at Node "<< myAddress << endl;
                }
                send(firstPacket, "localOut");
                emit(outputIfSignal, -1);  // -1: local
                delete pk;
                return;
            }
        }
        break;

        case 2: //regular_packet
        {
            EV << " case 2 update topo routing regularmsg" << endl;
            //regular message
            if (currTopoID ==pk->getTopologyID())
            {

                if (DEBUG){
                    EV << "local delivery of packet " << pk->getName() << endl;
                }
                for( int i=0;i<thisNode->getNumInLinks();i++)
                {
                    if( thisNode->getLinkIn(i)->getLocalGateId() == arrivingGate)
                    {
                        linkCounter[i] += 1;
                    }
                }
                emit(hopCountSignal, pk->getHopCount());
                send(pk, "localOut");
                emit(outputIfSignal, -1);  // -1: local
                return;

            }
        }
        break;

        case 3:
        {
            EV << " case 3 update topo routing handlemsg" << endl;
            topo = new cTopology("topo");
            std::vector<std::string> nedTypes;

            nedTypes.push_back(getParentModule()->getNedTypeName());
            topo->extractByNedTypeName(nedTypes);
            //randomize the weights again and build a new topology
            setLinksWeight(topo,1);
            cTopology* newLASTtopo = BuildLASTtopo(topo,alpha,m_var);
            if (DEBUG){
                EV << "[+] handleMessage, Case3 - calling copyTopology " << endl;
            }
            copyTopology(topo, newLASTtopo, "red");

            thisNode = topo->getNodeFor(getParentModule());
            initRtable(topo, thisNode);

            //send the new topology as a message
            currTopoID = rand()% 10000 + 1;;
            Packet *newTopoPacket = new Packet("Generated New Topology");
            newTopoPacket->setSrcAddr(myAddress);
            newTopoPacket->setDestAddr(myAddress);
            newTopoPacket->setPacketType(initiale_packet); // At the beginning of the world set the type of packet to 1 (=Initialize packet)
            newTopoPacket->setTopologyID(currTopoID);
            hasInitTopo =0;
            Routing::LoadPacket(newTopoPacket, topo); // Copy topology to the field in the packet
            if (hasGUI())
                getParentModule()->bubble("Generated New Topology");
            scheduleAt(simTime() + _sendTime->doubleValue(), newTopoPacket);

            //schdule a new changeTopology
            ScheduleChangeTopo();
        }
        break;
        //send_node_info_packet

        case 4:

        {
            EV << " case 4 update topo routing handlemsg" << endl;
            if (myAddress != ROOTNODE)
            {
                SendReportToRoot();
                //send a delayed message to redo the proccess
                ScheduleReportTrigger();
                return;
            }
            else
            {
                if (firstNodeInfo == 0 )
                {
                    newTopo = new cTopology("topo");
                    std::vector<std::string> nedTypes;

                    nedTypes.push_back(getParentModule()->getNedTypeName());
                    newTopo->extractByNedTypeName(nedTypes);
                    setLinksWeight(newTopo, 1);
                    firstNodeInfo = 1;
                }

                if (pk->getTopologyID() == currTopoID)
                {
                    addWeightsToTopo(pk);
                }
                updateCounter += 1;
                if (updateCounter >= NodesInTopo-1)
                {
                    addSelfWeightsToTopo();
                    if (hasGUI())
                        getParentModule()->bubble("Got All Info, Building New Topo!");
                    verifyWeightsForTopo();
                    cTopology* LASTtopo = BuildLASTtopo(newTopo,alpha,m_var);
                    insertLinksToList(LASTtopo);
                    int enabledCtr = printEdgesList();
                    if (DEBUG){
                        EV << "Routing::handleMessage::case4 :: sat index: " << myAddress <<
                                " before edges.clear()" << endl;
                    }
                    edges.clear();
                    if (DEBUG){
                        EV << "[+] handleMessage, Case4 - calling copyTopology " << endl;
                    }
                    if (enabledCtr > topo->getNumNodes() -2)
                    {
                        copyTopology(topo, LASTtopo, "green");
                    }
                    if (DEBUG){
                        printNodesList();
                    }
                    initRtable(topo, thisNode);
                    ScheduleNewTopoPacket();
                    updateCounter = 0;
                    firstNodeInfo = 0;
                    delete newTopo;
                }
                //if all of the nodes sent the message and Root had received all
                //update empty topology with the new weight
                //build new topology
                //send to the rest of the nodes
            }
        }
        break;
        }
    }
    else
    {
        EV << " case ELSE line 1509 update topo routing handlemsg" << endl;

        RoutingTable::iterator it = rtable.find(destAddr);
        if (it == rtable.end()) {
            if (DEBUG){
                EV << "address " << destAddr << " unreachable, discarding packet " << pk->getName() << endl;
            }
            emit(dropSignal, (long)pk->getByteLength());
            delete pk;
            return;
        }
        else if (currTopoID == pk->getTopologyID())
        {
            for( int i=0;i<thisNode->getNumInLinks();i++)
            {
                if( thisNode->getLinkIn(i)->getLocalGateId() ==pk->getArrivalGateId())
                {
                    linkCounter[i] += 1;
                }
            }
            int outGateIndex = (*it).second;
            if (DEBUG){
                EV << "forwarding packet " << pk->getName() << " on gate index " << outGateIndex << endl;
            }
            pk->setHopCount(pk->getHopCount()+1);
            // Add edge weight
            emit(outputIfSignal, outGateIndex);
            send(pk, "out", outGateIndex);
            return;
        }
        else
        {
            if (DEBUG){
                EV << "Packet topology ID =  " << pk->getTopologyID() << "do not match Node topology ID ( " <<
                        currTopoID <<") , discarding packet " << pk->getName() << endl;
            }
            emit(dropSignal, (long)pk->getByteLength());
            delete pk;
            return;
        }
    }
}


