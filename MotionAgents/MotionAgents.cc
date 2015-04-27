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

#include <OPENR/OSyslog.h>
#include <OPENR/core_macro.h>
#include <MoNetData.h>
#include "MotionAgents.h"

MotionAgents::MotionAgents() : motionAgentsState(MAS_IDLE),
                               moNetAgentManager(), neutralAgent(), mtnAgent(),
			       anyAgent()
{
}

OStatus
MotionAgents::DoInit(const OSystemEvent& event)
{
    NEW_ALL_SUBJECT_AND_OBSERVER;
    REGISTER_ALL_ENTRY;
    SET_ALL_READY_AND_NOTIFY_ENTRY;

    moNetAgentManager.RegisterMoNetAgent(&neutralAgent);
    moNetAgentManager.RegisterMoNetAgent(&mtnAgent);
    moNetAgentManager.RegisterMoNetAgent(&anyAgent);
    moNetAgentManager.Init();

    return oSUCCESS;
}

OStatus
MotionAgents::DoStart(const OSystemEvent& event)
{
    moNetAgentManager.Start(subject[sbjEffector]);
    motionAgentsState = MAS_START;

    ENABLE_ALL_SUBJECT;
    ASSERT_READY_TO_ALL_OBSERVER;

    return oSUCCESS;
}

OStatus
MotionAgents::DoStop(const OSystemEvent& event)
{
    motionAgentsState = MAS_IDLE;

    DISABLE_ALL_SUBJECT;
    DEASSERT_READY_TO_ALL_OBSERVER;

    return oSUCCESS;
}

OStatus
MotionAgents::DoDestroy(const OSystemEvent& event)
{
    DELETE_ALL_SUBJECT_AND_OBSERVER;
    return oSUCCESS;
}

void
MotionAgents::NotifyCommand(const ONotifyEvent& event)
{
    OSYSDEBUG(("MotionAgents::NotifyCommand()\n"));

    if (motionAgentsState == MAS_START) {

        MoNetAgentResult result;
        moNetAgentManager.NotifyCommand(event, &result);
        if (result.status != monetSUCCESS) {
            subject[sbjResult]->SetData(&result, sizeof(result));
            subject[sbjResult]->NotifyObservers();
        }

        observer[event.ObsIndex()]->AssertReady();
    }
}

void
MotionAgents::ReadyEffector(const OReadyEvent& event)
{
  //    OSYSDEBUG(("MotionAgents::ReadyEffector()\n"));

    if (motionAgentsState == MAS_START) {

        MoNetAgentResult result;
        moNetAgentManager.ReadyEffector(event, &result);
        if (result.status != monetSUCCESS) {
            subject[sbjResult]->SetData(&result, sizeof(result));
            subject[sbjResult]->NotifyObservers();
        }
    }
}

void
MotionAgents::NotifyPose(const ONotifyEvent& event)
{
    OSYSDEBUG(("MotionAgents::NotifyPose()\n"));

    if (motionAgentsState == MAS_START) {
        moNetAgentManager.NotifyPose(event);
        observer[event.ObsIndex()]->AssertReady();
    }
}
