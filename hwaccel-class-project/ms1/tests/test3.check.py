from check_common import *

test_name('test3')

try:
  for bs in [16, 64, 128]:
    data = load_testfile(f'out/test3-{bs}-1.json')

    out = data['sims']['host.host']['stdout']
    line = find_line(out, '^Cycles per operation: ([0-9]*)')
    if not line:
      fail('Could not find "Cycles per operation:" output')

    cycles = int(line.group(1))
    print(f'Blocksize={bs} Cycles={cycles}')

    if cycles > 250000000:
      fail('Blocked multiply should be at least 20% faster than baseline')
except:
  exception_thrown()
  fail('Parsing simulation output failed')

success()