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

#ifndef AnyAgent_h_DEFINED
#define AnyAgent_h_DEFINED

#include <MoNetData.h>
#include "MoNetAgent.h"

enum AnyAgentState {
    AAS_IDLE,
    AAS_MOVING_TO_ANY
};

class AnyAgent : public MoNetAgent {
public:
    AnyAgent();
    virtual ~AnyAgent() {}

    virtual bool AreYou(MoNetAgentID agent);
    virtual void Init();
    virtual void NotifyCommand(const MoNetAgentCommand& command,
                               MoNetAgentResult* result);
    virtual void ReadyEffector(const MoNetAgentCommand& command,
                               MoNetAgentResult* result);
    virtual void SetPose(const PoseData* data);

private:
    static const int MOVING_MAX_COUNTER  = 192; // 32ms * 192 = 6144ms

    MoNetStatus MoveToAny();
    void SetJointValue(RCRegion* rgn, int idx, double start, double end);
    void SetPrimitiveID(OCommandVectorData* cmdVec);

    AnyAgentState anyAgentState;
    int movingCounter;
    int max_counter;
};

#endif // AnyAgent_h_DEFINED
