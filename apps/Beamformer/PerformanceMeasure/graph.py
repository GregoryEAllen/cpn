import numpy
import pylab
import sys

num_cores = int(sys.argv[1])
files = sys.argv[3:]
outfile = sys.argv[2]
params = {
'text.fontsize': 10,
'legend.fontsize': 10,
}
pylab.rcParams.update(params)

def procfile(filen): 
    fid = open(filen)
    data = numpy.zeros((num_cores))
    for line in fid:
        l = line.strip().split(' ')
        #print l
        i = int(l[0]) - 1
        if data[i] > 0:
            data[i] = max(data[i], float(l[1]))
        else:
            data[i] = float(l[1])
    #print data
    return data

data = numpy.zeros((num_cores,len(files)))

for f in range(len(files)):
    data[:,f] = procfile(files[f])

#print data
y = numpy.arange(1,num_cores+1)
#print y

pylab.plot(y, data)
pylab.xlabel('Number of CPUs enabled')
pylab.ylabel('Throughput (samples/second)')
pylab.legend(files, loc=4)
pylab.savefig(outfile)

sys.exit()

pylab.figure()

data *= 577536
data /= 1024*1024*1024

tograph = numpy.zeros((num_cores,2))
tograph[:,0] = data[:,0]
tograph[:,1] = data[:,5]
pylab.plot(y, tograph)
pylab.xlabel('Number of CPUs enabled')
pylab.ylabel('Throughput (GFLOP/second)')
pylab.legend(['CPN', 'OMP'], loc='upper left')
pylab.savefig('flops.svg')


