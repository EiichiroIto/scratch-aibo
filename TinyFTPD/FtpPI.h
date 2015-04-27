//
// Copyright 2002,2003 Sony Corporation 
//
// Permission to use, copy, modify, and redistribute this software for
// non-commercial use is hereby granted.
//
// This software is provided "as is" without warranty of any kind,
// either expressed or implied, including but not limited to the
// implied warranties of fitness for a particular purpose.
//

#ifndef _FtpPI_h_DEFINED
#define _FtpPI_h_DEFINED

#include <OPENR/OList.h>
#include "TCPConnection.h"
#include "FtpConfig.h"
#include "FtpDTP.h"
#include "def.h"
#include "entry.h"

class FtpPI
{
public:
    FtpPI();
    virtual ~FtpPI() {}

    OStatus Initialize(const OID& myoid, const antStackRef& ipstack,
                       void* index, OList<Passwd, MAX_LOGIN> *pass);

    void ListenCont  (TCPEndpointListenMsg*  listenMsg);
    void SendCont    (TCPEndpointSendMsg*    sendMsg);
    void ReceiveCont (TCPEndpointReceiveMsg* receiveMsg);
    void CloseCont   (TCPEndpointCloseMsg*   closeMsg);

    void ListenContforDTP (TCPEndpointListenMsg*  listenMsg);
    void ConnectContforDTP(TCPEndpointConnectMsg* connectMsg);
    void SendContforDTP   (TCPEndpointSendMsg*    sendMsg);
    void ReceiveContforDTP(TCPEndpointReceiveMsg* receiveMsg);
    void CloseContforDTP  (TCPEndpointCloseMsg*   closeMsg);

    OStatus Close  ();

private:
    OStatus Listen ();
    OStatus Send   (FTPReplyCode, char *format, ...);
    OStatus Receive();

    bool RequestComplete();
    bool RequestProcess ();
    bool CommandParser  (char **cmd, char **param);

    OID myOID;
    antStackRef ipstackRef;
    void* continuation;
    TCPConnection connection;
    FTPLoginState state;
    FtpDTP ftpDTP;
    IPAddress ipaddr;

    OList<Passwd, MAX_LOGIN> *passwd;
};
#endif /* _FtpPI_h_DEFINED */

