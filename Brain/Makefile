#
# Copyright 2005 (C) Eiichiro ITO, GHC02331@nifty.com
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this library; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
#
# Eiichiro ITO, 15 October 2005
# mailto: GHC02331@nifty.com
#

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

all: brain.bin

%.o: %.cc
	$(CXX) $(CXXFLAGS) -o $@ -c $^

BrainStub.cc: stub.cfg
	$(STUBGEN) stub.cfg	

brain.bin: BrainStub.o brain.ocf Brain.o \
	Perception.o Sensor.o CdtFile.o \
	STN.o STNHolder.o Memory.o Logging.o
	$(MKBIN) $(MKBINFLAGS) -o $@ $^ $(LIBS)
	$(STRIP) $@

install: brain.bin
	gzip -c brain.bin > $(INSTALLDIR)/OPEN-R/MW/OBJS/BRAIN.BIN

clean:
	rm -f *.o *.bin *.elf *.snap.cc *~
	rm -f BrainStub.h BrainStub.cc def.h entry.h
	rm -f $(INSTALLDIR)/OPEN-R/MW/OBJS/BRAIN.BIN


STN.exe: STN.cc
	gcc -DMAIN STN.cc -o STN.exe -lstdc++

CdtFile.exe: CdtFile.cc
	gcc -DMAIN CdtFile.cc -o CdtFile.exe -lstdc++
