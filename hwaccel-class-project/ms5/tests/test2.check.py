from check_common import *

test_name('test2')

for (hw, clk) in [('comb', 9), ('vec', 8)]:
  data = load_testfile(f'out/test2-{hw}-{clk}-1.json')

  try:
    out = data['sims']['host.host']['stdout']
    line = find_line(out, '^Cycles per operation: ([0-9]*)')
    if not line:
      fail('Could not find "Cycles per operation:" output')

    cycles = int(line.group(1))

    print(f'HW {hw} with {clk}ns/clk results in {cycles} cycles/op')
  except Exception:
    exception_thrown()
    fail('Parsing simulation output failed')

success()
