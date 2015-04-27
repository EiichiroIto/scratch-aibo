#
# Copyright 2002,2003 Sony Corporation 
#
# Permission to use, copy, modify, and redistribute this software for
# non-commercial use is hereby granted.
#
# This software is provided "as is" without warranty of any kind,
# either expressed or implied, including but not limited to the
# implied warranties of fitness for a particular purpose.
#

LANG=
LANGUAGE=
OPENRSDK_ROOT?=/usr/local/OPEN_R_SDK
AIBO_MODEL?=ERS
INSTALLDIR=../MS
CXX=$(OPENRSDK_ROOT)/bin/mipsel-linux-g++
STRIP=$(OPENRSDK_ROOT)/bin/mipsel-linux-strip
MKBIN=$(OPENRSDK_ROOT)/OPEN_R/bin/mkbin
STUBGEN=$(OPENRSDK_ROOT)/OPEN_R/bin/stubgen2
MKBINFLAGS=-p $(OPENRSDK_ROOT)
LIBS=-L$(OPENRSDK_ROOT)/OPEN_R/lib -lObjectComm -lOPENR -lInternet -lantMCOOP
CXXFLAGS= \
	-O2 \
	-g \
	-I. \
	-I$(OPENRSDK_ROOT)/OPEN_R/include/R4000 \
	-I$(OPENRSDK_ROOT)/OPEN_R/include/MCOOP \
	-I$(OPENRSDK_ROOT)/OPEN_R/include \
	-I../common/include \
	-D$(AIBO_MODEL)

#
# When OPENR_DEBUG is defined, OSYSDEBUG() is available.
#
#CXXFLAGS+= -DOPENR_DEBUG

.PHONY: all install clean

all: tcpReceptor.bin

%.o: %.cc
	$(CXX) $(CXXFLAGS) -o $@ -c $^

TCPReceptorStub.cc: stub.cfg
	$(STUBGEN) stub.cfg	

tcpReceptor.bin: TCPReceptorStub.o tcpReceptor.ocf TCPReceptor.o TCPReceptor_app.o parse.o
	$(MKBIN) $(MKBINFLAGS) -o $@ $^ $(LIBS)
	$(STRIP) $@

install: tcpReceptor.bin
	gzip -c tcpReceptor.bin > $(INSTALLDIR)/OPEN-R/MW/OBJS/TCPRCPTR.BIN

clean:
	rm -f *.o *.bin *.elf *.snap.cc *~
	rm -f TCPReceptorStub.h TCPReceptorStub.cc def.h entry.h
	rm -f $(INSTALLDIR)/OPEN-R/MW/OBJS/TCPRCPTR.BIN
