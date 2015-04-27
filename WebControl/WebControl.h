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

#ifndef WebControl_h_DEFINED
#define WebControl_h_DEFINED

#include <OPENR/OObject.h>
#include <OPENR/OSubject.h>
#include <OPENR/OObserver.h>
#include "TCPConnection.h"
#include "HTTP.h"
#include "WebControlConfig.h"
#include "def.h"

class WebControl : public OObject {
public:
    WebControl();
    ~WebControl();

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
    static const int    WEBCONTROL_CONNECTION_MAX = 1;
    static const size_t WEBCONTROL_HTTP_BUFSIZE   = 4096;
    static const size_t WEBCONTROL_IMAGE_BUFSIZE  = 32768;

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
    TCPConnection  connection[WEBCONTROL_CONNECTION_MAX];
};

#endif // WebControl_h_DEFINED
