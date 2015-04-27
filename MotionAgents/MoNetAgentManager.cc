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

#include <OPENR/OPENRAPI.h>
#include <OPENR/OSyslog.h>
#include "MoNetAgentManager.h"
#include "MoNetAgent.h"

MoNetAgentManager::MoNetAgentManager() : activeMoNetAgent(0),
                                         activeAgentCommand(),
                                         moNetAgentList(),
                                         effectorSubject(0)
{
    for (int i = 0; i < NUM_JOINTS; i++) primitiveID[i] = oprimitiveID_UNDEF;
    for (int i = 0; i < NUM_COMMAND_VECTOR; i++) commandRegions[i] = 0;
}

void
MoNetAgentManager::Init()
{
    OSYSDEBUG(("MoNetAgentManager::Init()\n"));
    OpenPrimitives();
    NewCommandVectorData();

    list<MoNetAgent*>::iterator iter = moNetAgentList.begin();
    list<MoNetAgent*>::iterator last = moNetAgentList.end();
    while (iter != last) {
        (*iter)->Init();
        ++iter;
    }
}

void
MoNetAgentManager::Start(OSubject* effector)
{
    OSYSDEBUG(("MoNetAgentManager::Start()\n"));
    effectorSubject = effector;
}

void
MoNetAgentManager::RegisterMoNetAgent(MoNetAgent* m)
{
    OSYSDEBUG(("MoNetAgentManager::RegisterMoNetAgent()\n"));
    moNetAgentList.push_back(m);
    m->SetMoNetAgentManager(this);
}

void
MoNetAgentManager::NotifyCommand(const ONotifyEvent& event,
                                 MoNetAgentResult* result)
{
    MoNetAgentCommand* cmd = (MoNetAgentCommand*)event.Data(0);

    if (activeMoNetAgent != 0) {
        // BUSY
        result->agent      = cmd->agent;
        result->index      = cmd->index;
        result->status     = monetBUSY;
        result->endPosture = monetpostureUNDEF;
        return;
    }
    
    list<MoNetAgent*>::iterator iter = moNetAgentList.begin();
    list<MoNetAgent*>::iterator last = moNetAgentList.end();
    while (iter != last) {
        if ((*iter)->AreYou(cmd->agent) == true) {
            (*iter)->NotifyCommand(*cmd, result);
            if (result->status != monetSUCCESS) return;
            activeMoNetAgent   = *iter;
            activeAgentCommand = *cmd;
            return;
        }
        ++iter;
    }

    // INVALID_ARG
    result->agent      = cmd->agent;
    result->index      = cmd->index;
    result->status     = monetINVALID_ARG;
    result->endPosture = monetpostureUNDEF;
}

void
MoNetAgentManager::ReadyEffector(const OReadyEvent& event,
                                 MoNetAgentResult* result)
    
{
    if (activeMoNetAgent == 0) {
        result->agent      = monetagentUNDEF;
        result->index      = -1;
        result->status     = monetSUCCESS;
        result->endPosture = monetpostureUNDEF;
        return;
    }

    activeMoNetAgent->ReadyEffector(activeAgentCommand, result);
    if (result->status != monetSUCCESS) {
        activeMoNetAgent = 0;
        activeAgentCommand.Clear();
    }
}

void
MoNetAgentManager::OpenPrimitives()
{
    OSYSDEBUG(("MoNetAgentManager::OpenPrimitives()\n"));

    OStatus result;

    for (int i = 0; i < NUM_JOINTS; i++) {
        result = OPENR::OpenPrimitive(JOINT_LOCATOR[i], &primitiveID[i]);
        if (result != oSUCCESS) {
            OSYSLOG1((osyslogERROR, "%s : %s %d",
                      "MoNetAgentManager::OpenPrimitives()",
                      "OPENR::OpenPrimitive() FAILED", result));
        }
    }
}

void
MoNetAgentManager::NewCommandVectorData()
{
    OSYSDEBUG(("MoNetAgentManager::NewCommandVectorData()\n"));

    OStatus result;
    MemoryRegionID      cmdVecDataID;
    OCommandVectorData* cmdVecData;
    OCommandInfo*       info;

    for (int i = 0; i < NUM_COMMAND_VECTOR; i++) {

        result = OPENR::NewCommandVectorData(NUM_JOINTS, 
                                             &cmdVecDataID, &cmdVecData);
        if (result != oSUCCESS) {
            OSYSLOG1((osyslogERROR, "%s : %s %d",
                      "MoNetAgentManager::NewCommandVectorData()",
                      "OPENR::NewCommandVectorData() FAILED", result));
        }

        commandRegions[i] = new RCRegion(cmdVecData->vectorInfo.memRegionID,
                                         cmdVecData->vectorInfo.offset,
                                         (void*)cmdVecData,
                                         cmdVecData->vectorInfo.totalSize);

        cmdVecData->SetNumData(NUM_JOINTS);

        for (int j = 0; j < NUM_JOINTS; j++) {
            info = cmdVecData->GetInfo(j);
            info->Set(odataJOINT_COMMAND2, primitiveID[j], NUM_FRAMES);
        }
    }
}

RCRegion*
MoNetAgentManager::FindFreeCommandRegion()
{   
    for (int i = 0; i < NUM_COMMAND_VECTOR; i++) {
        if (commandRegions[i]->NumberOfReference() == 1) {
            return commandRegions[i];
        }
    }
    OSYSPRINT(("err\n"));
    OSYSLOG1((osyslogERROR, "%s : %s %d",
	      "MoNetAgentManager::FindFreeCommandRegion()",
	      "FAILED", 0));
    return 0;
}

int
MoNetAgentManager::GetIndexFromJointName( const char *jointName )
{
  for ( int i = 0; i < NUM_JOINTS; i ++ ) {
    if ( !strcmp( JOINT_LOCATOR[ i ], jointName ) ) {
      return i;
    }
  }
  OSYSLOG1((osyslogERROR, "%s : %s",
	    "MoNetAgentManager::GetIndexFromJointName()",
	    "invalid joint name"));
  return 0;
}

void
MoNetAgentManager::NotifyPose(const ONotifyEvent& event)
{
    PoseData* data = (PoseData*)event.Data(0);

    list<MoNetAgent*>::iterator iter = moNetAgentList.begin();
    list<MoNetAgent*>::iterator last = moNetAgentList.end();
    while (iter != last) {
        if ((*iter)->AreYou(monetagentANY) == true) {
            (*iter)->SetPose(data);
            return;
        }
        ++iter;
    }
}
