TARGETS = result.1000.5000.threshold.1-0-0.5.0.0 result.10000.5000.threshold.1-0-0.5.0.0 result.100000.5000.threshold.1-0-0.5.0.0 result.1000000.5000.threshold.1-0-0.5.0.0 result.1000.5000.threshold.1-0-0.5.5.0 result.10000.5000.threshold.1-0-0.5.5.0 result.100000.5000.threshold.1-0-0.5.5.0 result.1000000.5000.threshold.1-0-0.5.5.0 result.1000.5000.threshold.5-0-0.5.5.0 result.10000.5000.threshold.5-0-0.5.5.0 result.100000.5000.threshold.5-0-0.5.5.0 result.1000000.5000.threshold.5-0-0.5.5.0 result.10000000.5000.threshold.5-0-0.5.5.0 result.100000000.5000.threshold.5-0-0.5.5.0 result.1000.5000.threshold.0.5-0.5-0.5.5.5.0 result.10000.5000.threshold.0.5-0.5-0.5.5.5.0 result.100000.5000.threshold.0.5-0.5-0.5.5.5.0 result.1000000.5000.threshold.0.5-0.5-0.5.5.5.0 result.10000000.5000.threshold.0.5-0.5-0.5.5.5.0 result.100000000.5000.threshold.0.5-0.5-0.5.5.5.0 result.1000.5000.threshold.0.5-0.5-0.5.5.5.1 result.10000.5000.threshold.0.5-0.5-0.5.5.5.1 result.100000.5000.threshold.0.5-0.5-0.5.5.5.1 result.1000000.5000.threshold.0.5-0.5-0.5.5.5.1 result.10000000.5000.threshold.0.5-0.5-0.5.5.5.1 result.100000000.5000.threshold.0.5-0.5-0.5.5.5.1 result.1000.5000.threshold.0.5-0.5-0.5.5.5.2 result.10000.5000.threshold.0.5-0.5-0.5.5.5.2 result.100000.5000.threshold.0.5-0.5-0.5.5.5.2 result.1000000.5000.threshold.0.5-0.5-0.5.5.5.2 result.10000000.5000.threshold.0.5-0.5-0.5.5.5.2 result.100000000.5000.threshold.0.5-0.5-0.5.5.5.2 result.1000.5000.threshold.0.5-0.5-0.5.5.5.3 result.10000.5000.threshold.0.5-0.5-0.5.5.5.3 result.100000.5000.threshold.0.5-0.5-0.5.5.5.3 result.1000000.5000.threshold.0.5-0.5-0.5.5.5.3 result.10000000.5000.threshold.0.5-0.5-0.5.5.5.3 result.100000000.5000.threshold.0.5-0.5-0.5.5.5.3
result.1000.5000.threshold.1-0-0.5.0.0:
	$(PROG) -m 1000 -q 5000 -t 2500 -p 1,0,0 -i 5 -w 0 -z 0 -f result.1000.5000.threshold.1-0-0.5.0.0

result.10000.5000.threshold.1-0-0.5.0.0:
	$(PROG) -m 10000 -q 5000 -t 2500 -p 1,0,0 -i 5 -w 0 -z 0 -f result.10000.5000.threshold.1-0-0.5.0.0

result.100000.5000.threshold.1-0-0.5.0.0:
	$(PROG) -m 100000 -q 5000 -t 2500 -p 1,0,0 -i 5 -w 0 -z 0 -f result.100000.5000.threshold.1-0-0.5.0.0

result.1000000.5000.threshold.1-0-0.5.0.0:
	$(PROG) -m 1000000 -q 5000 -t 2500 -p 1,0,0 -i 5 -w 0 -z 0 -f result.1000000.5000.threshold.1-0-0.5.0.0

result.1000.5000.threshold.1-0-0.5.5.0:
	$(PROG) -m 1000 -q 5000 -t 2500 -p 1,0,0 -i 5 -w 5 -z 0 -f result.1000.5000.threshold.1-0-0.5.5.0

result.10000.5000.threshold.1-0-0.5.5.0:
	$(PROG) -m 10000 -q 5000 -t 2500 -p 1,0,0 -i 5 -w 5 -z 0 -f result.10000.5000.threshold.1-0-0.5.5.0

result.100000.5000.threshold.1-0-0.5.5.0:
	$(PROG) -m 100000 -q 5000 -t 2500 -p 1,0,0 -i 5 -w 5 -z 0 -f result.100000.5000.threshold.1-0-0.5.5.0

result.1000000.5000.threshold.1-0-0.5.5.0:
	$(PROG) -m 1000000 -q 5000 -t 2500 -p 1,0,0 -i 5 -w 5 -z 0 -f result.1000000.5000.threshold.1-0-0.5.5.0

result.1000.5000.threshold.5-0-0.5.5.0:
	$(PROG) -m 1000 -q 5000 -t 2500 -p 5,0,0 -i 5 -w 5 -z 0 -f result.1000.5000.threshold.5-0-0.5.5.0

result.10000.5000.threshold.5-0-0.5.5.0:
	$(PROG) -m 10000 -q 5000 -t 2500 -p 5,0,0 -i 5 -w 5 -z 0 -f result.10000.5000.threshold.5-0-0.5.5.0

result.100000.5000.threshold.5-0-0.5.5.0:
	$(PROG) -m 100000 -q 5000 -t 2500 -p 5,0,0 -i 5 -w 5 -z 0 -f result.100000.5000.threshold.5-0-0.5.5.0

result.1000000.5000.threshold.5-0-0.5.5.0:
	$(PROG) -m 1000000 -q 5000 -t 2500 -p 5,0,0 -i 5 -w 5 -z 0 -f result.1000000.5000.threshold.5-0-0.5.5.0

result.10000000.5000.threshold.5-0-0.5.5.0:
	$(PROG) -m 10000000 -q 5000 -t 2500 -p 5,0,0 -i 5 -w 5 -z 0 -f result.10000000.5000.threshold.5-0-0.5.5.0

result.100000000.5000.threshold.5-0-0.5.5.0:
	$(PROG) -m 100000000 -q 5000 -t 2500 -p 5,0,0 -i 5 -w 5 -z 0 -f result.100000000.5000.threshold.5-0-0.5.5.0

result.1000.5000.threshold.0.5-0.5-0.5.5.5.0:
	$(PROG) -m 1000 -q 5000 -t 2500 -p 0.5,0.5,0.5 -i 5 -w 5 -z 0 -f result.1000.5000.threshold.0.5-0.5-0.5.5.5.0

result.10000.5000.threshold.0.5-0.5-0.5.5.5.0:
	$(PROG) -m 10000 -q 5000 -t 2500 -p 0.5,0.5,0.5 -i 5 -w 5 -z 0 -f result.10000.5000.threshold.0.5-0.5-0.5.5.5.0

result.100000.5000.threshold.0.5-0.5-0.5.5.5.0:
	$(PROG) -m 100000 -q 5000 -t 2500 -p 0.5,0.5,0.5 -i 5 -w 5 -z 0 -f result.100000.5000.threshold.0.5-0.5-0.5.5.5.0

result.1000000.5000.threshold.0.5-0.5-0.5.5.5.0:
	$(PROG) -m 1000000 -q 5000 -t 2500 -p 0.5,0.5,0.5 -i 5 -w 5 -z 0 -f result.1000000.5000.threshold.0.5-0.5-0.5.5.5.0

result.10000000.5000.threshold.0.5-0.5-0.5.5.5.0:
	$(PROG) -m 10000000 -q 5000 -t 2500 -p 0.5,0.5,0.5 -i 5 -w 5 -z 0 -f result.10000000.5000.threshold.0.5-0.5-0.5.5.5.0

result.100000000.5000.threshold.0.5-0.5-0.5.5.5.0:
	$(PROG) -m 100000000 -q 5000 -t 2500 -p 0.5,0.5,0.5 -i 5 -w 5 -z 0 -f result.100000000.5000.threshold.0.5-0.5-0.5.5.5.0

result.1000.5000.threshold.0.5-0.5-0.5.5.5.1:
	$(PROG) -m 1000 -q 5000 -t 2500 -p 0.5,0.5,0.5 -i 5 -w 5 -z 1 -f result.1000.5000.threshold.0.5-0.5-0.5.5.5.1

result.10000.5000.threshold.0.5-0.5-0.5.5.5.1:
	$(PROG) -m 10000 -q 5000 -t 2500 -p 0.5,0.5,0.5 -i 5 -w 5 -z 1 -f result.10000.5000.threshold.0.5-0.5-0.5.5.5.1

result.100000.5000.threshold.0.5-0.5-0.5.5.5.1:
	$(PROG) -m 100000 -q 5000 -t 2500 -p 0.5,0.5,0.5 -i 5 -w 5 -z 1 -f result.100000.5000.threshold.0.5-0.5-0.5.5.5.1

result.1000000.5000.threshold.0.5-0.5-0.5.5.5.1:
	$(PROG) -m 1000000 -q 5000 -t 2500 -p 0.5,0.5,0.5 -i 5 -w 5 -z 1 -f result.1000000.5000.threshold.0.5-0.5-0.5.5.5.1

result.10000000.5000.threshold.0.5-0.5-0.5.5.5.1:
	$(PROG) -m 10000000 -q 5000 -t 2500 -p 0.5,0.5,0.5 -i 5 -w 5 -z 1 -f result.10000000.5000.threshold.0.5-0.5-0.5.5.5.1

result.100000000.5000.threshold.0.5-0.5-0.5.5.5.1:
	$(PROG) -m 100000000 -q 5000 -t 2500 -p 0.5,0.5,0.5 -i 5 -w 5 -z 1 -f result.100000000.5000.threshold.0.5-0.5-0.5.5.5.1

result.1000.5000.threshold.0.5-0.5-0.5.5.5.2:
	$(PROG) -m 1000 -q 5000 -t 2500 -p 0.5,0.5,0.5 -i 5 -w 5 -z 2 -f result.1000.5000.threshold.0.5-0.5-0.5.5.5.2

result.10000.5000.threshold.0.5-0.5-0.5.5.5.2:
	$(PROG) -m 10000 -q 5000 -t 2500 -p 0.5,0.5,0.5 -i 5 -w 5 -z 2 -f result.10000.5000.threshold.0.5-0.5-0.5.5.5.2

result.100000.5000.threshold.0.5-0.5-0.5.5.5.2:
	$(PROG) -m 100000 -q 5000 -t 2500 -p 0.5,0.5,0.5 -i 5 -w 5 -z 2 -f result.100000.5000.threshold.0.5-0.5-0.5.5.5.2

result.1000000.5000.threshold.0.5-0.5-0.5.5.5.2:
	$(PROG) -m 1000000 -q 5000 -t 2500 -p 0.5,0.5,0.5 -i 5 -w 5 -z 2 -f result.1000000.5000.threshold.0.5-0.5-0.5.5.5.2

result.10000000.5000.threshold.0.5-0.5-0.5.5.5.2:
	$(PROG) -m 10000000 -q 5000 -t 2500 -p 0.5,0.5,0.5 -i 5 -w 5 -z 2 -f result.10000000.5000.threshold.0.5-0.5-0.5.5.5.2

result.100000000.5000.threshold.0.5-0.5-0.5.5.5.2:
	$(PROG) -m 100000000 -q 5000 -t 2500 -p 0.5,0.5,0.5 -i 5 -w 5 -z 2 -f result.100000000.5000.threshold.0.5-0.5-0.5.5.5.2

result.1000.5000.threshold.0.5-0.5-0.5.5.5.3:
	$(PROG) -m 1000 -q 5000 -t 2500 -p 0.5,0.5,0.5 -i 5 -w 5 -z 3 -f result.1000.5000.threshold.0.5-0.5-0.5.5.5.3

result.10000.5000.threshold.0.5-0.5-0.5.5.5.3:
	$(PROG) -m 10000 -q 5000 -t 2500 -p 0.5,0.5,0.5 -i 5 -w 5 -z 3 -f result.10000.5000.threshold.0.5-0.5-0.5.5.5.3

result.100000.5000.threshold.0.5-0.5-0.5.5.5.3:
	$(PROG) -m 100000 -q 5000 -t 2500 -p 0.5,0.5,0.5 -i 5 -w 5 -z 3 -f result.100000.5000.threshold.0.5-0.5-0.5.5.5.3

result.1000000.5000.threshold.0.5-0.5-0.5.5.5.3:
	$(PROG) -m 1000000 -q 5000 -t 2500 -p 0.5,0.5,0.5 -i 5 -w 5 -z 3 -f result.1000000.5000.threshold.0.5-0.5-0.5.5.5.3

result.10000000.5000.threshold.0.5-0.5-0.5.5.5.3:
	$(PROG) -m 10000000 -q 5000 -t 2500 -p 0.5,0.5,0.5 -i 5 -w 5 -z 3 -f result.10000000.5000.threshold.0.5-0.5-0.5.5.5.3

result.100000000.5000.threshold.0.5-0.5-0.5.5.5.3:
	$(PROG) -m 100000000 -q 5000 -t 2500 -p 0.5,0.5,0.5 -i 5 -w 5 -z 3 -f result.100000000.5000.threshold.0.5-0.5-0.5.5.5.3

