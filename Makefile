CPP = g++
CPPFLAGS  = -std=c++11

all: 
	$(CPP) $(CPPFLAGS)  -o mainserver servermain.cpp
	$(CPP) $(CPPFLAGS)  -o serverA serverA.cpp
	$(CPP) $(CPPFLAGS)  -o serverB serverB.cpp

serverA:
	./serverA

serverB:
	./serverB

mainserver:
	./mainserver