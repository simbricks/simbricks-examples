import simbricks.orchestration.simulators as sim
import simbricks.orchestration.experiments as exp
from graphviz import Digraph, Graph
from simbricks.orchestration.e2e_topologies import E2EDumbbellTopology
from simbricks.orchestration.e2e_components import E2ESimpleNs3Host, E2ESimbricksHost


def experiment_graph(e: exp.Experiment):
    # Create Digraph object
    dot = Digraph()

    for s in e.all_simulators():
        args = {}
        if isinstance(s, sim.HostSim):
            args["shape"] = "circle"
            args["style"] = "filled"
            args["fillcolor"] = "cadetblue1"
        elif isinstance(s, sim.NetSim):
            args["shape"] = "diamond"
            args["style"] = "filled"
            args["fillcolor"] = "gold"
        elif isinstance(s, sim.NICSim):
            args["shape"] = "rectangle"
            args["style"] = "filled"
            args["fillcolor"] = "lightgreen"
        n = dot.node(s.full_name(), **args)

    for s in e.all_simulators():
        if isinstance(s, sim.HostSim):
            for d in s.pcidevs:
                dot.edge(s.full_name(), d.full_name(), color="green", label="PCIe")
        elif isinstance(s, sim.NetSim):
            for n in s.net_connect:
                dot.edge(s.full_name(), n.full_name(), color="red", label="Eth")
        elif isinstance(s, sim.NICSim):
            dot.edge(s.full_name(), s.network.full_name(), color="red", label="Eth")
    return dot


def experiment_graph_e2e_dumbbell(e2e_dumbbell_topo: E2EDumbbellTopology):
    dot = Graph()

    switch_args = {}
    switch_args["shape"] = "rectangle"
    switch_args["style"] = "filled"
    switch_args["fillcolor"] = "gold"
    dot.node(e2e_dumbbell_topo.left_switch.name, **switch_args)
    dot.node(e2e_dumbbell_topo.right_switch.name, **switch_args)
    dot.edge(
        e2e_dumbbell_topo.left_switch.name,
        e2e_dumbbell_topo.right_switch.name,
        color="red",
        label="Eth",
    )

    # plot left components
    for comp in e2e_dumbbell_topo.left_switch.components:
        args = {}
        if isinstance(comp, E2ESimpleNs3Host):
            args["shape"] = "rectangle"
            args["style"] = "filled"
            args["fillcolor"] = "gold"
            dot.node(comp.name, **args)
            dot.edge(
                comp.name,
                e2e_dumbbell_topo.left_switch.name,
                color="red",
                label="Eth",
            )
        elif isinstance(comp, E2ESimbricksHost):
            nic = comp.simbricks_component
            nic_name = f"NIC-{nic.name}"
            args["shape"] = "rectangle"
            args["style"] = "filled"
            args["fillcolor"] = "lightgreen"
            dot.node(nic_name, **args)
            dot.edge(
                nic_name,
                e2e_dumbbell_topo.left_switch.name,
                color="red",
                label="Eth",
            )

            host_node_name = f"{comp.name}-{comp.simbricks_component.name}"
            args = {}
            args["shape"] = "circle"
            args["style"] = "filled"
            args["fillcolor"] = "cadetblue1"
            dot.node(host_node_name, **args)
            dot.edge(
                host_node_name,
                nic_name,
                color="green",
                label="PCIe",
            )
        else:
            raise Exception("unknown component type")

    # plot right components
    for comp in e2e_dumbbell_topo.right_switch.components:
        args = {}
        if isinstance(comp, E2ESimpleNs3Host):
            args["shape"] = "rectangle"
            args["style"] = "filled"
            args["fillcolor"] = "gold"
            dot.node(comp.name, **args)
            dot.edge(
                comp.name,
                e2e_dumbbell_topo.right_switch.name,
                color="red",
                label="Eth",
            )
        elif isinstance(comp, E2ESimbricksHost):
            nic = comp.simbricks_component
            nic_name = f"NIC-{nic.name}"
            args["shape"] = "rectangle"
            args["style"] = "filled"
            args["fillcolor"] = "lightgreen"
            dot.node(nic_name, **args)
            dot.edge(
                nic_name,
                e2e_dumbbell_topo.right_switch.name,
                color="red",
                label="Eth",
            )

            host_node_name = f"{comp.name}-{comp.simbricks_component.name}"
            args = {}
            args["shape"] = "circle"
            args["style"] = "filled"
            args["fillcolor"] = "cadetblue1"
            dot.node(host_node_name, **args)
            dot.edge(
                host_node_name,
                nic_name,
                color="green",
                label="PCIe",
            )
        else:
            raise Exception("unknown component type")

    return dot
