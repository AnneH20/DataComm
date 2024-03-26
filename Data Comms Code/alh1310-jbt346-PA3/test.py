from mininet.topo import Topo
from mininet.net import Mininet
from mininet.util import dumpNodeConnections
from mininet.log import setLogLevel

TOPOS = {'mytopo' : (lambda : mytopo(4))}

class mytopo(Topo):
    "Single switch connected to n hosts."
    def build(self, n=4):
        switch1 = self.addSwitch('s1')
        switch2=self.addSwitch('s2')
        host1 = self.addHost('h%s' % (1))
        self.addLink(host1, switch1)
        host2 = self.addHost('h%s' % (2))
        self.addLink(host2, switch1)
        self.addLink(switch1, switch2)
        host3 = self.addHost('h%s' % (3))
        self.addLink(host3, switch2)
        host4 = self.addHost('h%s' % (4))
        self.addLink(host4, switch2)

def simpleTest():
    "Create and test a simple network"
    topo = mytopo(n=4)
    net = Mininet(topo)
    #net.start()
    #net.stop()

if __name__ == '__main__':
    # Tell mininet to print useful information
    setLogLevel('info')
    simpleTest()
