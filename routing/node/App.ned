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
// Application model to generate traffic for the network.
//
simple App like IApp
{
    parameters:
        int address;  // local node address
     //   string destAddresses;  // destination addresses
        volatile double sendIaTime @unit(s) = default(exponential(1ms)); // time between generating packets
        volatile double burstInterval @unit(s) = default(70 s);
        volatile int packetLength @unit(byte) = default(1500byte);  // length of one message 1 megabit
        volatile double burst_next_interval @unit(s) = default(truncnormal(100s,20s));
        volatile double burst_next_event @unit(s) = default(truncnormal(10s,2s));
        volatile int burst_size  = default(truncnormal(300,50));
        @display("i=block/browser");
        @signal[endToEndDelay](type="simtime_t");
        @signal[hopCount](type="long");
        @signal[sourceAddress](type="long");
        @statistic[endToEndDelay](title="end-to-end delay of arrived packets";unit=s;record=vector,mean,max,count;interpolationmode=none);
        @statistic[hopCount](title="hop count of arrived packets";interpolationmode=none;record=vector?,mean,max,count);
        @statistic[sourceAddress](title="source address of arrived packets";interpolationmode=none;record=vector?);
    gates:
        input in;
        output out;
}


