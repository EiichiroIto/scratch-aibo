#
# Copyright 2003 Sony Corporation 
#
# Permission to use, copy, modify, and redistribute this software for
# non-commercial use is hereby granted.
#
# This software is provided "as is" without warranty of any kind,
# either expressed or implied, including but not limited to the
# implied warranties of fitness for a particular purpose.
#

OPENRSDK_ROOT?=/usr/local/OPEN_R_SDK
INSTALLDIR=../MS
CXX=$(OPENRSDK_ROOT)/bin/mipsel-linux-g++
CC=$(OPENRSDK_ROOT)/bin/mipsel-linux-gcc
STRIP=$(OPENRSDK_ROOT)/bin/mipsel-linux-strip
MKBIN=$(OPENRSDK_ROOT)/OPEN_R/bin/mkbin
STUBGEN=$(OPENRSDK_ROOT)/OPEN_R/bin/stubgen2
MKBINFLAGS=-p $(OPENRSDK_ROOT)
LIBS=-L$(OPENRSDK_ROOT)/OPEN_R/lib -lObjectComm -lOPENR -lInternet -lantMCOOP
LIBJPEGDIR=../../sample/common/W3AIBO/jpeg-6b
LIBJPEG=$(LIBJPEGDIR)/libjpeg.a
CXXFLAGS= \
	-O2 \
	-g \
	-I. \
	-I$(OPENRSDK_ROOT)/OPEN_R/include/R4000 \
	-I$(OPENRSDK_ROOT)/OPEN_R/include/MCOOP \
	-I$(OPENRSDK_ROOT)/OPEN_R/include \
	-I$(LIBJPEGDIR)

#
# JPEGEncoder::ReconstructAndConvertYCbCr()
#
#CXXFLAGS+= -DRECONSTRUCT_POINTER_VERSION

#
# When OPENR_DEBUG is defined, OSYSDEBUG() is available.
#
#CXXFLAGS+= -DOPENR_DEBUG

.PHONY: all install clean

all: $(LIBJPEGDIR) w3aibo.bin

%.o: %.cc
	$(CXX) $(CXXFLAGS) -o $@ -c $^

%.o: %.c
	$(CC) $(CXXFLAGS) -o $@ -c $^

W3AIBOStub.cc: stub.cfg
	$(STUBGEN) stub.cfg	

$(LIBJPEGDIR):
	(cd ..; tar zxf jpegsrc.v6b.tar.gz)
	cp ../makefile.aibo $(LIBJPEGDIR)/Makefile
	cp $(LIBJPEGDIR)/jconfig.doc $(LIBJPEGDIR)/jconfig.h

$(LIBJPEG): $(LIBJPEGDIR)
	make -C $(LIBJPEGDIR)

w3aibo.bin: W3AIBOStub.o W3AIBO.o HTTP.o JPEGEncoder.o write_jpeg.o jpeg_mem_dest.o $(LIBJPEG) w3aibo.ocf 
	$(MKBIN) $(MKBINFLAGS) -o $@ $^ $(LIBS)
	$(STRIP) $@

install: all
	gzip -c w3aibo.bin > $(INSTALLDIR)/OPEN-R/MW/OBJS/W3AIBO.BIN

clean:
	make -C $(LIBJPEGDIR) clean
	rm -f *.o *.bin *.elf *.snap.cc *~
	rm -f W3AIBOStub.h W3AIBOStub.cc def.h entry.h
	rm -f $(INSTALLDIR)/OPEN-R/MW/OBJS/W3AIBO.BIN
