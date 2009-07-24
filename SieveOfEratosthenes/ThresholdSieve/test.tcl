
set filename "test.out"
set maxprimes 1000000
set queuesize 5000
set iterations 10
set thresholdstart 2
set thresholdend 20
set thresholdstep 1
set primestart 1
set primeend 6
set primestep 1

for {set t $thresholdstart} {$t < $thresholdend} {incr t $thresholdstep} {
	for {set p $primestart} {$p < $primeend} {incr p $primestep} {
		puts "[pwd]/ThresholdSieve -m $maxprimes -q $queuesize -t $t -p $p -i $iterations -f $filename"
		exec [pwd]/ThresholdSieve -m $maxprimes -q $queuesize -t $t -p $p -i $iterations -f $filename
	}
}

