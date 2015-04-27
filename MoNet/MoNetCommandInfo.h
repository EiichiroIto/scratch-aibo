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

#ifndef MoNetCommandInfo_h
#define MoNetCommandInfo_h

#include <MoNetData.h>

const size_t MAXNUM_AGENT_COMMANDS = 2;

class MoNetCommandInfo {
public:
    MoNetCommandInfo(bool syncKey);
    ~MoNetCommandInfo() {}

    bool SetAgentCommand(MoNetAgentID agent, int index,
                         MoNetPosture start, MoNetPosture end);
    void SetAgentResult(MoNetAgentID agent, int index, MoNetStatus st);
    void ClearAgentResult();
    bool IsDone();

    MoNetPosture StartPosture();
    MoNetPosture EndPosture();

    bool               UseSyncKey()        { return useSyncKey;       }
    int                NumAgentCommands()  { return numAgentCommands; }
    MoNetAgentCommand* AgentCommand(int i) { return &agentCommand[i]; }

private:
    bool              useSyncKey;
    int               numAgentCommands;
    MoNetStatus       status[MAXNUM_AGENT_COMMANDS];
    MoNetAgentCommand agentCommand[MAXNUM_AGENT_COMMANDS];
};


#endif // MoNetCommandInfo_h
