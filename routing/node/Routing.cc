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

#include "Routing.h"

//void Routing::finish(){
//    //a->clear();
//    topo->clear();
//}
//This function is not working correctly. The problem seems to be with the method getNode(int i)
void Routing::insertLinksToList(cTopology *topo) {
    int numInLinks = 0;

    for (int i = 0; i < topo->getNumNodes(); i++) {
        numInLinks = 0;
        numInLinks = topo->getNode(i)->getNumInLinks();
        for (int j = 0; j < numInLinks; j++) {
            edges.push_back(topo->getNode(i)->getLinkIn(j)); //lll: what with links out ???
        }
    }
}

/*
 * Args: None
 * Return: Void
 * Description: Prints out the source and destination of each LinkIn and its weight.
 */
int Routing::printEdgesList() {
    std::list<cTopology::LinkIn*>::iterator it;
    int source_node = 0;
    int dest_node = 0;
    int enabledCounter = 0;
    double wgt = 0;

    for (it = edges.begin(); it != edges.end(); it++) {
        source_node = (*it)->getRemoteNode()->getModule()->par("address");
        dest_node = (*it)->getLocalNode()->getModule()->par("address");
        wgt = (*it)->getWeight();
        if ((*it)->isEnabled()) {
            enabledCounter += 1;
        }
        if (DEBUG) {
            EV << "[+] printEdgesList:: " << "Source Node: " << source_node
                      << " Dest Node: " << dest_node << " Weight: " << wgt
                      << " Enabled:" << (*it)->isEnabled() << endl;
        }
    }
    return enabledCounter;
}

/*
 * Pass it pk = new cPacket();
 */
void Routing::LoadPacket(Packet *pk, cTopology *topo) {
    // Create a new topology, extracted from Ned types (e.g. Net60)
    cTopology *tempTopo = new cTopology("LoadPacket topo");
    std::vector<std::string> nedTypes;
    nedTypes.push_back(getParentModule()->getNedTypeName());
    tempTopo->extractByNedTypeName(nedTypes);

    // Disable all links to have a fresh start
    //lll: the bool (false) is for debugging
    Routing::disableAllInLinks(tempTopo, false);

    // Copy topology to send via packet (topo) to a new instance (tempTopo)
    Routing::copyTopology(tempTopo, topo, "green");

    // Packets for dispatching the topology will be identified as INIT packets
 //   pk->setName(pk->getName());
    // pk->setByteLength(l) //
    pk->setTopologyVar(tempTopo);

   // delete tempTopo;
}

void Routing::printNodesList() {
    std::list<cTopology::Node*>::iterator it;
    int temp;int kaka = 0;

    for (it = Nodes.begin(); it != Nodes.end(); it++) {
        //temp = (*it)->getModule()->par("address");
        temp = (*it)->getModule()->getIndex();
        kaka++;

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
void Routing::disableAllInLinks(cTopology *topo, bool debugging) {
    int numInLinks = 0,numInOut = 0;

    for (int i = 0; i < topo->getNumNodes(); i++) {
        numInLinks = 0;
        numInLinks = topo->getNode(i)->getNumInLinks();
        for (int j = 0; j < numInLinks; j++)
        {
            if (debugging)topo->getNode(i)->getLinkIn(j)->setWeight(-1);
            else
            {
                topo->getNode(i)->getLinkIn(j)->disable();
                cDisplayString& dispStr =
                        topo->getNode(i)->getLinkIn(j)->getRemoteGate()->getDisplayString();
                dispStr.setTagArg("ls", 0, "red");
                dispStr.setTagArg("ls", 1, "0");
            }
        }
        numInOut = 0;
        numInOut = topo->getNode(i)->getNumOutLinks();
        for (int j = 0; j < numInOut; j++)
        {
            if (debugging)topo->getNode(i)->getLinkOut(j)->setWeight(-1);
            else
            {
                topo->getNode(i)->getLinkOut(j)->disable();
                cDisplayString& dispStr =
                        topo->getNode(i)->getLinkOut(j)->getRemoteGate()->getDisplayString();
                dispStr.setTagArg("ls", 0, "red");
                dispStr.setTagArg("ls", 1, "0");
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
void Routing::checkVerticesConnectivity(cTopology *topo, int address) {
    int adrs = 0;

    EV << "[+] checkVerticesConnectivity:: Address is " << address << endl;

    for (int i = 0; i < topo->getNumNodes(); i++) {
        if (i == address)
            continue;

        topo->calculateWeightedSingleShortestPathsTo(topo->getNode(i));

        adrs = topo->getNode(i)->getModule()->par("address");

        if (topo->getNode(i)->getNumPaths() == 0)
            EV << "[+] checkVerticesConnectivity:: Node " << adrs
                      << " is not connected" << endl;
        else
            EV << "[+] checkVerticesConnectivity:: Node " << adrs
                      << " is connected" << endl;
    }
}

/*
 * Args: cTopology topo, source Node src and destination Node dest
 * Return: distance of shortest path from source to destination
 * Description: Find P(u,v) where u= Node* src and v= Node* dest.
 *                              Using calculateWeightedSingleShortestPathsTo function
 */
double Routing::findWeightOfShortestPath(cTopology *topo, cTopology::Node* src,
        cTopology::Node* dest) {

    double source_dist = 0;

    topo->calculateWeightedSingleShortestPathsTo(dest);

    source_dist = src->getDistanceToTarget();

    if (DEBUG) {
        EV << "[+] findWeightOfShortestPath:: Distance from source to dest is: "
                  << source_dist << endl;
    }

    return source_dist;
}

void Routing::setDualSideWeights(cTopology *MyTopo, int Node1ID, double wgt)
{
    int Node2ID = 0;
    int numOfLinks, numInLinks, N2numInLinks,N2numOutLinks;
    numInLinks = MyTopo->getNode(Node1ID)->getNumInLinks();
    cTopology::Node* tempNode = MyTopo->getNode(Node1ID);
    cTopology::Node* N2tempNode;

    for (int j = 0; j < numInLinks; j++)
    {
        Node2ID = MyTopo->getNode(Node1ID)->getLinkIn(j)->getRemoteNode()->getModule()->getIndex();
        if (Node2ID >= 0)
        {
            if (MyTopo->getNode(Node1ID)->getLinkIn(j)->getWeight() == -1)
                MyTopo->getNode(Node1ID)->getLinkIn(j)->setWeight(wgt);
        }
        N2tempNode = MyTopo->getNode(Node2ID);
        N2numInLinks = N2tempNode->getNumInLinks();
        for (int v = 0; v < N2numInLinks; v++) {
            if (N2tempNode->getLinkIn(v)->getRemoteNode()->getModule()->getIndex() == Node1ID
                    && MyTopo->getNode(Node2ID)->getLinkIn(v)->getWeight() == -1)
                MyTopo->getNode(Node2ID)->getLinkIn(v)->setWeight(wgt);
        }
    }
    int numOutLinks = MyTopo->getNode(Node1ID)->getNumOutLinks();
    for (int j = 0; j < numOutLinks; j++)
    {
        Node2ID = MyTopo->getNode(Node1ID)->getLinkOut(j)->getRemoteNode()->getModule()->getIndex();
        if (Node2ID >= 0)
        {
            if (MyTopo->getNode(Node1ID)->getLinkOut(j)->getWeight() == -1)
                MyTopo->getNode(Node1ID)->getLinkOut(j)->setWeight(wgt);
        }
        N2tempNode = MyTopo->getNode(Node2ID);
        N2numOutLinks = N2tempNode->getNumOutLinks();
        for (int v = 0; v < N2numOutLinks; v++) {
            if (N2tempNode->getLinkOut(v)->getRemoteNode()->getModule()->getIndex() == Node1ID
                    && MyTopo->getNode(Node2ID)->getLinkOut(v)->getWeight() == -1)
                MyTopo->getNode(Node2ID)->getLinkOut(v)->setWeight(wgt);
        }
    }
}

void Routing::setDualSideWeightsByQueueSize(cTopology *MyTopo, int Node1ID) {
      int Node2ID = 0;
      int numOfLinks, numInLinks, N2numInLinks,N2numOutLinks;
      numInLinks = MyTopo->getNode(Node1ID)->getNumInLinks();
      cTopology::Node* tempNode = MyTopo->getNode(Node1ID);
      cTopology::Node* N2tempNode;
      double maxwgt;
      for (int j = 0; j < numInLinks; j++)
      {
          //Node2ID the Node1ID connected to with link j
          Node2ID = MyTopo->getNode(Node1ID)->getLinkIn(j)->getRemoteNode()->getModule()->getIndex();

          maxwgt = (n_table[Node1ID][Node2ID] >= n_table[Node2ID][Node1ID]) ?  n_table[Node1ID][Node2ID] : n_table[Node2ID][Node1ID];
          MyTopo->getNode(Node1ID)->getLinkIn(j) -> setWeight(maxwgt);
          int tempNeighbor[6];
          int linkindex = -1;
          int *arr = getNeighborsArr(Node2ID,tempNeighbor);
          int neighboor = tempNeighbor[getNeighboorIndex(j)];
          for(int k=0;k<6;k++)
              if(tempNeighbor[k] == Node1ID)
                  linkindex = getLinkIndex(k);
          if(linkindex == -1)
              exit(1321);
          MyTopo->getNode(Node2ID)->getLinkIn(linkindex) -> setWeight(maxwgt);
      }
}

void Routing::verifyWeightsForTopo() {
    int Node2ID = 0;
    int numOflinks;
    cTopology::Node * tempNode;
    for (int i = 0; i < newTopo->getNumNodes(); i++) {
        tempNode = newTopo->getNode(i);
        setDualSideWeightsByQueueSize(newTopo, tempNode->getModule()->getIndex());
    }
}

/*
 * Args: cTopology variable, weight
 * Return: Void
 * Description: Set weights to LinkIn objects (edges), since this is the actual object that counts when computing
 * 'calculateWeightedSingleShortestPathsTo' (Disjktra Algorithm)
 */
void Routing::setLinksWeight(cTopology *MyTopo, double wgt) {
    int numInLinks = 0, Node2ID, Node1ID, numOfLinks;
    cTopology::Node* tempNode;
    cTopology::Node* temp;
    srand(time(NULL));
    int index=0;

    switch ((int) wgt) {
    case 0: // random

        for (int i = 0; i < MyTopo->getNumNodes(); i++) {
            wgt = truncnormal(50,20);//intuniform(80, 100, 0);//((int)simTime().dbl()*getSimulation()->getEventNumber())%117);

            Node1ID = i;
            setDualSideWeights(MyTopo, Node1ID, wgt);

        }
        break;

        //lll we change here to received link info from all topos.

    case 10:
        //lll: set the received values of each link
        for(int i=0;i<MyTopo->getNumNodes();i++)
        {
            setDualSideWeightsByQueueSize(MyTopo, i);
//            wgt = truncnormal(50,20);
//            setDualSideWeights(MyTopo, i, wgt);
        }

       break;
    case 1: //all ones - distributed
        for (int i = 0; i < MyTopo->getNumNodes(); i++) {
            numInLinks = 0;
            numInLinks = MyTopo->getNode(i)->getNumInLinks();
            for (int j = 0; j < numInLinks; j++) {
                MyTopo->getNode(i)->getLinkIn(j)->setWeight(1);
            }
        }break;

    case 2: //predefined const values - for debugging

        for (int i = 0; i < MyTopo->getNumNodes(); i++) {
            numInLinks = 0;
            numInLinks = MyTopo->getNode(i)->getNumInLinks();
            Node1ID = i;
            for (int j = 0; j < numInLinks; j++) {
                Node2ID =
                        MyTopo->getNode(i)->getLinkIn(j)->getRemoteNode()->getModule()->getIndex();
                if (Node2ID >= 0) {
                    tempNode = MyTopo->getNode(Node1ID);
                }
                wgt = i * j;
                if (wgt == 0)
                    wgt = 1;
                MyTopo->getNode(i)->getLinkIn(j)->setWeight(wgt);
            }
        }
        break;

    case 8:

        for (int i = 0; i < topo->getNumNodes(); i++) {
            numInLinks = 0;
            numInLinks = topo->getNode(i)->getNumInLinks();

            Node1ID = i;
            for (int j = 0; j < numInLinks; j++) {
                Node2ID =
                        topo->getNode(i)->getLinkIn(j)->getRemoteNode()->getModule()->getIndex();
                switch (Node1ID) {
                case 0:
                    switch (Node2ID) {
                    case 1:
                        wgt = 10;
                        break;
                    case 3:
                        wgt = 15;
                        break;
                    case 4:
                        wgt = 15;
                        break;
                    case 5:
                        wgt = 15;
                        break;
                    case 7:
                        wgt = 5;
                        break;
                    }
                    break;
                case 1:
                    if (Node2ID == 2)
                        wgt = 10;
                    break;
                case 2:
                    if (Node2ID == 3)
                        wgt = 10;
                    break;
                case 3:
                    if (Node2ID == 4)
                        wgt = 10;
                    else if (Node2ID == 5)
                        wgt = 5;
                    break;
                case 5:
                    if (Node2ID == 6)
                        wgt = 10;
                    break;
                case 6:
                    if (Node2ID == 7)
                        wgt = 10;
                    break;
                }
                if (Node2ID >= 0) {

                    tempNode = topo->getNode(Node1ID);
                    numOfLinks = tempNode->getNumInLinks();
                    for (int v = 0; v < numOfLinks; v++) {
                        if (tempNode->getLinkIn(v)->getRemoteNode()->getModule()->getIndex()
                                == Node2ID
                                && topo->getNode(Node1ID)->getLinkIn(v)->getWeight()
                                        == -1)
                            topo->getNode(Node1ID)->getLinkIn(v)->setWeight(
                                    wgt);
                    }
                    tempNode = topo->getNode(Node2ID);
                    numOfLinks = tempNode->getNumInLinks();

                    for (int v = 0; v < numOfLinks; v++) {
                        if (tempNode->getLinkIn(v)->getRemoteNode()->getModule()->getIndex()
                                == Node1ID
                                && topo->getNode(Node2ID)->getLinkIn(v)->getWeight()
                                        == -1)
                            topo->getNode(Node2ID)->getLinkIn(v)->setWeight(
                                    wgt);
                    }
                }
            }
        }
        break;

    default:

        for (int i = 0; i < topo->getNumNodes(); i++) {
            numInLinks = 0;
            numInLinks = topo->getNode(i)->getNumInLinks();
            for (int j = 0; j < numInLinks; j++) {
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
double Routing::getPuv(cTopology *topo, cTopology::LinkIn* edge) {
    double Puv = 0;

    cTopology::Node* u = edge->getRemoteNode(); // From Node
    cTopology::Node* v = edge->getLocalNode(); // To Node

    Puv = Routing::findWeightOfShortestPath(topo, u, v); // Get P(u,v) - the shortest path weight from u to v

    return Puv;
}

bool Routing::checkIfInWeightRange(double wgt) {
    return (wgt > MIN_VALUE && wgt < MAX_VALUE);
}

cTopology::Node* Routing::findFurthestNodeInGraph(cTopology *topo,cTopology::Node *destNode)
{
    cTopology::Node *tempNode = NULL, *resNode = NULL;
    int nodesCount = topo->getNumNodes(), dist = 0;
    int currDist = 0;
   // checkVerticesConnectivity(topo,destNode->getModuleId());
    topo->calculateWeightedSingleShortestPathsTo(destNode);
    for (int i = 0; i < nodesCount; i++)
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
int Routing::minKey(int key[], bool mstSet[], int NodeCount) {
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
cTopology * Routing::primMST(cTopology *OrigTopo, int RootNodeInd) {
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
    for (int i = 0; i < nodesCount; i++) {
        MstSet[i] = false;
        key[i] = INT_MAX;
        parent[i] = -2;
    }
    // Always include first 1st vertex in MST.
    // Make key 0 so that this vertex is picked as first vertex.

    //lll: key[RootNodeInd] = 0 - need to change when going distributed
    key[RootNodeInd] = RootNodeInd;//0;
    parent[RootNodeInd] = -1; // First node is always root of MST
    int u;
    for (int count = 0; count < nodesCount; count++) {

        u = minKey(key, MstSet, nodesCount);

        tempNode = OrigTopo->getNode(u);
        MstSet[u] = true;
        numOfLinks = tempNode->getNumInLinks();

        for (int v = 0; v < numOfLinks; v++) {
            tempNodeNieghbor =
                    tempNode->getLinkIn(v)->getRemoteNode()->getModule()->getIndex();
            edgeWeight = (int) tempNode->getLinkIn(v)->getWeight();
            if (MstSet[tempNodeNieghbor] == false
                    && edgeWeight < key[tempNodeNieghbor]) {
                parent[tempNodeNieghbor] = u;
                key[tempNodeNieghbor] = edgeWeight;
            }
        }
    }
    cTopology *MSTtopology = BuildTopoFromArray(parent, nodesCount, OrigTopo);

    MSTtopology->calculateWeightedSingleShortestPathsTo(
            MSTtopology->getNode(RootNodeInd));

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
int Routing::traverseDiameter(cTopology *topo, cTopology::Node *source,
        cTopology::Node *dest) {
    cTopology::Node *tempNode;
    cTopology::LinkOut *tempLinkOut;
    int hopCount = 0;
    int sourceIndex = 0, destIndex = 0, tempNodeIndex = 0;

    Nodes.clear();

    sourceIndex = source->getModule()->getIndex();
    destIndex = dest->getModule()->getIndex();

    if (DEBUG) {
        EV << "[+] Routing::traverseDiameter:: Source Index: " << sourceIndex
                  << endl;
        EV << "[+] Routing::traverseDiameter:: Dest Index: " << destIndex
                  << endl;
    }

    Nodes.push_back(source);
    tempLinkOut = source->cTopology::Node::getPath(0);
    tempNode = tempLinkOut->getRemoteNode();
//    if (DEBUG) {
//        EV << "[+] Routing::traverseDiameter:: Before while" << endl;
//    }
    while (tempNode != dest) {
        Nodes.push_back(tempNode);
        tempNodeIndex = tempNode->getModule()->getIndex();
        EV << "[+] tempNode Index: " << tempNodeIndex << endl;
        hopCount = hopCount + 1;
        tempNode = tempNode->cTopology::Node::getPath(0)->getRemoteNode(); //If not it should be local node
    }

    hopCount = hopCount + 1;
    Nodes.push_back(dest);
//    if (DEBUG) {
//        EV << "[+] Routing::traverseDiameter:: After while" << endl;
//    }

    return hopCount;
}

// Input: Global variable List if nodes (Nodes) - Doesn't pass that but uses it,
//color(ex. "red", "yellow", "green", width (ex. "1", "4")
// Return: void
void Routing::paintRouteInColor(const char* color, int width) {
    std::list<cTopology::Node*>::iterator it;
    cTopology::LinkOut *tempLinkOut;
    int currNodeIndex = 0;
    int numPath = 0;

    for (it = Nodes.begin(); it != Nodes.end(); it++) {
        numPath = (*it)->getNumPaths();
        if (numPath != 0) {
            tempLinkOut = (*it)->cTopology::Node::getPath(0);
            cDisplayString& dspString = (cDisplayString&) tempLinkOut->getLocalGate()->getDisplayString();
            currNodeIndex = (*it)->getModule()->getIndex();
            if (DEBUG) {
                EV
                          << "[+] Routing::paintRouteInColoer:: Coloring Node with Index: "
                          << currNodeIndex << " in " << color << endl;
            }
            dspString.setTagArg("ls", 0, color);
            dspString.setTagArg("ls", 1, width);
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
void Routing::copyTopology(cTopology* OrigTopo, cTopology* NewTopo,
        const char* colour) {
    //disable all the links in the original topology
    disableAllInLinks(OrigTopo, false);

    int nodesCount = OrigTopo->getNumNodes();
    cTopology::Node *tempOrigNode, *tempNewNode;
    int edgeCount;
    double edgeWeight;
    //run over all of the nodes in the topology
    for (int i = 0; i < nodesCount; i++) {
        //get the nodes from both topologies
        tempOrigNode = OrigTopo->getNode(i);
        tempNewNode = NewTopo->getNode(i);
        edgeCount = tempOrigNode->getNumInLinks();

        //run over all of the links of the node
        for (int u = 0; u < edgeCount; u++)
        {
            //if the link is enabled in the new topology, update the original topology and paint the connection
            if (tempNewNode->getLinkIn(u)->isEnabled())
            {
                tempOrigNode->getLinkIn(u)->enable();
                edgeWeight = tempNewNode->getLinkIn(u)->getWeight();
                tempOrigNode->getLinkIn(u)->setWeight(edgeWeight);
             //   if (DEBUG) {EV << "[+] copyTopology - Adding link From Node "  << tempOrigNode->getModule()->getIndex()<< " To Node "<< tempOrigNode->getLinkIn(u)->getRemoteNode()->getModule()->getIndex()<< endl;}
            }
        }
        edgeCount = tempOrigNode->getNumOutLinks();
        for (int u = 0; u < edgeCount; u++)
        {
            //if the link is enabled in the new topology, update the original topology and paint the connection
            if (tempNewNode->getLinkOut(u)->isEnabled())
            {
                tempOrigNode->getLinkOut(u)->enable();
                edgeWeight = tempNewNode->getLinkOut(u)->getWeight();
                tempOrigNode->getLinkOut(u)->setWeight(edgeWeight);
              //  if (DEBUG) {EV << "[+] copyTopology - Adding link From Node "  << tempOrigNode->getModule()->getIndex()<< " To Node "<< tempOrigNode->getLinkIn(u)->getRemoteNode()->getModule()->getIndex()<< endl;}
            }
        }
    }
//    if (DEBUG) {
//        EV << "[+] copyTopology - Done!" << endl;
//    }
    PaintTopology(topo, colour);
//    if (hasGUI())
//        getParentModule()->bubble("Topology Updated!");
}

void Routing::setActiveLinks()
{
    int countActiveLinks = 0;
    for(int i=0;i< topo->getNode(myAddress)->getNumInLinks();i++)
    {
        active_links_array[i] = -1;
        if(topo->getNode(myAddress)->getLinkIn(i)->isEnabled())
        {
            countActiveLinks++;
            active_links_array[i] = neighbors[getNeighboorIndex(i)];
        }
    }
    active_links = countActiveLinks;
}
cTopology* Routing::CreateTopology(cTopology *Origtopo, const char* topoName) {
    cTopology *NewTopology = new cTopology(topoName);
    std::vector<std::string> nedTypes;

    nedTypes.push_back(getParentModule()->getNedTypeName());
    NewTopology->extractByNedTypeName(nedTypes);
//    if (DEBUG) {
//        EV << "[+] CreateTopology- calling copyTopology " << endl;
//    }
    copyTopology(NewTopology, Origtopo, "green");
    return NewTopology;
}

void Routing::PaintTopology(cTopology* OrigTopo, const char* colour) {
    int nodesCount = OrigTopo->getNumNodes();
    cTopology::Node *tempOrigNode;
    int edgeCount;
    //run over all of the nodes in the topology
    for (int i = 0; i < nodesCount; i++) {
        //get the nodes from both topologies
        tempOrigNode = OrigTopo->getNode(i);
        edgeCount = tempOrigNode->getNumInLinks();

        //run over all of the links of the node
        for (int u = 0; u < edgeCount; u++) {
            //if the link is enabled in the new topology, update the original topology and paint the connection
            if (tempOrigNode->getLinkIn(u)->isEnabled()) {
                cDisplayString& dspString =(cDisplayString&) tempOrigNode->getLinkIn(u)->getRemoteGate()->getDisplayString();
                dspString.setTagArg("ls", 0, colour);
                dspString.setTagArg("ls", 1, 4);
            }
        }
    }
}

// Returns the index of satellite i
int getSatIndex(cTopology::Node *node) {
    return node->getModule()->getIndex();
}

void Routing::initRtable(cTopology* thisTopo, cTopology::Node *thisNode) {
    // find and store next hops
    for (int i = 0; i < thisTopo->getNumNodes(); i++) {
        if (thisTopo->getNode(i) == thisNode)
            continue;  // skip ourselves
        thisTopo->calculateWeightedSingleShortestPathsTo(thisTopo->getNode(i));

        if (thisNode->getNumPaths() == 0)
            continue;  // not connected

        cGate *parentModuleGate = thisNode->getPath(0)->getLocalGate();
//        cGate *check11;
//        int a = 0;
//        for(int j = 0; j<topo->getNode(myAddress)->getNumInLinks();j++)
//        {
//            check11 = topo->getNode(myAddress)->getLinkIn(j)->getLocalGate();
//            if(check11 -> getIndex() == parentModuleGate -> getIndex())
//                {
//                    a++;
//                    a++;
//                }
//        }
//
//        for(int j = 0; j<topo->getNode(myAddress)->getNumOutLinks();j++)
//        {
//            check11 = topo->getNode(myAddress)->getLinkOut(j)->getLocalGate();
//            if(check11 -> getIndex() == parentModuleGate -> getIndex())
//            {
//                a++;
//                a++;
//            }
//        }
        int gateIndex = parentModuleGate->getIndex();
        int address = thisTopo->getNode(i)->getModule()->par("address");
        rtable[address] = gateIndex;
        if (DEBUG) {
            EV << "  towards address " << address << " gateIndex is "
                      << gateIndex << endl;
        }
    }

}

int Routing::GetW(cTopology::Node *nodeU, cTopology::Node *nodeV) {

    int numOfLinks = nodeV->getNumInLinks();
    cTopology::LinkIn* checkedLink;
    for (int i = 0; i < numOfLinks; i++) {
        checkedLink = nodeV->getLinkIn(i);
        if (checkedLink->getRemoteNode()->getModule()->getIndex()
                == nodeU->getModule()->getIndex() && checkedLink->isEnabled()) {
            return checkedLink->getWeight();
        }
    }
    return 0;
}

cTopology* Routing::BuildSPT(cTopology* origTopo, int RootIndex) {
    if (DEBUG) {
        EV << "[+] BuildSPT- calling CreateTopology " << endl;
    }
    cTopology* SPTtopo = CreateTopology(origTopo,"SPT topology");
    SPTtopo->calculateWeightedSingleShortestPathsTo(SPTtopo->getNode(RootIndex));
    return SPTtopo;

}

void Routing::RelaxTopo(int ChildNodeIndex, int ParentNodeIndex,
        cTopology* origTopo, int parent[], int distance[])
{
    int LinkWeight = GetW(origTopo->getNode(ChildNodeIndex),origTopo->getNode(ParentNodeIndex));
    int compareWeight = distance[ParentNodeIndex] + LinkWeight;

    if (distance[ChildNodeIndex] > compareWeight)
    {
        distance[ChildNodeIndex] = compareWeight;
        parent[ChildNodeIndex] = ParentNodeIndex;
    }
}

void Routing::AddPathToLAST(int NodeIndex, cTopology* origTopo,
        cTopology* SPTTopo, int parent[], int distance[]) {
    if (distance[NodeIndex] > (int) SPTTopo->getNode(NodeIndex)->getDistanceToTarget()) {
        cTopology::LinkOut* tempLink = SPTTopo->getNode(NodeIndex)->cTopology::Node::getPath(0);
        if (tempLink != NULL) {
            int ParentNodeIndex = tempLink->getRemoteNode()->getModule()->getIndex();
            AddPathToLAST(ParentNodeIndex, origTopo, SPTTopo, parent, distance);
            RelaxTopo(NodeIndex, ParentNodeIndex, origTopo, parent, distance);
        }
    }
}

void Routing::DFS(int NodeIndex, int RootIndex, cTopology* origTopo,
    cTopology* MSTTopo, cTopology* SPTTopo, int alpha, int parent[],
    int distance[],int color[])
{
    if(color[NodeIndex] == 2)//2 - black
        return;


    int ParentNodeIndex, ChildNodeIndex;
    int multiplySPT = alpha * SPTTopo->getNode(NodeIndex)->getDistanceToTarget();
    if (distance[NodeIndex] > multiplySPT) {
        AddPathToLAST(NodeIndex, origTopo, SPTTopo, parent, distance);
    }
    cTopology::Node* MSTNode = MSTTopo->getNode(NodeIndex);
    int NumOfLinks = MSTNode->getNumInLinks();
    if (NodeIndex == RootIndex)
        ParentNodeIndex = -1;
    else
        ParentNodeIndex = MSTNode->cTopology::Node::getPath(0)->getRemoteNode()->getModule()->getIndex();
    color[NodeIndex] = 1;//1 - grey
    for (int i = 0; i < NumOfLinks; i++)
    {
        if (MSTNode->getLinkIn(i)->isEnabled())
        {
            ChildNodeIndex = MSTNode->getLinkIn(i)->getRemoteNode()->getModule()->getIndex();
            if(color[ChildNodeIndex] != 0)
                continue;
            if (ChildNodeIndex != ParentNodeIndex)
            {
                RelaxTopo(ChildNodeIndex, NodeIndex, origTopo, parent,distance);
                DFS(ChildNodeIndex, RootIndex, origTopo, MSTTopo, SPTTopo, alpha, parent, distance,color);
                RelaxTopo(NodeIndex, ChildNodeIndex, origTopo, parent, distance);
            }
        }
    }
    color[NodeIndex] = 2;//2 black
}

cTopology* Routing::BuildTopoFromArray(int parent[], int nodesCount,
        cTopology* OrigTopo) {
    if (DEBUG) {
        EV << "[+] BuildTopoFromArray- calling CreateTopology " << endl;
    }
    cTopology* newTopo = CreateTopology(OrigTopo,"BuildTopoFromArray" );
    cTopology::Node* tempNode;
    disableAllInLinks(newTopo, false);

    int Node1ID, Node2ID;
    for (int count = 0; count < nodesCount; count++) {
        Node1ID = count;
        Node2ID = parent[count];
        if (Node2ID >= 0) {
            tempNode = newTopo->getNode(Node1ID);
            changeConnectingLink(tempNode, Node2ID, true);

            tempNode = newTopo->getNode(Node2ID);
            changeConnectingLink(tempNode, Node1ID, true);
        }
    }
    return newTopo;
}

cTopology* Routing::BuildTopoFromMultiArray(int** parentArrays, int ArrayCount,cTopology* OrigTopo) {
    if (DEBUG){
         EV << "[+] BuildTopoFromMultiArray- calling CreateTopology " << endl;
     }
     cTopology* newTopoFromArr = CreateTopology(OrigTopo, "LAST topology");
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
    int color[num_of_hosts];
        for(int j=0;j<num_of_hosts;j++)color[j]=0;//while.1-grey.2-black

    // Key values used to pick minimum weight edge in cut
    int distance[nodesCount];


    // Initialize all keys as INFINITE
    for (int i = 0; i < nodesCount; i++){
        distance[i] = INT_MAX;
        parent[i] = -2;
    }
    parent[RootIndex] = 0;
    distance[RootIndex] = 0;

    DFS(RootIndex,RootIndex, origTopo, MSTTopo, SPTTopo, alpha, parent, distance,color);
    return parent;
}

int Routing::findDiameter(cTopology* currTopo, int startPoint) {
    //TraverseDiameter
    cTopology::Node* furthest_node_1 = findFurthestNodeInGraph(currTopo,
            currTopo->getNode(startPoint));
    cTopology::Node* furthest_node_2 = findFurthestNodeInGraph(currTopo,
            furthest_node_1);
    int diameterLength = traverseDiameter(currTopo, furthest_node_2,
            furthest_node_1);
//    if (DEBUG) {
//        EV << "[+] Diameter length is: " << diameterLength << endl;
//    }
    return diameterLength;
}

void Routing::changeConnectingLink(cTopology::Node* currNode, int otherNodeInd,
        bool state) {
    int numOfLinks = currNode->getNumInLinks();

    for (int v = 0; v < numOfLinks; v++)
    {
        if (currNode->getLinkIn(v)->getRemoteNode()->getModule()->getIndex() == otherNodeInd)
        {
            if (state == true) {
                currNode->getLinkIn(v)->enable();
                currNode->getLinkOut(v)->enable();
            } else {
                currNode->getLinkIn(v)->disable();
                currNode->getLinkOut(v)->disable();
            }
        }
    }
}
void Routing::addWeightsToTopo(Packet* pk) {
    int nodeIndex = pk->getSrcAddr();
    int linkWeight = 0;
    cTopology::Node* tempNode = newTopo->getNode(nodeIndex);
    for (int i = 0; i < (int) pk->getPacketListArraySize(); i++) {
        linkWeight = (pk->getPacketList(i) > 0) ? pk->getPacketList(i) : 1;
        if (tempNode->getLinkIn(i)->getWeight() < linkWeight)
            tempNode->getLinkIn(i)->setWeight(linkWeight);
    }
}

void Routing::addSelfWeightsToTopo() {
    cTopology::Node* tempNode = newTopo->getNode(ROOTNODE);
    int linkWeight;
    for (int i = 0; i < thisNode->getNumInLinks(); i++) {
        linkWeight = (linkCounter[i] > 0) ? linkCounter[i] : 1;
        if (tempNode->getLinkIn(i)->getWeight() < linkWeight)
            tempNode->getLinkIn(i)->setWeight(linkWeight);
    }
}
//llllast
cTopology* Routing::BuildLASTtopo(cTopology* origTopo,int alpha,int m)
{
       int* LASTparentArr [num_of_hosts/m];
       rootNodesArr[0]=topo->getNode(intuniform(0,4))->getModule()->getIndex();
       rootNodesArr[1]=topo->getNode(intuniform(5,9))->getModule()->getIndex();
       rootNodesArr[2]=topo->getNode(intuniform(10,14))->getModule()->getIndex();
       rootNodesArr[3]=topo->getNode(intuniform(15,19))->getModule()->getIndex();
       rootNodesArr[4]=topo->getNode(intuniform(20,24))->getModule()->getIndex();
       rootCounter=5;
       for(int i=0;i<rootCounter;i++)
       {
           int* parent = new int[num_of_hosts];
           LASTparentArr[i] = FindLASTForRoot(origTopo, alpha, rootNodesArr[i], parent);
       }
       cTopology* completeTopo = BuildTopoFromMultiArray(LASTparentArr, rootCounter, origTopo);
       return completeTopo;
}
void Routing::ScheduleNewTopoPacket() {
    char pkname[40];
    char *packetName = (char*) "pk-Initialize-topo";
    sprintf(pkname, "pk-%s", packetName);
    if (DEBUG) {
        EV << "generating packet " << pkname << endl;
    }

    _pk = new Packet(pkname);

    //set a unique ID for the topology created so that only the packets that are related to this topology will pass on
    currTopoID = rand() % 10000 + 1;
    ;
    _pk->setHasInitTopo(1); // RTE 0 finished with topology
    hasInitTopo = 0;
    _pk->setSrcAddr(myAddress);
    _pk->setDestAddr(myAddress);
    _pk->setPacketType(initiale_packet); // At the beginning of the world set the type of packet to 1 (=Initialize packet)
    _pk->setTopologyID(currTopoID);
    Routing::LoadPacket(_pk, topo); // Copy topology to the field in the packet
    scheduleAt(simTime() + _sendTime->doubleValue(), _pk);
}

void Routing::SendDuplicateTopology(Packet *pk, int destNodeInd) {
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
//lll:insert in the right cell and return 1 if the array is full or -1 if not

//for future use (link 5)
int Routing:: getNumOfLinks(){
    return 4;
}
//lll: triggering disterbuted leader selection packet
void Routing::scheduleChoosingLeaderPhaseOne() {
    Packet *phase1_initiate = new Packet("Selecting Leader Phase 1 update!");
    phase1_initiate->setSrcAddr(myAddress);
    phase1_initiate->setDestAddr(myAddress);
    phase1_initiate->setPacketType(phase1_update);
    phase1_initiate->setTopologyID(currTopoID);
    phase1_initiate -> setByteLength(1);
    scheduleAt(simTime(), phase1_initiate);
}

//lll:Send choosing leader phase 1 update message
void Routing::send_phase1_update(){
    int rootGateIndex;
    if(myAddress == 4)
        debugCounter++;
 //   flushQueue(phase1_update);//delete previous updates
        for (int i = 0; i < topo->getNode(myAddress)->getNumInLinks(); i++) {
            if (neighbors[i] != -1)
            {

                //creating update packet
                Packet *phase1_update_message = new Packet( "Selecting Leader Phase 1 update!");
                phase1_update_message->setSrcAddr(myAddress);
                phase1_update_message->setPacketType(phase1_update);
                phase1_update_message->setDestAddr(neighbors[i]);
                phase1_update_message->setTopologyID(currTopoID);
                phase1_update_message -> setByteLength(1);

                // leader_table is updated, the update message is the leader_table data
                for (int j = 0; j < num_of_hosts; j++)
                    phase1_update_message->data[j] = leader_table[j];

                //find the gate to node neighboors
                RoutingTable::iterator iter = rtable.find(neighbors[i]);
                rootGateIndex = (*iter).second;
               //delayTime = exponential( 1, intrand(6));
            //    delayTime = (simtime_t) _sendTime->doubleValue(); // one time read
              //  delayTime = (double)_sendTime->read();
                volatile double dddd = this->par("sendTime");
                if(phase1Flag)

                delayTime = 0;//exponential(0.01);
                sendDelayed(phase1_update_message,delayTime,"out",rootGateIndex);
            }// end if neighbors[i] != -1
        }//end for
        if(hasGUI())
            getParentModule()->bubble("sending phase1 updates");
}// end send_phase1_update()

//initialize leader table
void Routing::initial_leader_table() {
    //learn from the last update
    for (int i = 0; i < num_of_hosts; i++)
            leader_table[i] = -1;
}

//lll: Update the Node leader_table and check's if leader can be choosen. given id's are non negetive numbers. the tables holds num_of_hosts id's
void Routing::update_leader_table(Packet *pk) {
    //lll: for net100 :  run from 0 to 99
     int min = 101;

     if(myAddress == 0)
         debugCounter++;
    //learn from the last update
    for (int i = 0; i < num_of_hosts; i++)
    {
        if (pk->data[i] != -1)
            leader_table[i] = pk->data[i];

        //check min - leader
        if ( leader_table[i] <= min )
                  min = leader_table[i];
    }
    //leader has been found and he will initiate the next phase
    if (min != -1 && min == myAddress){
        phase1Flag = 0;
        leader = min;
        initial_leader_list();
        scheduleSendLinkInfo();
    } else if( min != -1 ){
        phase1Flag = 0;
        //send_phase1_update();
    }
}
void Routing::reset_ntable()
{
    for(int i = 0; i < num_of_hosts; i++)
        for(int j= 0; j<num_of_hosts;j++)
            n_table[i][j]=-0.9;
}
//Only the leader run this function. asks for links information to buid LAST topology
void Routing::scheduleSendLinkInfo() {

    flushQueue(phase1_update);
    int rootGateIndex;
    update_weight_table();
    for( int i = 0; i < MAXAMOUNTOFLINKS; i++ )
        if(neighbors[i] != -1)
            n_table[leader][neighbors[i]] = (double)queueSize[getLinkIndex(i)] / frameCapacity * 100;//(datarate/12000) * 100;

    for(int i = 0; i < num_of_hosts; i++){
        if(i != myAddress){
            Packet *phase2_initiate = new Packet("Send link info to Leader!");
             phase2_initiate->setSrcAddr(myAddress);
             phase2_initiate->setDestAddr(i);
             phase2_initiate->setPacketType(phase2_link_info);
             phase2_initiate->setTopologyID(currTopoID);
             phase2_initiate -> setByteLength(1);

             //find the gate to node neighboors
             RoutingTable::iterator iter = rtable.find(i);
             rootGateIndex = (*iter).second;
             delayTime = 0;//exponential(0.01);
             sendDelayed(phase2_initiate,delayTime,"out",rootGateIndex);
        }
    }

}

//initialize leader list
void Routing::initial_leader_list() {
    //learn from the last update
    for (int i = 0; i < num_of_hosts; i++)
    {
//            if(neighbors[i])
            leader_list[i]  =  -1 ;
    }
}


//only choosen leader use this function to initiate the third phase - distribute the topology
void Routing::schduleNewTopology() {
    int rootGateIndex;
    flushQueue(phase1_update);
    Routing::disableAllInLinks(topo, true); // set G'= {}
    //we random the weights on the first run and then the link weight is determinant in  update_topology routine
    if (!second_update_leader )
    {
        setLinksWeight(topo,0); // Set random weights to edges in the graph
    }
    else
    {
        setLinksWeight(topo, 10);
    }
    cTopology* LASTtopo = BuildLASTtopo(topo,alpha,m_var);
//    EV<<endl<<endl<<"rootNodesArr[0] = "<< rootNodesArr[0] <<endl<<endl;
//    EV<<endl<<endl<<"rootNodesArr[1] = "<< rootNodesArr[1] <<endl<<endl;
//    EV<<endl<<endl<<"rootNodesArr[2] = "<< rootNodesArr[2] <<endl<<endl;
//    EV<<endl<<endl<<"rootNodesArr[3] = "<< rootNodesArr[3] <<endl<<endl;

// first topology
    if(!second_update_leader )
    {
        copyTopology(topo, LASTtopo, "yellow");
        PaintTopology(topo, "green");

    }
    else
    {
        copyTopology(topo, LASTtopo, "green");
        PaintTopology(topo, "green");
        second_update_leader = true;
    }
    LASTtopo->clear();
    currTopoID = rand()% 100000 + 1;
    thisNode = topo->getNodeFor(getParentModule());
    initRtable(topo, thisNode);
    insertLinksToList(topo);
    int enabledCtr = printEdgesList();
    emit(TopoLinkCounter,enabledCtr);
    edges.clear();
    setMaxLeaderAmount(rootCounter);
    for(int i = 0; i <getMaxLeaderAmount();i++)
    {
        if(rootNodesArr[i] != -1)
        {
            Packet *phase4_initiate = new Packet("Roots will broadcast paths!!");
            phase4_initiate -> setSrcAddr(myAddress);
            phase4_initiate -> setDestAddr(rootNodesArr[i]);
            phase4_initiate -> setPacketType(leader_to_roots);
            phase4_initiate -> setTopologyID(currTopoID);
            phase4_initiate -> setHopCount(0);
            phase4_initiate -> setByteLength(1);
            //set how many roots in data[0]
            phase4_initiate -> data[0] = getMaxLeaderAmount();
            //set the roots id's in data[1 to amountOfLeaders + 1] given searching less than num_of_hosts - 1 roots
            for( int j = 0; j < getMaxLeaderAmount() + 1 ; j++ )
                phase4_initiate->data[ j + 1 ] = rootNodesArr[j];
            for( int j = getMaxLeaderAmount() + 1; j < num_of_hosts; j++ )
                phase4_initiate->data[j] = -1;
            Routing::LoadPacket(phase4_initiate, topo);


            //find the gate to node neighboors
           RoutingTable::iterator iter = rtable.find(rootNodesArr[i]);
           rootGateIndex = (*iter).second;
           send(phase4_initiate, "out",rootGateIndex);
        }
    }
    schduleTopologyUpdate();

}
void Routing::schduleTopologyUpdate()
{
    Packet *topoUpdate = new Packet("topo update!!");
    topoUpdate->setSrcAddr(myAddress);
    topoUpdate->setDestAddr(myAddress);
    topoUpdate  -> setPacketType(update_topology);
    topoUpdate  -> setTopologyID(currTopoID);
    scheduleAt( simTime() + changeRate, topoUpdate );
}
void Routing::setNeighborsIndex() {
    for ( int i = 0; i < topo -> getNode(myAddress)->getNumInLinks(); i++ )
        neighbors[i]=findDest(i,myAddress);
}
int Routing::findDest(int direction,int index){
    int row = 0;
    int base = sqrt(num_of_hosts);
    int col = index%base;
    int temp = index;
    bool torus = true;

    //set col
    while( temp >= base )
    {
        row += 1;
        temp = temp - base;
    }

    switch ( direction ) {
    case left_d:
    {
        if ( base*row < index && index <= row*base + ( base - 1 ) )
            return index - 1;
        else
            return torus ? ( row*base + (base - 1) ): -1;
    }break;
    case up_d:
    {
        if ( row > 0 )
            return ((row-1)*base+col);

        else
            return torus ? ( num_of_hosts - base + col ): -1;
    }break;
    case right_d:
    {
        if( base*row <= index && index < row*base + ( base - 1 ) )
            return (index+1);

        else
            return torus ? ( base * row ): -1;

    }break;
    case down_d:
    {
        if ( row != base-1 )
            return ( ( row + 1 ) * base + col );

        else
            return torus ? col : -1;

    }break;
    case diagonal_up:
    {
        if( row > 0  && col > 0)
        {
            return (row-1)*base + (col -1);
        }
        else
        {
            int x;
            if( col > 0 )
                x = col -1;
            else
                x = base -1;
            int y;
            if( row == 0 )
                y = base -1;
            else
                y = row -1;
            return x + (y*base);
        }

    }break;
    case diagonal_down:
    {
        if( row < base - 1 && col < base -1)
        {
            return (row+1)*base + col + 1;
        }
        else
        {
            int x,y;
            if(row == base - 1)
                y = 0;
            else
                y = row + 1;
            if(col == base -1 )
                x = 0;
            else
                x = col + 1;

            return x + y*base;
        }

    }break;
    default :
    {
        return -1;
    }
    }//end switch direction

}
void Routing::link_weight_update()
{
//    if(currTopoID != 0)
//    {
//    //    setDualSideWeights(topo, this, loadCounter)
//    }
}
//update weight of neigboors links to send to leader. index i stands for sattellie i. the weight is the value
void Routing::update_weight_table(){

    if(second_update)
    {
        return;
    }
    else
    {
        int linkID=0;

        for(int i = 0; i < topo->getNode(myAddress)->getNumInLinks();i++)
        {
            if(neighbors[i] != -1)
            {
                int j=0;
                EV<<"nod["<<myAddress<<"] "
                        "link ["<<linkID<<"] "
                                "= "<<thisNode->getLinkIn(linkID)->getWeight()<<
                        ". old method ="<<this->topo->getNode(myAddress)->getLinkIn(linkID)->getWeight()<<endl<<endl;
               // thisNode->getLinkIn(linkID)->setWeight(8.5222 );
                EV<<"link updated to : "<<thisNode->getLinkIn(linkID)->getWeight()<<endl;
                leader_list[neighbors[i]] = thisNode->getLinkIn(linkID)->getWeight();
                linkID++;
            }
        }
    }
}

void Routing::send_link_info_to_leader(){
    int rootGateIndex;
    Packet *phase2_send_link_info_to_leader= new Packet("Send link info to Leader!");
    phase2_send_link_info_to_leader->setSrcAddr(myAddress);
    phase2_send_link_info_to_leader->setDestAddr(leader);
    phase2_send_link_info_to_leader->setPacketType(phase2_link_info);
    //phase2_send_link_info_to_leader->setTopologyID(currTopoID);
    phase2_send_link_info_to_leader -> setByteLength(1);
    int i=0;
    for(i=0; i<topo->getNode(myAddress)->getNumInLinks(); i++)
//    for(int i = 0; i<4; i++)
    {
        phase2_send_link_info_to_leader->data[i]=neighbors[i];
        phase2_send_link_info_to_leader->datadouble[ i ] = queueSize[getLinkIndex(i)] / frameCapacity * 100;//(datarate/12000) * 100;
    }
    for(i; i<num_of_hosts; i++)
    {
        phase2_send_link_info_to_leader -> datadouble[i] = -1;
        phase2_send_link_info_to_leader -> data[i] = -1;
    }
    //find the gate to reach leader
    RoutingTable::iterator iter = rtable.find(leader);
    rootGateIndex = (*iter).second;
  //  delayTime = (simTime(), intrand(num_of_hosts));
    sendDelayed(phase2_send_link_info_to_leader,0, "out",rootGateIndex);
}

void Routing::flushQueue(int type){
        // find queueModule

    for( int i = 0; i < topo->getNode(myAddress)->getNumInLinks() ; i++ ){
         //cGate* test = thisNode->getLinkIn(i)->getLocalGate();
           cQueue *qptr= (cQueue *)(this->getParentModule()->getSubmodule("queue",i));
          // queueModule = dynamic_cast<cQueue *>(qptr->getOwnerModule());
           if (!qptr)
               error("Missing queueModule");
//           ASSERT(qptr);
//              while (!qptr->isEmpty()){
//                  cObject *msg = qptr->pop();
//                  // emit(dropPkIfaceDownSignal, msg); -- 'pkDropped' signals are missing in this module!
//                  delete msg;
//              }
              qptr->clear(); // clear request count
//
//           while( !qptr->isEmpty()){
//               c = qptr->pop();
//               //take(ch);
//               delete c;
//           }

           //const char *str1= thisNode->getLinkIn(i)->getLocalGate()->getName();
        }
}
void Routing::setMaxLeaderAmount(int amountofleaders){  rootCounter = amountofleaders;   }
int Routing::getMaxLeaderAmount(){    return rootCounter;   }

/*
 * Args: Packet * pk (holds the path information from root)
 * Return: bool - true if learned on all routes false otherwise
 * Description: verify if all paths to roots are known
 */
bool isPathSet(Packet *pk){
   return false;
}

int Routing::getPathID()
{
    return pathID;

}
/*
 * Args: Packet * pk (holds the path information from root)
 * Return: void
 * Description: set the better option (existing known route or incoming route
 */
void Routing::setPathToRoot(Packet *pk)
{
    int i=0,j=0,alternativePathWeight=0;
    //first
    if( pathID == -1 )
    {
        pathID = pk->getSrcAddr();
        i = 1;
        while( pk->data[i] != -1 )
        {
            path[i-1] = pk->data[i];
            alternativePathWeight += pk->datadouble[i-1];
            i++;
        }
        pathWeight = alternativePathWeight;

    }
    //new path arrived. will compare with old path.
    else
    {
        int contenderRoot= pk->getSrcAddr();
      //  while(pk->data[i] != -1)i++;
        while( pk->datadouble[j] !=-1 )
        {
            //path[j] = pk->data[i];
            alternativePathWeight += pk->datadouble[j];
            //i--;
            j++;
        }
        if( alternativePathWeight < pathWeight )
        {
            i=1;j=0;
            pathID = contenderRoot;
            pathWeight = alternativePathWeight;
            while(pk->data[i] != -1)i++;
            while( i > 1 )
            {
                path[j] = pk->data[i-1];
                //alternativePathWeight += pk->datadouble[j];
                i--;j++;
            }
            //clear history(old paths)
            while(j<num_of_hosts)
            {
                path[j] = -1;
                j++;
            }
        }
    }
}
//input : neighboor index
//output: queue index ("queue",index)
int Routing::getLinkIndex(int req){
    for(int queueIndex = 0; queueIndex< topo->getNode(myAddress)->getNumInLinks();queueIndex++)
        if(topo->getNode(myAddress)->getLinkIn(queueIndex)->getRemoteNode() == topo->getNode(neighbors[req]))
            return queueIndex;
    return -1;//if reachs here the link requsted is not existed exit the program
//    if(neighbors[req] == -1)
//          return 0;//if reachs here the link requsted is not existed
}
//input : queue index ("queue",index)
//output: neighboor index
int Routing::getNeighboorIndex(int req){
    for( int NeihboorIndex = 0; NeihboorIndex < MAXAMOUNTOFLINKS ;NeihboorIndex++ )
        if( neighbors[NeihboorIndex] != -1 )
            if(topo->getNode(myAddress)->getLinkIn(req)->getRemoteNode() == topo->getNode(neighbors[NeihboorIndex]))
                return NeihboorIndex;
    return -1;//if reachs here the link requsted is not existed exit the program
//    if(neighbors[req] == -1)
//          return 0;//if reachs here the link requsted is not existed
}

int Routing::loadBalance(int reqLink,int incominglink)
{
    double temp_load_threshold = load_threshold;
    int bestmatch = 110;
    int pos = 0;
    while( pos != 6  )
    {
       if( active_links_array[pos] != -1 )
       {
           int currentgate_id_local;
           currentgate_id_local = topo->getNode(myAddress)->getLinkIn(pos)->getLocalGateId();
           if( pos != reqLink && loadCounter[pos] < temp_load_threshold )
           {
               if(incominglink != currentgate_id_local)
               {
                   bestmatch = pos;
                   temp_load_threshold = loadCounter[pos];
               }
           }
       }
       pos++;
    }

    if( bestmatch != 110 )
       return bestmatch;
    else
       return reqLink;
}

int Routing::loadBalanceLinkPrediction(int reqLink,int incominglink)
{
    double temp_load_threshold = load_threshold;
    int bestmatch = 110;
    int pos = 0;
    while( pos != 6  )
    {
       if( active_links_array[pos] != -1 && !loadPredictionFlag[pos] )
       {
           int currentgate_id_local;
           currentgate_id_local = topo->getNode(myAddress)->getLinkIn(pos)->getLocalGateId();
           if( pos != reqLink && loadCounter[pos] < temp_load_threshold )
           {
               if(incominglink != currentgate_id_local)
               {
                   bestmatch = pos;
                   temp_load_threshold = loadCounter[pos];
               }
           }
       }
       pos++;
    }

    if( bestmatch != 110 )
       return bestmatch;
    else
       return reqLink;
}
int Routing:: loadBalanceLinkFinder( int reqLink ,int incominglink,Packet *pk )
{
  //  int queuegate = -1;
    if (pk -> getPacketType() == leader_to_roots ||  pk -> getPacketType() == route_from_root ||pk -> getPacketType() == phase2_link_info)
    {
        return reqLink;
    }
    if(pk->datadouble[0] == -5)
    {
        int loop = 0,i=0;
        while( i < num_of_hosts )
        {
            if( pk->datadouble[i] == -1)
                break;
            if( pk->datadouble[i] == myAddress )
            {
                loop++;
            }
            if( loop > 1 )
                return reqLink;
            i++;
        }
        pk -> datadouble[i] = myAddress;
    }
    double temp_load_threshold;
    //load balance - can't reroute to the origin link. LINK A - 20% with lp TRUE; LINK B - 40% with lp FALSE
    if ( load_balance_link_prediction )
    {
        if( loadCounter[reqLink] > load_threshold )
            return loadBalance(reqLink,incominglink);
        else if( loadPredictionFlag[reqLink] )
            return loadBalanceLinkPrediction(reqLink,incominglink);//find min link with loadPredictionFlag FALSE
        else
            return reqLink;
    }
    //without link prediction
    else
    {
        if(loadCounter[reqLink] < load_threshold)
            return reqLink;

        return loadBalance(reqLink,incominglink);
    }
}
//help to stop at specific node for debugging.
void Routing::debughelp()
{
    int a=1;
    if(myAddress == 0)
        a++;
    if(myAddress == 1)
           a++;
    if(myAddress == 2)
               a++;
    if(myAddress == 3)
               a++;
    if(myAddress == 4)
               a++;
    if(myAddress == 5)
               a++;
    if(myAddress == 6)
               a++;
    if(myAddress == 7)
               a++;
    if(myAddress == 8)
               a++;
    if(myAddress == 9)
               a++;
    if(myAddress == 10)
               a++;
    if(myAddress == 11)
               a++;
    if(myAddress == 12)
               a++;
    if(myAddress == 13)
               a++;
    if(myAddress == 14)
               a++;
    if(myAddress == 15)
               a++;
    if(myAddress == 16)
               a++;
    if(myAddress == 17)
               a++;
    if(myAddress == 18)
               a++;
    if(myAddress == 19)
               a++;
    if(myAddress == 20)
               a++;
    if(myAddress == 21)
               a++;
    if(myAddress == 22)
               a++;
    if(myAddress == 23)
               a++;
    if(myAddress == 24)
               a++;
}

void Routing::receiveSignal(cComponent *src, simsignal_t id, cObject *value,cObject *details){
    EV<<endl<<"signal has recieved";
//    //EV<<endl<<endl<<"x="<<l<<endl<<endl;
//    Packet * command;
    controlSignals *signalMessage;
    int counter;
    signalMessage = (controlSignals *)value;
    if (hasGUI())
           getParentModule()->bubble("ypi kai yey motherfucker");
    //EV<<endl<<"signalmessage id:"<<signalMessage->ID<<"signalMessage idcounter: "<<signalMessage->idCounter<<endl<<endl;
    //lll: need to select port(which queue is handling )
//    if(dynamic_cast<controlSignals*>(value))
//    {
//        signalMessage = (controlSignals *)value;
//        Packet *top;
//        if(!queue.isEmpty()){
//            Packet *pk = (Packet*)queue.pop();
//            EV<<endl<<endl<<"top of the queue is: "<<top->getPacketType()<<", dest add: "<<pk->getDestAddr()<<endl;
//            EV<<"packet from queue cast type : "<<pk->getPacketType()<<endl<<endl;
//
//            int case_type = signalMessage->phaseID;//data- using enums for phase's and routines
//            while(!queue.isEmpty()){
//                counter = 0;
//                Packet *tmp = (Packet*)queue.pop();
//                //signal phase1 (remove old update);
//                if( tmp->getPacketType() == phase1_update)
//                {
//                    if(top->getDestAddr() > tmp->getDestAddr() && top->getSrcAddr() == tmp->getSrcAddr())
//                    {
//                        delete (cObject*)top;
//                        top = tmp;
//                        counter++;//delete
//                    }
//                    else
//                    {
//                        queue.insert((cObject*)tmp);
//                        break;
//                    }
//                }
//
//            }//end while(!queue.isEmpty())
//            queue.insert((cObject*)top);
//            EV<<endl<<endl<<"siganl phase1."<<counter<<"  messages deleted from the queue"<< " x = "<<case_type <<endl<<endl;
//
////            switch(case_type){
////                case phase1_update :
////                {
////                    while(!queue.isEmpty()){
////                        counter = 0;
////                        Packet *tmp = (Packet*)queue.pop();
////                        //signal phase1 (remove old update);
////                        if( tmp->getPacketType() == phase1_update)
////                        {
////                            if(top->getSrcAddr() > tmp->getCreationTime().double() && top->getSrcAddr() == tmp->getSrcAddr())
////                            {
////                                delete (cObject*)tmp;
////                                counter++;//delete
////                            }
////                            else if(top->getDestAddr() == tmp->getDestAddr())
////                            {
////                                delete (cObject*)top;
////                                top = tmp;
////                                counter++;//delete
////                            }
////                            else
////                            {
////                                queue.insert((cObject*)tmp);
////                                break;
////                            }
////                        }
////
////                    }//end while(!queue.isEmpty())
////                    queue.insert((cObject*)top);
////                    EV<<endl<<endl<<"siganl phase1."<<counter<<"  messages deleted from the queue"<< " x = "<<case_type <<endl<<endl;
////                }break;
////                case phase2_link_info :
////                {
////                 //flush queue
////                    counter = 0;
////                    while(!queue.isEmpty())
////                    {
////                        Packet *tmp = (Packet *)queue.pop();
////                       if(tmp->getPacketType() == phase1_update)
////                       {
////                           delete (cObject*)tmp;
////                           counter++;
////                       }
////                       else
////                       {
////                           continue;
////                       }
////                    }
////                   EV<<endl<<endl<<"Phase2 message received. "<< " x = "<<case_type<<endl<<endl;
////                }break;
////                case 555:
////                {
////                    EV<<endl<<"foward message signal"<<" x = "<<case_type<<endl;
////                }break;
////                default:
////                {
////                    counter++;
////                }break;
////            }//end switch case
//            queue.insert(top);
//        }//end prime if top for the queue exist
//    }
//}
//    delete pk;
//    if(!this->queue.isEmpty()){
//     //   cObject *top = queue.pop();
//        counter = 0;
//        while(!this->queue.isEmpty()){
//          //  cObject *tmp = queue.pop();
//            //delete tmp;
//            counter++;
//        }
//      //  queue.insert(top);
//        EV<<endl<<endl<<"signal received at queue. "<<counter<<"  messages deleted from the queue"<<endl<<endl;
//
//}
}
/*
 * queueid = 1 char (0 to 3)
 * load = 00 - 99
 * connectedto = until end of char (\0)
 * queue size
 */
void Routing::signalStringParser(const char *s,int* queueid,uint32_t * load,int* connectedto,int *queuesize)
{
    int index = 0;
    //retrieve which queue is signaling the router (us), -48 for ascii conversion
    *queueid = s[index]-48;
    index++;//space;
    index++;//start of next argument

    //load argument: with up to 20 digits (10^20)
    *queuesize = s[index]-48;
    index++;
    while( s[index] != ' ' && s[index] != '\0')
    {
        *queuesize = *queuesize * 10 + (s[index]-48);
        index++;
    }
//    index++;//space;
//    index++;//start of next argument
//
//    *connectedto = s[index]-48;
//    index++;
//    while(s[index] != '\0')
//    {
//        *connectedto = *connectedto * 10 + s[index]-48;
//        index++;
//    }
//    index++;//space;
//    *queuesize = s[index] -48;
//    index++;//start of next argument
//    while(s[index] != '\0')
//    {
//        *queuesize = *queuesize * 10 + s[index]-48;
//        index++;
//    }
}

void Routing::receiveSignal(cComponent *source, simsignal_t signalID, const char *s, cObject *details)
{
    int queueid,connectedto,queuesize;
    uint32_t load;
    signalStringParser(s,&queueid,&load,&connectedto,&queuesize);


    loadCounter[queueid] = (double)queuesize / (double)frameCapacity * (double)100;//(datarate/12000) * 100;
    queueSize[queueid] = queuesize;
    if(loadCounter[queueid]>=50)
            int a= 1;
//    if (hasGUI())
//       getParentModule()->bubble(s);

}

void Routing::sendMessage(Packet * pk, int outGateIndex)
{
    pk->setHopCount(pk->getHopCount()+1);

    if( load_balance_mode && active_links > 2 )
    {
      int hotpotato;
      hotpotato = loadBalanceLinkFinder(outGateIndex,pk->getArrivalGateId(),pk);
      if( hotpotato != outGateIndex )
      {
          if(hasGUI())
            getParentModule()->bubble("LOAD BALANCING");
          pk -> setPacketType(hot_potato);
      }

        delayTime = 0;
        sendDelayed(pk,delayTime, "out",hotpotato);

    }
    else if( pk -> getHopCount() > 2*sqrt(NUMOFNODES) )
    {
        int i=0;
        //find the gate. send to the gate to activate the func
        while(i<topo->getNode(myAddress)->getNumInLinks())
        {
            if( thisNode->getLinkIn(i)->getLocalGateId() == pk->getArrivalGateId())
                break;
            i++;
        }
      check_and_cast<L2Queue*>(getParentModule()->getSubmodule("queue", i)) -> deleteMessageAndRecord(pk,false);
      delete pk;//TODO: drop count... delete proper
    }
    else
    {
      delayTime = 0;
      sendDelayed(pk,delayTime, "out",outGateIndex);
    }
}
//record last 10 topology's queue(or link) usage.uses overlap every 10 cycles
void Routing::updateLoadHistory()
{
    int load;
    int queuesize;
    L2Queue * temp;
    for( int i = 0; i < MAXAMOUNTOFLINKS; i++ )
    {
        if( loadTimerIndex%HISTORY_LENGTH != 0 )
        {
          //  Packet *pk = check_and_cast<Packet *>(msg);
            temp = (L2Queue*)getParentModule()->getSubmodule("queue", i);
            if(temp !=NULL)
            {
                queuesize = temp -> getQueueSize();
                if( queuesize  == 0 )
                {
                    load = 0;
                }
                else
                {
                    load = queuesize;
                }
                loadHistory[i][loadTimerIndex] = load;
            }
        }
        else
        {
            if(!loadPredictionIsOn)
            {
                LinkLoadPrediction();
                loadPredictionIsOn = true;
            }
            loadTimerIndex = 1;
        }
    }
    loadTimerIndex++;
}
void Routing::LinkLoadPrediction()
{
    int pattern[PATTERN_LENGTH];
    double loadPredicted;
    int history_temp[HISTORY_LENGTH];
    //Link prediction
    for ( int i = 0; i < MAXAMOUNTOFLINKS; i++ )
    {
        //its an inactive link at the moment
        if(active_links_array[i] == -1 || loadCounter[i] < 10 )
        {
            loadPredictionFlag[i] = false;
            continue;
        }
        for( int j = 0; j < HISTORY_LENGTH; j++ )
        {
            history_temp[j] = loadHistory[i][j];
        }
        int pivot=loadTimerIndex;
        if( pivot < PATTERN_LENGTH )
            pivot = HISTORY_LENGTH;
        for( int j = 0; j < PATTERN_LENGTH; j++ )
        {
           pattern[j] = history_temp[pivot - (PATTERN_LENGTH - j)];
        }
        loadPredicted = search(history_temp,pattern, ALPHA_P,BETA_P,PATTERN_LENGTH, loadTimerIndex);
        updateLoadBalancePrediction(i,loadPredicted);
        //cout<<"Node: "<<myAddress<<", link number: "<<i<<"load Predicted: "<<loadPredicted<<endl;
       // loadPrediction[i].record(loadPredicted);
    }
    Packet *loadpmRoutine = new Packet("load prediction routine");
    loadpmRoutine -> setPacketType(load_prediction_update);
    loadpmRoutine -> setSrcAddr(myAddress);
    loadpmRoutine -> setDestAddr(myAddress);
    scheduleAt( simTime() + LOAD_PREDICTION_INTERVAL, loadpmRoutine );
}
void Routing::updateLoadBalancePrediction(int index,int loadPredicted)
{
    double lp=(double)loadPredicted/frameCapacity*100;

    if(loadPredicted > 3360)
        debughelp();

    if(lp > 0.80*load_threshold)
        loadPredictionFlag[index]=true;
    else
        loadPredictionFlag[index]=false;
}
int Routing::totalErrors(int *pattern,int *history,int offset,int size_P){
    int total = 0;
    for(int i = 0; i < size_P; i++){
        total+=abs(pattern[i]-history[offset+i]);
    }
    return total;
}
void Routing::pi_func(int pattern[], int alpha,int size,int pi[]){
    int j,append=1;

    for (int i = 1; i < size; i++) {
        j = pi[i - 1];
        while (j > 0 && (abs(pattern[j] - pattern[i]) > alpha)) {
            j = pi[j - 1];
        } //while
        if (abs(pattern[j] - pattern[i]) <= alpha) {
            pi[append] = j + 1;
        } else {
            pi[append] = j;
        }
        append++;
    } //for
    int a=1;
    a++;
}
bool Routing::isRoot()
{
    return (rootID == -1) ? false:true;
}
double Routing::search(int *T,int *P,int alpha,int beta, int pattern_size, int History_size)
{
    int i=0, j=0;
    int temp_j;
    int offset;
    int pi[pattern_size];
    for(int i = 0; i < pattern_size; i++)
        pi[i] = 0;
    int malloc_flag =1;
    int overlapIndexesSize = 1;
    int *overlapIndexes;
    int error;
    double load_predict=0;


    pi_func(P, alpha,pattern_size,pi);

    overlapIndexes = (int*) malloc(sizeof(int) * (overlapIndexesSize));
    overlapIndexes[0] = -1;

    for (i = 1; i < History_size; i++) {
        if(i == History_size - pattern_size){
            break;
        }

        while ((j > 0) && (abs(T[i] - P[j]) > alpha) && j<pattern_size) {
            j = pi[j - 1];
            //          cout <<"j inside while: " << j << endl;
            //          break;
        }    //while
        if ((abs(T[i] - P[j])) <= alpha) {
            j++;
        }    //if

        if (j == pattern_size) {
            temp_j = j;
            j = pi[j - 1];
            offset = i - pattern_size + 1;
            error = totalErrors(P, T, offset, pattern_size);
        //    cout << "num of errors: " << error << endl;
            if (error <= beta) {
                if (malloc_flag == 1) {
                    malloc_flag = 0;
                    overlapIndexes[overlapIndexesSize - 1] = i - (temp_j - 1);
                } else {
                    overlapIndexesSize++;
                    overlapIndexes = (int*) realloc(overlapIndexes,
                            sizeof(int) * (overlapIndexesSize));
                    overlapIndexes[overlapIndexesSize - 1] = i - (temp_j - 1);
                }
//                cout << "index of good match: "
//                        << overlapIndexes[overlapIndexesSize - 1] << endl;
                j = 0;
            }
        }    //if
    }    //for

    //no pattern has found
    if(malloc_flag == 1){
        free(overlapIndexes);
        return -1;
    //one pattern has been found
    }else if(overlapIndexesSize==1){
            cout << T[overlapIndexes[0]+pattern_size] << endl;
            int history_index = overlapIndexes[0]+pattern_size;
            free(overlapIndexes);
            return T[history_index];
    //multiple patterns has been found. we will take the mean.
    }else{
        for(i=0; i<overlapIndexesSize;i++){
            load_predict += T[overlapIndexes[i]+pattern_size];
        }
        cout << load_predict/overlapIndexesSize << endl;
        free(overlapIndexes);
        return load_predict/overlapIndexesSize;
    }


}
void Routing::schduleBaseLine()
{
    //start app
    Packet *startSend = new Packet("initiate App");
    startSend -> setPacketType(app_initial);
    startSend -> setTopologyID(currTopoID);
    //delayTime = 0;
    send( startSend , "localOut" );
}
void Routing::saveTempLinks()
{
    for(int i = 0; i<MAXAMOUNTOFLINKS;i++)
    {
        tempActiveLinks[i] = -1;
        if(topo->getNode(myAddress)->getLinkIn(i)->isEnabled())
                 tempActiveLinks[i] = i;
    }

}
void Routing::keepAliveTempLinks()
{
    for(int i = 0; i<MAXAMOUNTOFLINKS;i++)
        if( tempActiveLinks[i] != -1 )
            topo->getNode(myAddress)->getLinkIn(tempActiveLinks[i])->enable();
    for(int i = 0; i<MAXAMOUNTOFLINKS;i++)
        if( tempActiveLinks[i] != -1 )
            topo->getNode(myAddress)->getLinkOut(tempActiveLinks[i])->enable();

    Packet *templink = new Packet("turn off link");
    templink -> setPacketType(turn_off_link);
    templink -> setSrcAddr(myAddress);
    templink -> setDestAddr(myAddress);
    scheduleAt( simTime() + 5, templink );

    int a=1;
    a=1111;

}
int* Routing::getNeighborsArr(int index,int* arr)
{
    for(int i = 0; i< MAXAMOUNTOFLINKS ; i++)
        arr[i]=check_and_cast<Routing *>(topo->getNode(index)->getModule()->getSubmodule("routing"))->neighbors[i];
    return arr;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

                                                                                // lllinitialize

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Routing::initialize() {

debughelp();

    //load balance workzone

//        queueschannel =
//        links_control[c] = check_and_cast<L2Queue *>(this->getParentModule()->getSubmodule("queue",c));
    LBS = registerSignal("load_balance_channel");
    links_control[0] = getParentModule()->getSubmodule("queue", 0);
    if( links_control[0] != NULL)
        links_control[0] -> subscribe(LBS, this);
    links_control[1] = getParentModule()->getSubmodule("queue", 1);
    if( links_control[1] != NULL)
        links_control[1] -> subscribe(LBS, this);
    links_control[2] = getParentModule()->getSubmodule("queue", 2);
    if( links_control[2] != NULL)
        links_control[2] -> subscribe(LBS, this);
    links_control[3] = getParentModule()->getSubmodule("queue", 3);
    if( links_control[3] != NULL)
        links_control[3] -> subscribe(LBS, this);
    loadPrediction[0].setName("link0");
    loadPrediction[1].setName("link1");
    loadPrediction[2].setName("link2");
    loadPrediction[3].setName("link3");
  // links_control -> subscribe(LBS, this);
    //load balance workzone
    myAddress = getParentModule()->par("address");
    hasInitTopo = 0;
    packetType = 1;
    loadTimerIndex = 1;
    for(int i = 0; i < 4; i++)
        for(int k = 0; k < 900; k++ )
            loadHistory[i][k] = 0;
    frameCapacity = getParentModule()->par("frameCapacity");
    packetLength = getParentModule()->getSubmodule("app")->par("packetLength");
    dropSignal = registerSignal("drop");
    outputIfSignal = registerSignal("outputIf");
    TopoLinkCounter = registerSignal("TopoLinkCounter");
    hopCountSignal = registerSignal("hopCount");
    m_var = getParentModule()->par("m_var");
    alpha = getParentModule()->par("alpha");
    //alpha =1;
    datarate =  check_and_cast<cDatarateChannel*>(getParentModule()->getSubmodule("queue", 0)->gate("line$o")->getTransmissionChannel()) ->getDatarate();
    load_balance_mode = getParentModule()->par("load_balance_mode");
    load_balance_link_prediction = getParentModule()->par("load_balance_link_prediction");
    load_threshold = getParentModule() -> par("load_threshold");
    LBS= cComponent::registerSignal ("load_balance_channel");

    fullTopoRun = false;                                    //getParentModule()->par("fullTopoRun");

    //m_var = 5;
    changeRate = getParentModule()->par("changeRate");
    topo = new cTopology("topo");
    std::vector<std::string> nedTypes;
    num_of_hosts = getParentModule()->par("num_of_satellite");
    nedTypes.push_back(getParentModule()->getNedTypeName());
    topo->extractByNedTypeName(nedTypes);
    NodesInTopo = topo->getNumNodes();
    EV << "cTopology found " << NodesInTopo << " nodes\n";
    thisNode = topo->getNodeFor(getParentModule());
    linkCounter = new int[thisNode->getNumInLinks()];
    //delayTime = exponential(1);
    _sendTime = &par("sendTime");
    for (int j = 0; j < thisNode->getNumInLinks(); j++) {
        linkCounter[j] = 0;
        loadPredictionFlag[j] = false;
    }
    loadPredictionIsOn = false;
    phase1Flag = 1;
    for (int j = 0; j < num_of_hosts; j++)
    {
            path[j] = -1;
    }
    pathID=-1;
    pathWeight=-1;
    pathcounter = 0;
    //set initial topo weight's (1on each link)
    setLinksWeight(topo, 0);
    //init Routing table from topology
    initRtable(topo, thisNode);
    //set neighbors for sending updates
    initial_leader_table() ;

    debughelp();

    //set who is my neighbor
    setNeighborsIndex();
    update_weight_table();



    leader = 0;
    // start simulation without choosing leader
    if(myAddress == 0 && !BASELINE_MODE )
    {
        schduleNewTopology();
    }

    // start simulation without choosing leader - baseline
    else
    {
        setLinksWeight(topo,0);
        baseSPT = BuildSPT(topo,myAddress);
        baseSPT -> calculateWeightedSingleShortestPathsTo(thisNode);
        initRtable(baseSPT,thisNode);
        schduleBaseLine();
    }

//    // start simulation with choosing leader
//
//        scheduleChoosingLeaderPhaseOne();

} // end of void Routing::initialize()

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

                                                                                // lllhandlemessage

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Routing::handleMessage(cMessage *msg) {
    Packet *pk = check_and_cast<Packet *>(msg);
  //  int arrivingGate = pk->getArrivalGateId();
    int destAddr = pk->getDestAddr();
    int srcAddr = pk->getSrcAddr();
    debugCounter++;
    debughelp();


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    // lll   destAddr != myAddress


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    if(destAddr == myAddress)
    {
        /*lll:  switch(pk->getPacketType() )
         * case 1 - initiale_packet  - ignite the app module in the node
         * case 2 - regular_packet -
         */

        switch( pk->getPacketType() )
        {
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            // lll phase1_update  destAddr == myAddress
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            case phase1_update:
            {
               // cObject *test = check_and_cast<cObject*>(pk);

                if (srcAddr == myAddress){
                // initial message (initialize or re election)
                    delete pk;
                    leader_table[myAddress] = myAddress; // first update its only me
                    if (hasGUI())
                        getParentModule()->bubble("scheduleChoosingLeaderPhase1!");
                    send_phase1_update();// update message
                }
                else
                {
                    //wait for get everyone's id's
                  //  debugCounter = debugCounter;
                    if(phase1Flag){
                        update_leader_table(pk);
                        delete pk;
                        send_phase1_update();
                    }
                    else
                        delete pk;
                }
            }break;//end case phase1_update
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            // lll phase2_link_info  destAddr == myAddress
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            case phase2_link_info:
            {
              //  EV<<endl<<"leader = "<<this->leader<<endl;
     //           emit(LBS,phase2_link_info,pk->dup());
                if(myAddress != leader){
                    leader = pk->getSrcAddr(); //only leader can send this type of message
                    delete pk;
                    //flushQueue(phase1_update);
                    //we have leader so we can raise the flag(if we didn't allready)
                    phase1Flag = 0;
                    //we will use this table to send link info to leader (Prim tree)
//                    initial_leader_list();
//                    link_weight_update();
//                    update_weight_table();
                   // send info to leader
                    send_link_info_to_leader();
                }//not leader
                //else : the LEADER saves the data
                else {
                    for( int i = 0; i < 4; i++ )
                        n_table[pk->getSrcAddr()][pk->data[i]] = pk->datadouble[i];

                    linkdatacounter++;
                    if(linkdatacounter == num_of_hosts-1 )
                    {
                        linkdatacounter = 0;
                        schduleNewTopology();
                      //  schduleTopologyUpdate();
                    }
                    delete pk;
                }
            }break;
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            // lll   leader_to_roots destAddr == myAddress. IAM ROOT . send route_from root
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            case leader_to_roots :
            {
                if( currTopoID != 0 && currTopoID != pk -> getTopologyID() )
                {
                    second_update = true;
                }

                if( second_update && currTopoID != 0 && !saveTempLinkFlag )
                {
                    saveTempLinkFlag = true;
                    saveTempLinks();
                    debugCounter++;
                }

                copyTopology( topo, pk->getTopologyVarForUpdate(), "green" );
                rootID = myAddress;
                currTopoID = pk -> getTopologyID();
                rootID_current_topo_ID = currTopoID;
                topocounter++;
                thisNode = topo -> getNodeFor( getParentModule() );
                initRtable( topo, thisNode );
                if( second_update && currTopoID != 0 && saveTempLinkFlag && !keepAliveTempLinksFlag)
                {
                    keepAliveTempLinksFlag = true;
                    keepAliveTempLinks();
                    debugCounter++;

                }
                setMaxLeaderAmount( pk -> data[0] );
                for( int i = 1; i < getMaxLeaderAmount() + 1; i++ )
                {
                    rootNodesArr[i-1]=pk->data[i];
//                    rootID = (rootNodesArr[i-1] == srcAddr) ? rootNodesArr[i-1] : -1;
                }
                for(int i=0;i<num_of_hosts;i++)
                {
                    if( i == myAddress )
                        continue;

                    RoutingTable::iterator iter = rtable.find(i);
                    int rootGateIndex = (*iter).second;

                    Packet *source_route = new Packet("path packet");
                    source_route -> setPacketType(route_from_root);
                    source_route -> setTopologyID(currTopoID);
                    source_route -> setSrcAddr(myAddress);
                    source_route -> setDestAddr(i);
                    source_route -> setByteLength(1);
                    source_route -> datadouble[0] = thisNode->getLinkOut(rootGateIndex)->getWeight();
                    source_route -> datadouble[1] = -1;
                    source_route -> data[0] = getMaxLeaderAmount();
                    source_route -> data[1] = myAddress;
                    pk -> setByteLength(1);
                    Routing::LoadPacket(source_route, topo);
                    for( int j = 2; j < num_of_hosts; j++ )
                    {
                        source_route->data[j] = -1;
                        source_route->datadouble[j] = -1;
                    }

                //    w=thisNode->getLinkOut(rootGateIndex)->getWeight();
                    send(source_route, "out",rootGateIndex);
                }
                //for all nodes create and send route_from_root;
                //rx : route_from_root save in table; activate app; send by table;
                delete pk;
                if (hasGUI())
                    getParentModule()->bubble("LAST tree created!");

            }break;
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            // lll   route_from_root destAddr == myAddress. node decide to which root to communicate.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            case route_from_root :
            {
                if( currTopoID != 0 && currTopoID != pk -> getTopologyID() )
                 {
                     second_update = true;
                 }
                if(  currTopoID != pk -> getTopologyID() )
                 {
                     topocounter++;
                 }
                //New Topology. reset root info
                if( currTopoID != 0 && currTopoID != pk -> getTopologyID() )
                {
                    //if i was a root in the old topology and now iam not.
                    if(second_update)
                        rootID = pk -> getTopologyID() == rootID_current_topo_ID ? rootID : -1;
                    for(int i= 0;i<num_of_hosts;i++)
                    {
                        rootNodesArr[i] = -1;
                        path[i] = -1;
                    }
                    pathcounter = 0;
                    pathID = -1;
                    pathWeight = -1;

                    if( second_update && currTopoID != 0 && !saveTempLinkFlag )
                   {
                        saveTempLinkFlag = true;
                        saveTempLinks();
                        debugCounter++;
                   }
                }
                currTopoID = pk -> getTopologyID();
                pathcounter ++;
                setMaxLeaderAmount( pk -> data[0] );
                //iam not root. so i need to learn about the path u
                double weightOfPath = 0;
                int j=0;
                while(pk->datadouble[j]!=-1)
                {
                   weightOfPath += pk->datadouble[j];
                   j++;
                }
                j=0;
                //learn about routes
                while(rootNodesArr[j] != -1 && rootNodesArr[j] != pk->getSrcAddr())j++;
                rootNodesArr[j]=pk->getSrcAddr();
                setPathToRoot(pk);
                copyTopology(topo, pk->getTopologyVarForUpdate(), "green");
                setActiveLinks();
                if( second_update && currTopoID != 0 && !keepAliveTempLinksFlag  && saveTempLinkFlag  )
                {
                    keepAliveTempLinksFlag = true;
                    keepAliveTempLinks();
                    debugCounter++;

                }
                thisNode = topo->getNodeFor(getParentModule());
                initRtable(topo, thisNode);
                //    int a = pk->data[0];
                delete pk;
               if (pathcounter == getMaxLeaderAmount() && !app_is_on)
               {
                   app_is_on = true;
                   //start app
                   Packet *startSend = new Packet("initiate App");
                   startSend -> setPacketType(app_initial);
                   startSend -> setTopologyID(currTopoID);
                   //delayTime = 0;
                   send( startSend , "localOut" );

                   //start load prediction recording
                   if(load_balance_link_prediction)
                   {
                        Packet *loadpm = new Packet("load prediction initial packet");
                        loadpm -> setPacketType(load_prediction);
                        loadpm -> setSrcAddr(myAddress);
                        loadpm -> setDestAddr(myAddress);
                        //we will start record in the 3rd cycle. when the system is stable
                        scheduleAt( simTime() + 3*changeRate, loadpm );
                   }

               }
               //we learend about all the roots
               //how to get the total value of the path
            }break;
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        // lll   message == myAddress.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            case message :
            {
                //recive message. send to app
                send( (cMessage*)pk , "localOut" );
            }break;
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        // lll   load_prediction message == myAddress.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            case load_prediction :
            {
                delete pk;
                updateLoadHistory();
                Packet *loadpm = new Packet("load prediction initial packet");
                loadpm -> setPacketType(load_prediction);
                loadpm -> setSrcAddr(myAddress);
                loadpm -> setDestAddr(myAddress);
                loadpm -> setHopCount(0);
                scheduleAt( simTime() + 0.2, loadpm );
            }break;
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        // lll   load_prediction_update message == myAddress.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            case load_prediction_update :
            {
                delete pk;
                LinkLoadPrediction();
            }break;
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        // lll  update_topology self message for leader
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            case update_topology:
            {
                fiftLink = (topocounter % 2 == 1) ? true : false;
                delete pk;
                double wgttemp;
                //double n_table[num_of_hosts][4];
                // reset_ntable();
                for(int i=0;i<num_of_hosts;i++)
                   for(int j=0;j<num_of_hosts;j++)
                       n_table[i][j]=-0.9;
                for(int i=0;i<num_of_hosts;i++)
                {
                    cTopology::Node * currNode = topo->getNode(i);
                    //negiboors of node i
                    int tempNeighbor[6];
                    int *arr = getNeighborsArr(i,tempNeighbor);
                    int neighboor;
                    //learn and update wgt's
                    for(int j=0;j<topo->getNode(i)->getNumInLinks();j++)
                    {
                        double wgt,queuesize;
                        neighboor = tempNeighbor[getNeighboorIndex(j)];
                        if(fiftLink)
                        {
                            queuesize = check_and_cast<L2Queue*>(topo->getNode(i)->getModule()->getSubmodule("queue", j)) -> getQueueSize();
                            wgt = queuesize / frameCapacity * 100;//(datarate/12000) * 100;
                            n_table[i][neighboor] = wgt;
                            wgt = (n_table[i][neighboor] > 100) ? 100 :  n_table[i][neighboor];
                            n_table[i][neighboor] = wgt;
                        }
                        else
                        {
                            //the first 4(0-3)links stay connected at all times
                            if(j<3)
                            {
                                queuesize = check_and_cast<L2Queue*>(topo->getNode(i)->getModule()->getSubmodule("queue", j)) -> getQueueSize();
                                wgt = queuesize / frameCapacity * 100;//(datarate/12000) * 100;
                                n_table[i][neighboor] = wgt;
                                wgt = n_table[i][tempNeighbor[getNeighboorIndex(j)]] > 100 ? 100 :  n_table[i][tempNeighbor[getNeighboorIndex(j)]] ;
                                n_table[i][neighboor] = wgt;
                            }
                            //the 5th and the 6th connection is turend off by setting value of inf
                            else
                            {
                                n_table[i][neighboor] = INT_MAX;
                            }
                        }
                        //will add 1 to avoid wgt of 0
                        wgttemp =  n_table[i][neighboor] + 1;

//                        if(n_table[neighboor][i] != -0.9)
//                            wgttemp =  n_table[i][neighboor] > n_table[neighboor]][i] ? n_table2[i][neighboor] : n_table[neighboor]][i];

                        currNode->getLinkIn(j)->setWeight(wgttemp);
                        currNode->getLinkOut(j)->setWeight(wgttemp);
                        //temp[i][j] = if exists max { queuesize from i to j ,queuesize from j to i }else -1;
                    }

                }
                int a= 1;
                schduleNewTopology();
            }break;
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        // lll  turn_off_link self message to disable temp links (after topo update
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            case turn_off_link:
            {
                for(int i = 0; i<MAXAMOUNTOFLINKS;i++)
                    if( tempActiveLinks[i] != -1 && active_links_array[i] == -1 )
                        topo->getNode(myAddress)->getLinkIn(tempActiveLinks[i])->disable();
                for(int i = 0; i<MAXAMOUNTOFLINKS;i++)
                    if( tempActiveLinks[i] != -1 && active_links_array[i] == -1 )
                        topo->getNode(myAddress)->getLinkOut(tempActiveLinks[i])->disable();
                saveTempLinkFlag = false;
                keepAliveTempLinksFlag = false;
            }break;
        }

       }
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    // lll   destAddr != myAddress

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    else{
        switch(pk->getPacketType()){
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            // lll  message destAddr != myAddress
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        case message :
        {
            if(BASELINE_MODE)
            {
//                cTopology::Node *targetnode = baseSPT->getNode(destAddr);
//                baseSPT->calculateWeightedSingleShortestPathsTo(targetnode);
//                cTopology::LinkOut *path = targetnode->  cTopology::Node::getPath(0);
//                cGate *parentModuleGate = thisNode->getPath(0)->getLocalGate();
//                int indexx = -> cTopology::LinkOut ;
                RoutingTable::iterator it = rtable.find(pk->getDestAddr());
                int outGateIndex = (*it).second;
                send(pk,"out",outGateIndex);
                break;
            }
            //load_balance
            if( pk -> getHopCount() > 20 )
            {
                delete pk;
                EV<<endl<<endl<<"message we DROP!@#!@"<<endl<<endl;
            }
            else if( pk -> datadouble[0] == hot_potato_lable)
            {
                //pk -> setHopCount(pk->getHopCount()+1);
                RoutingTable::iterator it = rtable.find(pk->getDestAddr());
                int outGateIndex = (*it).second;
                sendMessage(pk,outGateIndex);
            }
//            else
//            {
//                RoutingTable::iterator it = rtable.find(pk->getDestAddr());
//                int outGateIndex = (*it).second;
//                sendMessage(pk,outGateIndex);
//            }
         //    message i created
            else if( pk->getSrcAddr() == myAddress && pk->datadouble[0] == -2 )
            {
                int i = 0;
                while( path[i] != -1 )
                {
                    pk -> data[i] = path[i];
                    i++;
                }
                if( rootID != -1 )
                   pk -> datadouble[0] = -3;
                i = 0;
                while(pk->data[i] != -1)i++;
                i--;
                RoutingTable::iterator it = rtable.find(pk->data[i]);
                int outGateIndex = (*it).second;
                sendMessage(pk,outGateIndex);
            }
            //message on the way to root
            //message on the way to dst
            else
            {
                //shortest path from root to dest
                int rootID_temp;
                rootID_temp = pk->data[0];
                if(myAddress == rootID_temp)
                    pk->datadouble[0] = -3;

                //message on the way to dst
                if( rootID_temp == myAddress  || pk->datadouble[0] == -3 )
                {
                    if( hasGUI() && rootID == myAddress )
                        this->getParentModule()->bubble("root recived message");
                    RoutingTable::iterator it = rtable.find( pk->getDestAddr() );
                    int outGateIndex = (*it).second;
                   sendMessage(pk,outGateIndex);
                }
                //message on the way to root
                else
                {
                    int i = 0;
                    while(pk->data[i] != myAddress && i < num_of_hosts)
                        i++;
                    i--;
                    RoutingTable::iterator it = rtable.find(pk->data[i]);
                    int outGateIndex = (*it).second;
                    sendMessage(pk,outGateIndex);
        }

        }break;
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            // lll  hot_potato destAddr != myAddress hot potato case (load balance).
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        case hot_potato :
        {
          //  Packet * np = new Packet("loadbalancealternativeroute");
            pk -> setSrcAddr(myAddress);
            pk -> setDestAddr(pk->getDestAddr());
            //pk -> setHopCount(pk->getHopCount()+1);
            pk -> setPacketType(message);
            pk -> setBitLength((int64_t) 12000);
            int index=0;
            while(index<num_of_hosts)
            {
                pk -> data[index]=-1;
                pk ->datadouble[index] = -1;
                index++;
            }
            int i = 0;
            while( path[i] != -1 )
            {
                pk -> data[i] = path[i];
                i++;
            }
            i = 0;
            while(pk->data[i] != -1)i++;
            i--;
            int outGateIndex;;
            pk -> datadouble[0] = hot_potato_lable;
            RoutingTable::iterator it = rtable.find(pk->getDestAddr());
            outGateIndex = (*it).second;
           // delete pk;
            sendMessage(pk,outGateIndex);
        }break;
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            // lll  route_from_root destAddr != myAddress need to update route for destination.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        case route_from_root:
        {
//            pk->getTopologyVar()->clear();
           // currTopoID = pk->getTopologyID();
           // bool coolingLink = keepAliveTempLinksFlag;
            thisNode = topo->getNodeFor(getParentModule());
            initRtable(topo, thisNode);
            RoutingTable::iterator it = rtable.find(destAddr);
            int outGateIndex = (*it).second;
            int j=1;
            while(pk->data[j]!=-1)
            {
                if(pk->data[j] == myAddress)
                {
                    if(topocounter)
                    {
                        int aaaaa=1;
                        aaaaa++;
                    }
                    break;
                }
                j++;
            }
            if(pk->data[j] == myAddress)
            {
                sendDelayed(pk,delayTime , "out",outGateIndex);
            }
            else
            {
                pk->data[j]=myAddress;
                j=1;
                while(pk->datadouble[j]!=-1)j++;
                pk->datadouble[j]=thisNode->getLinkOut(outGateIndex)->getWeight();
               // pk->setHopCount(pk->getHopCount()+1);
                delayTime = 0;//exponential(0.01);
                sendDelayed(pk,delayTime , "out",outGateIndex);
            }
        }break;
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            // lll  leader_to_roots destAddr != myAddress
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        case  leader_to_roots :
        {
            setMaxLeaderAmount( pk->data[0] );
            copyTopology(this->topo, pk->getTopologyVarForUpdate(), "green");
            setActiveLinks();
           // pk->getTopologyVar()->clear();
            currTopoID = pk->getTopologyID();
            thisNode = topo->getNodeFor(getParentModule());
            initRtable(topo, thisNode);
           // pk -> setHopCount(pk->getHopCount()+1);
            RoutingTable::iterator it = rtable.find(destAddr);
            int outGateIndex = (*it).second;
            delayTime = 0;//exponential(0.01);
            send(pk,"out",outGateIndex);
        }break;
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            // lll  default: destAddr != myAddress
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        default :
        {

            RoutingTable::iterator it = rtable.find(destAddr);
            if (it == rtable.end()){
                if (DEBUG){
                    EV << "address " << destAddr << " unreachable, discarding packet " << pk->getName() << endl;
                }
             //   emit(dropSignal, (long)pk->getByteLength());
                delete pk;//TODO: drop count... delete proper
                return;
            }
            else if (currTopoID == pk->getTopologyID()){
                for( int i=0;i<thisNode->getNumInLinks();i++){
                    if( thisNode->getLinkIn(i)->getLocalGateId() ==pk->getArrivalGateId())
                    {
                        linkCounter[i] += 1;
                    }
                }
                int outGateIndex = (*it).second;
//                if (DEBUG) {
//                    EV << "forwarding packet " << pk->getName() << " on gate index " << outGateIndex << endl;
//                }
              //  pk->setHopCount(pk->getHopCount()+1);
                // Add edge weight
                emit(outputIfSignal, outGateIndex);
               // send(pk, "out", outGateIndex);
                delayTime = 0;//exponential(0.01);
                sendDelayed(pk,delayTime,"out",outGateIndex);
                return;
            }
             else
             {
//                 if (DEBUG) {
//                     EV << "Packet topology ID =  " << pk->getTopologyID() << "do not match Node topology ID ( " <<
//                    currTopoID <<") , discarding packet " << pk->getName() << endl;
//                 }
                 //TODO: drop count... delete proper
                emit(dropSignal, (long)pk->getByteLength());
                delete pk;
                return;
             }
        }break;
    }//end switch case incoming message
            }
}// end handle message
}
