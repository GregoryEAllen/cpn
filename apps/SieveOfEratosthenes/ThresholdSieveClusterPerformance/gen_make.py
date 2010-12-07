#!/usr/bin/python

import sys
try:
    import json
except ImportError:
    import simplejson as json

out = sys.stdout


conf_file = sys.argv[1]
fid =  open(conf_file)
conf = json.load(fid)
fid.close()

for (run_name, run_data) in conf['runs'].items():
    opts = { 'name': run_name, 'conf': conf_file, 'run_conf': conf['gen_conf'] }
    for num_kernels in range(1, len(conf['kernels']) + 1):
        opts['k'] = num_kernels
        out.write('result_files += %(name)s_conf.%(k)d.json\n'%opts)
        out.write('%(name)s_conf.%(k)d.json: %(conf)s %(run_conf)s\n'%opts)
        out.write('\t%(run_conf)s %(k)d %(conf)s %(name)s > $@\n\n'%opts)
        for iter in range(conf['iterations']):
            opts['i'] = iter
            out.write('%(name)s_%(k)d_results += %(name)s_result.%(k)d.%(i)d.json\n'%opts)
            out.write('%(name)s_result.%(k)d.%(i)d.json: %(name)s_conf.%(k)d.json\n'%opts)
            out.write('\t./runtest -P $(realpath %(name)s_conf.%(k)d.json) -f $(shell pwd)/$@\n\n'%opts)
        out.write('result_files += $(%(name)s_%(k)d_results)\n'%opts)
        out.write('result_files += %(name)s_results.%(k)d.json\n'%opts)
        out.write('%(name)s_results += %(name)s_results.%(k)d.json\n'%opts)
        out.write('%(name)s_results.%(k)d.json: $(%(name)s_%(k)d_results)\n'%opts)
        out.write('\t./consolidate_run.py %(k)d $^ > $@\n\n'%opts)
    out.write('all_results += %(name)s_results.json\n'%opts)
    out.write('%(name)s_results.json: $(%(name)s_results)\n'%opts)
    out.write('\t./consolidate_results.py $^ > $@\n\n')

out.write('result_files += $(all_results)\n')
