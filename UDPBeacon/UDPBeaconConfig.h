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

#ifndef UDPBeaconConfig_h_DEFINED
#define UDPBeaconConfig_h_DEFINED

#define UDPBEACON_CONNECTION_MAX  1
#define UDPBEACON_BUFFER_SIZE     512
#define UDPBEACON_PORT            54322

#ifdef DEFART
struct DefartFoundInfo {
    char tag[2];      // 'BL','GB','GY','PB','BP','PY','YP'
    char found[1];    // found = '0', '1'
    char x[3];        // 'XXX'
    char y[3];        // 'XXX'
    char w[3];        // 'XXX'
    char h[3];        // 'XXX'
    char pan[6];      // '-XXX.X'
    char tilt[6];     // '-XXX.X'
    char distance[3]; // 'XXX'
};

struct DefartSTNInfo {
    char tag[2];   // 'ST'
    char stnNo[2]; // 'XX'
    char stateIds[MAX_TASKS][2]; // 'aabbccddee...'
    char timer1[3]; // '000'
    char timer2[3]; // '000'
    char counter1[3]; // '000'
    char counter2[3]; // '000'
    char counter3[3]; // '000'
    char face[2]; // '00'
    char head[3]; // '000'
    char monet[4]; // '0000'
    char internal[2]; // '00'
};

struct DefartSensorInfo {
    char tag[2];      // 'SE'
    char bodyPSD[6];  // '999999'
    char accFR[9];    // '-99999999'
    char accLR[9];    // '-99999999'
    char accUD[9];    // '-99999999'
};

struct DefartBroadCastData {
    char magic[4];    // 'DEFA'
    char aibotag[2];  // '07' or '21'
    char msec[8];     // msec time in hex string
    char dummy1[1];   // ':'
    DefartFoundInfo ball;
    char dummy2[1];   // ':'
    DefartFoundInfo goalB;
    char dummy3[1];   // ':'
    DefartFoundInfo goalY;
    char dummy4[1];   // ':'
    DefartFoundInfo beaconBY;
    char dummy5[1];   // ':'
    DefartFoundInfo beaconYB;
    char dummy6[1];   // ':'
    DefartSTNInfo stnInfo;
    char dummy7[1];   // ':'
    DefartSensorInfo sensor;
    char dummy8[1];   // ':'
};
#endif /* DEFART */

struct TetoriLegData {
    char tag[2]; // 'LF','LR','RF','RR'
    char j1[6];  // '-XXX.X'
    char j2[6];  // '-XXX.X'
    char j3[6];  // '-XXX.X'
};

struct TetoriBroadCastData {
    char magic[4];    // 'TETO'
    char aibotag[2];  // '07' or '21'
    char msec[8];     // msec time in hex string
    char dummy1[1];   // ':'
    TetoriLegData lf; // 20bytes
    char dummy2[1];   // ':'
    TetoriLegData lr; // 20bytes
    char dummy3[1];   // ':'
    TetoriLegData rf; // 20bytes
    char dummy4[1];   // ':'
    TetoriLegData rr; // 20bytes
    char dummy5[1];   // ':'
};

#define TETORI_MAGIC "TETO"
#define DEFART_MAGIC "DEFA"
#ifdef ERS210
#define AIBO_TAG "21"
#endif /* ERS210 */
#ifdef ERS7
#define AIBO_TAG "07"
#endif /* ERS7 */

#endif // UDPBeaconConfig_h_DEFINED
