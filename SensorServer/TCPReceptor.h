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

#ifndef TCPReceptor_h_DEFINED
#define TCPReceptor_h_DEFINED

#include <OPENR/OObject.h>
#include <OPENR/OSubject.h>
#include <OPENR/OObserver.h>
#include "TCPConnection.h"
#include "TCPReceptorConfig.h"
#include "ControlInfo.h"
#include "VisionInfo.h"
#include "def.h"
#include <list>

enum TCPReceptorState {
    TCPRS_IDLE,
    TCPRS_LISTEN,
};

class TCPReceptor : public OObject {
public:
    TCPReceptor();
    virtual ~TCPReceptor() {}

    OSubject*   subject[numOfSubject];
    OObserver*  observer[numOfObserver];     

    virtual OStatus DoInit   (const OSystemEvent& event);
    virtual OStatus DoStart  (const OSystemEvent& event);
    virtual OStatus DoStop   (const OSystemEvent& event);
    virtual OStatus DoDestroy(const OSystemEvent& event);

    void ListenCont (ANTENVMSG msg);
    void SendCont   (ANTENVMSG msg);
    void ReceiveCont(ANTENVMSG msg);
    void CloseCont  (ANTENVMSG msg);

    void NotifyObjInfo( const ONotifyEvent& event );

private:
    OStatus Listen (int index);
    OStatus Send   (int index);
    OStatus Receive(int index);
    OStatus Close  (int index);
    OStatus InitTCPConnection(int index);
    void ParseMessage(const char *str);
    void ParseBroadcast(const char *str);
    void ParseSensorUpdate(const char *str);
    void SendMoNet(int monet);
    void SendHead(int hcmd, int arg);
    void SensorUpdate(const char *msg);
    OStatus SensorUpdate(int index, const char *msg);

    void SetSendData(int index);
    void SetReceiveData(int index);

    // -----
    antStackRef ipstackRef;
    TCPConnection connection[TCPRECEPTOR_CONNECTION_MAX];
    TCPReceptorState tcpReceptorState;

    int connectionIndex;
    enum EAiboImageType imageType;
};

#endif // TCPReceptor_h_DEFINED
