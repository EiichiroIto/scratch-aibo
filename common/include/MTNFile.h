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

#ifndef MTNFile_h_DEFINED
#define MTNFile_h_DEFINED

#include <Types.h>

struct MTNString {
    byte  length;
    char  name[1];
};

struct MTNKeyFrame {
    slongword  roll;            //             4 bytes
    slongword  pitch;           //             4 bytes
    slongword  yaw;             //             4 bytes
    slongword  data[1];         // numJoints * 4 bytes
};

struct MTNSection0 {            // 24 bytes (total)
    longword  sectionNum;       //  4 bytes
    longword  sectionSize;      //  4 bytes
    longword  numSections;      //  4 bytes
    word      majorVersion;     //  2 bytes
    word      minorVersion;     //  2 bytes
    word      numKeyFrames;     //  2 bytes
    word      frameRate;        //  2 bytes
    longword  option;           //  4 bytes
};

struct MTNSection1 {
    longword  sectionNum;       //  4 bytes
    longword  sectionSize;      //  4 bytes
    MTNString     motion;
    // MTNString  author;
    // MTNString  robotDesign;
};

struct MTNSection2 {
    longword       sectionNum;
    longword       sectionSize;
    word           numJoints;
    MTNString  locator[1];
};

struct MTNSection3 {
    longword  sectionNum;
    longword  sectionSize;
    longword  dataType;
    byte      keyFrame[1];
};

struct MTNFile {
    char         magic[4];
    MTNSection0  section0;
    MTNSection1  section1;

    word         GetNumJoints();
    word         GetNumKeyFrames();
    word         GetFrameRate();
    char*        GetName();
    char*        GetAuthor();
    char*        GetRobotDesign();
    char*        GetLocator(int index);
    MTNString*   GetLocator2(int index);
    longword     GetDataType();
    int          GetEachKeyFrameSize();
    int          GetTotalKeyFrameSize();
    MTNKeyFrame* GetKeyFrame(int index);
    int          GetNumInterpolate(int index);
    int          GetNumInterpolate8ms(int index);
    slongword    GetJointValue(int index, int jointIndex);
  
    MTNSection2* GetSection2();
    MTNSection3* GetSection3();

    void Print();
    void PrintKeyFrame(int index);

private:
    char* string_access(MTNString* mstr);
};

#endif // MTNFile_h_DEFINED
