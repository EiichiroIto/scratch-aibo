#
# Copyright 2002 Sony Corporation 
#
# Permission to use, copy, modify, and redistribute this software for
# non-commercial use is hereby granted.
#
# This software is provided "as is" without warranty of any kind,
# either expressed or implied, including but not limited to the
# implied warranties of fitness for a particular purpose.
#

CXX=g++
CXXFLAGS=-I../../MoNet
LIBS=

all: graph_test

%.o: %.c
	$(CXX) $(CXXFLAGS) -o $@ -c $^

graph_test: graph_test.o
	$(CXX) -o $@ graph_test.o $(LIBS)

clean:
	rm -f graph_test.exe graph_test *.o
