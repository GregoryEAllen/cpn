

proc command {maxprime queuesize threshold ppn iterations wheel zerocopy filename} {
    return "\$(PROG) -m $maxprime -q $queuesize -t $threshold -p $ppn -i $iterations -w $wheel -z $zerocopy -f $filename"
}
proc mkfilename {maxprime queuesize threshold ppn iterations wheel zerocopy} {
    return "result.$maxprime.$queuesize.threshold.[join [split $ppn {,}] {-}].$iterations.$wheel.$zerocopy-\$(OS)"
}

set resultdec {}
set resultrules {}
set num 0
# set defaults
set queuesize 5000
set threshold 2500
set ppn "1,0,0"
set iterations 5
set wheel 0
set zerocopy 0
set minimumprime 1000
set maximumprime 1000000

# First test for ppn = 1, w = 0

for {set maxprime $minimumprime} {$maxprime <= $maximumprime} {set maxprime [expr $maxprime * 10]} {
    set filename [mkfilename $maxprime $queuesize $threshold $ppn $iterations $wheel $zerocopy]
    lappend resultrules "$filename:\n\t[command $maxprime $queuesize $threshold $ppn $iterations $wheel $zerocopy $filename]\n"
    lappend resultdec $filename
    incr num
}

set wheel 5

for {set maxprime $minimumprime} {$maxprime <= $maximumprime} {set maxprime [expr $maxprime * 10]} {
    set filename [mkfilename $maxprime $queuesize $threshold $ppn $iterations $wheel $zerocopy]
    lappend resultrules "$filename:\n\t[command $maxprime $queuesize $threshold $ppn $iterations $wheel $zerocopy $filename]\n"
    lappend resultdec $filename
    incr num
}

set maximumprime 100000000
set ppn "5,0,0"

for {set maxprime $minimumprime} {$maxprime <= $maximumprime} {set maxprime [expr $maxprime * 10]} {
    set filename [mkfilename $maxprime $queuesize $threshold $ppn $iterations $wheel $zerocopy]
    lappend resultrules "$filename:\n\t[command $maxprime $queuesize $threshold $ppn $iterations $wheel $zerocopy $filename]\n"
    lappend resultdec $filename
    incr num
}

set ppn "0.5,0.5,0.5"

for {set maxprime $minimumprime} {$maxprime <= $maximumprime} {set maxprime [expr $maxprime * 10]} {
    set filename [mkfilename $maxprime $queuesize $threshold $ppn $iterations $wheel $zerocopy]
    lappend resultrules "$filename:\n\t[command $maxprime $queuesize $threshold $ppn $iterations $wheel $zerocopy $filename]\n"
    lappend resultdec $filename
    incr num
}

set zerocopy 1

for {set maxprime $minimumprime} {$maxprime <= $maximumprime} {set maxprime [expr $maxprime * 10]} {
    set filename [mkfilename $maxprime $queuesize $threshold $ppn $iterations $wheel $zerocopy]
    lappend resultrules "$filename:\n\t[command $maxprime $queuesize $threshold $ppn $iterations $wheel $zerocopy $filename]\n"
    lappend resultdec $filename
    incr num
}

set zerocopy 2

for {set maxprime $minimumprime} {$maxprime <= $maximumprime} {set maxprime [expr $maxprime * 10]} {
    set filename [mkfilename $maxprime $queuesize $threshold $ppn $iterations $wheel $zerocopy]
    lappend resultrules "$filename:\n\t[command $maxprime $queuesize $threshold $ppn $iterations $wheel $zerocopy $filename]\n"
    lappend resultdec $filename
    incr num
}

set zerocopy 3

for {set maxprime $minimumprime} {$maxprime <= $maximumprime} {set maxprime [expr $maxprime * 10]} {
    set filename [mkfilename $maxprime $queuesize $threshold $ppn $iterations $wheel $zerocopy]
    lappend resultrules "$filename:\n\t[command $maxprime $queuesize $threshold $ppn $iterations $wheel $zerocopy $filename]\n"
    lappend resultdec $filename
    incr num
}

set fd [open {results.mk} w]

puts $fd "TARGETS = [join $resultdec]"

foreach x $resultrules {
    puts $fd $x
}
close $fd
