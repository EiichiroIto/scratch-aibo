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

#ifndef JPEGEncoder_h_DEFINED
#define JPEGEncoder_h_DEFINED

#include <list>
using std::list;
#include <OPENR/ODataFormats.h>
#include <ant.h>

class JPEGEncoder {
public:
    JPEGEncoder();
    ~JPEGEncoder() {}

    bool Init(const antStackRef& ipstack);
    bool GetJPEG(OFbkImageVectorData* imageVec,
                 OFbkImageLayer layer, bool reconstruction,
                 int quality, byte** jpeg, int* size);
    void FreeJPEG(byte* jpeg);
    
    // for libjpeg test
    void Save(OFbkImageVectorData* imageVec,
              OFbkImageLayer layer, int quality, char* path);

    static const int NUM_JPEG_BUF  = 4;

private:
    //
    // [ERS-210/220]
    // bytes/pixel : 3
    // width       : 176 x 2
    // height      : 144 x 2
    //
    // [ERS-7]
    // bytes/pixel : 3
    // width       : 208 x 2
    // height      : 160 x 2
    //
    static const int IMAGE_BUFSIZE = 3*(2*208)*(2*160);
    static const int JPEG_BUFSIZE  = 64*1024;
    byte* Allocate();
    void  Free(byte* buf);

    void ConvertYCbCr(OFbkImageVectorData* imageVec,
                      OFbkImageLayer layer, byte* image,
                      int* width, int* height);
    void ReconstructAndConvertYCbCr(OFbkImageVectorData* imageVec,
                                    OFbkImageLayer layer, byte* image,
                                    int* width, int* height);
    
    void PutYCbCrPixel(byte* img, int w,
                       int x, int y, byte ypix, byte cb, byte cr) {
        byte* ptr = img + 3 * (w * y + x);
        ptr[0] = ypix;
        ptr[1] = cb;
        ptr[2] = cr;
    }

    byte ClipRange(int val) {
        if (val < 0)        { return 0;         }
        else if (val > 255) { return 255;       }
        else                { return (byte)val; }
    }

    antStackRef     ipstackRef;
    byte*           pixelInterleavedYCbCr;
    antSharedBuffer jpegBuf[NUM_JPEG_BUF];
    list<byte*>     freeJpegBufList;
};

#endif // JPEGEncoder_h_DEFINED
