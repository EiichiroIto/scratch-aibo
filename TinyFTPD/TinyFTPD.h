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

#ifndef _TinyFTPD_h_DEFINED
#define _TinyFTPD_h_DEFINED

#include <OPENR/OObject.h>
#include <OPENR/OSubject.h>
#include <OPENR/OObserver.h>
#include <OPENR/OList.h>
#include "FtpConfig.h"
#include "FtpPI.h"
#include "def.h"

class TinyFTPD : public OObject
{
public:
    TinyFTPD();
    virtual ~TinyFTPD() {}

    OSubject*   subject[numOfSubject];
    OObserver*  observer[numOfObserver];     

    virtual OStatus DoInit   (const OSystemEvent& event);
    virtual OStatus DoStart  (const OSystemEvent& event);
    virtual OStatus DoStop   (const OSystemEvent& event);
    virtual OStatus DoDestroy(const OSystemEvent& event);

    void ListenContforPI  (ANTENVMSG msg);
    void SendContforPI    (ANTENVMSG msg);
    void ReceiveContforPI (ANTENVMSG msg);
    void CloseContforPI   (ANTENVMSG msg);
    void ListenContforDTP (ANTENVMSG msg);
    void ConnectContforDTP(ANTENVMSG msg);
    void SendContforDTP   (ANTENVMSG msg);
    void ReceiveContforDTP(ANTENVMSG msg);
    void CloseContforDTP  (ANTENVMSG msg);

private:
    OStatus LoadPasswd();
    OStatus Load(char* path, byte** data, size_t* size);

    antStackRef ipstackRef;
    FtpPI ftpPI[FTP_CONNECTION_MAX];

    OList<Passwd, MAX_LOGIN> passwd;
};

#endif /* _TinyFTPD_h_DEFINED */
