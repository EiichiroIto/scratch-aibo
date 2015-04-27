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

#include "MoNetCommandInfo.h"

MoNetCommandInfo::MoNetCommandInfo(bool syncKey) : useSyncKey(syncKey),
                                                   numAgentCommands(0)
{
    for (int i = 0; i < MAXNUM_AGENT_COMMANDS; i++) {
        status[i] = monetUNDEF;
        agentCommand[i].Clear();
    }
}

bool
MoNetCommandInfo::SetAgentCommand(MoNetAgentID agent, int index,
                                  MoNetPosture start, MoNetPosture end)
{
    if (numAgentCommands == MAXNUM_AGENT_COMMANDS) return false;

    agentCommand[numAgentCommands].agent        = agent;
    agentCommand[numAgentCommands].index        = index;
    agentCommand[numAgentCommands].startPosture = start;
    agentCommand[numAgentCommands].endPosture   = end;
    numAgentCommands++;
    return true;
}

void
MoNetCommandInfo::SetAgentResult(MoNetAgentID agt, int idx, MoNetStatus st)
{
    for (int i = 0; i < numAgentCommands; i++) {
        if (agentCommand[i].agent == agt && agentCommand[i].index == idx) {
            status[i] = st;
            return;
        }
    }
}

void
MoNetCommandInfo::ClearAgentResult()
{
    for (int i = 0; i < numAgentCommands; i++) {
        status[i] = monetUNDEF;
    }
}

bool
MoNetCommandInfo::IsDone()
{
    for (int i = 0; i < numAgentCommands; i++) {
        if (status[i] == monetUNDEF) return false;
    }

    return true;
}

MoNetPosture
MoNetCommandInfo::StartPosture()
{
    MoNetPosture pos = monetpostureUNDEF;

    for (int i = 0; i < NumAgentCommands(); i++) {
        if (agentCommand[i].agent == monetagentSOUND) {
            pos = agentCommand[i].startPosture;
        } else {
            return agentCommand[i].startPosture;
        }
    }

    return pos;
}

MoNetPosture
MoNetCommandInfo::EndPosture()
{
    MoNetPosture pos = monetpostureUNDEF;

    for (int i = 0; i < NumAgentCommands(); i++) {
        if (agentCommand[i].agent == monetagentSOUND) {
            pos = agentCommand[i].endPosture;
        } else {
            return agentCommand[i].endPosture;
        }
    }
     
    return pos;
}
