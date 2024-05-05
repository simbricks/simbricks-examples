from check_common import *

test_name('test0')

for n in [128, 512]:
  data = load_testfile(f'out/test0-{n}-1.json')

  try:
    out = data['sims']['host.host']['stdout']
    line = find_line(out, '^Accelerator Size: ([0-9]*)')
    if not line:
      fail('Could not find "Accelerator Size:" output')

    size = int(line.group(1))
    if size != n:
        fail(f'Accelerator size from log ({size}) does not match expectation '
             f'({n})')
  except Exception:
    exception_thrown()
    fail('Parsing simulation output failed')

success()
