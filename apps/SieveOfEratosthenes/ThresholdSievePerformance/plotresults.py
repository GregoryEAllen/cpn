import numpy
import pylab

params = {
'text.fontsize': 10,
'legend.fontsize': 10,
}
pylab.rcParams.update(params)

data = numpy.loadtxt("results.data.txt")

x = data[:,0]
y = data[:,1:]

xx = numpy.tile(x,(y.shape[1],1)).T
z = xx/y

# sort by largest average non-NaN result
z[numpy.isnan(z)]=0 # zero the NaNs
mnz = numpy.sum(z,axis=0)/numpy.sum(z!=0,axis=0)# mean of non-zeros
idx = numpy.argsort(numpy.log(mnz))[::-1]

pylab.loglog(x,z[:,idx])
pylab.grid()
pylab.xlabel('num prime candidates')
pylab.ylabel('prime candidates per second')

fid = open("results.legend.txt","r");
lines = fid.readlines()
fid.close()

lgd = lines[1:]
lgd2 = [ lgd[i] for i in idx ]

pylab.legend(lgd2,loc='upper left')

pylab.savefig('result.pdf')

