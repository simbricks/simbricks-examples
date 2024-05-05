from check_common import *

test_name('test5')

try:
  bs = 64
  data = load_testfile(f'out/test5-{bs}-1.json')

  out = data['sims']['host.host']['stdout']
  line = find_line(out, '^Cycles per operation: ([0-9]*)')
  if not line:
    fail('Could not find "Cycles per operation:" output')

  cycles = int(line.group(1))
  print(f'Blocksize={bs} Cycles={cycles}')

  if cycles > 125000000:
    fail('Blocked multiply should be at least 50% faster than baseline')
except:
  exception_thrown()
  fail('Parsing simulation output failed')

success()