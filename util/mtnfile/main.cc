//
// Copyright 2002 Sony Corporation 
//
// Permission to use, copy, modify, and redistribute this software for
// non-commercial use is hereby granted.
//
// This software is provided "as is" without warranty of any kind,
// either expressed or implied, including but not limited to the
// implied warranties of fitness for a particular purpose.
//

#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <MTNFile.h>

main(int argc, char** argv)
{
    if (argc != 2) {
        fprintf(stderr, "Usage: mtnfile file.mtn\n");
        exit(1);
    }

    struct stat st;
    if (stat(argv[1], &st) != 0) {
        perror("stat");
        exit(1);
    }
    
    MTNFile* mtnfile = (MTNFile*)malloc(st.st_size);
    if (mtnfile == 0) {
        fprintf(stderr, "malloc() failed.\n");
        exit(1);
    }
    
    FILE* fp = fopen(argv[1], "rb");
    if (fp == 0) {
        fprintf(stderr, "can't open %s\n", argv[1]);
        exit(1);
    }
    
    if (fread((void*)mtnfile, 1, st.st_size, fp) != st.st_size) {
        fprintf(stderr, "fread() failed.\n");
        fclose(fp);
        exit(1);
    }

    fclose(fp);

    mtnfile->Print();
}
