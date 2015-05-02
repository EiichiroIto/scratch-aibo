//
// Copyright 2003 Sony Corporation 
//
// Permission to use, copy, modify, and redistribute this software for
// non-commercial use is hereby granted.
//
// This software is provided "as is" without warranty of any kind,
// either expressed or implied, including but not limited to the
// implied warranties of fitness for a particular purpose.
//

#include <stdlib.h>
#include <string.h>
#include <OPENR/OSyslog.h>
#include <OPENR/ODebug.h>
#include <OPENR/OPENRAPI.h>
#include <OPENR/core_macro.h>
#include <ant.h>
#include <EndpointTypes.h>
#include <TCPEndpointMsg.h>
#include "W3AIBO.h"
#include "entry.h"

W3AIBO::W3AIBO()
{
  cdtInfo[0]='\0';
}

OStatus
W3AIBO::DoInit(const OSystemEvent& event)
{
    OStatus result;

    NEW_ALL_SUBJECT_AND_OBSERVER;
    REGISTER_ALL_ENTRY;
    SET_ALL_READY_AND_NOTIFY_ENTRY;

    ipstackRef = antStackRef("IPStack");

    for (int index = 0; index < W3AIBO_CONNECTION_MAX; index++) {
        result = InitTCPConnection(index);
        if (result != oSUCCESS) {
            OSYSLOG1((osyslogERROR, "W3AIBO::DoInit() : NO MEMORY"));
        }
    }

    jpegEncoder.Init(ipstackRef);

    return oSUCCESS;
}

OStatus
W3AIBO::DoStart(const OSystemEvent& event)
{
    for (int index = 0; index < W3AIBO_CONNECTION_MAX; index++) {
        Listen(index);
    }

    ENABLE_ALL_SUBJECT;
    ASSERT_READY_TO_ALL_OBSERVER;

    return oSUCCESS;
}    

OStatus
W3AIBO::DoStop(const OSystemEvent& event)
{
    DISABLE_ALL_SUBJECT;
    DEASSERT_READY_TO_ALL_OBSERVER;

    return oSUCCESS;
}

OStatus
W3AIBO::DoDestroy(const OSystemEvent& event)
{
    DISABLE_ALL_SUBJECT;
    return oSUCCESS;
}

OStatus
W3AIBO::InitTCPConnection(int index)
{
    OSYSDEBUG(("W3AIBO::InitTCPConnection(%d)\n", index));

    connection[index].state = CONNECTION_CLOSED;

    // 
    // Allocate send buffer
    //
    antEnvCreateSharedBufferMsg sendBufferMsg(W3AIBO_HTTP_BUFSIZE);

    sendBufferMsg.Call(ipstackRef, sizeof(sendBufferMsg));
    if (sendBufferMsg.error != ANT_SUCCESS) {
        OSYSLOG1((osyslogERROR, "%s : %s[%d] antError %d",
                  "W3AIBO::InitTCPConnection()",
                  "Can't allocate send buffer",
                  index, sendBufferMsg.error));
        return oFAIL;
    }

    connection[index].sendBuffer = sendBufferMsg.buffer;
    connection[index].sendBuffer.Map();
    connection[index].sendData
        = (byte*)(connection[index].sendBuffer.GetAddress());
    connection[index].sendSize = 0;
    
    //
    // Allocate receive buffer
    //
    antEnvCreateSharedBufferMsg recvBufferMsg(W3AIBO_HTTP_BUFSIZE);
    
    recvBufferMsg.Call(ipstackRef, sizeof(recvBufferMsg));
    if (recvBufferMsg.error != ANT_SUCCESS) {
        OSYSLOG1((osyslogERROR, "%s : %s[%d] antError %d",
                  "W3AIBO::InitTCPConnection()",
                  "Can't allocate receive buffer",
                  index, recvBufferMsg.error));
        return oFAIL;
    }

    connection[index].recvBuffer = recvBufferMsg.buffer;
    connection[index].recvBuffer.Map();
    connection[index].recvData
        = (byte*)(connection[index].recvBuffer.GetAddress());
    connection[index].recvSize = 0;

    //
    // Allocate http buffer
    //
    connection[index].httpReq = (char*)malloc(W3AIBO_HTTP_BUFSIZE);
    if (connection[index].httpReq == 0) {
        OSYSLOG1((osyslogERROR, "%s : %s[%d]",
                  "W3AIBO::InitTCPConnection()",
                  "Can't allocate http buffer", index));
        return oFAIL;
    }
    connection[index].httpReqLen = 0;

    //
    // Initialize image info
    //
    connection[index].imageRequested = false;
    connection[index].layer          = ofbkimageLAYER_M;
    connection[index].reconstruction = false;
    connection[index].quality        = 75;
    connection[index].jpegData       = 0;
    connection[index].jpegSize       = 0;

    return oSUCCESS;
}

void
W3AIBO::NotifyImage(const ONotifyEvent& event)
{
    OFbkImageVectorData* imageVec = (OFbkImageVectorData*)event.Data(0);

    for (int index = 0; index < W3AIBO_CONNECTION_MAX; index++) {
        if (connection[index].imageRequested == true) {
        
            jpegEncoder.GetJPEG(imageVec, 
                                connection[index].layer,
                                connection[index].reconstruction,
                                connection[index].quality,
                                &(connection[index].jpegData),
                                &(connection[index].jpegSize));

            HTTPResponse(index, HTTP_OK);
            connection[index].imageRequested = false;
        }
    }

    observer[event.ObsIndex()]->AssertReady(event.SenderID());
}

OStatus
W3AIBO::Listen(int index)
{
    OSYSDEBUG(("W3AIBO::Listen(%d)\n", index));

    if (connection[index].state != CONNECTION_CLOSED) return oFAIL;

    //
    // Create endpoint
    //
    antEnvCreateEndpointMsg tcpCreateMsg(EndpointType_TCP,
                                         W3AIBO_HTTP_BUFSIZE * 2);
    tcpCreateMsg.Call(ipstackRef, sizeof(tcpCreateMsg));
    if (tcpCreateMsg.error != ANT_SUCCESS) {
        OSYSLOG1((osyslogERROR, "%s : %s[%d] antError %d",
                  "W3AIBO::Listen()",
                  "Can't create endpoint",
                  index, tcpCreateMsg.error));
        return oFAIL;
    }
    connection[index].endpoint = tcpCreateMsg.moduleRef;

    //
    // Listen
    //
    TCPEndpointListenMsg listenMsg(connection[index].endpoint,
                                   IP_ADDR_ANY, W3AIBO_PORT);
    listenMsg.continuation = (void*)index;

    listenMsg.Send(ipstackRef, myOID_,
                   Extra_Entry[entryListenCont], sizeof(listenMsg));
    
    connection[index].state = CONNECTION_LISTENING;

    return oSUCCESS;
}

void
W3AIBO::ListenCont(ANTENVMSG msg)
{
    TCPEndpointListenMsg* listenMsg = (TCPEndpointListenMsg*)antEnvMsg::Receive(msg);
    int index = (int)listenMsg->continuation;

    OSYSDEBUG(("W3AIBO::ListenCont(%d)\n", index));

    if (listenMsg->error != TCP_SUCCESS) {
        OSYSLOG1((osyslogERROR, "%s : %s %d",
                  "W3AIBO::ListenCont()",
                  "FAILED. listenMsg->error", listenMsg->error));
        Close(index);
        return;
    }

    connection[index].state = CONNECTION_CONNECTED;

    Receive(index);
}

OStatus
W3AIBO::Receive(int index)
{
    OSYSDEBUG(("W3AIBO::Receive(%d)\n", index));

    if (connection[index].state != CONNECTION_CONNECTED &&
        connection[index].state != CONNECTION_SENDING) return oFAIL;

    connection[index].recvSize = W3AIBO_HTTP_BUFSIZE;
    connection[index].httpReqLen = 0;

    TCPEndpointReceiveMsg receiveMsg(connection[index].endpoint,
                                     connection[index].recvData,
                                     1, connection[index].recvSize);
    receiveMsg.continuation = (void*)index;

    receiveMsg.Send(ipstackRef, myOID_,
                    Extra_Entry[entryReceiveCont], sizeof(receiveMsg));

    return oSUCCESS;
}

void
W3AIBO::ReceiveCont(ANTENVMSG msg)
{
    TCPEndpointReceiveMsg* receiveMsg = (TCPEndpointReceiveMsg*)antEnvMsg::Receive(msg);
    int index = (int)(receiveMsg->continuation);

    OSYSDEBUG(("W3AIBO::ReceiveCont(%d)\n", index));

    if (receiveMsg->error != TCP_SUCCESS) {
        OSYSLOG1((osyslogERROR, "%s : %s %d",
                  "W3AIBO::ReceiveCont()",
                  "FAILED. receiveMsg->error", receiveMsg->error));
        Close(index);
        return;
    }

    memcpy((void*)(connection[index].httpReq + connection[index].httpReqLen),
           connection[index].recvData, receiveMsg->sizeMin);

    connection[index].httpReqLen += receiveMsg->sizeMin;
    connection[index].httpReq[connection[index].httpReqLen] = '\0';

#ifdef OPENR_DEBUG
    DPRINTF(("receiveMsg->sizeMin %d\n", receiveMsg->sizeMin));
    for (int i = 0; i < receiveMsg->sizeMin; i++) {
        DPRINTF(("%02x ", connection[index].httpReq[i]));
    }
    DPRINTF(("\n"));
#endif

    if (strstr(connection[index].httpReq, "\r\n\r\n")) {

        DPRINTF(("%s", connection[index].httpReq));
        ProcessHTTPRequest(index);

    } else {

        Receive(index);

    }
}

OStatus
W3AIBO::SendHeader(int index)
{
    OSYSDEBUG(("W3AIBO::SendHeader(%d)\n", index));

    if (connection[index].sendSize == 0 ||
        connection[index].state != CONNECTION_CONNECTED) return oFAIL;

    TCPEndpointSendMsg sendMsg(connection[index].endpoint,
                               connection[index].sendData,
                               connection[index].sendSize);
    sendMsg.continuation = (void*)index;

    sendMsg.Send(ipstackRef, myOID_,
                 Extra_Entry[entrySendHeaderCont],
                 sizeof(TCPEndpointSendMsg));

    connection[index].state = CONNECTION_SENDING;
    connection[index].sendSize = 0;

    return oSUCCESS;
}

void
W3AIBO::SendHeaderCont(ANTENVMSG msg)
{
    TCPEndpointSendMsg* sendMsg = (TCPEndpointSendMsg*)antEnvMsg::Receive(msg);
    int index = (int)(sendMsg->continuation);

    OSYSDEBUG(("W3AIBO::SendHeaderCont(%d)\n", index));

    if (sendMsg->error != TCP_SUCCESS) {
        OSYSLOG1((osyslogERROR, "%s : %s %d",
                  "W3AIBO::SendHeaderCont()",
                  "FAILED. sendMsg->error", sendMsg->error));
        Close(index);
        return;
    }

    connection[index].state = CONNECTION_CONNECTED;
    SendImage(index);
}

OStatus
W3AIBO::SendImage(int index)
{
    OSYSDEBUG(("W3AIBO::SendImage(%d)\n", index));

    if (connection[index].state != CONNECTION_CONNECTED) {
        OSYSLOG1((osyslogERROR, "W3AIBO::SendImage() : state error"));
        return oFAIL;
    }
    
    TCPEndpointSendMsg sendMsg(connection[index].endpoint,
                               connection[index].jpegData,
                               connection[index].jpegSize);
    sendMsg.continuation = (void*)index;

    sendMsg.Send(ipstackRef, myOID_,
                 Extra_Entry[entrySendImageCont],
                 sizeof(TCPEndpointSendMsg));

    connection[index].state = CONNECTION_SENDING;
    return oSUCCESS;
}

void
W3AIBO::SendImageCont(ANTENVMSG msg)
{
    TCPEndpointSendMsg* sendMsg = (TCPEndpointSendMsg*)antEnvMsg::Receive(msg);
    int index = (int)(sendMsg->continuation);

    OSYSDEBUG(("W3AIBO::SendImageCont(%d)\n", index));

    jpegEncoder.FreeJPEG(connection[index].jpegData);

    if (sendMsg->error != TCP_SUCCESS) {
        OSYSLOG1((osyslogERROR, "%s : %s %d",
                  "W3AIBO::SendImageCont()",
                  "FAILED. sendMsg->error", sendMsg->error));
        Close(index);
        return;
    }

    connection[index].state = CONNECTION_CONNECTED;
    Close(index);
}

OStatus
W3AIBO::SendText(int index)
{
    OSYSDEBUG(("W3AIBO::SendText(%d)\n", index));

    if (connection[index].sendSize == 0 ||
        connection[index].state != CONNECTION_CONNECTED) return oFAIL;

    TCPEndpointSendMsg sendMsg(connection[index].endpoint,
                               connection[index].sendData,
                               connection[index].sendSize);
    sendMsg.continuation = (void*)index;

    sendMsg.Send(ipstackRef, myOID_,
                 Extra_Entry[entrySendTextCont],
                 sizeof(TCPEndpointSendMsg));

    connection[index].state = CONNECTION_SENDING;
    connection[index].sendSize = 0;

    return oSUCCESS;
}

void
W3AIBO::SendTextCont(ANTENVMSG msg)
{
    TCPEndpointSendMsg* sendMsg = (TCPEndpointSendMsg*)antEnvMsg::Receive(msg);
    int index = (int)(sendMsg->continuation);

    OSYSDEBUG(("W3AIBO::SendTextCont(%d)\n", index));

    if (sendMsg->error != TCP_SUCCESS) {
        OSYSLOG1((osyslogERROR, "%s : %s %d",
                  "W3AIBO::SendTextCont()",
                  "FAILED. sendMsg->error", sendMsg->error));
        Close(index);
        return;
    }

    connection[index].state = CONNECTION_CONNECTED;
    Close(index);
}

OStatus
W3AIBO::Close(int index)
{
    OSYSDEBUG(("W3AIBO::Close(%d)\n", index));

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
W3AIBO::CloseCont(ANTENVMSG msg)
{
    TCPEndpointCloseMsg* closeMsg = (TCPEndpointCloseMsg*)antEnvMsg::Receive(msg);
    int index = (int)(closeMsg->continuation);

    OSYSDEBUG(("W3AIBO::CloseCont(%d)\n", index));

    connection[index].state = CONNECTION_CLOSED;
    Listen(index);
}

void
W3AIBO::ProcessHTTPRequest(int index)
{
    char method[32];
    char uri[256];
    char httpVer[32];
  
    bool result = http.Parse(connection[index].httpReq, method, uri, httpVer);
    if (result != true) {
        HTTPResponse(index, HTTP_BAD_REQUEST);
        return;
    }

    OSYSDEBUG(("METHOD  : %s\n", method));
    OSYSDEBUG(("URI     : %s\n", uri));
    OSYSDEBUG(("VERSION : %s\n", httpVer));

    if (strcasecmp(method, "GET") != 0) {
        HTTPResponse(index, HTTP_NOT_IMPLEMENTED);
        return;
    } 

    if (strcmp(uri, W3AIBO_DEFAULT_URI) == 0) {
      BallResponse(index);
      return;
    } else if (strcmp(uri, W3AIBO_LAYER_C_URI) == 0) {
      connection[index].layer = ofbkimageLAYER_C;
      connection[index].reconstruction = false;
    } else if (strncmp(uri, W3AIBO_MONET_URI, strlen(W3AIBO_MONET_URI)) == 0) {
      int cmd = atoi(&uri[strlen(W3AIBO_MONET_URI)]);
      SendMoNet(cmd);
      BallResponse(index);
      return;
    } else if (strncmp(uri, W3AIBO_FACE_URI, strlen(W3AIBO_FACE_URI)) == 0) {
      int cmd = atoi(&uri[strlen(W3AIBO_FACE_URI)]);
      SendFace(cmd);
      BallResponse(index);
      return;
    } else if (strncmp(uri, W3AIBO_CONTROL_URI, strlen(W3AIBO_CONTROL_URI)) == 0) {
      int cmd = atoi(&uri[strlen(W3AIBO_CONTROL_URI)]);
      SendControl((EAiboControlID) cmd);
      BallResponse(index);
      return;
    } else if (strncmp(uri, W3AIBO_HEAD_URI, strlen(W3AIBO_HEAD_URI)) == 0) {
      int cmd = atoi(&uri[strlen(W3AIBO_HEAD_URI)]);
      SendHead(cmd, 0);
      BallResponse(index);
      return;
    } else if (strcmp(uri, W3AIBO_CDT_URI) == 0) {
      JsonResponse(index);
      return;
    } else if (strcmp(uri, W3AIBO_LAYER_M_URI) == 0) {

        connection[index].layer = ofbkimageLAYER_M;
        connection[index].reconstruction = false;

    } else if (strcmp(uri, W3AIBO_LAYER_L_URI) == 0) {

        connection[index].layer = ofbkimageLAYER_L;
        connection[index].reconstruction = false;

    } else if (strcmp(uri, W3AIBO_LAYER_MR_URI) == 0) {

        connection[index].layer = ofbkimageLAYER_M;
        connection[index].reconstruction = true;

    } else if (strcmp(uri, W3AIBO_LAYER_LR_URI) == 0) {
        
        connection[index].layer = ofbkimageLAYER_L;
        connection[index].reconstruction = true;

    } else {
        HTTPResponse(index, HTTP_NOT_FOUND);
        return;
    }

    connection[index].imageRequested = true;
}

void
W3AIBO::TextResponse(int index, const char *text, char *type)
{
  char* ptr;
  int len;

  ptr = (char*)connection[index].sendData;
  connection[index].sendSize = 0;
    
  len = http.Status(ptr, HTTP_OK);
  connection[index].sendSize += len;
  ptr += len;

  len = http.HeaderField(ptr, HTTP_SERVER, "W3AIBO/0.1");
  connection[index].sendSize += len;
  ptr += len;

  len = http.HeaderField(ptr, HTTP_CONTENT_TYPE, type);
  connection[index].sendSize += len;
  ptr += len;

  len = http.HeaderField(ptr, HTTP_HEADER_END, NULL);
  connection[index].sendSize += len;
  ptr += len;

  strcpy(ptr, text);
  len = strlen(text);
  connection[index].sendSize += len;
  ptr += len;

  SendText(index);
}

void
W3AIBO::BallResponse(int index)
{
  char* ptr;
  int len;

  ptr = (char*)connection[index].sendData;
  connection[index].sendSize = 0;
    
  len = http.Status(ptr, HTTP_OK);
  connection[index].sendSize += len;
  ptr += len;

  len = http.HeaderField(ptr, HTTP_SERVER, "W3AIBO/0.1");
  connection[index].sendSize += len;
  ptr += len;

  len = http.HeaderField(ptr, HTTP_CONTENT_TYPE, "text/javascript");
  connection[index].sendSize += len;
  ptr += len;

  len = http.HeaderField(ptr, HTTP_HEADER_END, NULL);
  connection[index].sendSize += len;
  ptr += len;

  *ptr++ = '{';
  connection[index].sendSize ++;

  strcpy(ptr, "\"ball\":");
  len = strlen(ptr);
  connection[index].sendSize += len;
  ptr += len;

  if (ball.found) {
    strcpy(ptr, "true");
  } else {
    strcpy(ptr, "false");
  }
  len = strlen(ptr);
  connection[index].sendSize += len;
  ptr += len;

  *ptr++ = '}';
  connection[index].sendSize ++;

  *ptr++ = 0;
  SendText(index);

#if 0
  if ( ball->found ) {
      sprintf(tmpbuf, "sensor-update ball true ballPan %.1f ballTilt %.1f ballDistance %d",
	      ball->objPos.pan, ball->objPos.tilt, ball->distance);
    } else {
      sprintf(tmpbuf, "sensor-update ball false");
    }
    SensorUpdate(tmpbuf);
  }
#endif
}

void
W3AIBO::JsonResponse(int index)
{
  char* ptr;
  int len;

  ptr = (char*)connection[index].sendData;
  connection[index].sendSize = 0;
    
  len = http.Status(ptr, HTTP_OK);
  connection[index].sendSize += len;
  ptr += len;

  len = http.HeaderField(ptr, HTTP_SERVER, "W3AIBO/0.1");
  connection[index].sendSize += len;
  ptr += len;


  len = http.HeaderField(ptr, HTTP_CONTENT_TYPE, "text/javascript");
  connection[index].sendSize += len;
  ptr += len;

  len = http.HeaderField(ptr, HTTP_HEADER_END, NULL);
  connection[index].sendSize += len;
  ptr += len;

  strcpy(ptr, cdtInfo);
  len = strlen(ptr);
  connection[index].sendSize += len;
  ptr += len;

  SendText(index);
}

void
W3AIBO::HTTPResponse(int index, HTTPStatus st)
{
    char* ptr;
    int len;

    ptr = (char*)connection[index].sendData;
    connection[index].sendSize = 0;
    
    len = http.Status(ptr, st);
    connection[index].sendSize += len;
    ptr += len;

    len = http.HeaderField(ptr, HTTP_SERVER, "W3AIBO/0.1");
    connection[index].sendSize += len;
    ptr += len;

    if (st == HTTP_OK) {

        len = http.HeaderField(ptr, HTTP_CONTENT_TYPE, "image/jpeg");
        connection[index].sendSize += len;
        ptr += len;

        len = http.HeaderField(ptr, HTTP_HEADER_END, NULL);
        connection[index].sendSize += len;
        ptr += len;

        SendHeader(index);

    } else { // HTTP error

        len = http.HeaderField(ptr, HTTP_CONTENT_TYPE, "text/html");
        connection[index].sendSize += len;
        ptr += len;

        len = http.HeaderField(ptr, HTTP_HEADER_END, NULL);
        connection[index].sendSize += len;
        ptr += len;
      
        if (st == HTTP_BAD_REQUEST) {

            OSYSDEBUG(("W3AIBO::HTTPResponse() : HTTP_BAD_REQUEST\n"));

            strcpy(ptr, "<html><body>400 Bad Request</body></html>");
            len = strlen("<html><body>400 Bad Request</body></html>");
            connection[index].sendSize += len;
            ptr += len;

        } else if (st == HTTP_NOT_IMPLEMENTED) {

            OSYSDEBUG(("W3AIBO::HTTPResponse() : HTTP_NOT_IMPLEMENTED\n"));

            strcpy(ptr, "<html><body>501 Not Implemented</body></html>");
            len = strlen("<html><body>501 Not Implemented</body></html>");
            connection[index].sendSize += len;
            ptr += len;

        } else if (st == HTTP_NOT_FOUND) {

            OSYSDEBUG(("W3AIBO::HTTPResponse() : HTTP_NOT_FOUND\n"));

            strcpy(ptr, "<html><body>404 Not Found</body></html>");
            len = strlen("<html><body>404 Not Found</body></html>");
            connection[index].sendSize += len;
            ptr += len;

        } else {

            OSYSLOG1((osyslogERROR, "W3AIBO::HTTPResponse() : INVALID ARG"));
            return;

        }

        SendText(index);
    }

#ifdef OPENR_DEBUG
    *ptr = 0;
    DPRINTF(("%s", connection[index].sendData));
#endif
}

void
W3AIBO::SendMoNet(int cmd)
{
  OSYSPRINT(("SendMoNet:cmd=%d\n", cmd));
  ControlInfo ci;
  ci.id = CtID_MonetSync;
  ci.iValue = cmd;
  subject[sbjControl]->SetData( &ci, sizeof(ci) );
  subject[sbjControl]->NotifyObservers();
}

void
W3AIBO::SendFace(int cmd)
{
  OSYSPRINT(("SendFace:cmd=%d\n", cmd));
  ControlInfo ci;
  ci.id = CtID_Face;
  ci.iValue = cmd;
  subject[sbjControl]->SetData( &ci, sizeof(ci) );
  subject[sbjControl]->NotifyObservers();
}

void
W3AIBO::SendControl(EAiboControlID id)
{
  OSYSPRINT(("SendControl:id=%d\n", id));
  ControlInfo ci;
  ci.id = id;
  ci.iValue = 0;
  subject[sbjControl]->SetData( &ci, sizeof(ci) );
  subject[sbjControl]->NotifyObservers();
}

void
W3AIBO::SendHead(int hcmd, int arg)
{
  OSYSPRINT(("SendHead:hcmd=%d,arg=%d\n", hcmd, arg));
  ControlInfo ci;
  ci.id = CtID_Head;
  ci.iValue = hcmd;
  ci.i2Value = arg;
  ci.dValue2[0] = 0;
  ci.dValue2[1] = 0;
  ci.dValue2[2] = 0;
  ci.dValue2[3] = 0;
  subject[sbjControl]->SetData( &ci, sizeof(ci) );
  subject[sbjControl]->NotifyObservers();
}

void
W3AIBO::NotifyResult(const ONotifyEvent& event)
{
  ControlInfo *r = (ControlInfo*)event.Data(0);
  OSYSPRINT(("NotifyResult(%d)", r->id));

  if (r->id == CtID_GetCdt) {
    strcpy(cdtInfo, r->sValue);
  }
  observer[event.ObsIndex()]->AssertReady(event.SenderID());
}

void
W3AIBO::NotifyObjInfo( const ONotifyEvent& event )
{
  // Visionから制御情報を受け取った
  const CdtInfo *cdtInfo = (CdtInfo*) event.Data(0);
  ball = cdtInfo->foundInfo[IO_BALL];
  observer[event.ObsIndex()]->AssertReady();
}
