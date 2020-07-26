


package networks;

import node.Node;
import ned.DatarateChannel;

network Net25
{
    //@display("bgi=maps/world;bgb=763,318");
         //      @display("bgi=background/earth(1);bgb=2000,1000");
    types:
        channel C extends DatarateChannel
        {
            parameters:
                delay = default(0ms);
                datarate = default(1Gbps);
        }
    submodules:
        rte[25]: Node 
        {
            parameters:
                address = index;
                @display("i=device/satellite,white");
        }
    connections allowunconnected:
        rte[0].port++ <--> C <--> rte[1].port++;
        rte[0].port++ <--> C <--> rte[5].port++;
        rte[1].port++ <--> C <--> rte[2].port++;
        rte[1].port++ <--> C <--> rte[6].port++;
        rte[2].port++ <--> C <--> rte[3].port++;
        rte[2].port++ <--> C <--> rte[7].port++;
        rte[3].port++ <--> C <--> rte[4].port++;
        rte[3].port++ <--> C <--> rte[8].port++;
        rte[4].port++ <--> C <--> rte[9].port++;
        rte[5].port++ <--> C <--> rte[6].port++;
        rte[5].port++ <--> C <--> rte[10].port++;
        rte[6].port++ <--> C <--> rte[7].port++;
        rte[6].port++ <--> C <--> rte[11].port++;
        rte[7].port++ <--> C <--> rte[8].port++;
        rte[7].port++ <--> C <--> rte[12].port++;
        rte[8].port++ <--> C <--> rte[9].port++;
        rte[8].port++ <--> C <--> rte[13].port++;
        rte[9].port++ <--> C <--> rte[14].port++;
        rte[10].port++ <--> C <--> rte[11].port++;
        rte[10].port++ <--> C <--> rte[15].port++;
        rte[11].port++ <--> C <--> rte[12].port++;
        rte[11].port++ <--> C <--> rte[16].port++;
        rte[12].port++ <--> C <--> rte[13].port++;
        rte[12].port++ <--> C <--> rte[17].port++;
        rte[13].port++ <--> C <--> rte[14].port++;
        rte[13].port++ <--> C <--> rte[18].port++;
        rte[14].port++ <--> C <--> rte[19].port++;
        rte[15].port++ <--> C <--> rte[16].port++;
        rte[15].port++ <--> C <--> rte[20].port++;
        rte[16].port++ <--> C <--> rte[17].port++;
        rte[16].port++ <--> C <--> rte[21].port++;
        rte[17].port++ <--> C <--> rte[18].port++;
        rte[17].port++ <--> C <--> rte[22].port++;
        rte[18].port++ <--> C <--> rte[19].port++;
        rte[18].port++ <--> C <--> rte[23].port++;
        rte[19].port++ <--> C <--> rte[24].port++;
        rte[20].port++ <--> C <--> rte[21].port++;
        rte[21].port++ <--> C <--> rte[22].port++;
        rte[22].port++ <--> C <--> rte[23].port++;
        rte[23].port++ <--> C <--> rte[24].port++;
}

