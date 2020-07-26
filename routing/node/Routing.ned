//
// This file is part of an OMNeT++/OMNEST simulation example.
//
// Copyright (C) 1992-2015 Andras Varga
//
// This file is distributed WITHOUT ANY WARRANTY. See the file
// `license' for details on this and other legal matters.
//

package node;

//
// Models a router.
//
simple Routing
{
    parameters:
        @display("i=block/switch");
        @signal[drop](type="long");
        @signal[outputIf](type="long");
        @signal[TopoLinkCounter](type="long");
        @signal[hopCount](type="long");
        @statistic[drop](title="dropped packet byte length";unit=bytes;record=vector?,count,sum;interpolationmode=none);
        @statistic[outputIf](title="output interface for each routed packet";record=histogram;interpolationmode=none);
        @statistic[TopoLinkCounter](title="Number of links enabled in Topology";interpolationmode=none;record=histogram,mean,max,count);
        @statistic[hopCount](title="count number of hops for packet";interpolationmode=none;record=histogram,mean,max,count);
        volatile double sendTime @unit(s) = default(exponential(10ms)); // time between generating packets
        
        	@signal[load_balance_channel](type="string");
    gates:
        input in[];
        output out[];
        input localIn;
        output localOut;
        
//		inout out[]; //lll : no queue
        
}


