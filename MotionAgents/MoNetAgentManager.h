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

#ifndef MoNetAgentManager_h_DEFINED
#define MoNetAgentManager_h_DEFINED

#include <list>
using namespace std;

#include <OPENR/OSubject.h>
#include <OPENR/OObserver.h>
#include <MoNetData.h>
#include "MoNetAgentManager.h"

#ifdef ERS210
const int NUM_JOINTS         = 16;
const int NUM_FRAMES         =  4;

const int HEAD_TILT  =  0;
const int HEAD_PAN   =  1;
const int HEAD_ROLL  =  2;
const int HEAD_MOUTH =  3;
const int RFLEG_J1   =  4;
const int RFLEG_J2   =  5;
const int RFLEG_J3   =  6;
const int LFLEG_J1   =  7;
const int LFLEG_J2   =  8;
const int LFLEG_J3   =  9;
const int RRLEG_J1   = 10;
const int RRLEG_J2   = 11;
const int RRLEG_J3   = 12;
const int LRLEG_J1   = 13;
const int LRLEG_J2   = 14;
const int LRLEG_J3   = 15;

static const char* const JOINT_LOCATOR[] = {
    "PRM:/r1/c1-Joint2:j1",             // HEAD TILT
    "PRM:/r1/c1/c2-Joint2:j2",          // HEAD PAN
    "PRM:/r1/c1/c2/c3-Joint2:j3",       // HEAD ROLL
    "PRM:/r1/c1/c2/c3/c4-Joint2:j4",    // HEAD MOUTH
    "PRM:/r4/c1-Joint2:j1",             // RFLEG J1 (Right Front Leg)
    "PRM:/r4/c1/c2-Joint2:j2",          // RFLEG J2
    "PRM:/r4/c1/c2/c3-Joint2:j3",       // RFLEG J3
    "PRM:/r2/c1-Joint2:j1",             // LFLEG J1 (Left Front Leg)
    "PRM:/r2/c1/c2-Joint2:j2",          // LFLEG J2
    "PRM:/r2/c1/c2/c3-Joint2:j3",       // LFLEG J3
    "PRM:/r5/c1-Joint2:j1",             // RRLEG J1 (Right Rear Leg)
    "PRM:/r5/c1/c2-Joint2:j2",          // RRLEG J2
    "PRM:/r5/c1/c2/c3-Joint2:j3",       // RRLEG J3
    "PRM:/r3/c1-Joint2:j1",             // LRLEG J1 (Left Rear Leg)
    "PRM:/r3/c1/c2-Joint2:j2",          // LRLEG J2
    "PRM:/r3/c1/c2/c3-Joint2:j3",       // LRLEG J3
};
#endif // ERS210
#ifdef ERS7
const int NUM_JOINTS         = 16;
const int NUM_FRAMES         =  4;

const int HEAD_TILT1 =  0;
const int HEAD_PAN   =  1;
const int HEAD_TILT2 =  2;
const int HEAD_MOUTH =  3;
const int RFLEG_J1   =  4;
const int RFLEG_J2   =  5;
const int RFLEG_J3   =  6;
const int LFLEG_J1   =  7;
const int LFLEG_J2   =  8;
const int LFLEG_J3   =  9;
const int RRLEG_J1   = 10;
const int RRLEG_J2   = 11;
const int RRLEG_J3   = 12;
const int LRLEG_J1   = 13;
const int LRLEG_J2   = 14;
const int LRLEG_J3   = 15;

static const char* const JOINT_LOCATOR[] = {
    "PRM:/r1/c1-Joint2:11",             // HEAD TILT1
    "PRM:/r1/c1/c2-Joint2:12",          // HEAD PAN
    "PRM:/r1/c1/c2/c3-Joint2:13",       // HEAD TILT2
    "PRM:/r1/c1/c2/c3/c4-Joint2:14",    // HEAD MOUTH
    "PRM:/r4/c1-Joint2:41",       // RFLEG J1 (Right Front Leg)
    "PRM:/r4/c1/c2-Joint2:42",    // RFLEG J2
    "PRM:/r4/c1/c2/c3-Joint2:43", // RFLEG J3
    "PRM:/r2/c1-Joint2:21",       // LFLEG J1 (Left Front Leg)
    "PRM:/r2/c1/c2-Joint2:22",    // LFLEG J2
    "PRM:/r2/c1/c2/c3-Joint2:23", // LFLEG J3
    "PRM:/r5/c1-Joint2:51",       // RRLEG J1 (Right Rear Leg)
    "PRM:/r5/c1/c2-Joint2:52",    // RRLEG J2
    "PRM:/r5/c1/c2/c3-Joint2:53", // RRLEG J3
    "PRM:/r3/c1-Joint2:31",       // LRLEG J1 (Left Rear Leg)
    "PRM:/r3/c1/c2-Joint2:32",    // LRLEG J2
    "PRM:/r3/c1/c2/c3-Joint2:33"  // LRLEG J3
};
#endif // ERS7

class MoNetAgent;

class MoNetAgentManager {
public:
    MoNetAgentManager();
    ~MoNetAgentManager() {}

    void Init();
    void Start(OSubject* effector);

    void RegisterMoNetAgent(MoNetAgent* m);

    void NotifyCommand(const ONotifyEvent& event, MoNetAgentResult* result);
    void ReadyEffector(const OReadyEvent& event,  MoNetAgentResult* result);
    void NotifyPose(const ONotifyEvent& event);

    OPrimitiveID PrimitiveID(int idx) { return primitiveID[idx]; }
    OSubject*    Effector()           { return effectorSubject;  }
    RCRegion*    FindFreeCommandRegion();
    int GetIndexFromJointName( const char *str );

private:
    void OpenPrimitives();
    void NewCommandVectorData();

    static const size_t NUM_COMMAND_VECTOR =  4;

    MoNetAgent*          activeMoNetAgent;
    MoNetAgentCommand    activeAgentCommand;
    list<MoNetAgent*>    moNetAgentList;

    OSubject*    effectorSubject;
    OPrimitiveID primitiveID[NUM_JOINTS];

    RCRegion*    commandRegions[NUM_COMMAND_VECTOR];
};

#endif // MoNetAgentManager_h_DEFINED
