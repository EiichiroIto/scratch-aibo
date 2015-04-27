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

#ifndef MTN_h_DEFINED
#define MTN_h_DEFINED

#include <OPENR/ODataFormats.h>
#include "MTNFile.h"

class MTN {
public:
    MTN();
    ~MTN() {}

    void  Set(MTNFile* file);
    char* GetName();
    char* GetRobotDesign();
    MTNFile* GetMTNFile();

    void  First();
    bool  More();
    void  Next(int numFrames);
    int   InterpolateCommandVectorData(OCommandVectorData* commandVec,
                                       int maxNumFrames);

private:
    int InterpolateCommandData(int jointIndex,
                               OCommandData* data,
                               int valueIndex,
                               int keyFrame, int frame, int maxNumFrames);

    MTNFile* mtnfile;
    int      currentKeyFrame;
    int      currentFrame;
};

#endif // MTN_h_DEFINED
