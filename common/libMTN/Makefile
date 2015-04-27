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

OPENRSDK_ROOT?=/usr/local/OPEN_R_SDK
PREFIX=$(OPENRSDK_ROOT)
CXX=$(PREFIX)/bin/mipsel-linux-g++
AR=$(PREFIX)/bin/mipsel-linux-ar
RANLIB=$(PREFIX)/bin/mipsel-linux-ranlib
CXXFLAGS= \
	-O2 \
	-g \
	-I. \
	-I$(PREFIX)/OPEN_R/include/R4000 \
	-I$(PREFIX)/OPEN_R/include \
	-I../include

#
# When OPENR_DEBUG is defined, OSYSDEBUG() is available.
#
#CXXFLAGS+= -DOPENR_DEBUG

OBJS		= \
	MTN.o \
	MTNFile.o \
	MTNFile_Print.o \

TARGET=libMTN.a

.PHONY: all clean

all: $(TARGET)

%.o: %.cc
	$(CXX) $(CXXFLAGS) -o $@ -c $^

$(TARGET): $(OBJS)
	rm -f $@
	$(AR) r $@ $(OBJS)
	$(RANLIB) $@

clean:
	rm -f $(TARGET) *.o 
