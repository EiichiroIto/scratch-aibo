/*
 * Copyright 2002,2003 Sony Corporation 
 *
 * Permission to use, copy, modify, and redistribute this software for
 * non-commercial use is hereby granted.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 */

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#define PORT     54321
#define BUFSIZE  512

main(int argc, char **argv)
{
    struct sockaddr_in  addr;
    struct hostent      *hp;
    int    fd;
    int    len;
    int    sizeof_addr;
    char   buf[BUFSIZE*2];
    int    ret;

    if (argc != 2){
        printf("Usage: udp_echo_client hostname\n");
        exit(1);
    }

    /*
     * Get a datagram socket (udp)
     */
    if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket");
        exit(1);
    }

    if ((hp = gethostbyname(argv[1])) == NULL) {
        perror("gethostbyname");
        exit(1);
    }

    memset((void*)&addr, 0, sizeof(addr));
    memcpy(&addr.sin_addr, hp->h_addr, hp->h_length);

    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);

    printf("Enter message to AIBO>");
    while (fgets(buf, BUFSIZE, stdin) != NULL) {
        len = strlen(buf) + 1;
        
        /*
         * Send a datagram to the server
         */
        if (sendto(fd, buf, len, 0, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
            perror("sendto");
            close(fd);
            exit(1);
        }

        /*
         * Receive a datagram back
         */
        sizeof_addr = sizeof(addr);
        ret = recvfrom(fd, buf, sizeof(buf), 0 , (struct sockaddr *)&addr, &sizeof_addr);
        if (ret < 0) {
            perror("recvfrom");
            close(fd);
            exit(1);
        }

        buf[ret] = '\0';
        printf("Receive from AIBO : %s", buf);
        printf("Enter message to AIBO>");
    }

    close(fd);
    exit(0);
}
