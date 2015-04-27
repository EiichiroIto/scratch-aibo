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

#include "MoNetAgent.h"

MoNetAgent::MoNetAgent() : moNetAgentManager(0)
{
}

void
MoNetAgent::SetMoNetAgentManager(MoNetAgentManager* m)
{
    moNetAgentManager = m;
}

void
MoNetAgent::Init()
{
}

bool
MoNetAgent::AreYou(MoNetAgentID agent)
{
    return false;
}

void
MoNetAgent::NotifyCommand(const MoNetAgentCommand& command,
                          MoNetAgentResult* result)
{
    result->agent      = monetagentUNDEF;
    result->index      = -1;
    result->status     = monetUNDEF;
    result->endPosture = monetpostureUNDEF;
}

void
MoNetAgent::ReadyEffector(const MoNetAgentCommand& command,
                          MoNetAgentResult* result)
{
    result->agent      = monetagentUNDEF;
    result->index      = -1;
    result->status     = monetUNDEF;
    result->endPosture = monetpostureUNDEF;
}

void
MoNetAgent::SetPose(const PoseData* data)
{
}
