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
LIBS=-lObjectComm -lOPENR
CXXFLAGS= \
	-O2 \
	-g \
	-I. \
	-I$(OPENRSDK_ROOT)/OPEN_R/include/R4000 \
	-I$(OPENRSDK_ROOT)/OPEN_R/include \
	-I../common/include \
	-D$(AIBO_MODEL)

.PHONY: all install clean

all: vision.bin

%.o: %.cc
	$(CXX) $(CXXFLAGS) -o $@ -c $^

VisionStub.cc: stub.cfg
	$(STUBGEN) stub.cfg	

vision.bin: VisionStub.o Vision.o Vision_Recognition.o Vision_Head.o \
	Vision_Param.o CdtVision.o Headings.o Vision.ocf
	$(MKBIN) $(MKBINFLAGS) -o $@ $^ $(LIBS)
	$(STRIP) $@

install: vision.bin
	gzip -c vision.bin > $(INSTALLDIR)/OPEN-R/MW/OBJS/VISION.BIN

clean:
	rm -f *.o *.bin *.elf *.snap.cc *~
	rm -f VisionStub.h VisionStub.cc def.h entry.h
	rm -f $(INSTALLDIR)/OPEN-R/MW/OBJS/VISION.BIN

CdtVision.exe: CdtVision.cc
	gcc -DMAIN CdtVision.cc -o CdtVision.exe -lstdc++

Headings: Headings.cc Headings.h
	cc -DMAIN -I../common/include Headings.cc -o Headings -lstdc++
