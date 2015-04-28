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

#include <string.h>
#include <OPENR/OSyslog.h>
#include <OPENR/OPENRAPI.h>
#include <ant.h>
#include <EndpointTypes.h>
#include <TCPEndpointMsg.h>
#include <OPENR/core_macro.h>
#include "TCPReceptor.h"

TCPReceptor::TCPReceptor() : tcpReceptorState(TCPRS_IDLE),
			     connectionIndex(-1)
{
}

OStatus
TCPReceptor::DoInit(const OSystemEvent& event)
{
    OSYSPRINT(("TCPReceptor::DoInit()\n"));

    NEW_ALL_SUBJECT_AND_OBSERVER;
    REGISTER_ALL_ENTRY;
    SET_ALL_READY_AND_NOTIFY_ENTRY;

    return oSUCCESS;
}

OStatus
TCPReceptor::DoStart(const OSystemEvent& event)
{
    OSYSPRINT(("TCPReceptor::DoStart()\n"));

    ipstackRef = antStackRef("IPStack");

    for (int index = 0; index < TCPRECEPTOR_CONNECTION_MAX; index++) {
        OStatus result = InitTCPConnection(index);
        if (result != oSUCCESS) return oFAIL;
    }

    ENABLE_ALL_SUBJECT;
    ASSERT_READY_TO_ALL_OBSERVER;

    tcpReceptorState = TCPRS_LISTEN;

    for (int index = 0; index < TCPRECEPTOR_CONNECTION_MAX; index++) {
        Listen(index);
    }

    return oSUCCESS;
}

OStatus
TCPReceptor::DoStop(const OSystemEvent& event)
{
    OSYSPRINT(("TCPReceptor::DoStop()\n"));

    tcpReceptorState = TCPRS_IDLE;

    DISABLE_ALL_SUBJECT;
    DEASSERT_READY_TO_ALL_OBSERVER;

    return oSUCCESS;
}

OStatus
TCPReceptor::DoDestroy(const OSystemEvent& event)
{
    DELETE_ALL_SUBJECT_AND_OBSERVER;

    return oSUCCESS;
}

OStatus
TCPReceptor::Listen(int index)
{
    OSYSPRINT(("TCPReceptor::Listen()\n"));

    if (connection[index].state != CONNECTION_CLOSED) return oFAIL;

    //
    // Create endpoint
    //
    antEnvCreateEndpointMsg tcpCreateMsg(EndpointType_TCP,
                                         TCPRECEPTOR_BUFFER_SIZE * 2);
    tcpCreateMsg.Call(ipstackRef, sizeof(tcpCreateMsg));
    if (tcpCreateMsg.error != ANT_SUCCESS) {
        OSYSLOG1((osyslogERROR, "%s : %s[%d] antError %d",
                  "TCPReceptor::Listen()",
                  "Can't create endpoint",
                  index, tcpCreateMsg.error));
        return oFAIL;
    }
    connection[index].endpoint = tcpCreateMsg.moduleRef;

    //
    // Listen
    //
    TCPEndpointListenMsg listenMsg(connection[index].endpoint,
                                   IP_ADDR_ANY, TCPRECEPTOR_PORT);
    listenMsg.continuation = (void*)index;

    listenMsg.Send(ipstackRef, myOID_,
                   Extra_Entry[entryListenCont], sizeof(listenMsg));
    
    connection[index].state = CONNECTION_LISTENING;

    return oSUCCESS;
}

void
TCPReceptor::ListenCont(ANTENVMSG msg)
{
    OSYSPRINT(("TCPReceptor::ListenCont()\n"));

    TCPEndpointListenMsg* listenMsg
        = (TCPEndpointListenMsg*)antEnvMsg::Receive(msg);
    int index = (int)listenMsg->continuation;

    if (listenMsg->error != TCP_SUCCESS) {
        OSYSLOG1((osyslogERROR, "%s : %s %d",
                  "TCPReceptor::ListenCont()",
                  "FAILED. listenMsg->error", listenMsg->error));
        Close(index);
        return;
    }

    connection[index].state = CONNECTION_CONNECTED;

    SetReceiveData(index);
    Receive(index);
}

OStatus
TCPReceptor::Send(int index)
{
    OSYSPRINT(("TCPReceptor::Send %d bytes\n", connection[index].sendSize ));

    if (connection[index].sendSize == 0 ||
        connection[index].state != CONNECTION_CONNECTED) return oFAIL;

    TCPEndpointSendMsg sendMsg(connection[index].endpoint,
                               connection[index].sendData,
                               connection[index].sendSize);
    sendMsg.continuation = (void*)index;

    sendMsg.Send(ipstackRef, myOID_,
                 Extra_Entry[entrySendCont],
                 sizeof(TCPEndpointSendMsg));

    connection[index].state = CONNECTION_SENDING;
    connection[index].sendSize = 0;
    return oSUCCESS;
}

void
TCPReceptor::SendCont(ANTENVMSG msg)
{
    OSYSPRINT(("TCPReceptor::SendCont()\n"));

    TCPEndpointSendMsg* sendMsg = (TCPEndpointSendMsg*)antEnvMsg::Receive(msg);
    int index = (int)(sendMsg->continuation);

    if (sendMsg->error != TCP_SUCCESS) {
        OSYSLOG1((osyslogERROR, "%s : %s %d",
                  "TCPReceptor::SendCont()",
                  "FAILED. sendMsg->error", sendMsg->error));
        Close(index);
        return;
    }

    connection[index].state = CONNECTION_CONNECTED;
}

OStatus
TCPReceptor::Receive(int index)
{
    OSYSPRINT(("TCPReceptor::Receive()\n"));

    if (connection[index].state != CONNECTION_CONNECTED &&
        connection[index].state != CONNECTION_SENDING) return oFAIL;

    TCPEndpointReceiveMsg receiveMsg(connection[index].endpoint,
                                     connection[index].recvData,
                                     1, connection[index].recvSize);
    receiveMsg.continuation = (void*)index;

    receiveMsg.Send(ipstackRef, myOID_,
                    Extra_Entry[entryReceiveCont], sizeof(receiveMsg));

    return oSUCCESS;
}

void
TCPReceptor::ReceiveCont(ANTENVMSG msg)
{
    OSYSPRINT(("TCPReceptor::ReceiveCont()\n"));

    TCPEndpointReceiveMsg* receiveMsg
        = (TCPEndpointReceiveMsg*)antEnvMsg::Receive(msg);
    int index = (int)(receiveMsg->continuation);

    if (receiveMsg->error != TCP_SUCCESS) {
	OSYSPRINT(( "Connection closed(%d)\n", receiveMsg->error ));
        Close(index);
        return;
    }

    connection[index].receiveNext = true;
    OSYSPRINT(( "sizeMin=%d\n", receiveMsg->sizeMin ));
    unsigned char *p = (unsigned char *)connection[index].recvData;
    OSYSPRINT(( "sizeMin=%d,(%d,%d,%d,%d)\n",
		receiveMsg->sizeMin,
		p[0], p[1], p[2], p[3] ));
    int size = (0xFF & p[3]) + (0xFF & p[2]) * 256;
    static char tmpbuf[256];
    memcpy(tmpbuf, &p[4], size);
    tmpbuf[size]=0;
    ParseMessage(tmpbuf);

    connection[index].state = CONNECTION_CONNECTED;
    SetReceiveData(index);
    Receive(index);
}

OStatus
TCPReceptor::Close(int index)
{
    OSYSPRINT(("TCPReceptor::Close()\n"));

    if (connection[index].state == CONNECTION_CLOSED ||
        connection[index].state == CONNECTION_CLOSING) return oFAIL;

    TCPEndpointCloseMsg closeMsg(connection[index].endpoint);
    closeMsg.continuation = (void*)index;

    closeMsg.Send(ipstackRef, myOID_,
                  Extra_Entry[entryCloseCont], sizeof(closeMsg));

    connection[index].state = CONNECTION_CLOSING;

    return oSUCCESS;
}

void
TCPReceptor::CloseCont(ANTENVMSG msg)
{
    OSYSPRINT(("TCPReceptor::CloseCont()\n"));

    TCPEndpointCloseMsg* closeMsg
        = (TCPEndpointCloseMsg*)antEnvMsg::Receive(msg);
    int index = (int)(closeMsg->continuation);

    connection[index].state = CONNECTION_CLOSED;
    Listen(index);
}

OStatus
TCPReceptor::InitTCPConnection(int index)
{
    OSYSPRINT(("TCPReceptor::InitTCPConnection()\n"));

    connection[index].state = CONNECTION_CLOSED;

    // 
    // Allocate send buffer
    //
    antEnvCreateSharedBufferMsg sendBufferMsg(TCPRECEPTOR_BUFFER_SIZE);

    sendBufferMsg.Call(ipstackRef, sizeof(sendBufferMsg));
    if (sendBufferMsg.error != ANT_SUCCESS) {
        OSYSLOG1((osyslogERROR, "%s : %s[%d] antError %d",
                  "TCPReceptor::InitTCPConnection()",
                  "Can't allocate send buffer",
                  index, sendBufferMsg.error));
        return oFAIL;
    }

    connection[index].sendBuffer = sendBufferMsg.buffer;
    connection[index].sendBuffer.Map();
    connection[index].sendData
        = (byte*)(connection[index].sendBuffer.GetAddress());

    //
    // Allocate receive buffer
    //
    antEnvCreateSharedBufferMsg recvBufferMsg(TCPRECEPTOR_BUFFER_SIZE);

    recvBufferMsg.Call(ipstackRef, sizeof(recvBufferMsg));
    if (recvBufferMsg.error != ANT_SUCCESS) {
        OSYSLOG1((osyslogERROR, "%s : %s[%d] antError %d",
                  "TCPReceptor::InitTCPConnection()",
                  "Can't allocate receive buffer",
                  index, recvBufferMsg.error));
        return oFAIL;
    }

    connection[index].recvBuffer = recvBufferMsg.buffer;
    connection[index].recvBuffer.Map();
    connection[index].recvData
        = (byte*)(connection[index].recvBuffer.GetAddress());

    return oSUCCESS;
}

void
TCPReceptor::SetSendData(int index)
{
    memcpy(connection[index].sendData,
           connection[index].recvData, connection[index].sendSize);
}

void
TCPReceptor::SetReceiveData(int index)
{
    connection[index].recvSize = TCPRECEPTOR_BUFFER_SIZE;
}

void
TCPReceptor::SensorUpdate(const char *msg)
{
  for (int index = 0; index < TCPRECEPTOR_CONNECTION_MAX; index++) {
    SensorUpdate(index, msg);
  }
}

OStatus
TCPReceptor::SensorUpdate(int index, const char *msg)
{
  if (connection[index].state != CONNECTION_CONNECTED) return oFAIL;
  OSYSPRINT(( "SensorUpdate(%d,%s)\n", index, msg ));
  int len = strlen(msg);
  byte *p = connection[index].sendData;
  p[0] = 0;
  p[1] = 0;
  p[2] = len / 256;
  p[3] = len % 256;
  memcpy( &p[4], msg, len );
  connection[index].sendSize = len + 4;
  Send(index);
  return oSUCCESS;
}
