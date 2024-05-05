import sys
import re
import json
import os
import traceback

def test_name(name):
  global tname
  print(f'TEST {name}')
  tname = name

def exception_thrown():
  e = sys.exc_info()[1]
  if isinstance(e, SystemExit):
    sys.exit(e.code)
  traceback.print_exc()

def fail(msg):
  global tname
  global test_fn
  print(f'FAILED {tname}:', msg, f' (inspect {test_fn})', file=sys.stderr)
  sys.exit(1)

def success():
  global tname
  print(f'SUCCESS {tname}')
  sys.exit(0)

def find_line(out, pattern):
  r = re.compile(pattern)
  for l in out:
    m = r.match(l)
    if m:
      return m
  return None


def load_testfile(fn):
  global test_fn
  test_fn = fn

  if not os.path.isfile(test_fn):
    fail('Simulation has not produced JSON output')

  try:
    with open(test_fn, 'r') as f:
      data = json.load(f)

    if not data['success']:
      fail(f'Simulation was not successful')
  except Exception:
    traceback.print_exc()
    fail('Loading simulation JSON output failed')

  return data
  
  
