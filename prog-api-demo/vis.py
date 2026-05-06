# Copyright 2021 Max Planck Institute for Software Systems, and
# National University of Singapore
#
# Permission is hereby granted, free of charge, to any person obtaining
# a copy of this software and associated documentation files (the
# "Software"), to deal in the Software without restriction, including
# without limitation the rights to use, copy, modify, merge, publish,
# distribute, sublicense, and/or sell copies of the Software, and to
# permit persons to whom the Software is furnished to do so, subject to
# the following conditions:
#
# The above copyright notice and this permission notice shall be
# included in all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
# IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
# CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
# TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
# SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

import asyncio
from matplotlib import pyplot as plt
from simbricks.client import simb_client
from simbricks.client.opus.base import create_run, ConsoleLineGenerator
from loop import instantiations, iperf_rates
from helpers import parse_Iperf_line_bytes

"""
Simple example of how to use SimBRicks api to start and analyse a simulation programatically.

In this file, the instantiations defined in 'loop.py' are submitted for execution to the backend.

The simulator output for each of them is parsed to create a plot comparing the three different Runs with each other.
"""


async def run_and_parse(instantiation) -> list[float]:
    sbc = await simb_client()
    run_id = await create_run(instantiation)
    line_gen = ConsoleLineGenerator(run_id, True, sbc)
    throughputs = []
    async for _, line in line_gen.generate_lines():
        through = parse_Iperf_line_bytes(line)
        if through is None:
            continue
        throughputs.append(through)

    return throughputs


def plot_throughputs(throughputs: list[list[float]]):
    averages = []
    for throughs in throughputs:
        assert len(throughs) > 0
        averages.append(sum(throughs) / len(throughs))

    plt.bar(iperf_rates, averages)
    plt.title("Parameters Comparison Chart")
    plt.xlabel("Configuration Parameters")
    plt.ylabel("Average MBytes Transferred")
    plt.savefig("demochart.png")


async def amain():
    throughputs = []
    for instantiation in instantiations:
        throughs = await run_and_parse(instantiation)
        throughputs.append(throughs[1:10])

    plot_throughputs(throughputs)


if __name__ == "__main__":
    asyncio.run(amain())
