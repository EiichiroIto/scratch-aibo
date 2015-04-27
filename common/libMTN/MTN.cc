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

#include <string.h>
#include <OPENR/OSyslog.h>
#include "MTN.h"

MTN::MTN() : mtnfile(0), currentKeyFrame(0), currentFrame(0)
{
}

void
MTN::Set(MTNFile* file)
{
    mtnfile = file;
    First();
}

char*
MTN::GetName()
{
    static char name[128]; // magic number !! Improve later

    char* ptr = mtnfile->GetName();
    if (ptr == 0) {
        name[0] = '\0';
    } else {
        strcpy(name, ptr);
    }

    return name;
}

char*
MTN::GetRobotDesign()
{
    static char robotDesign[32]; // magic number !! Improve later

    char* ptr = mtnfile->GetRobotDesign();
    if (ptr == 0) {
        robotDesign[0] = '\0';
    } else {
        strcpy(robotDesign, ptr);
    }

    return robotDesign;
}

MTNFile*
MTN::GetMTNFile()
{
    return mtnfile;
}

void
MTN::First()
{
    currentKeyFrame = 0;
    currentFrame    = 0;
}

bool
MTN::More()
{
    if (mtnfile == 0) return false;
    return (currentKeyFrame >= mtnfile->GetNumKeyFrames() - 1) ? false : true;
}

void
MTN::Next(int numFrames)
{
    if (mtnfile == 0) return; // do nothing

    currentFrame += numFrames;
    int interpolate = mtnfile->GetNumInterpolate8ms(currentKeyFrame);

    while (currentFrame > interpolate) {
        currentFrame = currentFrame - interpolate - 1;
        currentKeyFrame++;
        interpolate = mtnfile->GetNumInterpolate8ms(currentKeyFrame);
        if (interpolate == -1) {
            currentFrame = 0;
            break;
        }
    }
    
    OSYSDEBUG(("MTN::Next() %d %d %d\n",
               numFrames, currentKeyFrame, currentFrame));
}

int
MTN::InterpolateCommandVectorData(OCommandVectorData* commandVec,
                                  int maxNumFrames)
{
    OSYSDEBUG(("MTN::InterpolateCommandVectorData() %d %d %d\n",
               currentKeyFrame, currentFrame, 
               mtnfile->GetNumInterpolate8ms(currentKeyFrame)));

    if (mtnfile == 0) return -1;
    if (More() != true) return -1;

    int numFrames = 0;
    for (int i = 0; i < mtnfile->GetNumJoints(); i++) {
        OCommandInfo* info = commandVec->GetInfo(i);
        OCommandData* data = commandVec->GetData(i);
        numFrames = InterpolateCommandData(i, data, 0,
                                           currentKeyFrame,
                                           currentFrame, maxNumFrames);
        info->Set(odataJOINT_COMMAND2, info->primitiveID, numFrames);
    }

    return numFrames;
}

int
MTN::InterpolateCommandData(int jointIndex,
                            OCommandData* data, int valueIndex,
                            int keyFrame, int frame, int maxNumFrames)
{
    OSYSDEBUG(("MTN::InterpolateCommandData() %d %d %d %d %d\n",
               jointIndex, valueIndex, keyFrame, frame, maxNumFrames));

    if (keyFrame >= mtnfile->GetNumKeyFrames() - 1) return 0;

    MTNKeyFrame* keyFrame0   = mtnfile->GetKeyFrame(keyFrame);
    MTNKeyFrame* keyFrame1   = mtnfile->GetKeyFrame(keyFrame + 1);
    slongword value0         = keyFrame0->data[jointIndex];
    slongword value1         = keyFrame1->data[jointIndex];
    slongword numInterpolate = mtnfile->GetNumInterpolate8ms(keyFrame);
    slongword divide         = numInterpolate + 1;
    slongword delta          = (value1 - value0) / divide;
    
    int numFrames = 0;
    OJointCommandValue2* jvalue = (OJointCommandValue2*)data->value;
    while (valueIndex < maxNumFrames) {

        jvalue[valueIndex].value = value0 + frame * delta;
        valueIndex++;
        frame++;
        numFrames++; 

        if (frame > numInterpolate) {
            keyFrame++;
            int n = InterpolateCommandData(jointIndex,
                                           data, valueIndex,
                                           keyFrame, 0, maxNumFrames);
            return numFrames + n;
        }
    }

    return numFrames;
}
