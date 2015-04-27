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
AIBO_MODEL?=ERS
PREFIX=$(OPENRSDK_ROOT)
INSTALLDIR=../MS
CXX=$(PREFIX)/bin/mipsel-linux-g++
STRIP=$(PREFIX)/bin/mipsel-linux-strip
MKBIN=$(PREFIX)/OPEN_R/bin/mkbin
STUBGEN=$(PREFIX)/OPEN_R/bin/stubgen2
MKBINFLAGS=-p $(PREFIX)
LIBS=-L$(PREFIX)/OPEN_R/lib -lObjectComm -lOPENR
LIBODADIR=../common/libODA
LIBODA=$(LIBODADIR)/libODA.a
LIBMTNDIR=../common/libMTN
LIBMTN=$(LIBMTNDIR)/libMTN.a
CXXFLAGS= \
	-O2 \
	-g \
	-I. \
	-I$(PREFIX)/OPEN_R/include/R4000 \
	-I$(PREFIX)/OPEN_R/include \
	-I../common/include \
	-D$(AIBO_MODEL)

#
# When OPENR_DEBUG is defined, OSYSDEBUG() is available.
#
#CXXFLAGS+= -DOPENR_DEBUG

.PHONY: all install clean

all: motionAgents.bin 

%.o: %.cc
	$(CXX) $(CXXFLAGS) -o $@ -c $^

MotionAgentsStub.cc: stub.cfg
	$(STUBGEN) stub.cfg	

$(LIBODA):
	(cd $(LIBODADIR); make)	

$(LIBMTN):
	(cd $(LIBMTNDIR); make)	

motionAgents.bin: MotionAgentsStub.o MotionAgents.o MoNetAgentManager.o MoNetAgent.o NeutralAgent.o MTNAgent.o AnyAgent.o $(LIBODA) $(LIBMTN) motionAgents.ocf
	$(MKBIN) $(MKBINFLAGS) -o $@ $^ $(LIBS)
	$(STRIP) $@

install: motionAgents.bin
	gzip -c motionAgents.bin > $(INSTALLDIR)/OPEN-R/MW/OBJS/MTNAGTS.BIN

clean:
	(cd $(LIBODADIR); make clean)
	(cd $(LIBMTNDIR); make clean)
	rm -f *.o *.bin *.elf *.snap.cc *~
	rm -f MotionAgentsStub.h MotionAgentsStub.cc def.h entry.h
	rm -f $(INSTALLDIR)/OPEN-R/MW/OBJS/MTNAGTS.BIN

