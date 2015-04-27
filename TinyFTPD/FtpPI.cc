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

#include <stdarg.h>
#include <OPENR/OSyslog.h>
#include <EndpointTypes.h>
#include <TCPEndpointMsg.h>
#include "FtpPI.h"

FtpPI::FtpPI()
{
}

OStatus
FtpPI::Initialize(const OID& myoid, const antStackRef& ipstack,
                  void* index, OList<Passwd, MAX_LOGIN> *pass)
{
    OSYSDEBUG(("FtpPI::Initialize()\n"));

    myOID        = myoid;
    ipstackRef   = ipstack;
    continuation = index;
    passwd       = pass;
 
    state = FTP_NOT_LOGIN;
    connection.state = CONNECTION_CLOSED;

    // 
    // Allocate send buffer
    //
    antEnvCreateSharedBufferMsg sendBufferMsg(FTP_BUFFER_SIZE);

    sendBufferMsg.Call(ipstackRef, sizeof(sendBufferMsg));
    if (sendBufferMsg.error != ANT_SUCCESS) {
        OSYSLOG1((osyslogERROR, "%s : %s[%d] antError %d",
                  "FtpPI::Initialize()",
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
                  "FtpPI::Initialize()",
                  "Can't allocate receive buffer",
                  index, recvBufferMsg.error));
        return oFAIL;
    }

    connection.recvBuffer = recvBufferMsg.buffer;
    connection.recvBuffer.Map();
    connection.recvData = (byte*)(connection.recvBuffer.GetAddress());
    
    ftpDTP.Initialize(myOID, ipstackRef, index);
    Listen();

    return oSUCCESS;
}

OStatus
FtpPI::Listen()
{
    OSYSDEBUG(("FtpPI::Listen()\n"));

    if (connection.state != CONNECTION_CLOSED) return oFAIL;

    //
    // Create endpoint
    //
    antEnvCreateEndpointMsg tcpCreateMsg(EndpointType_TCP,
                                         FTP_BUFFER_SIZE * 2);

    tcpCreateMsg.Call(ipstackRef, sizeof(tcpCreateMsg));
    if (tcpCreateMsg.error != ANT_SUCCESS) {
        OSYSLOG1((osyslogERROR, "%s : %s[%d] antError %d",
                  "FtpPI::Listen()",
                  "Can't create endpoint",
                  (int)continuation, tcpCreateMsg.error));
        return oFAIL;
    }
    connection.endpoint = tcpCreateMsg.moduleRef;

    //
    // Listen
    //
    TCPEndpointListenMsg listenMsg(connection.endpoint,
                                   IP_ADDR_ANY, FTP_LISTEN_PORT);
    listenMsg.continuation = continuation;

    listenMsg.Send(ipstackRef, myOID,
                   Extra_Entry[entryListenContforPI], sizeof(listenMsg));
    
    connection.state = CONNECTION_LISTENING;

    return oSUCCESS;
}

void
FtpPI::ListenCont(TCPEndpointListenMsg* listenMsg)
{
    OSYSDEBUG(("FtpPI::ListenCont() %x %d - %x %d\n",
               listenMsg->lAddress,
               listenMsg->lPort,
               listenMsg->fAddress,
               listenMsg->fPort));

    if (listenMsg->error != TCP_SUCCESS) {
        OSYSLOG1((osyslogERROR, "%s : %s %d",
                  "FtpPI::ListenCont()",
                  "FAILED. listenMsg->error", listenMsg->error));
        Close();
        return;
    }

    ipaddr = listenMsg->lAddress;

    connection.state = CONNECTION_CONNECTED;
    Send(FTP_REPLY_SERVICE_READY, "AIBO FTP Server ready");
    connection.recvSize = 0;
    Receive();
}

OStatus
FtpPI::Send(FTPReplyCode code, char *format, ...)
{
    OSYSDEBUG(("FtpPI::Send()\n"));
    
    if (connection.state != CONNECTION_CONNECTED) return oFAIL;

    char buf[MAX_STRING_LENGTH];
    va_list ap;
    va_start(ap, format);
    vsprintf(buf, format, ap);
    va_end(ap);

    sprintf((char *)connection.sendData, "%03d %s\r\n", code, buf);

    TCPEndpointSendMsg sendMsg(connection.endpoint,
                               connection.sendData,
                               strlen((char *)connection.sendData));
    sendMsg.continuation = continuation;

    sendMsg.Send(ipstackRef, myOID,
                 Extra_Entry[entrySendContforPI], sizeof(sendMsg));

    connection.state = CONNECTION_SENDING;
    connection.sendSize = 0;
    return oSUCCESS;
}

void
FtpPI::SendCont(TCPEndpointSendMsg* sendMsg)
{
    OSYSDEBUG(("FtpPI::SendCont()\n"));

    if (sendMsg->error != TCP_SUCCESS) {
        OSYSLOG1((osyslogERROR, "%s : %s %d",
                  "FtpPI::SendCont()",
                  "FAILED. sendMsg->error", sendMsg->error));
        Close();
        return;
    }

    connection.state = CONNECTION_CONNECTED;
}

OStatus
FtpPI::Receive()
{
    OSYSDEBUG(("FtpPI::Receive()\n"));

    if (connection.state != CONNECTION_CONNECTED
        && connection.state != CONNECTION_SENDING) return oFAIL;

    TCPEndpointReceiveMsg receiveMsg(connection.endpoint,
                                     connection.recvData,
                                     1,
                                     FTP_BUFFER_SIZE);
    receiveMsg.continuation = continuation;
    receiveMsg.Send(ipstackRef, myOID,
                    Extra_Entry[entryReceiveContforPI], sizeof(receiveMsg));
    return oSUCCESS;
}

void
FtpPI::ReceiveCont(TCPEndpointReceiveMsg* receiveMsg)
{
    bool doReceive = false;

    if (receiveMsg->error == TCP_CONNECTION_CLOSED) {
        Close();
        return;
    }

    if (receiveMsg->error != TCP_SUCCESS) {
        OSYSLOG1((osyslogERROR, "%s : %s %d",
                  "FtpPI::ReceiveCont()",
                  "FAILED. receiveMsg->error", receiveMsg->error));
        Close();
        return;
    }

    connection.recvSize += receiveMsg->sizeMin;
    
    if (RequestComplete()) {
        RequestProcess();
    } else {
        OSYSDEBUG(("FtpPI::Request not complete\n"));
        doReceive = true;
    }
    
    // receive some more
    if (doReceive) {
        if (connection.recvSize < FTP_BUFFER_SIZE) {
            Receive();
        } else {
            Close();
        }
    }
}

OStatus
FtpPI::Close()
{
    OSYSDEBUG(("FtpPI::Close()\n"));

    if (connection.state == CONNECTION_CLOSED
        || connection.state == CONNECTION_CLOSING) return oFAIL;

    TCPEndpointCloseMsg closeMsg(connection.endpoint);
    closeMsg.continuation = continuation;

    closeMsg.Send(ipstackRef, myOID,
                  Extra_Entry[entryCloseContforPI], sizeof(closeMsg));

    connection.state = CONNECTION_CLOSING;
    ftpDTP.Close();

    return oSUCCESS;
}

void
FtpPI::CloseCont(TCPEndpointCloseMsg* closeMsg)
{
    OSYSDEBUG(("FtpPI::CloseCont()\n"));
    
    connection.state = CONNECTION_CLOSED;
    Listen();
}

void
FtpPI::ListenContforDTP(TCPEndpointListenMsg* listenMsg)
{
    if (listenMsg->error == TCP_SUCCESS) {
        switch(ftpDTP.GetType()) {
        case FTP_DATA_I:
            Send(FTP_REPLY_OPEN_CONNECTION,
                 "Binary data connection for %s.",
                 ftpDTP.GetFilename());
            break;

        case FTP_DATA_A:
            Send(FTP_REPLY_OPEN_CONNECTION,
                 "ASCII data connection for %s.",
                 ftpDTP.GetFilename());
            break;
        }
    }
    ftpDTP.ListenCont(listenMsg);
    return;
}

void
FtpPI::ConnectContforDTP(TCPEndpointConnectMsg* connectMsg)
{
    if (connectMsg->error == TCP_SUCCESS) {
        switch(ftpDTP.GetType()) {
        case FTP_DATA_I:
            Send(FTP_REPLY_OPEN_CONNECTION,
                 "Binary data connection for %s.",
                 ftpDTP.GetFilename());
            break;

        case FTP_DATA_A:
            Send(FTP_REPLY_OPEN_CONNECTION,
                 "ASCII data connection for %s.",
                 ftpDTP.GetFilename());
            break;
        }
    }

    ftpDTP.ConnectCont(connectMsg);
    return;
}

void
FtpPI::SendContforDTP(TCPEndpointSendMsg* sendMsg)
{
    if(ftpDTP.SendCont(sendMsg)) {
        Send(FTP_REPLY_CLOSE_DATA, "Transfer complete.");
        connection.recvSize = 0;
        Receive();
    } else if (ftpDTP.GetState() != CONNECTION_SENDING) {
        Send(FTP_REPLY_TRANSFER_ABORT, "Transfer abort.");
        connection.recvSize = 0;
        Receive();
    }
}

void
FtpPI::ReceiveContforDTP(TCPEndpointReceiveMsg* receiveMsg)
{
    if (ftpDTP.ReceiveCont(receiveMsg)) {
        Send(FTP_REPLY_CLOSE_DATA, "Transfer complete.");
        connection.recvSize = 0;
        Receive();
    } else if (ftpDTP.GetState() != CONNECTION_RECEIVING) {
        Send(FTP_REPLY_TRANSFER_ABORT, "Transfer abort.");
        connection.recvSize = 0;
        Receive();
    }
}

void
FtpPI::CloseContforDTP(TCPEndpointCloseMsg* closeMsg)
{
    ftpDTP.CloseCont(closeMsg);
}
