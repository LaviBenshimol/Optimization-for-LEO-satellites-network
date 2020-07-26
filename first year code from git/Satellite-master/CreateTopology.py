import getopt
import math
import os
import sys

PreCalcPrint = "\n\n\n" \
               "package networks;\n\n"\
               "import node.Node;\n"\
               "import ned.DatarateChannel;\n\n"\
               "network Net{0}\n" \
               "~\n" \
               "    //@display(\"bgi=maps/world;bgb=763,318\");\n\
               @display(\"bgi=background/earth(1);bgb=2000,1000\");\n" \
               "    types:\n" \
               "        channel C extends DatarateChannel\n" \
               "        ~\n" \
               "            parameters:\n" \
               "                delay = default(0.1ms);\n" \
               "                datarate = default(1Gbps);\n" \
               "        $$\n" \
               "    submodules:\n" \
               "        rte[{1}]: Node \n" \
               "        ~\n" \
               "            parameters:\n" \
               "                address = index;\n" \
               "                @display(\"i=device/satellite,white\");\n" \
               "        $$\n" \
               "    connections allowunconnected:\n"

BaseLine = "        rte[{0}].port++ <--> C <--> rte[{1}].port++;\n\r"


def CreateTopo(NumOfNodes, NumOfColumns, IsSphere=False):

    NetName = "Net{}.ned".format(NumOfNodes)
    if NumOfColumns == 0:
        Modulo = int(math.sqrt(NumOfNodes))
        if math.pow(Modulo, 2) != NumOfNodes:
            raise Exception("the Number of node given doesn't have a sqrt1\n"
                            "Please insert the right NumOfNodes or add Number of columns wanted")
    elif NumOfColumns > 0:
        Modulo = int(NumOfNodes/NumOfColumns)

    with open(NetName, "w") as file:
        file.write(PreCalcPrint.format(NumOfNodes, NumOfNodes).replace("~","{").replace("$$","}"))
        for index in range(NumOfNodes-1):
            if (index +1 ) % Modulo == 0:
                #one line down
                file.write(BaseLine.format(str(index), str(index + Modulo)))
            elif index >= NumOfNodes - Modulo:
                # one line to the right
                file.write(BaseLine.format(str(index), str(index + 1)))
            else:
                file.write(BaseLine.format(str(index), str(index + 1)))
                file.write(BaseLine.format(str(index), str(index + Modulo)))
        file.write("}\n\n")


def getTopoArgs(argv):
    NumOfNode = 0
    NumOfCols = 0
    Sphere = False
    outputfile = ''
    try:
        opts, args = getopt.getopt(argv, "n:hcs:", ["cfile=", "ofile="])
    except getopt.GetoptError:
        print ('test.py -n Number of Nodes -c Number of Columns -s Sphere')
        sys.exit(2)
    for opt, arg in opts:
        if opt == '-h':
            print ('test.py -n [Number of Nodes] [Optional]\n'
                   '-n Number of Nodes ==> a number that has a square root (4,64,400)\n'
                   '                       If the number is does not have a square root,you must insert Num of Columns '
                   '-c Number of Columns ==> Optional, if not given the Output will be a square\n '
                   '-s Sphere\n')
            sys.exit()
        elif opt=="-n":
            NumOfNode = int(arg)
        elif opt=="-c":
            NumOfCols = int(arg)
        elif opt=="-s":
            Sphere = True
    if NumOfNode == 0:
        raise Exception("Must provide Number of nodes!!\n"
                        "Please refer to help -h")
    return NumOfNode, NumOfCols, Sphere


if __name__ == '__main__':

    NumOfNode, NumOfCols, Sphere = getTopoArgs(sys.argv[1:])

    CreateTopo(NumOfNode, NumOfCols, Sphere)
