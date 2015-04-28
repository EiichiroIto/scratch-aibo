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

#include <OPENR/OSyslog.h>
#include <OPENR/OFbkImage.h>
#include "JPEGEncoder.h"
#include "write_jpeg.h"

JPEGEncoder::JPEGEncoder() : ipstackRef(),
                             pixelInterleavedYCbCr(0),
                             freeJpegBufList()
{
}

bool
JPEGEncoder::Init(const antStackRef& ipstack)
{
    ipstackRef = ipstack;

    pixelInterleavedYCbCr = (byte*)malloc(IMAGE_BUFSIZE);
    if (pixelInterleavedYCbCr == 0) {
        OSYSLOG1((osyslogERROR, "JPEGEncoder::Init() : NO MEMORY"));
        return false;
    }

    for (int i = 0; i < NUM_JPEG_BUF; i++) {

        antEnvCreateSharedBufferMsg jpegBufferMsg(JPEG_BUFSIZE);
        
        jpegBufferMsg.Call(ipstackRef, sizeof(jpegBufferMsg));
        if (jpegBufferMsg.error != ANT_SUCCESS) {
            OSYSLOG1((osyslogERROR, "%s : NO MEMORY antError %d",
                      "JPEGEncoder::Init()", jpegBufferMsg.error));
            return false;
        }

        jpegBuf[i] = jpegBufferMsg.buffer;
        jpegBuf[i].Map();
        freeJpegBufList.push_back((byte*)jpegBuf[i].GetAddress());
    }

    return true;
}

void
JPEGEncoder::FreeJPEG(byte* jpeg)
{
    Free(jpeg);
}

bool
JPEGEncoder::GetJPEG(OFbkImageVectorData* imageVec,
                     OFbkImageLayer layer, bool reconstruction,
                     int quality, byte** jpeg, int* size)
{
    int w, h;
    if (reconstruction == true) {
        ReconstructAndConvertYCbCr(imageVec, layer,
                                   pixelInterleavedYCbCr, &w, &h);
    } else {
        ConvertYCbCr(imageVec, layer, pixelInterleavedYCbCr, &w, &h);
    }

    byte* jbuf = Allocate();
    if (jbuf == 0) return false;

    int jsize = write_jpeg_mem(pixelInterleavedYCbCr,
                               w, h, quality, jbuf, JPEG_BUFSIZE);
    *jpeg = jbuf;
    *size = jsize;

    return true;
}

void
JPEGEncoder::Save(OFbkImageVectorData* imageVec,
                  OFbkImageLayer layer, int quality, char* path)
{
    int w, h;
    ConvertYCbCr(imageVec, layer, pixelInterleavedYCbCr, &w, &h);

    FILE* fp = fopen(path, "w");
    if (fp == NULL) {
        OSYSLOG1((osyslogERROR, "JPEGEncoder::Save() : can't open %s", path));
        return;
    }
    write_jpeg_file(pixelInterleavedYCbCr, w, h, quality, fp);
    fclose(fp);
}

byte*
JPEGEncoder::Allocate()
{
    if (freeJpegBufList.size() == 0) return 0;

    byte* buf = freeJpegBufList.front();
    freeJpegBufList.pop_front();
    return buf;
}

void
JPEGEncoder::Free(byte* buf)
{
    freeJpegBufList.push_back(buf);
}

void
JPEGEncoder::ConvertYCbCr(OFbkImageVectorData* imageVec,
                          OFbkImageLayer layer, byte* image,
                          int* width, int* height)
{
    OFbkImageInfo* info = imageVec->GetInfo(layer);
    byte*          data = imageVec->GetData(layer);

    OFbkImage yImg(info, data, ofbkimageBAND_Y);
    OFbkImage crImg(info, data, ofbkimageBAND_Cr);
    OFbkImage cbImg(info, data, ofbkimageBAND_Cb);

    int w = yImg.Width();
    int h = yImg.Height();
    int skip = yImg.Skip();

    byte* iptr = image;
    
    for (int y = 0; y < h; y++) {

        byte* yline  = yImg.Pointer()  + (w + skip) * y;
        byte* crline = crImg.Pointer() + (w + skip) * y;
        byte* cbline = cbImg.Pointer() + (w + skip) * y;

        for (int x = 0; x < w; x++) {
            *iptr++ = yline[x];     // Y
            *iptr++ = cbline[x];    // Cb
            *iptr++ = crline[x];    // Cr
        }

    }

    *width  = w;
    *height = h;
}

#ifndef RECONSTRUCT_POINTER_VERSION
void
JPEGEncoder::ReconstructAndConvertYCbCr(OFbkImageVectorData* imageVec,
                                        OFbkImageLayer layer, byte* image,
                                        int* width, int* height)
{
    OFbkImageInfo* info = imageVec->GetInfo(layer);
    byte*          data = imageVec->GetData(layer);

    OFbkImage yLLImg(info, data, ofbkimageBAND_Y); // Y_LL
    OFbkImage yLHImg(info, data, ofbkimageBAND_Y_LH);
    OFbkImage yHLImg(info, data, ofbkimageBAND_Y_HL);
    OFbkImage yHHImg(info, data, ofbkimageBAND_Y_HH);

    OFbkImage crImg(info, data, ofbkimageBAND_Cr);
    OFbkImage cbImg(info, data, ofbkimageBAND_Cb);

    int w = yLLImg.Width();
    int h = yLLImg.Height();

    for (int y = 0; y < h; y++) {

        for (int x = 0; x < w; x++) {
            //
            // yLH, yHL, yHH : offset binary [0, 255] -> signed int [-128, 127]
            //
            int yLL = (int)yLLImg.Pixel(x, y);
            int yLH = (int)yLHImg.Pixel(x, y) - 128;
            int yHL = (int)yHLImg.Pixel(x, y) - 128;
            int yHH = (int)yHHImg.Pixel(x, y) - 128;

            int a = yLL + yLH + yHL + yHH; // ypix11
            int b = 2 * (yLL + yLH);       // ypix11 + ypix01
            int c = 2 * (yLL + yHL);       // ypix11 + ypix10
            int d = 2 * (yLL + yHH);       // ypix11 + ypix00
            
            byte ypix00 = ClipRange(d - a);
            byte ypix10 = ClipRange(c - a);
            byte ypix01 = ClipRange(b - a);
            byte ypix11 = ClipRange(a);
            
            byte cb = cbImg.Pixel(x, y);
            byte cr = crImg.Pixel(x, y);
            
            PutYCbCrPixel(image, 2*w, 2*x,   2*y,   ypix00, cb, cr);
            PutYCbCrPixel(image, 2*w, 2*x+1, 2*y,   ypix10, cb, cr);
            PutYCbCrPixel(image, 2*w, 2*x,   2*y+1, ypix01, cb, cr);
            PutYCbCrPixel(image, 2*w, 2*x+1, 2*y+1, ypix11, cb, cr);
        }

    }

    *width  = 2 * w;
    *height = 2 * h;
}
#else
void
JPEGEncoder::ReconstructAndConvertYCbCr(OFbkImageVectorData* imageVec,
                                        OFbkImageLayer layer, byte* image,
                                        int* width, int* height)
{
    OFbkImageInfo* info = imageVec->GetInfo(layer);
    byte*          data = imageVec->GetData(layer);

    OFbkImage yLLImg(info, data, ofbkimageBAND_Y); // Y_LL
    OFbkImage yLHImg(info, data, ofbkimageBAND_Y_LH);
    OFbkImage yHLImg(info, data, ofbkimageBAND_Y_HL);
    OFbkImage yHHImg(info, data, ofbkimageBAND_Y_HH);

    OFbkImage crImg(info, data, ofbkimageBAND_Cr);
    OFbkImage cbImg(info, data, ofbkimageBAND_Cb);

    byte* yLLPtr = yLLImg.Pointer();
    byte* yLHPtr = yLHImg.Pointer();
    byte* yHLPtr = yHLImg.Pointer();
    byte* yHHPtr = yHHImg.Pointer();

    byte* crPtr = crImg.Pointer();
    byte* cbPtr = cbImg.Pointer();

    int w    = yLLImg.Width();
    int h    = yLLImg.Height();
    int skip = yLLImg.Skip();
    OSYSDEBUG(("w %d h %d skip %d\n", w, h, skip));

    byte* iptr0 = image;
    byte* iptr1 = image + 3 * (2 * w);
    
    for (int y = 0; y < h; y++) {

        for (int x = 0; x < w; x++) {
            //
            // yLH, yHL, yHH : offset binary [0, 255] -> signed int [-128, 127]
            //
            int yLL = (int)*yLLPtr++;
            int yLH = (int)*yLHPtr++ - 128;
            int yHL = (int)*yHLPtr++ - 128;
            int yHH = (int)*yHHPtr++ - 128;

            int a = yLL + yLH + yHL + yHH; // ypix11
            int b = 2 * (yLL + yLH);       // ypix11 + ypix01
            int c = 2 * (yLL + yHL);       // ypix11 + ypix10
            int d = 2 * (yLL + yHH);       // ypix11 + ypix00
            
            byte ypix00 = ClipRange(d - a);
            byte ypix10 = ClipRange(c - a);
            byte ypix01 = ClipRange(b - a);
            byte ypix11 = ClipRange(a);
            
            byte cb = *cbPtr++;
            byte cr = *crPtr++;
            
            iptr0[0] = ypix00;
            iptr0[1] = cb;
            iptr0[2] = cr;

            iptr0[3] = ypix10;
            iptr0[4] = cb;
            iptr0[5] = cr;

            iptr1[0] = ypix01;
            iptr1[1] = cb;
            iptr1[2] = cr;

            iptr1[3] = ypix11;
            iptr1[4] = cb;
            iptr1[5] = cr;

            iptr0 += 6;
            iptr1 += 6;
        }

        yLLPtr += skip;
        yLHPtr += skip;
        yHLPtr += skip;
        yHHPtr += skip;

        cbPtr  += skip;
        crPtr  += skip;

        iptr0  += 3 * (2 * w);
        iptr1  += 3 * (2 * w);
    }

    *width  = 2 * w;
    *height = 2 * h;
}
#endif // RECONSTRUCT_POINTER_VERSION
