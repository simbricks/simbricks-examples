from check_common import *

test_name('test2')
data = load_testfile('out/test2-1.json')

try:
  out = data['sims']['host.host']['stdout']
  line = find_line(out, '^STATUS: Success matrices match')
  if not line:
    fail('Could not find "STATUS: Success matrices match" output')

  sim_out = data['sims']['dev.host.accel']['stderr']
  dma_r = find_line(sim_out, '^DMA READS = ([0-9]*)')
  dma_w = find_line(sim_out, '^DMA WRITES = ([0-9]*)')
  if int(dma_r.group(1)) > 0 or int(dma_w.group(1)) > 0:
    fail('DMA is disabled in this test but devices issues dma')
except:
  exception_thrown()
  fail('Parsing simulation output failed')

success()