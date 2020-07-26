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

#include <map>
#include <omnetpp.h>
#include "Packet_m.h"
#include <algorithm>


#define MAX_VALUE 10000
#define MIN_VALUE 0
#define PARAM 2

using namespace omnetpp;

/**
 * Demonstrates static routing, utilizing the cTopology class.
 */
class Routing : public cSimpleModule
{
private:
    int myAddress;
    int hasInitTopo; // Flag to determine if a node has the updated topology. otherwise, it might send messages through links that should not be exist.
    int packetType; // packetType 1 => Init mode. A command to distribute the topology among all nodes.
    cPar *_sendTime;  // volatile parameter
    // packetType 2 => Normal mode. These are regular packets generated from App.cc that will ride on the updated topology.
    Packet *_pk = NULL;

    typedef std::map<int, int> RoutingTable;  // destaddr -> gateindex
    RoutingTable rtable;
    typedef std::map<int,int> DiameterTable; // curAddr -> nextHop
    DiameterTable diameter;
    typedef std::list<cTopology::LinkIn*> LinkIns;
    LinkIns edges;
    typedef std::list<cTopology::Node*> NodesList;
    NodesList Nodes;

    cTopology::Node *thisNode;
    cTopology *topo;
    simsignal_t dropSignal;
    simsignal_t outputIfSignal;

protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;


    void setLinksWeight(cTopology *topo, double wgt);
    void disableAllInLinks(cTopology *topo, bool debugging);
    void insertLinksToList(cTopology *topo);
    void checkVerticesConnectivity(cTopology *topo, int address);
    double findWeightOfShortestPath(cTopology *topo,cTopology::Node* src, cTopology::Node* dest);
    double getPuv(cTopology *topo, cTopology::LinkIn* edge);
    bool checkIfInWeightRange(double wgt);
    void printEdgesList();
    void findDiameterOfGraph(cTopology *topo, DiameterTable newDiameter);
    cTopology::Node* findFurthestNodeInGraph(cTopology *topo, cTopology::Node *destNode);
    cTopology * primMST(cTopology *OrigTopo, int RootNodeInd);
    // int printMST(int parent[], cTopology *MSTTopo);
    int minKey(int key[], bool mstSet[],int NodeCount);
    int traverseDiameter(cTopology *topo, cTopology::Node *source, cTopology::Node *dest);
    void printNodesList();
    void paintRouteInColor(const char* color, int width);
    void copyTopology(cTopology* OrigTopo,cTopology* NewTopo);
    void PaintTopology(cTopology* OrigTopo);
    int getSatIndex(cTopology::Node *node);
    void LoadPacket(Packet *pk, cTopology *topo);
    void initRtable(cTopology* thisTopo, cTopology::Node *thisNode);
    int GetW(cTopology::Node *nodeU, cTopology::Node *nodeV);
    cTopology* CreateTopology(cTopology *Origtopo);
    cTopology* BuildSPT(cTopology* origTopo, int RootIndex);
    void DFS(int NodeIndex,int RootIndex,  cTopology* origTopo, cTopology* MSTTopo, cTopology* SPTTopo, int alpha, int parent[], int distance[]);
    void RelaxTopo(int ChildNodeIndex,int ParentNodeIndex,cTopology* origTopo,  int parent[], int distance[]);
    cTopology* FindLASTForRoot(cTopology* origTopo,int alpha, int RootIndex);
    void AddPathToLAST(int NodeIndex, cTopology* origTopo,cTopology* SPTTopo, int parent[], int distance[]);
    cTopology* BuildTopoFromArray(int parent[], int nodesCount,cTopology* OrigTopo);
};

Define_Module(Routing);




/*
 * Args: left and right cTopology::Link objects
 * Return: True if left operand is smaller than right operand
 * Description: Compare function for std::list. Returns true if link's weight on the left is smaller than link's weight
 * on the right. We do that so we can use std::sort method to sort the edges in a non-decreasing order.
 * The edges, as 'calculateWeightedSingleShortestPathsTo' calculates them, are represented by LinkIn which is an
 * alias class to 'Link'.

bool Routing::cmp(const cTopology::LinkIn &lsrc, const cTopology::LinkIn &rdst)
{
    return lsrc.getWeight() < rdst.getWeight();
}
 */
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
void Routing::printEdgesList()
{
    std::list<cTopology::LinkIn*>::iterator it;
    int source_node = 0;
    int dest_node = 0;
    double wgt = 0;

    for(it = edges.begin() ; it != edges.end() ; it++)
    {
        source_node = (*it)->getRemoteNode()->getModule()->par("address");
        dest_node = (*it)->getLocalNode()->getModule()->par("address");
        wgt = (*it)->getWeight();

        EV << "[+] printEdgesList:: " << "Source Node: " << source_node << " Dest Node: " << dest_node << " Weight: " << wgt << " Enabled:" << (*it)->isEnabled() << endl;
    }
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
    Routing::copyTopology(tempTopo, topo);

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
 * Description: Find P(u,v) where u= Node* src and v= Node* dest. Using calculateWeightedSingleShortestPathsTo function
 */
double Routing::findWeightOfShortestPath(cTopology *topo,cTopology::Node* src, cTopology::Node* dest)
{

    double source_dist = 0;

    topo->calculateWeightedSingleShortestPathsTo(dest);

    source_dist = src->getDistanceToTarget();

    //EV << "[+] findWeightOfShortestPath:: Distance from source to dest is: " << source_dist << endl;

    return source_dist;
}

/*
 * Args: cTopology variable, weight
 * Return: Void
 * Description: Set weights to LinkIn objects (edges), since this is the actual object that counts when computing
 * 'calculateWeightedSingleShortestPathsTo' (Disjktra Algorithm)
 */
void Routing::setLinksWeight(cTopology *topo, double wgt)
{
    int numInLinks = 0, Node2ID, Node1ID, numOfLinks;
    cTopology::Node* tempNode;
    srand(time(NULL));
    switch((int)wgt)
    {
    case 0:

        for (int i=0; i<topo->getNumNodes(); i++)
        {
            numInLinks = 0;
            numInLinks = topo->getNode(i)->getNumInLinks();
            wgt = rand() % 100 + 1; // 1 to 100

            Node1ID = i;
            for(int j=0; j<numInLinks; j++)
            {

                Node2ID = topo->getNode(i)->getLinkIn(j)->getRemoteNode()->getModule()->getIndex();
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

    case 1:

        for (int i=0; i<topo->getNumNodes(); i++)
        {
            numInLinks = 0;
            numInLinks = topo->getNode(i)->getNumInLinks();
            for(int j=0; j<numInLinks; j++)
            {
                wgt = i*j;
                if(wgt==0)
                    wgt = 1;
                topo->getNode(i)->getLinkIn(j)->setWeight(wgt);
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
 * Description: Find P(u,v) where u= Node* src and v= Node* dest. Using calculateWeightedSingleShortestPathsTo function
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

//find the diameter of a given topology -
//Parameters - topology of a graph
//the function will get the MST of the graph, and then
//it will run on a random node "findFurthestNodeInGraph" function,
//and will run it again on the same node. - the route between the first result node
//and the second result node will be the diameter of the graph
// the the function will build the route of that diameter and return it
void Routing::findDiameterOfGraph(cTopology *topo, DiameterTable newDiameter){

    cTopology::Node *baseNode =topo->getNode(0), *nextHop;
    cTopology::Node *destNode= NULL, *srcNode= NULL;
    cTopology::LinkIn *tempLink = NULL;
    DiameterTable tempDiameter;
    int nodeInd =0, numOfNodes = topo->getNumNodes();
    int linkInd = 0;

    //find the start of the diameter
    srcNode = findFurthestNodeInGraph(topo,baseNode);
    //find the end of the diameter
    destNode = findFurthestNodeInGraph(topo,srcNode);
    //build the diameter route from src to dest
    nextHop = srcNode;

    for (int i=0;i<numOfNodes;i++){
        //TODO - fix logic
        topo->calculateWeightedSingleShortestPathsTo(nextHop);
        nodeInd = nextHop->getModule()->par("address");

        linkInd = nextHop->getPath(0)->getLocalGate()->getIndex();
        tempLink = nextHop->getLinkIn(linkInd);

        if( tempLink->getLocalNode() == nextHop)
            nextHop = tempLink->getRemoteNode();
        else
            nextHop = tempLink->getLocalNode();

        tempDiameter[nodeInd] = nextHop->getModule()->par("address");
    }
    newDiameter = tempDiameter;




}

cTopology::Node* Routing::findFurthestNodeInGraph(cTopology *topo, cTopology::Node *destNode){
    cTopology::Node *tempNode =NULL, *resNode = NULL;
    int nodesCount = topo->getNumNodes(), dist=0;

    topo->calculateWeightedSingleShortestPathsTo(destNode);
    for(int i=0;i<nodesCount;i++)
    {
        tempNode = topo->getNode(i);
        if (destNode != tempNode)
            if (dist < tempNode->getDistanceToTarget()){
                dist = tempNode->getDistanceToTarget();
                resNode = tempNode;
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



// A utility function to print the
// constructed MST stored in parent[]

/*
 *
 int Routing::printMST(int parent[], cTopology *MSTTopo)
{
    EV <<"Edge \tWeight\n";
    for (int i = 1; i < V; i++)
        EV << "TODO\n";
        //cout<<parent[i]<<" - "<<i<<" \t"<<graph[i][parent[i]]<<" \n";
}

 */
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
    int sourceIndex = 0, destIndex = 0, outGateIndex = 0, tempNodeIndex = 0;

    EV << "[+] Routing::traverseDiameter" << endl;

    sourceIndex = source->getModule()->getIndex();
    destIndex = dest->getModule()->getIndex();

    EV << "[+] Routing::traverseDiameter:: Source Index: " << sourceIndex << endl;
    EV << "[+] Routing::traverseDiameter:: Dest Index: " << destIndex << endl;

    //int destAddress = topo->getNode(destIndex)->getModule()->par("address");

    //cDisplayString& dispStr; //= source->getLinkIn(j)->getRemoteGate()->getDisplayString();



    //dispStr.setTagArg("ls", 0, "red");

    //edges.push_back(topo->getNode(i)->getLinkIn(j));
    Nodes.push_back(source);
    tempLinkOut = source->cTopology::Node::getPath(0);
    tempNode = tempLinkOut->getRemoteNode();

    EV << "[+] Routing::traverseDiameter:: Before while" << endl;
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
    EV << "[+] Routing::traverseDiameter:: After while" << endl;

    return hopCount;
}

// Input: Global variable List if nodes (Nodes) - Doesn't pass that but uses it, color(ex. "red", "yellow", "green", width (ex. "1", "4")
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
            EV << "[+] Routing::paintRouteInColoer:: Coloring Node with Index: " << currNodeIndex << " in " << color << endl;
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
void Routing::copyTopology(cTopology* OrigTopo,cTopology* NewTopo)
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
                EV << "[+] copyTopology - Adding link From Node " << tempOrigNode->getModule()->getIndex()<< " To Node " << tempOrigNode->getLinkIn(u)->getRemoteNode()->getModule()->getIndex()<< endl;
            }
        }
    }
    PaintTopology(OrigTopo);
}

cTopology* Routing::CreateTopology(cTopology *Origtopo)
{
    cTopology *NewTopology = new cTopology("topo");
    std::vector<std::string> nedTypes;

    nedTypes.push_back(getParentModule()->getNedTypeName());
    NewTopology->extractByNedTypeName(nedTypes);
    copyTopology(NewTopology, Origtopo);
    return NewTopology;
}


void Routing::PaintTopology(cTopology* OrigTopo)
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
                dspString.setTagArg("ls",0,"green");
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
        EV << "  towards address " << address << " gateIndex is " << gateIndex << endl;
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
    cTopology* SPTtopo = CreateTopology(origTopo);

    SPTtopo->calculateWeightedSingleShortestPathsTo(SPTtopo->getNode(RootIndex));
    return SPTtopo;

}

void Routing::RelaxTopo(int ChildNodeIndex,int ParentNodeIndex,cTopology* origTopo,  int parent[], int distance[])
{
    int LinkWeight = GetW(origTopo->getNode(ChildNodeIndex), origTopo->getNode(ParentNodeIndex));
    int compareWeight = distance[ParentNodeIndex] + LinkWeight;
    if (ChildNodeIndex == 5)
        int i=0;
    if (distance[ChildNodeIndex] > compareWeight){
        distance[ChildNodeIndex] = compareWeight;
        parent[ChildNodeIndex] = ParentNodeIndex;
    }
}

void Routing::AddPathToLAST(int NodeIndex, cTopology* origTopo,cTopology* SPTTopo, int parent[], int distance[])
{
    if (distance[NodeIndex] > (int)SPTTopo->getNode(NodeIndex)->getDistanceToTarget()){
        cTopology::LinkOut* tempLink = SPTTopo->getNode(NodeIndex)->cTopology::Node::getPath(0);
        if (tempLink > 0)
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
    cTopology* newTopo = CreateTopology(OrigTopo);
    cTopology::Node* tempNode;
    int numOfLinks;
    disableAllInLinks(newTopo, false);

    int Node1ID, Node2ID;
    for (int count = 0; count < nodesCount; count++)
    {
        Node1ID = count;
        Node2ID = parent[count];
        if (Node2ID >= 0){
            tempNode = newTopo->getNode(Node1ID);
            numOfLinks = tempNode->getNumInLinks();

            for (int v=0; v< numOfLinks; v++)
            {
                if(tempNode->getLinkIn(v)->getRemoteNode()->getModule()->getIndex() == Node2ID)
                    tempNode->getLinkIn(v)->enable();
            }
            tempNode = newTopo->getNode(Node2ID);
            numOfLinks = tempNode->getNumInLinks();

            for (int v=0; v< numOfLinks; v++)
            {
                if(tempNode->getLinkIn(v)->getRemoteNode()->getModule()->getIndex() == Node1ID)
                    tempNode->getLinkIn(v)->enable();
            }
        }
    }
    return newTopo;
}


cTopology* Routing::FindLASTForRoot(cTopology* origTopo,int alpha, int RootIndex)
{
    cTopology* MSTTopo = primMST(origTopo, RootIndex);
    cTopology* SPTTopo = BuildSPT(origTopo, RootIndex);

    int nodesCount = origTopo->getNumNodes();

    int parent[nodesCount];
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
    return BuildTopoFromArray(parent,nodesCount, origTopo);
}



/**
 *LAST function to create
 *
 * BuildSPT(origTopo, RootIndex) - Done
 *
 * Find-Last(MST,SPT,Root ,alpha, Original Topo) - Done (Not Checked)
 * BuildLastTopo()
 * CombineLastTopo()
 * DFS(node, STP, MST, Original Topo) - Done (Not Checked)
 * RelaxTopo(node,parent node, dist[],parent[], Original Topo) - Done (Not Checked)
 * initialize(dist[],parent[]) - Done -> in FindLASTForRoot
 * Add-Path(node, SPT, Original Topo) - Done (Not Checked)
 * GetW(node, parent node) - Done
 * get-SPT-parent(node)
 * get-MST-child(node)
 *
 *
 *
 *
 */

void Routing::initialize()
{
    myAddress = getParentModule()->par("address");
    hasInitTopo = 0;
    packetType = 1;
    dropSignal = registerSignal("drop");
    outputIfSignal = registerSignal("outputIf");

    //
    // Brute force approach -- every node does topology discovery on its own,
    // and finds routes to all other nodes independently, at the beginning
    // of the simulation. This could be improved: (1) central routing database,
    // (2) on-demand route calculation
    //
    topo = new cTopology("topo");
    std::vector<std::string> nedTypes;

    nedTypes.push_back(getParentModule()->getNedTypeName());
    topo->extractByNedTypeName(nedTypes);
    EV << "cTopology found " << topo->getNumNodes() << " nodes\n";

    thisNode = topo->getNodeFor(getParentModule());



    Routing::disableAllInLinks(topo, true); // set G'= {}

    //Routing::checkVerticesConnectivity(topo, myAddress);
    /*
    EV << "[+] myAddress: " << myAddress << endl;
    Routing::setLinksWeight(topo,1); // Set random weights to edges in the graph
    Routing::insertLinksToList(topo); // Set std::list edges with all incoming edges in the graph
    Routing::printEdgesList();
    edges.sort([](const cTopology::LinkIn *left, const cTopology::LinkIn *right) {return left->getWeight() < right->getWeight();}); // Sort E by non-decreasing weight
    EV << "[+] Sorted Edges . . ." << endl;
    Routing::printEdgesList();

    std::list<cTopology::LinkIn*>::iterator it;
    double Puv = 0; // Weight(P(u,v))
    double We = 0; // Weight(e)
    for(it=edges.begin(); it!=edges.end(); it++)
    {
        Puv = Routing::getPuv(topo,(*it)); // Weight(P(u,v))
        We = (*it)->getWeight(); // Weight(e)
        cDisplayString& dspString = (*it)->getRemoteGate()->getDisplayString();
        cDisplayString& dspString2 = (*it)->getLocalGate()->getDisplayString();

        if(!checkIfInWeightRange(Puv))
        {
            (*it)->enable();
            EV << "[+] inf is checked and is infinity and now is enabled" << endl;
            dspString.setTagArg("ls",0,"yellow");
            dspString.setTagArg("ls",1,"4");
        }
        else if(We * PARAM < Puv)
        {
            (*it)->enable();
            EV << "[+] W(e)*r < P(u,v) and now is enabled" << endl;
            dspString.setTagArg("ls",0,"yellow");
            dspString.setTagArg("ls",1,"4");
            //cDisplayString& dispStr = (*it)->getLocalNode()->getModule()->getDisplayString();
            //dispStr.setTagArg("ls", 0, "red");
            //dspString.setTagArg("ls",1,"3");
            //EV << "display string   "<< dispStr.getTagArg("ls", 0) << endl ;
        }
        else {
            EV << "[+] LinIn is Enabled: " << (*it)->isEnabled() << endl;
            EV << "[+] getRemoteGate()->getFullName(): " << (*it)->getRemoteGate()->getFullName() <<endl;
            EV << "[+] getLocalGate Before: " << dspString <<endl;
            //dspString.setTagArg("ls",0,"red");
            dspString.setTagArg("ls",1,"0");
            EV << "[+] getLocalGate After: " << dspString <<endl;
        }
    }
     */

    //Routing::printEdgesList();
    //Routing::setLinksWeight(topo,8); // Set random weights to edges in the graph
    //FindLASTForRoot(topo, 2, 0);
    //PaintTopology(topo);
    if(1== 0)
    {
        Routing::setLinksWeight(topo,1); // Set random weights to edges in the graph
        cTopology* LASTtopo = FindLASTForRoot(topo,2,0);
        copyTopology(topo, LASTtopo);
        PaintTopology(topo);
    }

    if(this->myAddress  ==0)
    {

        Routing::setLinksWeight(topo,1); // Set random weights to edges in the graph

        cTopology *MSTtopo = primMST(topo, 1);


        cTopology::Node* furthest_node_1 = Routing::findFurthestNodeInGraph(MSTtopo,MSTtopo->getNode(0));
        EV << "[+] Furthest Node 1 Name: " << furthest_node_1->getModule()->getName() << endl;
        EV << "[+] Furthest Node 1 Index: " << furthest_node_1->getModule()->getIndex() << endl;
        cTopology::Node* furthest_node_2 = Routing::findFurthestNodeInGraph(MSTtopo,furthest_node_1);
        EV << "[+] Furthest Node 2: " << furthest_node_2->getModule()->getName() << endl;
        EV << "[+] Furthest Node 2 Index: " << furthest_node_2->getModule()->getIndex() << endl;

        EV << "[+] Before Routing::traverseDiameter" << endl;
        int diameterLength = Routing::traverseDiameter(MSTtopo, furthest_node_2,furthest_node_1);
        EV << "[+] After Routing::traverseDiameter" << endl;
        EV << "[+] Diameter length is: " << diameterLength << endl;

        copyTopology(topo, MSTtopo);

        Routing::printNodesList();


        //build an primMST tree and print it
        initRtable(topo, thisNode);

        char pkname[40];
        char *packetName = (char*)"pk-Initialize-topo";
        sprintf(pkname, "pk-%s", packetName);
        EV << "generating packet " << pkname << endl;

        _sendTime = &par("sendTime");

        _pk = new Packet(pkname);


        _pk->sethasInitTopo(1); // RTE 0 finished with topology
        hasInitTopo = 1;
        _pk->setSrcAddr(myAddress);
        _pk->setDestAddr(myAddress);
        _pk->setpacketType(1); // At the beginning of the world set the type of packet to 1 (=Initialize packet)
        Routing::LoadPacket(_pk, topo); // Copy topology to the field in the packet

        PaintTopology(topo);
        Routing::paintRouteInColor("red", 4);
        scheduleAt(_sendTime->doubleValue(), _pk);
    }

}


void Routing::handleMessage(cMessage *msg)
{
    Packet *pk = check_and_cast<Packet *>(msg);
    Packet *dupPk = NULL;
    int i=0;

    int destAddr = pk->getDestAddr();
    int srcAddr = pk->getSrcAddr();
    if (2 == 0){
        cTopology* LASTtopo = FindLASTForRoot(topo,2,0);
        copyTopology(topo, LASTtopo);
        PaintTopology(topo);
    }
    // Self message
    if(destAddr == myAddress)
    {
        if (hasInitTopo ==0)
        {
            // copyTopology
            copyTopology(topo, pk->getTopologyVar());
            initRtable(topo, thisNode);
            hasInitTopo = 1;
        }
        cTopology::Node *tempNode = topo->getNode(myAddress);
        int numInLinks = tempNode->getNumInLinks(), destNodeInd;

        for (i=0 ; i<numInLinks ; i++)
        {
            if(tempNode->getLinkIn(i)->isEnabled())
            {
                destNodeInd = tempNode->getLinkIn(i)->getRemoteNode()->getModule()->getIndex();
                if (destNodeInd != pk->getSrcAddr()){
                    dupPk = pk->dup();
                    dupPk->setDestAddr(destNodeInd);
                    dupPk->setSrcAddr(myAddress);
                    dupPk->setTopologyVar(pk->getTopologyVar());
                    RoutingTable::iterator it = rtable.find(destNodeInd);


                    int outGateIndex = (*it).second;
                    send(dupPk, "out", outGateIndex);
                }
            }
        }
    }




    if (destAddr == myAddress) {
        EV << "local delivery of packet " << pk->getName() << endl;
        send(pk, "localOut");
        emit(outputIfSignal, -1);  // -1: local
        return;
    }
    delete topo;
    /*
        // Init packet, conduct broadcast
        if (pk->get)
        {
            for( int i = 0 ; i < 10 ; i++)
            {
                //Packet dupPK = pk->dup();
                // If gate #i is enabled
                    // Send to gate #i
                    // Use an array in each iteration to identify incoming gate to prevent loops in broadcast
            }

        }
     */
    RoutingTable::iterator it = rtable.find(destAddr);
    if (it == rtable.end()) {
        EV << "address " << destAddr << " unreachable, discarding packet " << pk->getName() << endl;
        emit(dropSignal, (long)pk->getByteLength());
        delete pk;
        return;
    }

    int outGateIndex = (*it).second;
    EV << "forwarding packet " << pk->getName() << " on gate index " << outGateIndex << endl;
    pk->setHopCount(pk->getHopCount()+1);
    emit(outputIfSignal, outGateIndex);

    //send(pk, "out", outGateIndex);
}


