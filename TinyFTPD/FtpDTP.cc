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

#include <OPENR/OSyslog.h>
#include <OPENR/ODebug.h>
#include "FtpDTP.h"

FtpDTP::FtpDTP()
{
}

OStatus
FtpDTP::Initialize(const OID& myoid, const antStackRef& ipstack, void* index)
{
    OSYSDEBUG(("FtpDTP::Initialize()\n"));

    myOID        = myoid;
    ipstackRef   = ipstack;
    continuation = index;

    // initialize FTP connection
    method   = FTP_METHOD_NOOP;
    dataType = FTP_DATA_A;
    passive  = false;
    connectPort = FTP_DATA_PORT;

    // initialize File System
    ResetFilename();
    strcpy(ftpDir, "/MS/");

    connection.state = CONNECTION_CLOSED;

    // 
    // Allocate send buffer
    //
    antEnvCreateSharedBufferMsg sendBufferMsg(FTP_BUFFER_SIZE);

    sendBufferMsg.Call(ipstackRef, sizeof(sendBufferMsg));
    if (sendBufferMsg.error != ANT_SUCCESS) {
        OSYSLOG1((osyslogERROR, "%s : %s[%d] antError %d",
                  "FtpDTP::Initialize()",
                  "Can't allocate send buffer",
                  index, sendBufferMsg.error));
        return oFAIL;
    }

    connection.sendBuffer = sendBufferMsg.buffer;
    connection.sendBuffer.Map();
    connection.sendData = (byte*)(connection.sendBuffer.GetAddress());

    //
    // Allocate receive buffer
    //
    antEnvCreateSharedBufferMsg recvBufferMsg(FTP_BUFFER_SIZE);

    recvBufferMsg.Call(ipstackRef, sizeof(recvBufferMsg));
    if (recvBufferMsg.error != ANT_SUCCESS) {
        OSYSLOG1((osyslogERROR, "%s : %s[%d] antError %d",
                  "FtpDTP::Initialize()",
                  "Can't allocate receive buffer",
                  index, recvBufferMsg.error));
        return oFAIL;
    }

    connection.recvBuffer = recvBufferMsg.buffer;
    connection.recvBuffer.Map();
    connection.recvData = (byte*)(connection.recvBuffer.GetAddress());

    return oSUCCESS;
}

OStatus
FtpDTP::Listen()
{
    OSYSDEBUG(("FtpDTP::Listen()\n"));

    if (connection.state != CONNECTION_CLOSED) return oFAIL;

    //
    // Create endpoint
    //
    antEnvCreateEndpointMsg tcpCreateMsg(EndpointType_TCP,
                                         FTP_BUFFER_SIZE * 2);

    tcpCreateMsg.Call(ipstackRef, sizeof(tcpCreateMsg));
    if (tcpCreateMsg.error != ANT_SUCCESS) {
        OSYSLOG1((osyslogERROR, "%s : %s[%d] antError %d",
                  "FtpDTP::Listen()",
                  "Can't create endpoint",
                  (int)continuation, tcpCreateMsg.error));
        return oFAIL;
    }
    connection.endpoint = tcpCreateMsg.moduleRef;

    //
    // Listen
    //
    TCPEndpointListenMsg listenMsg(connection.endpoint,
                                   IP_ADDR_ANY, connectPort);
    listenMsg.continuation = continuation;

    listenMsg.Send(ipstackRef, myOID,
                   Extra_Entry[entryListenContforDTP], sizeof(listenMsg));
    
    connection.state = CONNECTION_LISTENING;

    return oSUCCESS;
}

OStatus
FtpDTP::Connect()
{
    OSYSDEBUG(("FtpDTP::Connect()\n"));

    if (connection.state != CONNECTION_CLOSED) return oFAIL;

    //
    // Create endpoint
    //
    antEnvCreateEndpointMsg tcpCreateMsg(EndpointType_TCP,
                                         FTP_BUFFER_SIZE * 2);

    tcpCreateMsg.Call(ipstackRef, sizeof(tcpCreateMsg));
    if (tcpCreateMsg.error != ANT_SUCCESS) {
        OSYSLOG1((osyslogERROR, "%s : %s[%d] antError %d",
                  "FtpDTP::Connect()",
                  "Can't create endpoint",
                  (int)continuation, tcpCreateMsg.error));
        return oFAIL;
    }
    connection.endpoint = tcpCreateMsg.moduleRef;

    //
    // Connect
    //
    TCPEndpointConnectMsg connectMsg(connection.endpoint,
                                     0, FTP_DATA_PORT,
                                     connectIP, connectPort);
    connectMsg.continuation = continuation;

    connectMsg.Send(ipstackRef, myOID,
                    Extra_Entry[entryConnectContforDTP], sizeof(connectMsg));
    
    connection.state = CONNECTION_CONNECTING;

    return oSUCCESS;
}

bool
FtpDTP::ListenCont(TCPEndpointListenMsg* listenMsg)
{
    OSYSDEBUG(("FtpDTP::ListenCont() %x %d - %x %d\n",
               listenMsg->lAddress,
               listenMsg->lPort,
               listenMsg->fAddress,
               listenMsg->fPort
               ));

    if (listenMsg->error != TCP_SUCCESS) {
        OSYSLOG1((osyslogERROR, "%s : %s %d",
                  "FtpDTP::ListenCont()",
                  "FAILED. listenMsg->error", listenMsg->error));
        Close();
        return false;
    }

    connection.state = CONNECTION_CONNECTED;

    switch (method) {
    case FTP_METHOD_RETR:
        RetrieveSend();
        break;

    case FTP_METHOD_STOR:
        connection.recvSize = 0;
        Receive();
        break;

    case FTP_METHOD_LIST:
        if (!ListSend()) {
            Close();
        }
        break;
    default:
        OSYSDEBUG(("not yet\n"));
        break;
    }

    return true;
}

bool
FtpDTP::ConnectCont(TCPEndpointConnectMsg* connectMsg)
{
    OSYSDEBUG(("FtpDTP::ConnectCont()\n"));

    if (connectMsg->error != TCP_SUCCESS) {
        OSYSLOG1((osyslogERROR, "%s : %s %d",
                  "FtpDTP::ConnectCont()",
                  "FAILED. connectMsg->error", connectMsg->error));
        Close();
        return false;
    }

    connection.state = CONNECTION_CONNECTED;

    switch (method) {
    case FTP_METHOD_RETR:
        RetrieveSend();
        break;

    case FTP_METHOD_STOR:
        connection.recvSize = 0;
        Receive();
        break;

    case FTP_METHOD_LIST:
        if (!ListSend()) {
            Close();
        }
        break;
    }
    return true;
}

OStatus
FtpDTP::Send()
{
    OSYSDEBUG(("FtpDTP::Send() "));

    if (connection.state != CONNECTION_CONNECTED) return oFAIL;

    OSYSDEBUG(("%d\n", connection.sendSize));

    TCPEndpointSendMsg sendMsg(connection.endpoint,
                               connection.sendData,
                               connection.sendSize);
    sendMsg.continuation = continuation;

    sendMsg.Send(ipstackRef, myOID,
                 Extra_Entry[entrySendContforDTP],
                 sizeof(TCPEndpointSendMsg));

    connection.state = CONNECTION_SENDING;
    connection.sendSize = 0;
    return oSUCCESS;
}

bool
FtpDTP::SendCont(TCPEndpointSendMsg* sendMsg)
{
    OSYSDEBUG(("FtpDTP::SendCont()\n"));

    if (sendMsg->error != TCP_SUCCESS) {
        if (sendMsg->error == TCP_BUFFER_INVALID
            && method == FTP_METHOD_NOOP) {
            Close();
            return true;
        } else {
            OSYSLOG1((osyslogERROR, "%s : %s %d",
                      "FtpDTP::SendCont()",
                      "FAILED. sendMsg->error", sendMsg->error));
            Close();
            return false;
        }
    }

    connection.state = CONNECTION_CONNECTED;

    switch (method) {
    case FTP_METHOD_RETR:
        if (!RetrieveSend()) {
            Close();
            return true;
        }
        break;

    case FTP_METHOD_LIST:
        if (!ListSend()) {
            Close();
            return true;
        }
        break;

    default :
        Close();
        return true;
    }
    return false;
}

OStatus
FtpDTP::Receive()
{

    if (connection.state != CONNECTION_CONNECTED
        && connection.state != CONNECTION_SENDING) return oFAIL;

    OSYSDEBUG(("FtpDTP::Receive()\n"));

    TCPEndpointReceiveMsg receiveMsg(connection.endpoint,
                                     connection.recvData,
                                     1, FTP_BUFFER_SIZE);
    receiveMsg.continuation = continuation;

    receiveMsg.Send(ipstackRef, myOID,
                    Extra_Entry[entryReceiveContforDTP], sizeof(receiveMsg));

    connection.state = CONNECTION_RECEIVING;
    return oSUCCESS;
}

bool
FtpDTP::ReceiveCont(TCPEndpointReceiveMsg* receiveMsg)
{
    OSYSDEBUG(("FtpDTP::ReceiveCont()\n"));

    connection.recvSize = receiveMsg->sizeMin;
    if (receiveMsg->error == TCP_CONNECTION_CLOSED) {
        Save(connection.recvData, 0);
        Close();
        return true;
    }

    if (receiveMsg->error != TCP_SUCCESS) {
        OSYSLOG1((osyslogERROR, "%s : %s %d",
                  "FtpDTP::ReceiveCont()",
                  "FAILED. receiveMsg->error", receiveMsg->error));
        Close();
        return false;
    }

    // receive some more
    Save(connection.recvData, receiveMsg->sizeMin, false);
    connection.state = CONNECTION_CONNECTED;
    connection.recvSize = 0;
    Receive();

    return false;
}

OStatus
FtpDTP::Close()
{
    OSYSDEBUG(("FtpDTP::Close() %d\n", connection.state));

    if (connection.state == CONNECTION_CLOSED
        || connection.state == CONNECTION_CLOSING) return oFAIL;

    TCPEndpointCloseMsg closeMsg(connection.endpoint);
    closeMsg.continuation = continuation;

    closeMsg.Send(ipstackRef, myOID,
                  Extra_Entry[entryCloseContforDTP], sizeof(closeMsg));

    connection.state = CONNECTION_CLOSING;
    ResetFilename();
    method   = FTP_METHOD_NOOP;
    dataType = FTP_DATA_A;
    passive  = false;
    connectPort = FTP_DATA_PORT;

    return oSUCCESS;
}

void
FtpDTP::CloseCont(TCPEndpointCloseMsg* closeMsg)
{
    OSYSDEBUG(("FtpDTP::CloseCont()\n"));
    
    connection.state = CONNECTION_CLOSED;
}

