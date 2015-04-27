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

#ifndef _FtpDTP_h_DEFINED
#define _FtpDTP_h_DEFINED

#include <stdio.h>
#include <dirent.h>
#include <EndpointTypes.h>
#include <TCPEndpointMsg.h>
#include "TCPConnection.h"
#include "FtpConfig.h"

#include "entry.h"
#include "def.h"

class FtpDTP
{
public:
    FtpDTP();
    virtual ~FtpDTP() {}

    OStatus Initialize(const OID& myoid,
                       const antStackRef& ipstack, void* index);
    bool ListenCont (TCPEndpointListenMsg*  listenMsg);
    bool ConnectCont(TCPEndpointConnectMsg* connectMsg);
    bool SendCont   (TCPEndpointSendMsg*    sendMsg);
    bool ReceiveCont(TCPEndpointReceiveMsg* receiveMsg);
    void CloseCont  (TCPEndpointCloseMsg*   closeMsg);

    OStatus Close  ();

    // Set
    void SetType(FTPDataType type) {dataType = type;};
    void SetUser(char *user);
    void SetHome(char *home);

    // Get
    ConnectionState GetState() {return connection.state;};
    FTPDataType GetType() {return dataType;};
    char * GetFilename() {return ftpFile;};
    char * GetDirectry() {return ftpDir;};
    size_t GetFileSize(char *name);
    char * GetUser() {return ftpUser;};

    // Method
    Port SetIP   (IPAddress ip);
    bool SetPort (char* ipport);
    bool Retrieve(char* filename);
    bool Store   (char* filename);
    bool ChangeDir(char* dir);
    bool MakeDir(char* dir);
    bool RemoveDir(char* dir);
    bool Delete(char* filename);
    bool List    (char* dir);
    bool RenameFrom(char *file);
    bool RenameTo(char *file);
    void ResetFilename();

private:
    OStatus Listen ();
    OStatus Connect();
    OStatus Send   ();
    OStatus Receive();

    void Save(byte *data, int length, bool end = true);
    void DirNorm(char *dir);
    bool RetrieveSend();
    bool ListSend();

    OID myOID;
    antStackRef ipstackRef;

    // connection info
    IPAddress connectIP;
    Port      connectPort;
    FTPMethod method;
    bool passive;

    // File Info
    FTPDataType dataType;
    char ftpUser[MAX_STRING_LENGTH];
    char ftpHome[MAX_STRING_LENGTH];
    char ftpDir[MAX_STRING_LENGTH];
    char ftpFile[MAX_STRING_LENGTH];
    bool listLong;
    bool total;
    FILE* fp;
    DIR*  dirp;

    void* continuation;
    TCPConnection connection;
};

#endif /* _FtpDTP_h_DEFINED */
