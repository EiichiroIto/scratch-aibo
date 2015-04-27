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
#include "MTNAgent.h"

#ifdef ERS210
#define isValidAibo(x) (strcmp(x, "DRX-900") == 0 || strcmp(x, "DRX-910") == 0)
#endif
#ifdef ERS7
#define isValidAibo(x) (strcmp(x, "DRX-1000") == 0)
#endif

MTNAgent::MTNAgent() : motionDataID(odesigndataID_UNDEF), motionODA(), mtn()
{
}

void
MTNAgent::Init()
{
    LoadMotionODA();
}

bool
MTNAgent::AreYou(MoNetAgentID agent)
{
    OSYSDEBUG(("MTNAgent::AreYou(%d)\n", agent));
    return (agent == monetagentMTN) ? true : false;
}

void
MTNAgent::NotifyCommand(const MoNetAgentCommand& command,
                        MoNetAgentResult* result)
{
    OSYSDEBUG(("MTNAgent::NotifyCommand()\n"));
    OSYSDEBUG(("  agent        : %d\n",   command.agent));
    OSYSDEBUG(("  index        : %d\n",   command.index));
    OSYSDEBUG(("  syncKey      : %08x\n", command.syncKey));
    OSYSDEBUG(("  startPosture : %d\n",   command.startPosture));
    OSYSDEBUG(("  endPosture   : %d\n",   command.endPosture));

    if (command.index == monetagentMTN_NULL_MOTION) {
        result->agent      = command.agent;
        result->index      = command.index;
        result->status     = monetCOMPLETION;
        result->endPosture = command.endPosture;
        return;
    }

    MTNFile* mtnfile = (MTNFile*)motionODA.GetData(command.index);
    if (mtnfile == 0) {
        result->agent      = command.agent;
        result->index      = command.index;
        result->status     = monetINVALID_ARG;
        result->endPosture = command.startPosture;
        return;
    }
    
    mtn.Set(mtnfile);
    MoNetStatus status = Move(command.syncKey);

    result->agent      = command.agent;
    result->index      = command.index;
    result->status     = status;
    result->endPosture = monetpostureUNDEF;
}

void
MTNAgent::ReadyEffector(const MoNetAgentCommand& command,
                        MoNetAgentResult* result)
{
    result->agent      = command.agent;
    result->index      = command.index;
    result->endPosture = monetpostureUNDEF;

    MoNetStatus status = Move(ovrsynckeyUNDEF);
    if (status == monetCOMPLETION) {
        result->endPosture = command.endPosture;
    }

    result->status = status;
}

void
MTNAgent::LoadMotionODA()
{
    OSYSDEBUG(("LoadMotionODA()\n"));

    OStatus result;
    byte*   addr;
    size_t  size;

    result = OPENR::FindDesignData(MONET_MOTION_KEYWORD,
                                   &motionDataID, &addr, &size);
    if (result != oSUCCESS) {
        OSYSLOG1((osyslogERROR, "%s : %s %d",
                  "MTNAgent::LoadMotionODA()",
                  "OPENR::FindDesignData() FAILED", result));
        return;
    }

    motionODA.Set(addr);
}

MoNetStatus
MTNAgent::Move(OVRSyncKey syncKey)
{
    RCRegion* rgn = moNetAgentManager->FindFreeCommandRegion();
    OCommandVectorData* cmdVecData = (OCommandVectorData*)rgn->Base();
    cmdVecData->vectorInfo.syncKey = syncKey;

    MoNetStatus st = SetPrimitiveID(cmdVecData, mtn.GetRobotDesign());
    if (st != monetSUCCESS) {
        OSYSLOG1((osyslogERROR, "%s : %s %s",
                  "MTNAgent::Move()",
                  "SetPrimitiveID() FAILED", mtn.GetName()));
        return monetINVALID_ARG;
    }

    int nframes = mtn.InterpolateCommandVectorData(cmdVecData, NUM_FRAMES);
    if (nframes == -1) {
        OSYSLOG1((osyslogERROR, "%s : %s %s",
                  "MTNAgent::Move()",
                  "mtn.InterpolateCommandVectorData() FAILED", mtn.GetName()));
        return monetINVALID_ARG;
    }

    mtn.Next(nframes);

    moNetAgentManager->Effector()->SetData(rgn);
    moNetAgentManager->Effector()->NotifyObservers();

    return (mtn.More() == true) ? monetSUCCESS : monetCOMPLETION;
}

MoNetStatus
MTNAgent::SetPrimitiveID(OCommandVectorData* cmdVec, char* robotDesign)
{
    if ( isValidAibo( robotDesign ) ) {

        if (cmdVec->vectorInfo.maxNumData < DRX900_NUM_JOINTS) {
	    OSYSPRINT(( "SetPrimitiveID::a\n" ));
            return monetINVALID_ARG;
        }

	MTNFile* mtnfile = mtn.GetMTNFile();
	int numJoints = (int) mtnfile->GetNumJoints();

        cmdVec->SetNumData( numJoints );
        for (int i = 0; i < numJoints; i++) {
            OCommandInfo* info = cmdVec->GetInfo(i);
	    int idx = moNetAgentManager->GetIndexFromJointName( mtnfile->GetLocator(i) );
            OPrimitiveID id = moNetAgentManager->PrimitiveID( idx );
            info->Set(odataJOINT_COMMAND2, id, 0);
        }
        
        return monetSUCCESS;

    } else {

	OSYSPRINT(( "SetPrimitiveID::InvalidAibo(%s)\n", robotDesign ));
        return monetINVALID_ARG;

    }
}
