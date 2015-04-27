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

#ifndef MoNet_h_DEFINED
#define MoNet_h_DEFINED

#include <OPENR/OObject.h>
#include <OPENR/OSubject.h>
#include <OPENR/OObserver.h>
#include <OPENR/ODataArchive.h>
#include <MoNetData.h>
#include <ODA.h>
#include "MoNetCommandInfoManager.h"
#include "DirectedGraph.h"
#include "CommandNode.h"
#include "CommandArc.h"
#include "def.h"
#include "Posture.h"

enum MoNetState {
    MNS_IDLE,
    MNS_START,
    MNS_AGENT_RUNNING
};

class MoNet : public OObject {
public:
    MoNet();
    virtual ~MoNet() {}

    OSubject*  subject[numOfSubject];
    OObserver* observer[numOfObserver];

    virtual OStatus DoInit   (const OSystemEvent& event);
    virtual OStatus DoStart  (const OSystemEvent& event);
    virtual OStatus DoStop   (const OSystemEvent& event);
    virtual OStatus DoDestroy(const OSystemEvent& event);

    void NotifyClientCommand(const ONotifyEvent& event);
    void NotifyAgentResult(const ONotifyEvent& event);

private:
    void LoadMotionODA();
    void LoadSoundODA();
    void ReadMoNetConfig(const char* path);
    void ReadMoNetCommandConfig(const char* path);

    MoNetAgentID ToMoNetAgentID(char* agt);
    int          ToCommandIndex(const ODA& oda, char* cmd, char* idx);
    MoNetPosture ToStartPosture(char* cmd);
    MoNetPosture ToEndPosture(char* cmd);

    MoNetPosture ToPosture(char* pos);

    MoNetCommandID Execute(MoNetCommandID id);
    void ExecuteCommandInfo(MoNetCommandInfo* cinfo);
    int  NewAndDivideSyncKey(int ndivkey, OVRSyncKey* divkey);

    void ReplyClientResult(MoNetCommandID id,
                           MoNetStatus st, MoNetPosture pos);

    static const size_t LINEBUFSIZE = 256;
    static const size_t NUMBUFSIZE  = 8;

    MoNetState moNetState;
    MoNetCommandInfoManager moNetCommandInfoManager;
    DirectedGraph<CommandNode, CommandArc> moNet;
    list<CommandArc> commandArcPath;
    MoNetCommandID currentCommandID;
    MoNetPosture   currentPosture;
    ODesignDataID  motionDataID;
    ODesignDataID  soundDataID;
    ODA motionODA;
    ODA soundODA;
    Posture posture;
};

#endif // MoNet_h_DEFINED
