//
// Copyright 2002 Sony Corporation 
//
// Permission to use, copy, modify, and redistribute this software for
// non-commercial use is hereby granted.
//
// This software is provided "as is" without warranty of any kind,
// either expressed or implied, including but not limited to the
// implied warranties of fitness for a particular purpose.
//

#ifndef TCPConnection_h_DEFINED
#define TCPConnection_h_DEFINED

#include <ant.h>

enum ConnectionState {
    CONNECTION_CLOSED,
    CONNECTION_CONNECTING,
    CONNECTION_CONNECTED,
    CONNECTION_LISTENING,
    CONNECTION_SENDING,
    CONNECTION_RECEIVING,
    CONNECTION_CLOSING
};

struct TCPConnection {
    antModuleRef     endpoint;
    ConnectionState  state;

    // send buffer
    antSharedBuffer  sendBuffer;
    byte*            sendData;
    int              sendSize;
    
    // receive buffer
    antSharedBuffer  recvBuffer;
    byte*            recvData;
    int              recvSize;
};

#endif // TCPConnection_h_DEFINED
