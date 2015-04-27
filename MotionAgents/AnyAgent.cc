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
#include "AnyAgent.h"

double ANY_ANGLE[NUM_JOINTS] = {
    0, 0, 0,   // Head
    0, 0, 0,   // RFLEG
    0, 0, 0,   // LFLEG
    0, 0, 0,   // RRLEG
    0, 0, 0    // LRLEG
};

AnyAgent::AnyAgent() : anyAgentState(AAS_IDLE), movingCounter(-1), max_counter( MOVING_MAX_COUNTER)
{
}

void
AnyAgent::Init()
{
}

bool
AnyAgent::AreYou(MoNetAgentID agent)
{
    OSYSDEBUG(("AnyAgent::AreYou(%d)\n", agent));
    return (agent == monetagentANY) ? true : false;
}

void
AnyAgent::NotifyCommand(const MoNetAgentCommand& command,
                            MoNetAgentResult* result)
{
    OSYSPRINT(("AnyAgent::NotifyCommand()\n"));
    OSYSPRINT(("  agent        : %d\n",   command.agent));
    OSYSPRINT(("  index        : %d\n",   command.index));
    OSYSPRINT(("  syncKey      : %08x\n", command.syncKey));
    OSYSPRINT(("  startPosture : %d\n",   command.startPosture));
    OSYSPRINT(("  endPosture   : %d\n",   command.endPosture));

    if (command.index != monetagentANY_SET_POSE ) {
        result->agent      = command.agent;
        result->index      = command.index;
        result->status     = monetINVALID_ARG;
        result->endPosture = monetpostureUNDEF;
        return;
    }

    movingCounter = -1;
    max_counter = MOVING_MAX_COUNTER;
    MoNetStatus s = MoveToAny();

    result->agent      = monetagentANY;
    result->index      = command.index;
    result->endPosture = monetpostureUNDEF;
    if (s == monetCOMPLETION) {
	result->status     = monetCOMPLETION;
	result->endPosture = monetpostureANY;
    } else {
	result->status = monetSUCCESS;
    }

    anyAgentState = AAS_MOVING_TO_ANY;
}

void
AnyAgent::ReadyEffector(const MoNetAgentCommand& command,
                            MoNetAgentResult* result)
{
    result->agent      = monetagentANY;
    result->index      = monetagentANY_SET_POSE;
    result->endPosture = monetpostureUNDEF;
    
    if (anyAgentState == AAS_IDLE) {

        ; // do nothing

    } else if (anyAgentState == AAS_MOVING_TO_ANY) {

	OSYSPRINT(("AAS_MOVING_TO_ANY\n"));
        MoNetStatus s = MoveToAny();
        if (s == monetCOMPLETION) {
            result->status     = monetCOMPLETION;
            result->endPosture = monetpostureSLEEP;
        } else {
            result->status = monetSUCCESS;
        }
    }
}

MoNetStatus
AnyAgent::MoveToAny()
{
    static double start[NUM_JOINTS];
    static double delta[NUM_JOINTS];

    if (movingCounter == -1) {
	int i;

        for ( i = 0; i < NUM_JOINTS; i++) {
            OJointValue current;
            OPrimitiveID jointID = moNetAgentManager->PrimitiveID(i);
            OPENR::GetJointValue(jointID, &current);
            start[i] = degrees(current.value/1000000.0);
        }

	double max_delta = 0;
	for ( i = 0; i < NUM_JOINTS; i ++ ) {
	    double delta = fabs(ANY_ANGLE[i] - start[i]);
	    if ( delta > max_delta ) {
		max_delta = delta;
	    }
	}

	//max_counter = (int) (max_delta / 5.0) + 1;
	max_counter = (int) (max_delta / 2.0) + 1;
	double ndiv = (double) max_counter;

        for ( i = 0; i < NUM_JOINTS; i++ ) {
            delta[i] = (ANY_ANGLE[i] - start[i]) / ndiv;
        }

        movingCounter = 0;

        RCRegion* rgn = moNetAgentManager->FindFreeCommandRegion();

	OCommandVectorData* cmdVecData = (OCommandVectorData*)rgn->Base();
	SetPrimitiveID( cmdVecData );

        for ( i = 0; i < NUM_JOINTS; i++ ) {
            SetJointValue(rgn, i, start[i], start[i] + delta[i]);
            start[i] += delta[i];
        }

        moNetAgentManager->Effector()->SetData(rgn);
        movingCounter ++;
    }

    RCRegion* rgn = moNetAgentManager->FindFreeCommandRegion();
    for (int i = 0; i < NUM_JOINTS; i++) {
        SetJointValue(rgn, i, start[i], start[i] + delta[i]);
        start[i] += delta[i];
    }

    moNetAgentManager->Effector()->SetData(rgn);
    moNetAgentManager->Effector()->NotifyObservers();

    movingCounter++;
    return (movingCounter >= max_counter) ? monetCOMPLETION : monetSUCCESS;
}

void
AnyAgent::SetJointValue(RCRegion* rgn, int idx, double start, double end)
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
AnyAgent::SetPose(const PoseData* data)
{
    ANY_ANGLE[RFLEG_J1] = data->frontRight[0];
    ANY_ANGLE[RFLEG_J2] = data->frontRight[1];
    ANY_ANGLE[RFLEG_J3] = data->frontRight[2];
    ANY_ANGLE[LFLEG_J1] = data->frontLeft[0];
    ANY_ANGLE[LFLEG_J2] = data->frontLeft[1];
    ANY_ANGLE[LFLEG_J3] = data->frontLeft[2];
    ANY_ANGLE[RRLEG_J1] = data->rearRight[0];
    ANY_ANGLE[RRLEG_J2] = data->rearRight[1];
    ANY_ANGLE[RRLEG_J3] = data->rearRight[2];
    ANY_ANGLE[LRLEG_J1] = data->rearLeft[0];
    ANY_ANGLE[LRLEG_J2] = data->rearLeft[1];
    ANY_ANGLE[LRLEG_J3] = data->rearLeft[2];

    OSYSDEBUG(("frontLeft:%4.1f,%4.1f,%4.1f\n",
	       ANY_ANGLE[LFLEG_J1], ANY_ANGLE[LFLEG_J2], ANY_ANGLE[LFLEG_J3]));
    OSYSDEBUG(("rearLeft:%4.1f,%4.1f,%4.1f\n",
	       ANY_ANGLE[LRLEG_J1], ANY_ANGLE[LRLEG_J2], ANY_ANGLE[LRLEG_J3]));
    OSYSDEBUG(("frontRight:%4.1f,%4.1f,%4.1f\n",
	       ANY_ANGLE[RFLEG_J1], ANY_ANGLE[RFLEG_J2], ANY_ANGLE[RFLEG_J3]));
    OSYSDEBUG(("rearRight:%4.1f,%4.1f,%4.1f\n",
	       ANY_ANGLE[RRLEG_J1], ANY_ANGLE[RRLEG_J2], ANY_ANGLE[RRLEG_J3]));
}

void
AnyAgent::SetPrimitiveID(OCommandVectorData* cmdVecData)
{
    cmdVecData->SetNumData(NUM_JOINTS);

    for (int j = 0; j < NUM_JOINTS; j++) {
	OCommandInfo* info = cmdVecData->GetInfo(j);
	info->Set(odataJOINT_COMMAND2, moNetAgentManager->PrimitiveID(j),
		  NUM_FRAMES);
    }
}
