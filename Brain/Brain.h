//
// Copyright 2005 (C) Eiichiro ITO, GHC02331@nifty.com
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
// Eiichiro ITO, 15 October 2005
// mailto: GHC02331@nifty.com
//

#ifndef Brain_h_DEFINED
#define Brain_h_DEFINED

#include <OPENR/OObject.h>
#include <OPENR/OSubject.h>
#include <OPENR/OObserver.h>
#include "def.h"
#include <MoNetData.h>
#include <list>
#include "VisionInfo.h"
#include "CdtFile.h"
#include "STNHolder.h"
#include "Perception.h"
#include "Sensor.h"
#include "Memory.h"
#include "Logging.h"

enum BrainState {
    BRAIN_IDLE,
    BRAIN_START,
    BRAIN_WAITING_RESULT,
    BRAIN_MONET,
    BRAIN_STN
};

static const char* const DEFART_CONFIG  = "/MS/OPEN-R/MW/CONF/DEFART.CFG";
const MoNetCommandID SLEEP2SLEEP_NULL = 0; // see /OPEN-R/MW/CONF/MONETCMD.CFG

class Brain : public OObject {
public:
    Brain();
    virtual ~Brain() {}

    OSubject*   subject[numOfSubject];
    OObserver*  observer[numOfObserver];     

    virtual OStatus DoInit   (const OSystemEvent& event);
    virtual OStatus DoStart  (const OSystemEvent& event);
    virtual OStatus DoStop   (const OSystemEvent& event);
    virtual OStatus DoDestroy(const OSystemEvent& event);

    void ReadyCommand(const OReadyEvent& event);
    void NotifyResult(const ONotifyEvent& event);
    void NotifyObjInfo(const ONotifyEvent& event);
    void NotifySensor(const ONotifyEvent& event);
    void NotifyControl( const ONotifyEvent& event );
    void NotifyMessage( const ONotifyEvent& event );
    int GetHcmdSeqno() const;
    Memory		memory;
    Logging		logging;

private:
    void OpenPrimitive();
    void SetCamCPCPrimitives();

    void StartProgram( int stnNo );
    void StopProgram();
    bool ProcessProgram();
    void MonetExecute( MoNetCommandID cmdID );
    void MonetExecuteDirect( MoNetCommandID cmdID );
    void HeadExecute( int hcmd, int iValue, const HeadingPosition *hpos );
    void FaceExecute( int fcmd );
    void InternalExecute( int icmd );
    void SendCurrentInfo();
    void ConfigFromString( const char *buf );
    bool ReadDefartConfig( const char *path );
    void ClearLastCmds();

    // -----
    OPrimitiveID        fbkID;
    CdtFile             cdtfile;
    BrainState          brainState;
    list<MoNetCommandID> commandQueue;
    bool                waitingResult;
    Perception          perception;
    STNHolder           stnHolder;
    Sensor              sensor;
    int                 verboseMode;
    int			hcmdSeqno;
    STNCmds		lastCmds;
    int                 camWhiteBalance;
    int                 camShutterSpeed;
    int                 camGain;
};

struct MessageInfo {
    int msgId;
};

#endif // Brain_h_DEFINED
