#!/usr/bin/python

import sys
try:
    import json
except ImportError:
    import simplejson as json

results = {}

for f in sys.argv[1:]:
    fid = open(f)
    r = json.load(f)
    fid.close()
    for k, v in r.items():
        results[k] = v

json.dump(results, sys.stdout)
