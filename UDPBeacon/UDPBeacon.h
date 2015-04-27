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

#ifndef UDPBeacon_h_DEFINED
#define UDPBeacon_h_DEFINED

#include <OPENR/OObject.h>
#include <OPENR/OSubject.h>
#include <OPENR/OObserver.h>
#include "ControlInfo.h"
#include "VisionInfo.h"
#include "UDPConnection.h"
#include "UDPBeaconConfig.h"
#include "def.h"

enum UDPBeaconState {
    USS_IDLE,
    USS_START,
};

#ifdef ERS210
const int HEAD_TILT  =  0;
const int HEAD_PAN   =  1;
const int HEAD_ROLL  =  2;
const int RFLEG_J1   =  3;
const int RFLEG_J2   =  4;
const int RFLEG_J3   =  5;
const int LFLEG_J1   =  6;
const int LFLEG_J2   =  7;
const int LFLEG_J3   =  8;
const int RRLEG_J1   =  9;
const int RRLEG_J2   = 10;
const int RRLEG_J3   = 11;
const int LRLEG_J1   = 12;
const int LRLEG_J2   = 13;
const int LRLEG_J3   = 14;
const int BODY_PSD   = 15;
const int ACC_FR     = 16;
const int ACC_LR     = 17;
const int ACC_UD     = 18;
const int NUM_JOINTS = 19;

static const char* const JOINT_LOCATOR[] = {
    "PRM:/r1/c1-Joint2:j1",       // HEAD TILT
    "PRM:/r1/c1/c2-Joint2:j2",    // HEAD PAN
    "PRM:/r1/c1/c2/c3-Joint2:j3", // HEAD ROLL

    "PRM:/r4/c1-Joint2:j1",       // RFLEG J1 (Right Front Leg)
    "PRM:/r4/c1/c2-Joint2:j2",    // RFLEG J2
    "PRM:/r4/c1/c2/c3-Joint2:j3", // RFLEG J3

    "PRM:/r2/c1-Joint2:j1",       // LFLEG J1 (Left Front Leg)
    "PRM:/r2/c1/c2-Joint2:j2",    // LFLEG J2
    "PRM:/r2/c1/c2/c3-Joint2:j3", // LFLEG J3

    "PRM:/r5/c1-Joint2:j1",       // RRLEG J1 (Right Rear Leg)
    "PRM:/r5/c1/c2-Joint2:j2",    // RRLEG J2
    "PRM:/r5/c1/c2/c3-Joint2:j3", // RRLEG J3

    "PRM:/r3/c1-Joint2:j1",       // LRLEG J1 (Left Rear Leg)
    "PRM:/r3/c1/c2-Joint2:j2",    // LRLEG J2
    "PRM:/r3/c1/c2/c3-Joint2:j3", // LRLEG J3

    "PRM:/r1/c1/c2/c3/p1-Sensor:p1", // Body Sensor
    "PRM:/a1-Sensor:a1",          // ACC Sensor Front-Rear
    "PRM:/a2-Sensor:a2",          // ACC Sensor Left-Right
    "PRM:/a3-Sensor:a3",          // ACC Sensor Up-Down
};
#endif /* ERS210 */
#ifdef ERS7
const int HEAD_TILT1 =  0;
const int HEAD_PAN   =  1;
const int HEAD_TILT2 =  2;
const int RFLEG_J1   =  3;
const int RFLEG_J2   =  4;
const int RFLEG_J3   =  5;
const int LFLEG_J1   =  6;
const int LFLEG_J2   =  7;
const int LFLEG_J3   =  8;
const int RRLEG_J1   =  9;
const int RRLEG_J2   = 10;
const int RRLEG_J3   = 11;
const int LRLEG_J1   = 12;
const int LRLEG_J2   = 13;
const int LRLEG_J3   = 14;
const int BODY_PSD   = 15;
const int ACC_FR     = 16;
const int ACC_LR     = 17;
const int ACC_UD     = 18;
const int NUM_JOINTS = 19;

static const char* const JOINT_LOCATOR[] = {
    "PRM:/r1/c1-Joint2:11",       // HEAD TILT1
    "PRM:/r1/c1/c2-Joint2:12",    // HEAD PAN
    "PRM:/r1/c1/c2/c3-Joint2:13", // HEAD TILT2

    "PRM:/r4/c1-Joint2:41",       // RFLEG J1 (Right Front Leg)
    "PRM:/r4/c1/c2-Joint2:42",    // RFLEG J2
    "PRM:/r4/c1/c2/c3-Joint2:43", // RFLEG J3

    "PRM:/r2/c1-Joint2:21",       // LFLEG J1 (Left Front Leg)
    "PRM:/r2/c1/c2-Joint2:22",    // LFLEG J2
    "PRM:/r2/c1/c2/c3-Joint2:23", // LFLEG J3

    "PRM:/r5/c1-Joint2:51",       // RRLEG J1 (Right Rear Leg)
    "PRM:/r5/c1/c2-Joint2:52",    // RRLEG J2
    "PRM:/r5/c1/c2/c3-Joint2:53", // RRLEG J3

    "PRM:/r3/c1-Joint2:31",       // LRLEG J1 (Left Rear Leg)
    "PRM:/r3/c1/c2-Joint2:32",    // LRLEG J2
    "PRM:/r3/c1/c2/c3-Joint2:33", // LRLEG J3

    "PRM:/p1-Sensor:p1",          // Body Sensor
    "PRM:/a1-Sensor:a1",          // ACC Sensor Front-Rear
    "PRM:/a2-Sensor:a2",          // ACC Sensor Left-Right
    "PRM:/a3-Sensor:a3",          // ACC Sensor Up-Down
};
#endif /* ERS7 */

class UDPBeacon : public OObject {
public:
    UDPBeacon();
    virtual ~UDPBeacon() {}

    OSubject*   subject[numOfSubject];
    OObserver*  observer[numOfObserver];     

    virtual OStatus DoInit   (const OSystemEvent& event);
    virtual OStatus DoStart  (const OSystemEvent& event);
    virtual OStatus DoStop   (const OSystemEvent& event);
    virtual OStatus DoDestroy(const OSystemEvent& event);

    void SendCont   (ANTENVMSG msg);
    void ReceiveCont(ANTENVMSG msg);
    void CloseCont  (ANTENVMSG msg);

    void NotifySensor(const ONotifyEvent& event);
#ifdef DEFART
    void NotifyObjInfo( const ONotifyEvent& event );
    void NotifySTNInfo( const ONotifyEvent& event );
#endif /* DEFART */

private:
    OStatus Bind   (int index);
    OStatus Send   (int index);
    OStatus Receive(int index);
    OStatus Close  (int index);
    OStatus InitUDPBuffer(int index);
    OStatus CreateUDPEndpoint(int index);
    void OpenPrimitives();
    void InitSensorIndex(OSensorFrameVectorData* sensorVec);
#ifdef DEFART
    void SetupDefartData( DefartBroadCastData *data );
    void FillFoundInfo( DefartFoundInfo *ptr, const char *tag,
			const FoundInfo *foundInfo );
    void FillSTNInfo( DefartSTNInfo *ptr, const char *tag,
		      const STNInfo *stnInfo );
    void FillSensorInfo( DefartSensorInfo *ptr, const char *tag );
#endif /* DEFART */
    void SetupTetoriData( TetoriBroadCastData *data );
    void FillLegData( TetoriLegData *ptr, const char *tag,
		      OSensorFrameVectorData* sensorVec,
		      int j1SensorIdx, int j2SensorIdx, int j3SensorIdx );

    UDPBeaconState udpBeaconState;
    bool initSensorIndex;
    int sensorCount;
    int sensorIndex[NUM_JOINTS];
    RCRegion* latestSensorRegion;
    OPrimitiveID jointID[NUM_JOINTS];
#ifdef DEFART
    CdtInfo cdtInfo;
    STNInfo stnInfo;
#endif /* DEFART */

    antStackRef    ipstackRef;
    UDPConnection  connection[UDPBEACON_CONNECTION_MAX];
};

#endif // UDPBeacon_h_DEFINED
