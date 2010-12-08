#!/usr/bin/python

import sys

try:
    import json
except ImportError:
    import simplejson as json
import numpy
import pylab

out_file = sys.argv[1]
conf_file = sys.argv[2]

fid = open(conf_file)
conf = json.load(fid)
fid.close()

x_len = len(conf['kernels'])
num_runs = len(conf['runs'])

def load_res(file):
    fid = open(file + '_result.json')
    res = json.load(fid)
    fid.close()
    data = numpy.zeros((x_len))
    for (k, v) in res.items():
        data[int(k) - 1] = v['maxprime']/v['realtime']
    return data


labels = conf['runs']
data = numpy.zeros((x_len, len(labels)))

for i in range(len(labels)):
    data[:,i] = load_res(labels[i])

y = numpy.arange(1, x_len + 1)

print y, data

pylab.plot(y, data)
pylab.xlabel('Number of nodes')
pylab.ylabel('Prime per second')
pylab.legend(labels, loc='upper left')
pylab.savefig(out_file)

