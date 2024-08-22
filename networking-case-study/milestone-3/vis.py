import sys
import pathlib

workspace_path = pathlib.Path(__file__).parents[2]
sys.path.insert(0, f"{workspace_path}/utils")
from visualize import experiment_graph_e2e_dumbbell
import demo

# visualize simulation topology
if __name__ == "__main__":
    for e in demo.experiments:
        digraph = experiment_graph_e2e_dumbbell(e.networks[0].e2e_topologies[0])
        digraph.render(filename=f"{e.name}-digraph")
