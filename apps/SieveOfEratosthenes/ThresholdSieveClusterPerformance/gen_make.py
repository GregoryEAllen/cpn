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

opts = {'conf': conf_file, 'gen_conf': conf['gen_conf'] }
for run_name in conf['runs']:
    opts['name'] = run_name
    opts['run_conf'] = '%s.json'%(run_name)
    for num_kernels in range(1, len(conf['kernels']) + 1):
        opts['k'] = num_kernels
        out.write('result_files += %(name)s_conf.%(k)d.json\n'%opts)
        out.write('%(name)s_conf.%(k)d.json: %(gen_conf)s %(run_conf)s\n'%opts)
        out.write('\t%(gen_conf)s %(k)d %(conf)s %(name)s > $@\n\n'%opts)
        for iter in range(conf['iterations']):
            opts['i'] = iter
            out.write('%(name)s_%(k)d_result += %(name)s_result.%(k)d.%(i)d.json\n'%opts)
            out.write('%(name)s_result.%(k)d.%(i)d.json: %(name)s_conf.%(k)d.json\n'%opts)
            out.write('\t./runtest -P $(realpath %(name)s_conf.%(k)d.json) -V -f $(shell pwd)/$@\n\n'%opts)
        out.write('result_files += $(%(name)s_%(k)d_result)\n'%opts)
        out.write('%(name)s_result += %(name)s_result.%(k)d.json\n'%opts)
        out.write('%(name)s_result.%(k)d.json: $(%(name)s_%(k)d_result)\n'%opts)
        out.write('\t./consolidate_run.py %(k)d $^ > $@\n\n'%opts)
    out.write('result_files += $(%(name)s_result)\n'%opts)
    out.write('all_result += %(name)s_result.json\n'%opts)
    out.write('%(name)s_result.json: $(%(name)s_result)\n'%opts)
    out.write('\t./consolidate_results.py $^ > $@\n\n')

out.write('result_files += $(all_result)\n')
out.write('result_files += results.pdf\n')

out.write('results.pdf: $(all_result)\n')
out.write('\t./graph.py $@ %(conf)s\n\n'%opts)
