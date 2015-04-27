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
#include "NeutralAgent.h"

NeutralAgent::NeutralAgent() : neutralAgentState(NAS_IDLE),
			       counter(-1), max_counter(SLEEPING_MAX_COUNTER)
{
}

void
NeutralAgent::Init()
{
}

bool
NeutralAgent::AreYou(MoNetAgentID agent)
{
    OSYSDEBUG(("NeutralAgent::AreYou(%d)\n", agent));
    return (agent == monetagentNEUTRAL) ? true : false;
}

void
NeutralAgent::NotifyCommand(const MoNetAgentCommand& command,
                            MoNetAgentResult* result)
{
    OSYSDEBUG(("NeutralAgent::NotifyCommand()\n"));
    OSYSDEBUG(("  agent        : %d\n",   command.agent));
    OSYSDEBUG(("  index        : %d\n",   command.index));
    OSYSDEBUG(("  syncKey      : %08x\n", command.syncKey));
    OSYSDEBUG(("  startPosture : %d\n",   command.startPosture));
    OSYSDEBUG(("  endPosture   : %d\n",   command.endPosture));

    if ( command.agent != monetagentNEUTRAL ) {
        result->agent      = command.agent;
        result->index      = command.index;
        result->status     = monetINVALID_ARG;
        result->endPosture = monetpostureUNDEF;
        return;
    }
    if ( command.index == monetagentNEUTRAL_NT2SLEEP ) {
	AdjustDiffJointValue(command.syncKey);
	neutralAgentState = NAS_ADJUSTING_DIFF_JOINT_VALUE;

	result->agent      = monetagentNEUTRAL;
	result->index      = monetagentNEUTRAL_NT2SLEEP;
	result->status     = monetSUCCESS;
	result->endPosture = monetpostureUNDEF;

	counter = -1;
	max_counter = SLEEPING_MAX_COUNTER;
    }
    if ( command.index == monetagentNEUTRAL_ANY2SLEEP ) {
	result->agent      = monetagentNEUTRAL;
	result->index      = monetagentNEUTRAL_ANY2SLEEP;
	result->endPosture = monetpostureUNDEF;

	counter = -1;
	max_counter = SLEEPING_MAX_COUNTER;
        MoNetStatus s = MoveToSleeping();
        if (s == monetCOMPLETION) {
            result->status     = monetCOMPLETION;
            result->endPosture = monetpostureSLEEP;
        } else {
            result->status = monetSUCCESS;
        }
    }
}

void
NeutralAgent::ReadyEffector(const MoNetAgentCommand& command,
                            MoNetAgentResult* result)
{
    result->agent      = command.agent;
    result->index      = command.index;
    result->endPosture = monetpostureUNDEF;
    
    if (neutralAgentState == NAS_IDLE) {

        ; // do nothing

    } else if (neutralAgentState == NAS_ADJUSTING_DIFF_JOINT_VALUE) {

        OSYSDEBUG(("NAS_ADJUSTING_DIFF_JOINT_VALUE\n"));
        SetJointGain();
        MoNetStatus s = MoveToSleeping();
        neutralAgentState = NAS_MOVING_TO_SLEEPING;
        result->status = monetSUCCESS;

    } else if (neutralAgentState == NAS_MOVING_TO_SLEEPING) {

      //OSYSDEBUG(("NAS_MOVING_TO_SLEEPING\n"));
        MoNetStatus s = MoveToSleeping();
        if (s == monetCOMPLETION) {
            result->status     = monetCOMPLETION;
            result->endPosture = monetpostureSLEEP;
        } else {
            result->status = monetSUCCESS;
        }
    }
}

MoNetStatus
NeutralAgent::AdjustDiffJointValue(OVRSyncKey syncKey)
{
    OJointValue current[NUM_JOINTS];
    RCRegion* rgn = moNetAgentManager->FindFreeCommandRegion();
    OCommandVectorData* cmdVecData = (OCommandVectorData*)rgn->Base();
    cmdVecData->vectorInfo.syncKey = syncKey;

    for (int i = 0; i < NUM_JOINTS; i++) {
        OJointValue current;
        OPENR::GetJointValue(moNetAgentManager->PrimitiveID(i), &current);
        SetJointValue(rgn, i,
                      degrees(current.value/1000000.0),
                      degrees(current.value/1000000.0));
    }

    moNetAgentManager->Effector()->SetData(rgn);
    moNetAgentManager->Effector()->NotifyObservers();

    return monetCOMPLETION;
}

MoNetStatus
NeutralAgent::MoveToSleeping()
{
    static double start[NUM_JOINTS];
    static double delta[NUM_JOINTS];

    OSYSDEBUG(("MoveToSleeping:%d/%d\n", counter, max_counter));
    if (counter == -1) {
	int i;

        for ( i = 0; i < NUM_JOINTS; i++) {
            OJointValue current;
            OPrimitiveID jointID = moNetAgentManager->PrimitiveID(i);
            OPENR::GetJointValue(jointID, &current);
            start[i] = degrees(current.value/1000000.0);
        }

	double max_delta = 0;
	for ( i = 0; i < NUM_JOINTS; i ++ ) {
	    double delta = fabs(SLEEPING_ANGLE[i] - start[i]);
	    if ( delta > max_delta ) {
		max_delta = delta;
	    }
	}

	//max_counter = (int) (max_delta / 5.0) + 1;
	max_counter = (int) (max_delta / 2.0) + 1;
	double ndiv = (double) max_counter;

        for ( i = 0; i < NUM_JOINTS; i++ ) {
            delta[i] = (SLEEPING_ANGLE[i] - start[i]) / ndiv;
        }

        counter = 0;

        RCRegion* rgn = moNetAgentManager->FindFreeCommandRegion();

	OCommandVectorData* cmdVecData = (OCommandVectorData*)rgn->Base();
	SetPrimitiveID( cmdVecData );

        for ( i = 0; i < NUM_JOINTS; i++ ) {
            SetJointValue(rgn, i, start[i], start[i] + delta[i]);
            start[i] += delta[i];
        }

        moNetAgentManager->Effector()->SetData(rgn);
        counter ++;
    }

    RCRegion* rgn = moNetAgentManager->FindFreeCommandRegion();
    for (int i = 0; i < NUM_JOINTS; i++) {
        SetJointValue(rgn, i, start[i], start[i] + delta[i]);
        start[i] += delta[i];
    }

    moNetAgentManager->Effector()->SetData(rgn);
    moNetAgentManager->Effector()->NotifyObservers();

    counter++;
    return (counter >= max_counter) ? monetCOMPLETION : monetSUCCESS;
}

void
NeutralAgent::SetJointGain()
{
#ifdef ERS210
    OPrimitiveID tiltID = moNetAgentManager->PrimitiveID(HEAD_TILT);
    OPrimitiveID panID  = moNetAgentManager->PrimitiveID(HEAD_PAN);
    OPrimitiveID rollID = moNetAgentManager->PrimitiveID(HEAD_ROLL);
    OPrimitiveID mouthID = moNetAgentManager->PrimitiveID(HEAD_MOUTH);

    OPENR::EnableJointGain(tiltID);
    OPENR::SetJointGain(tiltID,
                        TILT_PGAIN, TILT_IGAIN, TILT_DGAIN,
                        PSHIFT, ISHIFT, DSHIFT);
    
    OPENR::EnableJointGain(panID);
    OPENR::SetJointGain(panID,
                        PAN_PGAIN, PAN_IGAIN, PAN_DGAIN,
                        PSHIFT, ISHIFT, DSHIFT);

    OPENR::EnableJointGain(rollID);
    OPENR::SetJointGain(rollID,
                        ROLL_PGAIN, ROLL_IGAIN, ROLL_DGAIN,
                        PSHIFT, ISHIFT, DSHIFT);

    OPENR::EnableJointGain(mouthID);
    OPENR::SetJointGain(mouthID,
                        MOUTH_PGAIN, MOUTH_IGAIN, MOUTH_DGAIN,
                        PSHIFT, ISHIFT, DSHIFT);
#endif
#ifdef ERS7
    OPrimitiveID tilt1ID = moNetAgentManager->PrimitiveID(HEAD_TILT1);
    OPrimitiveID panID   = moNetAgentManager->PrimitiveID(HEAD_PAN);
    OPrimitiveID tilt2ID = moNetAgentManager->PrimitiveID(HEAD_TILT2);
    OPrimitiveID mouthID = moNetAgentManager->PrimitiveID(HEAD_MOUTH);

    OPENR::EnableJointGain(tilt1ID);
    OPENR::SetJointGain(tilt1ID,
                        TILT1_PGAIN, TILT1_IGAIN, TILT1_DGAIN,
                        PSHIFT, ISHIFT, DSHIFT);
    
    OPENR::EnableJointGain(panID);
    OPENR::SetJointGain(panID,
                        PAN_PGAIN, PAN_IGAIN, PAN_DGAIN,
                        PSHIFT, ISHIFT, DSHIFT);

    OPENR::EnableJointGain(tilt2ID);
    OPENR::SetJointGain(tilt2ID,
                        TILT2_PGAIN, TILT2_IGAIN, TILT2_DGAIN,
                        PSHIFT, ISHIFT, DSHIFT);

    OPENR::EnableJointGain(mouthID);
    OPENR::SetJointGain(mouthID,
                        MOUTH_PGAIN, MOUTH_IGAIN, MOUTH_DGAIN,
                        PSHIFT, ISHIFT, DSHIFT);
#endif

    int base = RFLEG_J1;
    for (int i = 0; i < 4; i++) {

        OPrimitiveID j1ID = moNetAgentManager->PrimitiveID(base + 3 * i);
        OPrimitiveID j2ID = moNetAgentManager->PrimitiveID(base + 3 * i + 1);
        OPrimitiveID j3ID = moNetAgentManager->PrimitiveID(base + 3 * i + 2);
        
        OPENR::EnableJointGain(j1ID);        
        OPENR::SetJointGain(j1ID,
                            J1_PGAIN, J1_IGAIN, J1_DGAIN,
                            PSHIFT, ISHIFT, DSHIFT);

        OPENR::EnableJointGain(j2ID);
        OPENR::SetJointGain(j2ID,
                            J2_PGAIN, J2_IGAIN, J2_DGAIN,
                            PSHIFT, ISHIFT, DSHIFT);

        OPENR::EnableJointGain(j3ID);
        OPENR::SetJointGain(j3ID,
                            J3_PGAIN, J3_IGAIN, J3_DGAIN,
                            PSHIFT, ISHIFT, DSHIFT);
    }
}

void
NeutralAgent::SetJointValue(RCRegion* rgn, int idx, double start, double end)
{
    OCommandVectorData* cmdVecData = (OCommandVectorData*)rgn->Base();

    OPrimitiveID jointID = moNetAgentManager->PrimitiveID(idx);
    OCommandInfo* info = cmdVecData->GetInfo(idx);
    info->Set(odataJOINT_COMMAND2, jointID, NUM_FRAMES);

    OCommandData* data = cmdVecData->GetData(idx);
    OJointCommandValue2* jval = (OJointCommandValue2*)data->value;

    double delta = end - start;
    for (int i = 0; i < NUM_FRAMES; i++) {
        double dval = start + (delta * i) / (double)NUM_FRAMES;
        jval[i].value = oradians(dval);
    }
}

void
NeutralAgent::SetPrimitiveID(OCommandVectorData* cmdVecData)
{
    cmdVecData->SetNumData(NUM_JOINTS);

    for (int j = 0; j < NUM_JOINTS; j++) {
	OCommandInfo* info = cmdVecData->GetInfo(j);
	info->Set(odataJOINT_COMMAND2, moNetAgentManager->PrimitiveID(j),
		  NUM_FRAMES);
    }
}
