import simbricks.orchestration.simulators as sim
from graphviz import Digraph

def experiment_graph(e):
    # Create Digraph object
    dot = Digraph()

    for s in e.all_simulators():
        args = {}
        if isinstance(s, sim.HostSim):
            args['shape'] = 'circle'
            args['style'] = 'filled'
            args['fillcolor'] = 'cadetblue1'
        elif isinstance(s, sim.NetSim):
            args['shape'] = 'diamond'
            args['style'] = 'filled'
            args['fillcolor'] = 'gold'
        elif isinstance(s, sim.NICSim):
            args['shape'] = 'rectangle'
            args['style'] = 'filled'
            args['fillcolor'] = 'lightgreen'
        n = dot.node(s.full_name(), **args)


    for s in e.all_simulators():
        if isinstance(s, sim.HostSim):
            for d in s.pcidevs:
                dot.edge(s.full_name(),
                         d.full_name(),
                         color='green',
                         label='PCIe')
        elif isinstance(s, sim.NetSim):
            for n in s.net_connect:
                dot.edge(s.full_name(),
                         n.full_name(),
                         color='red',
                         label='Eth')
        elif isinstance(s, sim.NICSim):
            dot.edge(s.full_name(),
                     s.network.full_name(),
                     color='red',
                     label='Eth')
    return dot