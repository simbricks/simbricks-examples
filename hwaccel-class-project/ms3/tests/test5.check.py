from check_common import *

test_name('test5')

# first check that test 1 passes, because performance numbers make no sense
# without correctness
data = load_testfile('out/test4-1.json')
try:
  out = data['sims']['host.host']['stdout']
  line = find_line(out, '^STATUS: Success matrices match')
  if not line:
    fail('Test 4 is not passing, thus test 5 is meaningless')
except Exception:
    exception_thrown()
    fail('Parsing test 0 simulation output failed')

cycle_times = []

for lat in [10000000000, 1000000000, 1000000, 1000]:
  data = load_testfile(f'out/test5-{lat}-1.json')

  try:
    out = data['sims']['host.host']['stdout']
    line = find_line(out, '^Cycles per operation: ([0-9]*)')
    if not line:
      fail('Could not find "Cycles per operation:" output')

    cycles = int(line.group(1))
    cycle_times.append(cycles)
    print(f'Delay {lat} -> {cycles} Cycles/op')

    sim_out = data['sims']['dev.host.accel']['stderr']
    dma_r = find_line(sim_out, '^DMA READS = ([0-9]*)')
    dma_w = find_line(sim_out, '^DMA WRITES = ([0-9]*)')
    if int(dma_r.group(1)) == 0 or int(dma_w.group(1)) == 0:
      fail('DMA is enabled, but device is not using DMA in at least one '
          'direction')
  except Exception:
    exception_thrown()
    fail('Parsing simulation output failed')


if cycle_times[0] >= 678050068:
  fail('Slowest operation should be at least 25% faster than ms2')
if cycle_times[-1] >= 1321944633:
  fail('Fastest operation should be at least 80% faster than ms2')

success()
