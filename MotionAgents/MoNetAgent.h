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

#ifndef MoNetAgent_h_DEFINED
#define MoNetAgent_h_DEFINED

#include <MoNetData.h>
#include "MoNetAgentManager.h"

class MoNetAgent {
public:
    MoNetAgent();
    ~MoNetAgent() {}

    virtual void Init();
    virtual bool AreYou(MoNetAgentID agent);
    virtual void NotifyCommand(const MoNetAgentCommand& command,
                               MoNetAgentResult* result);
    virtual void ReadyEffector(const MoNetAgentCommand& command,
                               MoNetAgentResult* result);
    virtual void SetPose(const PoseData* data);

protected:
    friend class MoNetAgentManager;
    void SetMoNetAgentManager(MoNetAgentManager* m);

    MoNetAgentManager* moNetAgentManager;
};

#endif // MoNetAgent_h_DEFINED
