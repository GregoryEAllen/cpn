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

results.sort(lambda x, y: x['realtime'] - y['realtime'])

json.dump({sys.argv[1]: results[0] }, sys.stdout)

