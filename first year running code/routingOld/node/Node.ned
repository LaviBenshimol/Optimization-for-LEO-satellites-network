// This file is part of an OMNeT++/OMNEST simulation example. Copyright (C) 1992-2015 Andras Varga This file is distributed WITHOUT ANY WARRANTY. See the file.`license' for details on this and other legal matters.

package node;
import inet.mobility.contract.IMobility;

//
// A "Node" consists of a Routing plus an App module which represents the
// aggregate traffic of the subnets connected to that router.
//
module Node
{
    parameters:
        int address;
        int deltaValue = default(4);
        int Hops = default(4);
        int alpha;
        int m_var;
        //lll : num of nodes
        int numofsatellite = default(25);
        int changeRate = default(1);
        bool fullTopoRun = default(false);
        string appType;
        string mobilityType = default("SuperpositioningMobility");
        @networkNode();
        @statistic[coordX](title="coord X"; source="xCoord(mobilityPos(mobilityStateChanged))"; record=vector; interpolationmode=linear);
        @statistic[coordY](title="coord Y"; source="yCoord(mobilityPos(mobilityStateChanged))"; record=vector; interpolationmode=linear);
        @display("i=misc/satellite2,gold;bgb=366,218");
    gates:
        inout port[];
    submodules:
        app: <appType> like IApp {
            parameters:
                address = address;
                @display("p=140,42");
        }
        routing: Routing {
            parameters:
                @display("p=140,109");
            gates:
//                out[sizeof(port)];
                in[sizeof(port)];
                out[sizeof(port)];
        }
        queue[sizeof(port)]: L2Queue {
            parameters:
                @display("p=65,155,row");
        }
    //        mobility: <mobilityType> like IMobility {
    //            @display("p=310,60");
    //        }
    connections:
        routing.localOut --> app.in;
        routing.localIn <-- app.out;
        for i=0..sizeof(port)-1 {
                        routing.out[i] --> queue[i].in;
                        routing.in[i] <-- queue[i].out;
                        queue[i].line <--> port[i];
//         routing.out[i] <--> port[i]; 
        }
}
