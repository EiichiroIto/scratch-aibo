//
// Copyright 2005 (C) Eiichiro ITO, GHC02331@nifty.com
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
// Eiichiro ITO, 15 October 2005
// mailto: GHC02331@nifty.com
//
// History
//  26 Feb 2007, Eiichiro ITO, removed SEARCH_BEACON_BY/YB from TrackingObject.
//
#ifndef Vision_h_DEFINED
#define Vision_h_DEFINED

#include <OPENR/OObject.h>
#include <OPENR/OSubject.h> 
#include <OPENR/OObserver.h>
#include <OPENR/OFbkImage.h>
#include <SystemTime.h>
#include "def.h"
#include "command.h"
#include "VisionInfo.h"
#include "CdtVision.h"
#include "MoNetData.h"
#include "Vision_Param.h"
#include "Headings.h"

#define max(x,y) ((x)>(y)?(x):(y))
#define min(x,y) ((x)<(y)?(x):(y))

enum VisionState {
    IOS_IDLE,
    IOS_START,
};
enum HeadingMode {
    HM_IDLE,
    HM_NONE,
    HM_ADJUSTING,
    HM_HEADING,
    HM_TRACKING,
};

struct ObjectIdentity {
    bool seems;           // 見えているかどうか？
    int count;            // seemsが変化してから経過した時間
    int foundThreshold;   // 見えてから「見つけた」と判断するまでの時間
    int lostThreshold;    // 見失ってから「見えなくなった」と判断するまでの時間
};

#define MAX_HEADING_POSITION 20

static const int HPOS_MIN     = 0;
static const int HPOS_MAX     = 1;
static const int HPOS_LIMIT   = 2;
static const int HPOS_DEFAULT = 3;
static const int HPOS_URIGHT  = 4;
static const int HPOS_UCENTER = 5;
static const int HPOS_ULEFT   = 6;
static const int HPOS_MRIGHT  = 7;
static const int HPOS_MCENTER = 8;
static const int HPOS_MLEFT   = 9;
static const int HPOS_DRIGHT  = 10;
static const int HPOS_DCENTER = 11;
static const int HPOS_DLEFT   = 12;
static const int HPOS_MRIGHT2 = 13;
static const int HPOS_MLEFT2  = 14;
static const int HPOS_DRIGHT2 = 15;
static const int HPOS_DLEFT2  = 16;
static const int HPOS_DCENTER2= 17;

static const char* const LAYERM_FILEFMT  = "/MS/OPEN-R/MW/DATA/E/%02d.DLM";
static const char* const LAYERC_FILEFMT  = "/MS/OPEN-R/MW/DATA/E/%02d.DLC";

enum TrackingObject {
    TRACK_NONE = 0, TRACK_BALL,
    TRACK_GOAL_B, TRACK_GOAL_Y,
    TRACK_BEACON_BY, TRACK_BEACON_YB,
    SEARCH_GOAL, SEARCH_BEACON,
    SEARCH_ANY
};

class Vision : public OObject {
private:
    static const size_t      NUM_COMMAND_VECTOR = 4;
    static const size_t      NUM_SENSOR_VECTOR  = 2;
    static const size_t      NUM_JOINTS         = 4;
    static const size_t      NUM_FRAMES         = 5;
    static const int         TILT_INDEX         = 0;
    static const int         PAN_INDEX          = 1;
    static const int         TILT2_INDEX        = 2;
    static const int         MOUTH_INDEX        = 3;
    static const byte        BALL_THRESHOLD     = 10;

public:  
    Vision();
    virtual ~Vision() {}
  
    OSubject*  subject[numOfSubject];
    OObserver* observer[numOfObserver];

    virtual OStatus DoInit   (const OSystemEvent& event);
    virtual OStatus DoStart  (const OSystemEvent& event);
    virtual OStatus DoStop   (const OSystemEvent& event);
    virtual OStatus DoDestroy(const OSystemEvent& event);
    
    void NotifyImage(const ONotifyEvent& event);
    void ReadyObjInfo(const OReadyEvent& event);
    void NotifyCommand(const ONotifyEvent& event);
    void NotifySensor(const ONotifyEvent& event);
    void NotifyPosture(const ONotifyEvent& event);
    void ReadyHeading(const OReadyEvent& event);

private:
    void InitializeJointInfo();
    void OpenPrimitive();
    void NewCommandVectorData();
    void PrintTagInfo(OFbkImageVectorData* imageVec);
    void ClearVisionInfo();
    void UpdateVisionInfo( const OFbkImage& cdtImage, const HeadingPosition *hpos );
    void IdentifyObject( ObjectIdentity *identity, FoundInfo *info, bool hasSeen );
    void IgnoreInclusions( ObjPosition *detected, int channel, EAiboIdentifiedObject label );
    void IgnoreInclusions( int valid_channel, int invalid_channel, EAiboIdentifiedObject label );
    void DetectBeacon( EAiboIdentifiedObject labelBO,
		       EAiboIdentifiedObject labelOB,
		       int channel, CdtObject* blue,
		       FoundInfo *fInfoBO, FoundInfo *fInfoOB,
		       bool *foundBeaconBO, bool *foundBeaconOB );
    void DetectBeacon();
    void DetectBall();
    void DetectGoal( EAiboIdentifiedObject label, int channel, ObjectIdentity *identity, FoundInfo *info );
    void UpdateFoundInfo();
    void FilterCImage( byte *dst, const byte *src, int w, int h );

    void ResumeSearching();
    void UpdateHeading();
    void MoveHead( const HeadingPosition *end );
    void SetJointValue(RCRegion* rgn, int idx, double start, double end);
    RCRegion* FindFreeRegion();

    void InitSensorIndex(OSensorFrameVectorData* sensorVec);
    void GetHeadingPosition(longword frameNum, HeadingPosition *hpos );
    bool InitHeading( int hcmd );
    void AddHeading( const HeadingPosition *hpos );
    void SetSpeedAll( double speed );
    void SetSpeedLast( double speed );
    bool RemoveHeading();
    void SetHeading( const HeadingPosition *hpos );
    void ForceHeading( int hcmd, const HeadingPosition *hpos );
    void InitTracking( TrackingObject tobj, double deltaTilt );
    void ExecuteHeading( int hcmd, int cmdID );
    void AdjustDiffJointValue();
    void SetupSearchBallHeadings( int hcmd, TrackingObject tobj );
    void SetupSearchHeadings( int hcmd, TrackingObject tobj );

    VisionState  visionState;
    HeadingMode  headingMode;

    RCRegion* layerMImage;
    RCRegion* layerCImage;

    CdtInfo cdtInfo;
    ObjectIdentity ballIdentity;
    ObjectIdentity goalBIdentity;	// 青ゴール
    ObjectIdentity goalYIdentity;	// 黄ゴール
    ObjectIdentity beaconBYIdentity;	// 青−黄（上から）
    ObjectIdentity beaconYBIdentity;	// 黄−青

    OPrimitiveID jointID[NUM_JOINTS];
    RCRegion* region[NUM_COMMAND_VECTOR];
    int sensoridx[NUM_JOINTS];
    list<RCRegion*> sensorRegions;

    HeadingPosition last, next;

    CdtVision cdtVision;
    int verboseMode;
    MoNetPosture currentPosture;
    int currentMsec;
    int currentLayerM;			// LayerM保存番号(0..9)
    int currentLayerC;			// LayerC保存番号(0..9)
    // HeadingPosition
    int currentHcmd;
    HeadingPosition hPos[MAX_HEADING_POSITION];
    int nHPos;
    int waitingCount;
    int seqno;
    // Tracking
    TrackingObject trackingObj;
    TrackingObject lastTrackingObj;
    double deltaTilt;
    // Headings
    Headings headings;
    // Seaching
    bool foundThenTracking;
};

#endif // Vision_h_DEFINED
