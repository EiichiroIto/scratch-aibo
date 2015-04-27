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

#ifndef MTNAgent_h_DEFINED
#define MTNAgent_h_DEFINED

#include <OPENR/ODataArchive.h>
#include <MoNetData.h>
#include <ODA.h>
#include "MoNetAgent.h"
#include "MTN.h"

#ifdef ERS210
const int DRX900_NUM_JOINTS = 16;
const int DRX900_INDEX[] = {
    HEAD_TILT,
    HEAD_PAN,
    HEAD_ROLL,
    HEAD_MOUTH,
    LFLEG_J1,
    LFLEG_J2,
    LFLEG_J3,
    LRLEG_J1,
    LRLEG_J2,
    LRLEG_J3,
    RFLEG_J1,
    RFLEG_J2,
    RFLEG_J3,
    RRLEG_J1,
    RRLEG_J2,
    RRLEG_J3
};
#endif
#ifdef ERS7
const int DRX900_NUM_JOINTS = 16;
const int DRX900_INDEX[] = {
    HEAD_TILT1,
    HEAD_PAN,
    HEAD_TILT2,
    HEAD_MOUTH,
    LFLEG_J1,
    LFLEG_J2,
    LFLEG_J3,
    LRLEG_J1,
    LRLEG_J2,
    LRLEG_J3,
    RFLEG_J1,
    RFLEG_J2,
    RFLEG_J3,
    RRLEG_J1,
    RRLEG_J2,
    RRLEG_J3
};
#endif

class MTNAgent : public MoNetAgent {
public:
    MTNAgent();
    virtual ~MTNAgent() {}

    virtual bool AreYou(MoNetAgentID agent);
    virtual void Init();
    virtual void NotifyCommand(const MoNetAgentCommand& command,
                               MoNetAgentResult* result);
    virtual void ReadyEffector(const MoNetAgentCommand& command,
                               MoNetAgentResult* result);

private:
    void        LoadMotionODA();
    MoNetStatus SetPrimitiveID(OCommandVectorData* cmdVec, char* robotDesign);
    MoNetStatus Move(OVRSyncKey syncKey);

    ODesignDataID  motionDataID;
    ODA            motionODA;
    MTN            mtn;
};

#endif // MTNAgent_h_DEFINED
