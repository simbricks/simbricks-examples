import sys
import pathlib

workspace_path = pathlib.Path(__file__).parents[2]
sys.path.insert(0, f"{workspace_path}/utils")
from visualize import experiment_graph
import demo

# visualize simulation topology
if __name__ == "__main__":
    for e in demo.experiments:
        digraph = experiment_graph(e)
        digraph.render(filename=f"{e.name}-digraph")
