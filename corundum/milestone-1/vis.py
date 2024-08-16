import sys

sys.path.insert(0, "/workspaces/simbricks-examples/utils")
from visualize import experiment_graph
import demo_1_a

# visualize simulation topology
if __name__ == "__main__":
    for e in demo_1_a.experiments:
        digraph = experiment_graph(e)
        digraph.render(filename=f"{e.name}-digraph")
