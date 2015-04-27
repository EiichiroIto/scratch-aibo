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

#ifndef NeutralAgent_h_DEFINED
#define NeutralAgent_h_DEFINED

#include <MoNetData.h>
#include "MoNetAgent.h"

#ifdef ERS210
const double SLEEPING_ANGLE[] = {
    -10,  // TILT
    0,    // PAN
    0,    // ROLL
    -3,    // MOUTH

    57,   // RFLEG J1
    0,    // RFLEG J2
    72,   // RFLEG J3

    57,   // LFLEG J1
    0,    // LFLEG J2
    72,   // LFLEG J3

    52,   // RRLEG J1
    0,    // RRLEG J2
    83,   // RRLEG J3

    52,   // LRLEG J1
    0,    // LRLEG J2
    83    // LRLEG J3
};
#endif // ERS210
#ifdef ERS7
const double SLEEPING_ANGLE[] = {
    0,    // TILT1    = /r1/c1-Joint2:11
    0,    // PAN      = /r1/c1/c2-Joint2:12
    0,    // TILT2    = /r1/c1/c2/c3-Joint2:13
   -3,    // MOUTH    = /r1/c1/c2/c3/c4-Joint2:14

    54,   // RFLEG J1 = /r4/c1-Joint2:41
    5,    // RFLEG J2 = /r4/c1/c2-Joint2:42
    73,   // RFLEG J3 = /r4/c1/c2/c3-Joint2:43

    54,   // LFLEG J1 = /r2/c1-Joint2:21
    5,    // LFLEG J2 = /r2/c1/c2-Joint2:22
    73,   // LFLEG J3 = /r2/c1/c2/c3-Joint2:23

    59,   // RRLEG J1 = /r5/c1-Joint2:51
    5,    // RRLEG J2 = /r5/c1/c2-Joint2:52
    71,   // RRLEG J3 = /r5/c1/c2/c3-Joint2:53

    59,   // LRLEG J1 = /r3/c1-Joint2:31
    5,    // LRLEG J2 = /r3/c1/c2-Joint2:32
    71    // LRLEG J3 = /r3/c1/c2/c3-Joint2:33
};
#endif // ERS7

enum NeutralAgentState {
    NAS_IDLE,
    NAS_ADJUSTING_DIFF_JOINT_VALUE,
    NAS_MOVING_TO_SLEEPING
};

class NeutralAgent : public MoNetAgent {
public:
    NeutralAgent();
    virtual ~NeutralAgent() {}

    virtual bool AreYou(MoNetAgentID agent);
    virtual void Init();
    virtual void NotifyCommand(const MoNetAgentCommand& command,
                               MoNetAgentResult* result);
    virtual void ReadyEffector(const MoNetAgentCommand& command,
                               MoNetAgentResult* result);

private:
#ifdef ERS210
    static const word   TILT_PGAIN = 0x000a;
    static const word   TILT_IGAIN = 0x0008;
    static const word   TILT_DGAIN = 0x000c;

    static const word   PAN_PGAIN  = 0x000d;
    static const word   PAN_IGAIN  = 0x0008;
    static const word   PAN_DGAIN  = 0x000b;

    static const word   ROLL_PGAIN = 0x000a;
    static const word   ROLL_IGAIN = 0x0008;
    static const word   ROLL_DGAIN = 0x000c;

    static const word   MOUTH_PGAIN = 0x000e;
    static const word   MOUTH_IGAIN = 0x0008;
    static const word   MOUTH_DGAIN = 0x0010;

    static const word   J1_PGAIN   = 0x0016;
    static const word   J1_IGAIN   = 0x0004;
    static const word   J1_DGAIN   = 0x0008;

    static const word   J2_PGAIN   = 0x0014;
    static const word   J2_IGAIN   = 0x0004;
    static const word   J2_DGAIN   = 0x0006;

    static const word   J3_PGAIN   = 0x0023;
    static const word   J3_IGAIN   = 0x0004;
    static const word   J3_DGAIN   = 0x0005;

    static const word   PSHIFT     = 0x000e;
    static const word   ISHIFT     = 0x0002;
    static const word   DSHIFT     = 0x000f;
#endif // ERS210
#ifdef ERS7
    static const word   TILT1_PGAIN = 0x000a;
    static const word   TILT1_IGAIN = 0x0004;
    static const word   TILT1_DGAIN = 0x0002;

    static const word   PAN_PGAIN = 0x0008;
    static const word   PAN_IGAIN = 0x0002;
    static const word   PAN_DGAIN = 0x0004;

    static const word   TILT2_PGAIN = 0x0008;
    static const word   TILT2_IGAIN = 0x0004;
    static const word   TILT2_DGAIN = 0x0002;

    static const word   MOUTH_PGAIN = 0x0008;
    static const word   MOUTH_IGAIN = 0x0000;
    static const word   MOUTH_DGAIN = 0x0004;

    static const word   J1_PGAIN = 0x0010;
    static const word   J1_IGAIN = 0x0004;
    static const word   J1_DGAIN = 0x0001;

    static const word   J2_PGAIN = 0x000a;
    static const word   J2_IGAIN = 0x0004;
    static const word   J2_DGAIN = 0x0001;

    static const word   J3_PGAIN = 0x0010;
    static const word   J3_IGAIN = 0x0004;
    static const word   J3_DGAIN = 0x0001;

    static const word   PSHIFT   = 0x000e;
    static const word   ISHIFT   = 0x0002;
    static const word   DSHIFT   = 0x000f;
#endif // ERS7

    static const int SLEEPING_MAX_COUNTER  = 192; // 32ms * 192 = 6144ms

    MoNetStatus AdjustDiffJointValue(OVRSyncKey syncKey);
    MoNetStatus MoveToSleeping();

    void SetJointGain();
    void SetJointValue(RCRegion* rgn, int idx, double start, double end);
    void SetPrimitiveID(OCommandVectorData* cmdVec);

    NeutralAgentState neutralAgentState;
    int counter;
    int max_counter;
};

#endif // NeutralAgent_h_DEFINED
