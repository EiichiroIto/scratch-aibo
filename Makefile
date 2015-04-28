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

LANG=
LANGUAGE=
COMPONENTS=Brain MoNet MotionAgents Vision Face PowerMonitor SensorServer W3AIBO
INSTALLDIR=$(shell pwd)/MS
TARGETS=all install clean
RELEASEDIR=/defart/release

.PHONY: $(TARGETS)

$(TARGETS):
	for dir in $(COMPONENTS); do \
		(cd $$dir && $(MAKE) INSTALLDIR=$(INSTALLDIR) $@) \
	done

copy: version
	-mount $(MSTICK)
	-mv $(MSTICK)/open-r/emon.log ..
	-cat $(MSTICK)/open-r/VERSION.txt
#	rm -f $(MSTICK)/open-r/emon.log
	rm -f $(MSTICK)/open-r/mw/objs/*.BIN
	rm -f $(MSTICK)/open-r/mw/objs/*.bin
	cp -r MS/OPEN-R $(MSTICK)/
	sync
	umount $(MSTICK)
#	-df | lgrep $(MSTICK)

release: version
	rm -f $(RELEASEDIR)/$(AIBO_MODEL)/open-r/mw/objs/*.BIN
	rm -f $(RELEASEDIR)/$(AIBO_MODEL)/open-r/mw/objs/*.bin
	cp -r MS/OPEN-R $(RELEASEDIR)/$(AIBO_MODEL)
	sync

trash:
	find . -name '*~' -exec rm \{\} \;

version:
	pwd | sed -n 's/.*\/DefartAibo\(.*\)/DefartAIBO\1-$(AIBO_MODEL)/p' > MS/OPEN-R/defart-ver.txt

binary: version trash
	(cd MS;	zip -r ../../$(shell cat MS/OPEN-R/defart-ver).zip OPEN-R; cd ..)
