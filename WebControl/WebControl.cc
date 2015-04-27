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
#include "WebControl.h"
#include "entry.h"
#include "VisionInfo.h"

WebControl::WebControl()
{
}

WebControl::~WebControl()
{
  for (int index = 0; index < WEBCONTROL_CONNECTION_MAX; index++) {
    byte *buf = connection[index].imageData;
    if ( buf ) {
      free(buf);
      connection[index].imageData = 0;
    }
  }
}


OStatus
WebControl::DoInit(const OSystemEvent& event)
{
    OStatus result;

    NEW_ALL_SUBJECT_AND_OBSERVER;
    REGISTER_ALL_ENTRY;
    SET_ALL_READY_AND_NOTIFY_ENTRY;

    ipstackRef = antStackRef("IPStack");

    for (int index = 0; index < WEBCONTROL_CONNECTION_MAX; index++) {
        result = InitTCPConnection(index);
        if (result != oSUCCESS) {
            OSYSLOG1((osyslogERROR, "WebControl::DoInit() : NO MEMORY"));
        }
    }

    return oSUCCESS;
}

OStatus
WebControl::DoStart(const OSystemEvent& event)
{
    for (int index = 0; index < WEBCONTROL_CONNECTION_MAX; index++) {
        Listen(index);
    }

    ENABLE_ALL_SUBJECT;
    ASSERT_READY_TO_ALL_OBSERVER;

    return oSUCCESS;
}    

OStatus
WebControl::DoStop(const OSystemEvent& event)
{
    DISABLE_ALL_SUBJECT;
    DEASSERT_READY_TO_ALL_OBSERVER;

    return oSUCCESS;
}

OStatus
WebControl::DoDestroy(const OSystemEvent& event)
{
    DISABLE_ALL_SUBJECT;
    return oSUCCESS;
}

OStatus
WebControl::InitTCPConnection(int index)
{
    OSYSDEBUG(("WebControl::InitTCPConnection(%d)\n", index));

    connection[index].state = CONNECTION_CLOSED;

    // 
    // Allocate send buffer
    //
    antEnvCreateSharedBufferMsg sendBufferMsg(WEBCONTROL_HTTP_BUFSIZE);

    sendBufferMsg.Call(ipstackRef, sizeof(sendBufferMsg));
    if (sendBufferMsg.error != ANT_SUCCESS) {
        OSYSLOG1((osyslogERROR, "%s : %s[%d] antError %d",
                  "WebControl::InitTCPConnection()",
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
    antEnvCreateSharedBufferMsg recvBufferMsg(WEBCONTROL_HTTP_BUFSIZE);
    
    recvBufferMsg.Call(ipstackRef, sizeof(recvBufferMsg));
    if (recvBufferMsg.error != ANT_SUCCESS) {
        OSYSLOG1((osyslogERROR, "%s : %s[%d] antError %d",
                  "WebControl::InitTCPConnection()",
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
    connection[index].httpReq = (char*)malloc(WEBCONTROL_HTTP_BUFSIZE);
    if (connection[index].httpReq == 0) {
        OSYSLOG1((osyslogERROR, "%s : %s[%d]",
                  "WebControl::InitTCPConnection()",
                  "Can't allocate http buffer", index));
        return oFAIL;
    }
    connection[index].httpReqLen = 0;

    //
    // Initialize image info
    //
    connection[index].imageRequested = false;
    connection[index].layer          = ofbkimageLAYER_H;
    connection[index].imageData       = (byte*)malloc(WEBCONTROL_IMAGE_BUFSIZE);
    if (connection[index].imageData == 0) {
        OSYSLOG1((osyslogERROR, "%s : %s[%d]",
                  "WebControl::InitTCPConnection()",
                  "Can't allocate jpeg buffer", index));
        return oFAIL;
    }

    connection[index].imageSize       = 0;

    return oSUCCESS;
}

void
WebControl::NotifyImage(const ONotifyEvent& event)
{
  OFbkImageVectorData* imageVec = (OFbkImageVectorData*)event.Data(0);

  for (int index = 0; index < WEBCONTROL_CONNECTION_MAX; index++) {
    if (connection[index].imageRequested == true) {
      byte* dst = connection[index].imageData;
      int layer = connection[index].layer;
      //info = imageVec->GetInfo(layer);
      byte *src = imageVec->GetData(layer);
      if (layer == ofbkimageLAYER_L) {
	connection[index].imageSize = LAYER_L_IMAGESIZE;
	for ( int y = 0; y < LAYER_L_HEIGHT; y += 1 ) {
	  memcpy( dst, src, LAYER_L_WIDTH * 3 );
	  dst += LAYER_L_WIDTH * 3;
	  src += LAYER_L_WIDTH * 3;
	  src += LAYER_L_WIDTH * 3;
	}
      } if (layer == ofbkimageLAYER_M) {
	connection[index].imageSize = LAYER_M_IMAGESIZE;
	for ( int y = 0; y < LAYER_M_HEIGHT; y += 1 ) {
	  memcpy( dst, src, LAYER_M_WIDTH * 3 );
	  dst += LAYER_M_WIDTH * 3;
	  src += LAYER_M_WIDTH * 3;
	  src += LAYER_M_WIDTH * 3;
	}
      } if (layer == ofbkimageLAYER_C) {
	connection[index].imageSize = LAYER_C_IMAGESIZE;
	memcpy( dst, src, LAYER_C_IMAGESIZE );
      }
      HTTPResponse(index, HTTP_OK);
      connection[index].imageRequested = false;
    }
  }
  observer[event.ObsIndex()]->AssertReady(event.SenderID());
}

OStatus
WebControl::Listen(int index)
{
    OSYSDEBUG(("WebControl::Listen(%d)\n", index));

    if (connection[index].state != CONNECTION_CLOSED) return oFAIL;

    //
    // Create endpoint
    //
    antEnvCreateEndpointMsg tcpCreateMsg(EndpointType_TCP,
                                         WEBCONTROL_HTTP_BUFSIZE * 2);
    tcpCreateMsg.Call(ipstackRef, sizeof(tcpCreateMsg));
    if (tcpCreateMsg.error != ANT_SUCCESS) {
        OSYSLOG1((osyslogERROR, "%s : %s[%d] antError %d",
                  "WebControl::Listen()",
                  "Can't create endpoint",
                  index, tcpCreateMsg.error));
        return oFAIL;
    }
    connection[index].endpoint = tcpCreateMsg.moduleRef;

    //
    // Listen
    //
    TCPEndpointListenMsg listenMsg(connection[index].endpoint,
                                   IP_ADDR_ANY, WEBCONTROL_PORT);
    listenMsg.continuation = (void*)index;

    listenMsg.Send(ipstackRef, myOID_,
                   Extra_Entry[entryListenCont], sizeof(listenMsg));
    
    connection[index].state = CONNECTION_LISTENING;

    return oSUCCESS;
}

void
WebControl::ListenCont(ANTENVMSG msg)
{
    TCPEndpointListenMsg* listenMsg = (TCPEndpointListenMsg*)antEnvMsg::Receive(msg);
    int index = (int)listenMsg->continuation;

    OSYSDEBUG(("WebControl::ListenCont(%d)\n", index));

    if (listenMsg->error != TCP_SUCCESS) {
        OSYSLOG1((osyslogERROR, "%s : %s %d",
                  "WebControl::ListenCont()",
                  "FAILED. listenMsg->error", listenMsg->error));
        Close(index);
        return;
    }

    connection[index].state = CONNECTION_CONNECTED;

    Receive(index);
}

OStatus
WebControl::Receive(int index)
{
    OSYSDEBUG(("WebControl::Receive(%d)\n", index));

    if (connection[index].state != CONNECTION_CONNECTED &&
        connection[index].state != CONNECTION_SENDING) return oFAIL;

    connection[index].recvSize = WEBCONTROL_HTTP_BUFSIZE;
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
WebControl::ReceiveCont(ANTENVMSG msg)
{
    TCPEndpointReceiveMsg* receiveMsg = (TCPEndpointReceiveMsg*)antEnvMsg::Receive(msg);
    int index = (int)(receiveMsg->continuation);

    OSYSDEBUG(("WebControl::ReceiveCont(%d)\n", index));

    if (receiveMsg->error != TCP_SUCCESS) {
        OSYSLOG1((osyslogERROR, "%s : %s %d",
                  "WebControl::ReceiveCont()",
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
WebControl::SendHeader(int index)
{
    OSYSDEBUG(("WebControl::SendHeader(%d)\n", index));

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
WebControl::SendHeaderCont(ANTENVMSG msg)
{
    TCPEndpointSendMsg* sendMsg = (TCPEndpointSendMsg*)antEnvMsg::Receive(msg);
    int index = (int)(sendMsg->continuation);

    OSYSDEBUG(("WebControl::SendHeaderCont(%d)\n", index));

    if (sendMsg->error != TCP_SUCCESS) {
        OSYSLOG1((osyslogERROR, "%s : %s %d",
                  "WebControl::SendHeaderCont()",
                  "FAILED. sendMsg->error", sendMsg->error));
        Close(index);
        return;
    }

    connection[index].state = CONNECTION_CONNECTED;

    if (connection[index].imageSize == 0) {
      Close(index);
    } else {
      SendImage(index);
    }
}

OStatus
WebControl::SendImage(int index)
{
    OSYSDEBUG(("WebControl::SendImage(%d)\n", index));

    if (connection[index].state != CONNECTION_CONNECTED) {
        OSYSLOG1((osyslogERROR, "WebControl::SendImage() : state error"));
        return oFAIL;
    }
    
    TCPEndpointSendMsg sendMsg(connection[index].endpoint,
                               connection[index].imageData,
                               connection[index].imageSize);
    sendMsg.continuation = (void*)index;

    sendMsg.Send(ipstackRef, myOID_,
                 Extra_Entry[entrySendImageCont],
                 sizeof(TCPEndpointSendMsg));

    connection[index].state = CONNECTION_SENDING;
    return oSUCCESS;
}

void
WebControl::SendImageCont(ANTENVMSG msg)
{
    TCPEndpointSendMsg* sendMsg = (TCPEndpointSendMsg*)antEnvMsg::Receive(msg);
    int index = (int)(sendMsg->continuation);

    OSYSDEBUG(("WebControl::SendImageCont(%d)\n", index));

    if (sendMsg->error != TCP_SUCCESS) {
        OSYSLOG1((osyslogERROR, "%s : %s %d",
                  "WebControl::SendImageCont()",
                  "FAILED. sendMsg->error", sendMsg->error));
        Close(index);
        return;
    }

    connection[index].state = CONNECTION_CONNECTED;
    Close(index);
}

OStatus
WebControl::SendError(int index)
{
    OSYSDEBUG(("WebControl::SendError(%d)\n", index));

    if (connection[index].sendSize == 0 ||
        connection[index].state != CONNECTION_CONNECTED) return oFAIL;

    TCPEndpointSendMsg sendMsg(connection[index].endpoint,
                               connection[index].sendData,
                               connection[index].sendSize);
    sendMsg.continuation = (void*)index;

    sendMsg.Send(ipstackRef, myOID_,
                 Extra_Entry[entrySendErrorCont],
                 sizeof(TCPEndpointSendMsg));

    connection[index].state = CONNECTION_SENDING;
    connection[index].sendSize = 0;

    return oSUCCESS;
}

void
WebControl::SendErrorCont(ANTENVMSG msg)
{
    TCPEndpointSendMsg* sendMsg = (TCPEndpointSendMsg*)antEnvMsg::Receive(msg);
    int index = (int)(sendMsg->continuation);

    OSYSDEBUG(("WebControl::SendErrorCont(%d)\n", index));

    if (sendMsg->error != TCP_SUCCESS) {
        OSYSLOG1((osyslogERROR, "%s : %s %d",
                  "WebControl::SendErrorCont()",
                  "FAILED. sendMsg->error", sendMsg->error));
        Close(index);
        return;
    }

    connection[index].state = CONNECTION_CONNECTED;
    Close(index);
}

OStatus
WebControl::Close(int index)
{
    OSYSDEBUG(("WebControl::Close(%d)\n", index));

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
WebControl::CloseCont(ANTENVMSG msg)
{
    TCPEndpointCloseMsg* closeMsg = (TCPEndpointCloseMsg*)antEnvMsg::Receive(msg);
    int index = (int)(closeMsg->continuation);

    OSYSDEBUG(("WebControl::CloseCont(%d)\n", index));

    connection[index].state = CONNECTION_CLOSED;
    Listen(index);
}

void
WebControl::ProcessHTTPRequest(int index)
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

  connection[index].imageRequested = false;
  if (strcmp(uri, WEBCONTROL_DEFAULT_URI) == 0) {
    HTTPResponse(index, HTTP_OK_STATUS);
  } else if (strcmp(uri, WEBCONTROL_LAYER_M_URI) == 0) {
    connection[index].layer = ofbkimageLAYER_M;
    connection[index].imageRequested = true;
  } else if (strcmp(uri, WEBCONTROL_LAYER_L_URI) == 0) {
    connection[index].layer = ofbkimageLAYER_L;
    connection[index].imageRequested = true;
  } else if (strcmp(uri, WEBCONTROL_LAYER_C_URI) == 0) {
    connection[index].layer = ofbkimageLAYER_C;
    connection[index].imageRequested = true;
  } else {
    HTTPResponse(index, HTTP_NOT_FOUND);
  }
}

void
WebControl::HTTPResponse(int index, HTTPStatus st)
{
    char* ptr;
    int len;

    ptr = (char*)connection[index].sendData;
    connection[index].sendSize = 0;
    
    len = http.Status(ptr, st);
    connection[index].sendSize += len;
    ptr += len;

    len = http.HeaderField(ptr, HTTP_SERVER, "WebControl/0.1");
    connection[index].sendSize += len;
    ptr += len;

    if (st == HTTP_OK) {

        len = http.HeaderField(ptr, HTTP_CONTENT_TYPE, "application/octet-stream");
        connection[index].sendSize += len;
        ptr += len;

        len = http.HeaderField(ptr, HTTP_HEADER_END, NULL);
        connection[index].sendSize += len;
        ptr += len;

        SendHeader(index);

    } else if ( st == HTTP_OK_STATUS ) {
      len = http.HeaderField(ptr, HTTP_CONTENT_TYPE, "text/javascript");
      connection[index].sendSize += len;
      ptr += len;

      len = http.HeaderField(ptr, HTTP_HEADER_END, NULL);
      connection[index].sendSize += len;
      ptr += len;

      const char *str = "{\"a\": 1}";
      len = strlen(str);
      connection[index].sendSize += len;
      memcpy(ptr, str, len);
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

            OSYSDEBUG(("WebControl::HTTPResponse() : HTTP_BAD_REQUEST\n"));

            strcpy(ptr, "<html><body>400 Bad Request</body></html>");
            len = strlen("<html><body>400 Bad Request</body></html>");
            connection[index].sendSize += len;
            ptr += len;

        } else if (st == HTTP_NOT_IMPLEMENTED) {

            OSYSDEBUG(("WebControl::HTTPResponse() : HTTP_NOT_IMPLEMENTED\n"));

            strcpy(ptr, "<html><body>501 Not Implemented</body></html>");
            len = strlen("<html><body>501 Not Implemented</body></html>");
            connection[index].sendSize += len;
            ptr += len;

        } else if (st == HTTP_NOT_FOUND) {

            OSYSDEBUG(("WebControl::HTTPResponse() : HTTP_NOT_FOUND\n"));

            strcpy(ptr, "<html><body>404 Not Found</body></html>");
            len = strlen("<html><body>404 Not Found</body></html>");
            connection[index].sendSize += len;
            ptr += len;

        } else {

            OSYSLOG1((osyslogERROR, "WebControl::HTTPResponse() : INVALID ARG"));
            return;

        }

        SendError(index);
    }

#ifdef OPENR_DEBUG
    *ptr = 0;
    DPRINTF(("%s", connection[index].sendData));
#endif
}
