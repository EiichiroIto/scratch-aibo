//
// Copyright 2003 Sony Corporation 
//
// Permission to use, copy, modify, and redistribute this software for
// non-commercial use is hereby granted.
//
// This software is provided "as is" without warranty of any kind,
// either expressed or implied, including but not limited to the
// implied warranties of fitness for a particular purpose.
//

#ifndef write_jpeg_h_DEFINED
#define write_jpeg_h_DEFINED

#ifdef __cplusplus
extern "C" {
#endif
    int write_jpeg_mem(unsigned char*, int, int, int, unsigned char*, int);
    void write_jpeg_file(unsigned char*, int, int, int, FILE*);
#ifdef __cplusplus
}
#endif

#endif /* write_jpeg_h_DEFINED */
