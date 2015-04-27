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
#include "MTNFile.h"

word
MTNFile::GetNumJoints()
{
    return (GetSection2())->numJoints;
}

word
MTNFile::GetNumKeyFrames()
{
    return section0.numKeyFrames;
}

word
MTNFile::GetFrameRate()
{
    return section0.frameRate;
}

char*
MTNFile::GetName()
{
    MTNString* motion = &(section1.motion);
    return string_access(motion);
}

char*
MTNFile::GetAuthor()
{
    MTNString* tmp = &(section1.motion);
    MTNString* author = (MTNString*)((byte*)tmp->name + tmp->length);
    return string_access(author);
}

char*
MTNFile::GetRobotDesign()
{
    MTNString* tmp = &(section1.motion);
    tmp = (MTNString*)((byte*)tmp->name + tmp->length);
    MTNString* design = (MTNString*)((byte*)tmp->name + tmp->length);
    return string_access(design);
}

char*
MTNFile::GetLocator(int index)
{
    MTNString* locator = (GetSection2())->locator;
    for (int i = 0; i < index; i++)
        locator = (MTNString*)((byte*)locator->name + locator->length);

    return string_access(locator);
}

MTNString*
MTNFile::GetLocator2(int index)
{
    MTNString* locator = (GetSection2())->locator;
    for (int i = 0; i < index; i++)
        locator = (MTNString*)((byte*)locator->name + locator->length);

    return locator;
}

longword
MTNFile::GetDataType()
{
    return (GetSection3())->dataType;
}

int
MTNFile::GetEachKeyFrameSize()
{
    int sizeofKeyFrame = (3 + GetNumJoints()) * sizeof(slongword);
    return sizeofKeyFrame;
}

int
MTNFile::GetTotalKeyFrameSize()
{
    int numKeyFrames = GetNumKeyFrames();
    int keyFrameSize = GetEachKeyFrameSize() * numKeyFrames
        + sizeof(slongword) * (numKeyFrames-1); // numInterpolate
    return keyFrameSize;
}

MTNKeyFrame*
MTNFile::GetKeyFrame(int index)
{
    byte* ptr = (GetSection3())->keyFrame;
    ptr = ptr + (GetEachKeyFrameSize() + sizeof(slongword)) * index;
    return (MTNKeyFrame*)ptr;
}

int
MTNFile::GetNumInterpolate(int index)
{
    if (index >= GetNumKeyFrames() - 1) return -1;

    byte* ptr = (GetSection3())->keyFrame;
    int offset = GetEachKeyFrameSize() + sizeof(slongword);
    ptr = ptr + (index + 1) * offset - 4;
    return (int)*((int*)ptr);
}

int
MTNFile::GetNumInterpolate8ms(int index)
{
    int n = GetNumInterpolate(index);
    return 2 * n + 1;
}

slongword
MTNFile::GetJointValue(int index, int jointIndex)
{
    MTNKeyFrame* kf = GetKeyFrame(index);
    return kf->data[jointIndex];
}

MTNSection2*
MTNFile::GetSection2()
{
    byte* ptr = (byte*)&section1;
    int offset = section1.sectionSize;
    return (MTNSection2*)(ptr + offset);
}

MTNSection3*
MTNFile::GetSection3()
{
    byte* ptr = (byte*)GetSection2();
    int offset = (GetSection2())->sectionSize;
    return (MTNSection3*)(ptr + offset);
}

char*
MTNFile::string_access(MTNString* mstr)
{
    static char buffer[256];    // magic number !! Improve later

    memcpy(buffer, mstr->name, mstr->length);
    buffer[mstr->length] = 0;
    return buffer;
}
