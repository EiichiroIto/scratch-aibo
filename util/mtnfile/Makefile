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

PREFIX=/usr/local/OPEN_R_SDK
MTNINC=../../common/include
MTNSRC=../../common/libMTN
CXX=g++
CXXFLAGS=-I$(PREFIX)/OPEN_R/include -I$(MTNINC)
LIBS=

all: mtnfile

SRCS		= \
	main.cc \
	$(MTNSRC)/MTNFile.cc \
	$(MTNSRC)/MTNFile_Print.cc \

mtnfile: $(SRCS)
	$(CXX) $(CXXFLAGS) -o $@ $(SRCS) $(LIBS)

clean:
	rm -f mtnfile.exe mtnfile *.o
