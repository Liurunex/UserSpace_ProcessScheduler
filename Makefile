CC = gcc
CFLAGS = -W -Wall
#CFLAGS= -W -Wall -g
LDFLAGS=
#LDFLAGS= -g
PROGS = 1 2 3 4 5

all: $(PROGS)

1: uspsv1.o p1fxns.o
	cc $(LDFLAGS) -o 1 uspsv1.o p1fxns.o
2: uspsv2.o p1fxns.o
	cc $(LDFLAGS) -o 2 uspsv2.o p1fxns.o
3: uspsv3.o p1fxns.o
	cc $(LDFLAGS) -o 3 uspsv3.o p1fxns.o
4: uspsv4.o p1fxns.o
	cc $(LDFLAGS) -o 4 uspsv4.o p1fxns.o
5: uspsv5.o p1fxns.o
	cc $(LDFLAGS) -o 5 uspsv5.o p1fxns.o
	
uspsv1.o: uspsv1.c p1fxns.h
uspsv2.o: uspsv2.c p1fxns.h
uspsv3.o: uspsv3.c p1fxns.h
uspsv4.o: uspsv4.c p1fxns.h
uspsv5.o: uspsv5.c p1fxns.h
p1fxns.o: p1fxns.c p1fxns.h

clean:
	rm -f *~ *.o $(PROGS)