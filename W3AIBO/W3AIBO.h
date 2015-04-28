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

#ifndef W3AIBO_h_DEFINED
#define W3AIBO_h_DEFINED

#include <OPENR/OObject.h>
#include <OPENR/OSubject.h>
#include <OPENR/OObserver.h>
#include "TCPConnection.h"
#include "HTTP.h"
#include "JPEGEncoder.h"
#include "W3AIBOConfig.h"
#include "def.h"

class W3AIBO : public OObject {
public:
    W3AIBO();
    virtual ~W3AIBO() {}

    OSubject*   subject[numOfSubject];
    OObserver*  observer[numOfObserver];     
    
    virtual OStatus DoInit   (const OSystemEvent& event);
    virtual OStatus DoStart  (const OSystemEvent& event);
    virtual OStatus DoStop   (const OSystemEvent& event);
    virtual OStatus DoDestroy(const OSystemEvent& event);

    void NotifyImage(const ONotifyEvent& event);

    void ListenCont    (ANTENVMSG msg);
    void ReceiveCont   (ANTENVMSG msg);
    void SendHeaderCont(ANTENVMSG msg);
    void SendImageCont (ANTENVMSG msg);
    void SendErrorCont (ANTENVMSG msg);
    void CloseCont     (ANTENVMSG msg);

private:
    static const int    W3AIBO_CONNECTION_MAX = JPEGEncoder::NUM_JPEG_BUF;
    static const size_t W3AIBO_HTTP_BUFSIZE   = 4096;

    OStatus InitTCPConnection(int index);
    OStatus Listen    (int index);
    OStatus Receive   (int index);
    OStatus SendHeader(int index);
    OStatus SendImage (int index);
    OStatus SendError (int index);
    OStatus Close     (int index);

    void ProcessHTTPRequest(int index);
    void HTTPResponse      (int index, HTTPStatus st);

    antStackRef    ipstackRef;
    HTTP           http;
    JPEGEncoder    jpegEncoder;
    TCPConnection  connection[W3AIBO_CONNECTION_MAX];
};

#endif // W3AIBO_h_DEFINED
