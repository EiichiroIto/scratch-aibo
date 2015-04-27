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

CC=gcc
CFLAGS=
LIBS=

all: udp_echo_client

%.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $^

udp_echo_client: udp_echo_client.o
	$(CC) -o $@ udp_echo_client.o $(LIBS)

clean:
	rm -f udp_echo_client.exe udp_echo_client *.o
