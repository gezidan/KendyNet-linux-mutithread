all:
	gcc -g -o test test.c network.a -I./include -lpthread
timer:
	gcc -g -o test testtimer.c timer.a -I./include -lrt
thread:
	gcc -g -o test testthread.c network.a -I./include -lpthread
wheel:
	gcc -g -o test testwheel.c timer.a -I./include -lrt