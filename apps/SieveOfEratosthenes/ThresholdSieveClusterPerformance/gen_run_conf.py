#!/usr/bin/python

import sys
try:
    import json
except ImportError:
    import simplejson as json


out = sys.stdout
num_nodes = int(sys.argv[1])
conf_file = sys.argv[2]
run_name = sys.argv[3]

fid = open(conf_file)
conf = json.load(fid)
fid.close()
fid = open('%s.json'%(run_name))
run_conf = json.load(fid)
fid.close()

run_conf['kernels'] = conf['kernels'][0:num_nodes]

json.dump(run_conf, out)

