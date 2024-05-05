from check_common import *

test_name('test4')
data = load_testfile('out/test4-1.json')

try:
  out = data['sims']['host.host']['stdout']
  line = find_line(out, '^STATUS: Success matrices match')
  if not line:
    fail('Could not find "STATUS: Success matrices match" output')
except:
  exception_thrown()
  fail('Parsing simulation output failed')

success()