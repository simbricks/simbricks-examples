import os


linemap = {}

for tag in os.listdir('runs/'):
    if 'x' not in tag:
        continue
    ps = tag.split('x')
    sz = int(ps[0])
    period = int(ps[1])

    # get static analysis result
    watts = 0.0
    wns = 0.0
    stafn = f'runs/{tag}/logs/synthesis/2-sta.log'
    staf = open(stafn, 'r')
    in_power = False
    for l in staf:
        if l.startswith('power_report'):
            in_power = True
        if l.startswith('power_report_end'):
            in_power = False
        if in_power:
            if l.startswith('Total'):
                ps = l.split()
                watts = float(ps[4])
        if l.startswith('wns'):
            ps = l.split()
            wns = ps[1]
    staf.close()

    # get area        
    areafn = f'runs/{tag}/reports/floorplan/3-initial_fp_core_area.rpt'
    area = 0.0
    with open(areafn, 'r') as f:
        l = f.read()
        ps = l.split()
        x1 = float(ps[0])
        y1 = float(ps[1])
        x2 = float(ps[2])
        y2 = float(ps[3])
        w = (x2 - x1) / 1000.
        h = (y2 - y1) / 1000.
        area = w * h

    l = f'{sz},{period},{wns},{watts},{area}'
    if sz not in linemap:
        linemap[sz] = {}
    linemap[sz][period] = l

print('size,period,wns,power,area')
for sz in sorted(linemap.keys()):
    for period in sorted(linemap[sz].keys()):
        print(linemap[sz][period])
