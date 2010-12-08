#!/usr/bin/python

import sys
try:
    import json
except ImportError:
    import simplejson as json


results = []
for f in sys.argv[2:]:
    fid = open(f)
    result = json.load(fid)
    fid.close()
    results += result

def cmp(x, y):
    if x['realtime'] < y['realtime']:
        return -1
    if x['realtime'] > y['realtime']:
        return 1
    return 0

results.sort(cmp)

json.dump({sys.argv[1]: results[0] }, sys.stdout)

