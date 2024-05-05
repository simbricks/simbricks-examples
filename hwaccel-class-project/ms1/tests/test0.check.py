from check_common import *

test_name('test0')
data = load_testfile('out/test0-1.json')

try:
  out = data['sims']['host.host']['stdout']
  line = find_line(out, '^Cycles per operation: ([0-9]*)')
  if not line:
    fail('Could not find "Cycles per operation:" output')

  cycles = line.group(1)
  print('Cycles: ', cycles)
except:
  exception_thrown()
  fail('Parsing simulation output failed')

success()