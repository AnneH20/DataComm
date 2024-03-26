#!/usr/bin/python                                                                            
# this file wasn't working to take the screenshots, so I rewrote it a bit to test.py to get the screenshots
# the contents shouldn't have changed though. but I wanted to include both so you can see why the screenshots say test.py
                                                                                             
from mininet.topo import Topo
from mininet.net import Mininet
from mininet.util import dumpNodeConnections
from mininet.log import setLogLevel

class SingleSwitchTopo(Topo):
    # "Single switch connected to n hosts."
    # def build(self, n=2):
    def build(self, n=4):
        "Add 2 Switches"
        s1 = self.addSwitch('s1')
        s2 = self.addSwitch('s2')

        "Add 4 Hosts"
        h1 = self.addHost('h1')
        h2 = self.addHost('h2')
        h3 = self.addHost('h3')
        h4 = self.addHost('h4')

        "Add links between the 2 Switches and 4 Hosts"
        self.addLink(h1, s1)
        self.addLink(h2, s1)
        self.addLink(s1, s2)
        self.addLink(h3, s2)
        self.addLink(h4, s2)

        # Python's range(N) generates 0..N-1
        # for h in range(n):
            # host = self.addHost('h%s' % (h + 1))
            # self.addLink(host, s1)

def simpleTest():
    "Create and test a simple network"
    topo = SingleSwitchTopo(n=4)
    net = Mininet(topo)
    net.start()
    print( "Dumping host connections" )
    dumpNodeConnections(net.hosts)
    # print( "Testing network connectivity" )
    # net.pingAll()
    # net.stop()

if __name__ == '__main__':
    # Tell mininet to print useful information
    setLogLevel('info')
    simpleTest()