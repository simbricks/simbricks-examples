from check_common import *

test_name('test1')

# first check that test 1 passes, because performance numbers make no sense
# without correctness
data = load_testfile('out/test0-1.json')
try:
  out = data['sims']['host.host']['stdout']
  line = find_line(out, '^STATUS: Success matrices match')
  if not line:
    fail('Test 0 is not passing, thus test 1 is meaningless')
except Exception:
    exception_thrown()
    fail('Parsing test 0 simulation output failed')

cycle_times = []

for lat in [10000000000, 1000000000, 1000000, 1000]:
  data = load_testfile(f'out/test1-{lat}-1.json')

  try:
    out = data['sims']['host.host']['stdout']
    line = find_line(out, '^Cycles per operation: ([0-9]*)')
    if not line:
      fail('Could not find "Cycles per operation:" output')

    cycles = int(line.group(1))
    cycle_times.append((lat, cycles))
    print(f'Delay {lat} -> {cycles} Cycles/op')
  except Exception:
    exception_thrown()
    fail('Parsing simulation output failed')

perf_targets = {
  10000000000: (655449609, '0%'),
  1000000000: (75454613, '5%'),
  1000000: (10043642, '35%'),
  1000: (9994685, '35%'),
}

for (lat, cycles) in cycle_times:
  (target, perc) = perf_targets[lat]
  if cycles > target:
    fail(f'For {lat} cycles should be at least {perc} faster than ms3')
success()
