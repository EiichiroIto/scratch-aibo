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

#include <sys/stat.h>
#include <OPENR/OSyslog.h>
#include <EndpointTypes.h>
#include <TCPEndpointMsg.h>
#include "TinyFTPD.h"

TinyFTPD::TinyFTPD() : passwd()
{
}

OStatus
TinyFTPD::DoInit(const OSystemEvent& event)
{
    OSYSDEBUG(("TinyFTPD::DoInit()\n"));
    return oSUCCESS;
}

OStatus
TinyFTPD::DoStart(const OSystemEvent& event)
{
    OSYSDEBUG(("TinyFTPD::DoStart()\n"));

    ipstackRef = antStackRef("IPStack");

    // Get Passwd
    OStatus status = LoadPasswd();
    if (oSUCCESS != status) {
        OSYSLOG1((osyslogERROR, "TinyFTPD::Load Fail %d", status));
        return oFAIL;
    }

    // initialize connection
    for(int index = 0; index < FTP_CONNECTION_MAX; index++) {
        ftpPI[index].Initialize(myOID_, ipstackRef, (void*)index, &passwd);
    }

    return oSUCCESS;
}

OStatus
TinyFTPD::DoStop(const OSystemEvent& event)
{
    OSYSDEBUG(("TinyFTPD::DoStop()\n"));
    for(int index = 0; index < FTP_CONNECTION_MAX; index++) {
        ftpPI[index].Close();
    }
    return oSUCCESS;
}

OStatus
TinyFTPD::DoDestroy(const OSystemEvent& event)
{
    OSYSDEBUG(("TinyFTPD::DoDestroy()\n"));
    return oSUCCESS;
}

void
TinyFTPD::ListenContforPI(ANTENVMSG msg)
{
    TCPEndpointListenMsg* listenMsg
        = (TCPEndpointListenMsg*)antEnvMsg::Receive(msg);
    int index = (int)(listenMsg->continuation);

    ftpPI[index].ListenCont(listenMsg);
}

void
TinyFTPD::SendContforPI(ANTENVMSG msg)
{
    TCPEndpointSendMsg* sendMsg = (TCPEndpointSendMsg*)antEnvMsg::Receive(msg);
    int index = (int)(sendMsg->continuation);

    ftpPI[index].SendCont(sendMsg);
}

void
TinyFTPD::ReceiveContforPI(ANTENVMSG msg)
{
    TCPEndpointReceiveMsg* receiveMsg
        = (TCPEndpointReceiveMsg*)antEnvMsg::Receive(msg);
    int index = (int)(receiveMsg->continuation);

    ftpPI[index].ReceiveCont(receiveMsg);
}

void
TinyFTPD::CloseContforPI(ANTENVMSG msg)
{
    TCPEndpointCloseMsg* closeMsg
        = (TCPEndpointCloseMsg*)antEnvMsg::Receive(msg);
    int index = (int)(closeMsg->continuation);
   
    ftpPI[index].CloseCont(closeMsg);
}

void
TinyFTPD::ListenContforDTP(ANTENVMSG msg)
{
    TCPEndpointListenMsg* listenMsg
        = (TCPEndpointListenMsg*)antEnvMsg::Receive(msg);
    int index = (int)(listenMsg->continuation);

    ftpPI[index].ListenContforDTP(listenMsg);
}

void
TinyFTPD::ConnectContforDTP(ANTENVMSG msg)
{
    TCPEndpointConnectMsg* connectMsg
        = (TCPEndpointConnectMsg*)antEnvMsg::Receive(msg);
    int index = (int)(connectMsg->continuation);

    ftpPI[index].ConnectContforDTP(connectMsg);
}

void
TinyFTPD::SendContforDTP(ANTENVMSG msg)
{
    TCPEndpointSendMsg* sendMsg
        = (TCPEndpointSendMsg*)antEnvMsg::Receive(msg);
    int index = (int)(sendMsg->continuation);

    ftpPI[index].SendContforDTP(sendMsg);
}

void
TinyFTPD::ReceiveContforDTP(ANTENVMSG msg)
{
    TCPEndpointReceiveMsg* receiveMsg
        = (TCPEndpointReceiveMsg*)antEnvMsg::Receive(msg);
    int index = (int)(receiveMsg->continuation);

    ftpPI[index].ReceiveContforDTP(receiveMsg);
}

void
TinyFTPD::CloseContforDTP(ANTENVMSG msg)
{
    TCPEndpointCloseMsg* closeMsg
        = (TCPEndpointCloseMsg*)antEnvMsg::Receive(msg);
    int index = (int)(closeMsg->continuation);

    ftpPI[index].CloseContforDTP(closeMsg);
}

OStatus
TinyFTPD::LoadPasswd()
{
    OSYSDEBUG(("TinyFTPD::LoadPasswd()\n"));

    size_t bufsize;
    char* buf;

    OStatus result = Load(FTP_PASSWD_PATH, (byte**)&buf, &bufsize);
    if (result != oSUCCESS) {
        return result;
    }

    char* end = buf + bufsize;
    char* cur = buf;
    char* head = buf;
    Passwd pass;

    while (cur < end) {
        while (isspace(*cur)) cur++;
        head = cur;
        while (!isspace(*cur) && (cur < end)) cur++;
        if (cur >= end) break;
        *cur++ = '\0';
        strcpy(pass.user, head);

        while (isspace(*cur)) cur++;
        head = cur;
        while (!isspace(*cur) && (cur < end)) cur++;
        if (cur >= end) break;
        *cur++ = '\0';
        strcpy(pass.pass, head);

        while (isspace(*cur)) cur++;
        head = cur;
        while (!isspace(*cur) && (cur < end)) cur++;
        if (cur >= end) break;
        *cur++ = '\0';
        strcpy(pass.home, head);

        if (passwd.PushBack(pass)) break;
    }

    DeleteRegion(buf);
    return oSUCCESS;
}

OStatus
TinyFTPD::Load(char* path, byte** data, size_t* size)
{
    FILE* fp;
    byte* readBuf = 0;
    int readBufSize;
    
    *data = 0;
    *size = 0;

    struct stat statbuf;
    int ret = stat(path, &statbuf);
    if (ret != 0) return oNOT_FOUND;

    readBufSize = statbuf.st_size;

    OSYSDEBUG(("%s size %d readBufSize %d\n",
               path, statbuf.st_size, readBufSize));

    readBuf = (byte*)malloc(readBufSize);
    if (readBuf == NULL) return oNO_MEMORY;

    fp = fopen(path, "r");
    if (fp == 0) {
        free(readBuf);
        return oFAIL;
    }

    ret = fread(readBuf, 1, statbuf.st_size, fp);    
    fclose(fp);

    if (ret != statbuf.st_size) {
        free(readBuf);
        return oFAIL;
    }
    
	*data = readBuf;
	*size = statbuf.st_size;

    return oSUCCESS;
}
