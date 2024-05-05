from check_common import *

test_name('test2')

# first check that test 1 passes, because performance numbers make no sense
# without correctness
data = load_testfile('out/test1-1.json')
try:
  out = data['sims']['host.host']['stdout']
  line = find_line(out, '^STATUS: Success matrices match')
  if not line:
    fail('Test 1 is not passing, thus test 2 is meaningless')
except Exception:
    exception_thrown()
    fail('Parsing test 1 simulation output failed')

cycle_times = []

for lat in [10000000000, 1000000000, 1000000, 1000]:
  data = load_testfile(f'out/test2-{lat}-1.json')

  try:
    out = data['sims']['host.host']['stdout']
    line = find_line(out, '^Cycles per operation: ([0-9]*)')
    if not line:
      fail('Could not find "Cycles per operation:" output')

    cycles = int(line.group(1))
    cycle_times.append(cycles)
    print(f'Delay {lat} -> {cycles} Cycles/op')
  except Exception:
    exception_thrown()
    fail('Parsing simulation output failed')

# check that difference between the slowest (10ms) and fastest (1ns) is at least
# 80% of the 10ms it takes for the operation
diff = cycle_times[0] - cycle_times[-1]
if diff < (0.8 * 10000000):
  fail('Slowest operation is not at least 8ms faster than fastest')

success()
