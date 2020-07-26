package networks;

import node.Node;
import ned.DatarateChannel;

network Net100
{
    //@display("bgi=maps/world;bgb=763,318");
         //      @display("bgi=background/earth(1);bgb=2000,1000");
    types:
        channel C extends DatarateChannel
        {
            parameters:
                delay = default(0ms);
                datarate = default(0.5Gbps);
        }
    submodules:
        rte[100]: Node 
        {
            parameters:           
                address = index;
				@display("i=device/satellite,white;p=,,m,10,500,500");
        }
    connections allowunconnected:
        rte[0].port++ <--> C <--> rte[1].port++;
        rte[0].port++ <--> C <--> rte[10].port++;
        rte[1].port++ <--> C <--> rte[2].port++;
        rte[1].port++ <--> C <--> rte[11].port++;
        rte[2].port++ <--> C <--> rte[3].port++;
        rte[2].port++ <--> C <--> rte[12].port++;
        rte[3].port++ <--> C <--> rte[4].port++;
        rte[3].port++ <--> C <--> rte[13].port++;
        rte[4].port++ <--> C <--> rte[5].port++;
        rte[4].port++ <--> C <--> rte[14].port++;
        rte[5].port++ <--> C <--> rte[6].port++;
        rte[5].port++ <--> C <--> rte[15].port++;
        rte[6].port++ <--> C <--> rte[7].port++;
        rte[6].port++ <--> C <--> rte[16].port++;
        rte[7].port++ <--> C <--> rte[8].port++;
        rte[7].port++ <--> C <--> rte[17].port++;
        rte[8].port++ <--> C <--> rte[9].port++;
        rte[8].port++ <--> C <--> rte[18].port++;
        rte[9].port++ <--> C <--> rte[19].port++;
        rte[10].port++ <--> C <--> rte[11].port++;
        rte[10].port++ <--> C <--> rte[20].port++;
        rte[11].port++ <--> C <--> rte[12].port++;
        rte[11].port++ <--> C <--> rte[21].port++;
        rte[12].port++ <--> C <--> rte[13].port++;
        rte[12].port++ <--> C <--> rte[22].port++;
        rte[13].port++ <--> C <--> rte[14].port++;
        rte[13].port++ <--> C <--> rte[23].port++;
        rte[14].port++ <--> C <--> rte[15].port++;
        rte[14].port++ <--> C <--> rte[24].port++;
        rte[15].port++ <--> C <--> rte[16].port++;
        rte[15].port++ <--> C <--> rte[25].port++;
        rte[16].port++ <--> C <--> rte[17].port++;
        rte[16].port++ <--> C <--> rte[26].port++;
        rte[17].port++ <--> C <--> rte[18].port++;
        rte[17].port++ <--> C <--> rte[27].port++;
        rte[18].port++ <--> C <--> rte[19].port++;
        rte[18].port++ <--> C <--> rte[28].port++;
        rte[19].port++ <--> C <--> rte[29].port++;
        rte[20].port++ <--> C <--> rte[21].port++;
        rte[20].port++ <--> C <--> rte[30].port++;
        rte[21].port++ <--> C <--> rte[22].port++;
        rte[21].port++ <--> C <--> rte[31].port++;
        rte[22].port++ <--> C <--> rte[23].port++;
        rte[22].port++ <--> C <--> rte[32].port++;
        rte[23].port++ <--> C <--> rte[24].port++;
        rte[23].port++ <--> C <--> rte[33].port++;
        rte[24].port++ <--> C <--> rte[25].port++;
        rte[24].port++ <--> C <--> rte[34].port++;
        rte[25].port++ <--> C <--> rte[26].port++;
        rte[25].port++ <--> C <--> rte[35].port++;
        rte[26].port++ <--> C <--> rte[27].port++;
        rte[26].port++ <--> C <--> rte[36].port++;
        rte[27].port++ <--> C <--> rte[28].port++;
        rte[27].port++ <--> C <--> rte[37].port++;
        rte[28].port++ <--> C <--> rte[29].port++;
        rte[28].port++ <--> C <--> rte[38].port++;
        rte[29].port++ <--> C <--> rte[39].port++;
        rte[30].port++ <--> C <--> rte[31].port++;
        rte[30].port++ <--> C <--> rte[40].port++;
        rte[31].port++ <--> C <--> rte[32].port++;
        rte[31].port++ <--> C <--> rte[41].port++;
        rte[32].port++ <--> C <--> rte[33].port++;
        rte[32].port++ <--> C <--> rte[42].port++;
        rte[33].port++ <--> C <--> rte[34].port++;
        rte[33].port++ <--> C <--> rte[43].port++;
        rte[34].port++ <--> C <--> rte[35].port++;
        rte[34].port++ <--> C <--> rte[44].port++;
        rte[35].port++ <--> C <--> rte[36].port++;
        rte[35].port++ <--> C <--> rte[45].port++;
        rte[36].port++ <--> C <--> rte[37].port++;
        rte[36].port++ <--> C <--> rte[46].port++;
        rte[37].port++ <--> C <--> rte[38].port++;
        rte[37].port++ <--> C <--> rte[47].port++;
        rte[38].port++ <--> C <--> rte[39].port++;
        rte[38].port++ <--> C <--> rte[48].port++;
        rte[39].port++ <--> C <--> rte[49].port++;
        rte[40].port++ <--> C <--> rte[41].port++;
        rte[40].port++ <--> C <--> rte[50].port++;
        rte[41].port++ <--> C <--> rte[42].port++;
        rte[41].port++ <--> C <--> rte[51].port++;
        rte[42].port++ <--> C <--> rte[43].port++;
        rte[42].port++ <--> C <--> rte[52].port++;
        rte[43].port++ <--> C <--> rte[44].port++;
        rte[43].port++ <--> C <--> rte[53].port++;
        rte[44].port++ <--> C <--> rte[45].port++;
        rte[44].port++ <--> C <--> rte[54].port++;
        rte[45].port++ <--> C <--> rte[46].port++;
        rte[45].port++ <--> C <--> rte[55].port++;
        rte[46].port++ <--> C <--> rte[47].port++;
        rte[46].port++ <--> C <--> rte[56].port++;
        rte[47].port++ <--> C <--> rte[48].port++;
        rte[47].port++ <--> C <--> rte[57].port++;
        rte[48].port++ <--> C <--> rte[49].port++;
        rte[48].port++ <--> C <--> rte[58].port++;
        rte[49].port++ <--> C <--> rte[59].port++;
        rte[50].port++ <--> C <--> rte[51].port++;
        rte[50].port++ <--> C <--> rte[60].port++;
        rte[51].port++ <--> C <--> rte[52].port++;
        rte[51].port++ <--> C <--> rte[61].port++;
        rte[52].port++ <--> C <--> rte[53].port++;
        rte[52].port++ <--> C <--> rte[62].port++;
        rte[53].port++ <--> C <--> rte[54].port++;
        rte[53].port++ <--> C <--> rte[63].port++;
        rte[54].port++ <--> C <--> rte[55].port++;
        rte[54].port++ <--> C <--> rte[64].port++;
        rte[55].port++ <--> C <--> rte[56].port++;
        rte[55].port++ <--> C <--> rte[65].port++;
        rte[56].port++ <--> C <--> rte[57].port++;
        rte[56].port++ <--> C <--> rte[66].port++;
        rte[57].port++ <--> C <--> rte[58].port++;
        rte[57].port++ <--> C <--> rte[67].port++;
        rte[58].port++ <--> C <--> rte[59].port++;
        rte[58].port++ <--> C <--> rte[68].port++;
        rte[59].port++ <--> C <--> rte[69].port++;
        rte[60].port++ <--> C <--> rte[61].port++;
        rte[60].port++ <--> C <--> rte[70].port++;
        rte[61].port++ <--> C <--> rte[62].port++;
        rte[61].port++ <--> C <--> rte[71].port++;
        rte[62].port++ <--> C <--> rte[63].port++;
        rte[62].port++ <--> C <--> rte[72].port++;
        rte[63].port++ <--> C <--> rte[64].port++;
        rte[63].port++ <--> C <--> rte[73].port++;
        rte[64].port++ <--> C <--> rte[65].port++;
        rte[64].port++ <--> C <--> rte[74].port++;
        rte[65].port++ <--> C <--> rte[66].port++;
        rte[65].port++ <--> C <--> rte[75].port++;
        rte[66].port++ <--> C <--> rte[67].port++;
        rte[66].port++ <--> C <--> rte[76].port++;
        rte[67].port++ <--> C <--> rte[68].port++;
        rte[67].port++ <--> C <--> rte[77].port++;
        rte[68].port++ <--> C <--> rte[69].port++;
        rte[68].port++ <--> C <--> rte[78].port++;
        rte[69].port++ <--> C <--> rte[79].port++;
        rte[70].port++ <--> C <--> rte[71].port++;
        rte[70].port++ <--> C <--> rte[80].port++;
        rte[71].port++ <--> C <--> rte[72].port++;
        rte[71].port++ <--> C <--> rte[81].port++;
        rte[72].port++ <--> C <--> rte[73].port++;
        rte[72].port++ <--> C <--> rte[82].port++;
        rte[73].port++ <--> C <--> rte[74].port++;
        rte[73].port++ <--> C <--> rte[83].port++;
        rte[74].port++ <--> C <--> rte[75].port++;
        rte[74].port++ <--> C <--> rte[84].port++;
        rte[75].port++ <--> C <--> rte[76].port++;
        rte[75].port++ <--> C <--> rte[85].port++;
        rte[76].port++ <--> C <--> rte[77].port++;
        rte[76].port++ <--> C <--> rte[86].port++;
        rte[77].port++ <--> C <--> rte[78].port++;
        rte[77].port++ <--> C <--> rte[87].port++;
        rte[78].port++ <--> C <--> rte[79].port++;
        rte[78].port++ <--> C <--> rte[88].port++;
        rte[79].port++ <--> C <--> rte[89].port++;
        rte[80].port++ <--> C <--> rte[81].port++;
        rte[80].port++ <--> C <--> rte[90].port++;
        rte[81].port++ <--> C <--> rte[82].port++;
        rte[81].port++ <--> C <--> rte[91].port++;
        rte[82].port++ <--> C <--> rte[83].port++;
        rte[82].port++ <--> C <--> rte[92].port++;
        rte[83].port++ <--> C <--> rte[84].port++;
        rte[83].port++ <--> C <--> rte[93].port++;
        rte[84].port++ <--> C <--> rte[85].port++;
        rte[84].port++ <--> C <--> rte[94].port++;
        rte[85].port++ <--> C <--> rte[86].port++;
        rte[85].port++ <--> C <--> rte[95].port++;
        rte[86].port++ <--> C <--> rte[87].port++;
        rte[86].port++ <--> C <--> rte[96].port++;
        rte[87].port++ <--> C <--> rte[88].port++;
        rte[87].port++ <--> C <--> rte[97].port++;
        rte[88].port++ <--> C <--> rte[89].port++;
        rte[88].port++ <--> C <--> rte[98].port++;
        rte[89].port++ <--> C <--> rte[99].port++;
        rte[90].port++ <--> C <--> rte[91].port++;
        rte[91].port++ <--> C <--> rte[92].port++;
        rte[92].port++ <--> C <--> rte[93].port++;
        rte[93].port++ <--> C <--> rte[94].port++;
        rte[94].port++ <--> C <--> rte[95].port++;
        rte[95].port++ <--> C <--> rte[96].port++;
        rte[96].port++ <--> C <--> rte[97].port++;
        rte[97].port++ <--> C <--> rte[98].port++;
        rte[98].port++ <--> C <--> rte[99].port++;

//rte[0].port++ <--> C <--> rte[90].port++;
//rte[1].port++ <--> C <--> rte[91].port++;
//rte[2].port++ <--> C <--> rte[92].port++;
//rte[3].port++ <--> C <--> rte[93].port++;
//rte[4].port++ <--> C <--> rte[94].port++;
//rte[5].port++ <--> C <--> rte[95].port++;
//rte[6].port++ <--> C <--> rte[96].port++;
//rte[7].port++ <--> C <--> rte[97].port++;
//rte[8].port++ <--> C <--> rte[98].port++;
//rte[9].port++ <--> C <--> rte[99].port++;
//rte[0].port++ <--> C <--> rte[9].port++;
//rte[10].port++ <--> C <--> rte[19].port++;
//rte[20].port++ <--> C <--> rte[29].port++;
//rte[30].port++ <--> C <--> rte[39].port++;
//rte[40].port++ <--> C <--> rte[49].port++;
//rte[50].port++ <--> C <--> rte[59].port++;
//rte[60].port++ <--> C <--> rte[69].port++;
//rte[70].port++ <--> C <--> rte[79].port++;
//rte[80].port++ <--> C <--> rte[89].port++;
//rte[90].port++ <--> C <--> rte[99].port++;
}

