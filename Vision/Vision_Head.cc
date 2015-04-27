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
//  26 Feb 2007, Eiichiro ITO, modified beacon codes.
//
#include <OPENR/ODataFormats.h>
#include <OPENR/OFbkImage.h>
#include <OPENR/OPENRAPI.h>
#include <OPENR/OSyslog.h>
#include <OPENR/core_macro.h>
#include "Vision.h"

#ifdef ERS210
static const char* const JOINT_LOCATOR[] = {
    "PRM:/r1/c1-Joint2:j1",          // HEAD TILT
    "PRM:/r1/c1/c2-Joint2:j2",       // HEAD PAN
    "PRM:/r1/c1/c2/c3-Joint2:j3",    // HEAD ROLL
    "PRM:/r1/c1/c2/c3/c4-Joint2:j4", // MOUTH
};
static const HeadingPosition HPositions[] = {
    { -60, -89, -29, -40, 1.0 }, // MIN
    {  43,  89,  29,  -3, 1.0 }, // MAX
    { 1.9, 2.7, 2.7,   4, 1.0 }, // LIMIT
    {   0,   0,   0,  -3, 1.0 }, // DEFAULT
    {  40, -88,   0,  -3, 1.0 }, // URIGHT
    {  40,   0,   0,  -3, 1.0 }, // UCENTER
    {  40,  88,   0,  -3, 1.0 }, // ULEFT
    {   0, -88,   0,  -3, 1.0 }, // MRIGHT
    {   0,   0,   0,  -3, 1.0 }, // MCENTER
    {   0,  88,   0,  -3, 1.0 }, // MLEFT
    { -80, -88,   0,  -3, 1.0 }, // DRIGHT
    { -80,   0,   0,  -3, 1.0 }, // DCENTER
    { -80,  88,   0,  -3, 1.0 }, // DLEFT
    {   0, -44,   0,  -3, 1.0 }, // MRIGHT2
    {   0,  44,   0,  -3, 1.0 }, // MLEFT2
    { -55, -44,   0,  -3, 1.0 }, // DRIGHT2
    { -55,  44,   0,  -3, 1.0 }, // DLEFT2
    { -55,   0,   0,  -3, 1.0 }, // DCENTER2
};
#endif // ERS210

#ifdef ERS7
static const char* const JOINT_LOCATOR[] = {
    "PRM:/r1/c1-Joint2:11",          // HEAD TILT1
    "PRM:/r1/c1/c2-Joint2:12",       // HEAD PAN
    "PRM:/r1/c1/c2/c3-Joint2:13",    // HEAD TILT2
    "PRM:/r1/c1/c2/c3/c4-Joint2:14", // MOUTH
};
static const HeadingPosition HPositions[] = {
    { -75, -88, -15, -55, 1.0 }, // MIN
    {   0,  88,  45,  -3, 1.0 }, // MAX
    { 1.4, 4.6, 2.6, 4.6, 1.0 }, // LIMIT
    {   0,   0,  10,  -3, 1.0 }, // DEFAULT
    {   0, -88,  20,  -3, 1.0 }, // URIGHT
    {   0,   0,  20,  -3, 1.0 }, // UCENTER
    {   0,  88,  20,  -3, 1.0 }, // ULEFT
    {   0, -88,  10,  -3, 1.0 }, // MRIGHT
    {   0,   0,  10,  -3, 1.0 }, // MCENTER
    {   0,  88,  10,  -3, 1.0 }, // MLEFT
    {   0, -88, -15,  -3, 1.0 }, // DRIGHT
    { -60,   0, -10,  -3, 1.0 }, // DCENTER
    {   0,  88, -15,  -3, 1.0 }, // DLEFT
    {   0, -30,   0,  -3, 1.0 }, // MRIGHT2
    {   0,  30,   0,  -3, 1.0 }, // MLEFT2
    {   0, -44,  10,  -3, 1.2 }, // DRIGHT2
    {   0,  44,  10,  -3, 1.2 }, // DLEFT2
    {   0,   0, -15,  -3, 1.0 }, // DCENTER2
};
#endif // ERS7

#define isNearLeft() (last.pan > 0)

void
Vision::InitializeJointInfo()
{
    for (int i = 0; i < NUM_JOINTS; i++) {
	jointID[i] = oprimitiveID_UNDEF;
    }
    for (int i = 0; i < NUM_COMMAND_VECTOR; i++) {
	region[i] = 0;
    }
    for (int i = 0; i < NUM_JOINTS; i++) {
	sensoridx[i] = -1;
    }
}

void
Vision::OpenPrimitive()
{
    OStatus result;

    for (int i = 0; i < NUM_JOINTS; i++) {
        result = OPENR::OpenPrimitive(JOINT_LOCATOR[i], &jointID[i]);
        if (result != oSUCCESS) {
            OSYSLOG1((osyslogERROR, "%s : %s (%s) %d",
                      "Vision::OpenPrimitives()",
                      "OPENR::OpenPrimitive() FAILED",
		      JOINT_LOCATOR[i],
		      result));
        }
    }
}

void
Vision::NewCommandVectorData()
{
    OStatus result;
    MemoryRegionID      cmdVecDataID;
    OCommandVectorData* cmdVecData;
    OCommandInfo*       info;

    for (int i = 0; i < NUM_COMMAND_VECTOR; i++) {
        result = OPENR::NewCommandVectorData( NUM_JOINTS,
					      &cmdVecDataID, &cmdVecData);
        if (result != oSUCCESS) {
            OSYSLOG1((osyslogERROR, "%s : %s %d",
                      "Vision::NewCommandVectorData()",
                      "OPENR::NewCommandVectorData() FAILED", result));
        }
        region[i] = new RCRegion(cmdVecData->vectorInfo.memRegionID,
                                 cmdVecData->vectorInfo.offset,
                                 (void*)cmdVecData,
                                 cmdVecData->vectorInfo.totalSize);
        cmdVecData->SetNumData(NUM_JOINTS);
        for (int j = 0; j < NUM_JOINTS; j++) {
            info = cmdVecData->GetInfo(j);
            info->Set(odataJOINT_COMMAND2, jointID[j], NUM_FRAMES);
        }
    }
}

void
Vision::SetupSearchBallHeadings( int hcmd, TrackingObject tobj )
{
    //OSYSPRINT(("Vision::SetupSearchBallHeadings %d.\n", tobj ));
    if ( InitHeading( hcmd ) ) {
	if ( isNearLeft() ) {
	    AddHeading( &HPositions[HPOS_DLEFT] );
	    AddHeading( &HPositions[HPOS_DLEFT] );
	    SetSpeedLast( 4.0 );
	    AddHeading( &HPositions[HPOS_MLEFT2] );
	    AddHeading( &HPositions[HPOS_MLEFT2] );
	    SetSpeedLast( 4.0 );
	    AddHeading( &HPositions[HPOS_MRIGHT2] );
	    AddHeading( &HPositions[HPOS_MRIGHT2] );
	    SetSpeedLast( 4.0 );
	    AddHeading( &HPositions[HPOS_DRIGHT] );
	    AddHeading( &HPositions[HPOS_DRIGHT] );
	    SetSpeedLast( 4.0 );
	} else {
	    AddHeading( &HPositions[HPOS_DRIGHT] );
	    AddHeading( &HPositions[HPOS_DRIGHT] );
	    SetSpeedLast( 4.0 );
	    AddHeading( &HPositions[HPOS_MRIGHT2] );
	    AddHeading( &HPositions[HPOS_MRIGHT2] );
	    SetSpeedLast( 4.0 );
	    AddHeading( &HPositions[HPOS_MLEFT2] );
	    AddHeading( &HPositions[HPOS_MLEFT2] );
	    SetSpeedLast( 4.0 );
	    AddHeading( &HPositions[HPOS_DLEFT] );
	    AddHeading( &HPositions[HPOS_DLEFT] );
	    SetSpeedLast( 4.0 );
	}
	AddHeading( &HPositions[HPOS_DEFAULT] );
	AddHeading( &HPositions[HPOS_DCENTER2] );
	AddHeading( &HPositions[HPOS_DCENTER2] );
	SetSpeedLast( 4.0 );
	AddHeading( &HPositions[HPOS_DEFAULT] );
	AddHeading( &HPositions[HPOS_DEFAULT] );
	SetSpeedLast( 4.0 );
	RemoveHeading();
	trackingObj = tobj;
	lastTrackingObj = TRACK_NONE;
	deltaTilt = 0;
	foundThenTracking = true;
    }
}

void
Vision::SetupSearchHeadings( int hcmd, TrackingObject tobj )
{
    //OSYSPRINT(("Vision::SetupSearchHeadings %d.\n", tobj ));
    if ( InitHeading( hcmd ) ) {
	if ( isNearLeft() ) {
	    AddHeading( &HPositions[HPOS_MLEFT] );
	    AddHeading( &HPositions[HPOS_MLEFT] );
	    SetSpeedLast( 4.0 );
	    AddHeading( &HPositions[HPOS_MRIGHT] );
	    AddHeading( &HPositions[HPOS_MRIGHT] );
	    SetSpeedLast( 4.0 );
	} else {
	    AddHeading( &HPositions[HPOS_MRIGHT] );
	    AddHeading( &HPositions[HPOS_MRIGHT] );
	    SetSpeedLast( 4.0 );
	    AddHeading( &HPositions[HPOS_MLEFT] );
	    AddHeading( &HPositions[HPOS_MLEFT] );
	    SetSpeedLast( 4.0 );
	}
	AddHeading( &HPositions[HPOS_DEFAULT] );
	AddHeading( &HPositions[HPOS_DEFAULT] );
	SetSpeedLast( 4.0 );
	RemoveHeading();
	trackingObj = tobj;
	lastTrackingObj = TRACK_NONE;
	deltaTilt = 0;
	foundThenTracking = true;
    }
}

void
Vision::NotifyCommand(const ONotifyEvent& event)
{
    VCmdInfo *vCmdInfo = (VCmdInfo*) event.Data(0);
    int hcmd = vCmdInfo->hcmd;
    int arg1 = vCmdInfo->iValue;
    char path[256];

    if ( visionState == IOS_IDLE ) {
	OSYSPRINT(("Vision::NotifyCommand\n" ));
	return;
    }
    seqno = vCmdInfo->seqno;
    if ( verboseMode >= 9 ) {
	OSYSPRINT(("Vision::NotifyCommand hcmd=%d,seqno=%d\n", hcmd, seqno ));
    }
    if ( hcmd >= 100 ) {
	arg1 = hcmd % 100;
	hcmd = hcmdExecute;
    }
    if ( hcmd == hcmdInitialize ) {
	OSYSPRINT(("Vision::NotifyCommand initialize.\n"));
	headingMode = HM_ADJUSTING;
	AdjustDiffJointValue();
    } else if ( hcmd == hcmdSetVerboseMode ) {
	OSYSPRINT(("Vision::NotifyCommand set verbose mode to %d\n",
		   arg1 ));
	verboseMode = arg1;
    } else if ( headingMode == HM_IDLE ) {
	OSYSPRINT(("Vision::NotifyCommand cmd(%d) ignored in idle mode\n",
		   hcmd ));
    } else {
	if ( hcmd == hcmdRelease ) {
	    OSYSPRINT(("Vision::NotifyCommand release.\n"));
	    headingMode = HM_IDLE;
	} if ( hcmd == hcmdSetHeading ) {
	    ForceHeading( hcmd, &vCmdInfo->hpos );
	} else if ( hcmd == hcmdNormal ) {
	    ForceHeading( hcmd, &HPositions[HPOS_DEFAULT] );
	} else if ( hcmd == hcmdDown ) {
	    ForceHeading( hcmd, &HPositions[HPOS_DCENTER] );
	} else if ( hcmd == hcmdReadHeadings ) {
	    headings.ReadConfig( HEAD_MOTION_CONFIG );
	} else if ( hcmd == hcmdExecute ) {
	    ExecuteHeading( hcmd, arg1 );
	} else if ( hcmd == hcmdStop ) {
	    headingMode = HM_NONE;
	    currentHcmd = hcmdNone;
	} else if ( hcmd == hcmdSearchBallSlow ) {
	    SetupSearchBallHeadings( hcmd, TRACK_BALL );
	} else if ( hcmd == hcmdSearchBall ) {
	    SetupSearchBallHeadings( hcmd, TRACK_BALL );
	} else if ( hcmd == hcmdSearchAny ) {
	    SetupSearchHeadings( hcmd, SEARCH_ANY );
	} else if ( hcmd == hcmdSearchBeacon ) {
	    SetupSearchHeadings( hcmd, SEARCH_BEACON );
	} else if ( hcmd == hcmdSearchBeaconBY ) {
	    SetupSearchHeadings( hcmd, TRACK_BEACON_BY );
	} else if ( hcmd == hcmdSearchBeaconYB ) {
	    SetupSearchHeadings( hcmd, TRACK_BEACON_YB );
	} else if ( hcmd == hcmdSearchGoal ) {
	    SetupSearchHeadings( hcmd, SEARCH_GOAL );
	} else if ( hcmd == hcmdSearchGoalB ) {
	    SetupSearchHeadings( hcmd, TRACK_GOAL_B );
	} else if ( hcmd == hcmdSearchGoalY ) {
	    SetupSearchHeadings( hcmd, TRACK_GOAL_Y );
	} else if ( hcmd == hcmdTrackBall ) {
	    InitTracking( TRACK_BALL, 0 );
	} else if ( hcmd == hcmdTrackBallUpper ) {
	    InitTracking( TRACK_BALL, 15 );
	} else if ( hcmd == hcmdTrackGoalB ) {
	    InitTracking( TRACK_GOAL_B, 0 );
	} else if ( hcmd == hcmdTrackGoalY ) {
	    InitTracking( TRACK_GOAL_Y, 0 );
	} else if ( hcmd == hcmdTrackBeaconBY ) {
	    InitTracking( TRACK_BEACON_BY, 0 );
	} else if ( hcmd == hcmdTrackBeaconYB ) {
	    InitTracking( TRACK_BEACON_YB, 0 );
	} else if ( hcmd == hcmdFaceToBall ) {
	    if ( InitHeading( hcmd ) ) {
		HeadingPosition hpos = HPositions[HPOS_DEFAULT];
		hpos.pan = vCmdInfo->hpos.pan;
		AddHeading( &hpos );
		RemoveHeading();
		trackingObj = TRACK_BALL;
		lastTrackingObj = TRACK_NONE;
		OSYSPRINT(("Vision::NotifyCommand FaceToBall(%d).\n",
			   trackingObj ));
		deltaTilt = 0;
		foundThenTracking = true;
	    }
	} else if ( hcmd == hcmdTakeLayerM ) {
	    sprintf( path, LAYERM_FILEFMT, currentLayerM );
	    FILE *fp = fopen( path, "wb" );
	    fwrite( layerMImage->Base(), 1, LAYER_M_IMAGESIZE, fp );
	    fclose( fp );
	    currentLayerM = (currentLayerM + 1) % 10;
	} else if ( hcmd == hcmdTakeLayerC ) {
	    sprintf( path, LAYERC_FILEFMT, currentLayerC );
	    FILE *fp = fopen( path, "wb" );
	    fwrite( layerCImage->Base(), 1, LAYER_C_IMAGESIZE, fp );
	    fclose( fp );
	    currentLayerC = (currentLayerC + 1) % 10;
	}
    }

    observer[event.ObsIndex()]->AssertReady(event.SenderID());
}

void
Vision::UpdateHeading()
{
    if ( headingMode == HM_TRACKING ) {
	double ballPan;
	double ballTilt;

	if ( trackingObj == TRACK_BALL ) {
	    if ( cdtInfo.foundInfo[ IO_BALL ].found ) {
		ballPan  = cdtInfo.foundInfo[ IO_BALL ].objPos.pan;
		ballTilt = cdtInfo.foundInfo[ IO_BALL ].objPos.tilt;
		ballTilt += deltaTilt;
#ifdef ERS7
		ballTilt = min(ballTilt,15.0);
#endif /* ERS7 */
	    } else {
		ResumeSearching();
		return;
	    }
	} else if ( trackingObj == TRACK_GOAL_B ) {
	    EAiboIdentifiedObject obj = IO_GOAL_B;
	    if ( cdtInfo.foundInfo[ obj ].found ) {
		ballPan  = cdtInfo.foundInfo[ obj ].objPos.pan;
		ballTilt = 10;
	    } else {
		ResumeSearching();
		return;
	    }
	} else if ( trackingObj == TRACK_GOAL_Y ) {
	    EAiboIdentifiedObject obj = IO_GOAL_Y;
	    if ( cdtInfo.foundInfo[ obj ].found ) {
		ballPan  = cdtInfo.foundInfo[ obj ].objPos.pan;
		ballTilt = 10;
	    } else {
		ResumeSearching();
		return;
	    }
	} else if ( trackingObj == TRACK_BEACON_BY ) {
	    EAiboIdentifiedObject obj = IO_BEACON_BY;
	    if ( cdtInfo.foundInfo[ obj ].found ) {
		ballPan  = cdtInfo.foundInfo[ obj ].objPos.pan;
		ballTilt = 20;
	    } else {
		ResumeSearching();
		return;
	    }
	} else if ( trackingObj == TRACK_BEACON_YB ) {
	    EAiboIdentifiedObject obj = IO_BEACON_YB;
	    if ( cdtInfo.foundInfo[ obj ].found ) {
		ballPan  = cdtInfo.foundInfo[ obj ].objPos.pan;
		ballTilt = 20;
	    } else {
		ResumeSearching();
		return;
	    }
	} else {
	    MoveHead( &next );
	    return;
	}
	HeadingPosition current;
	current = HPositions[HPOS_DEFAULT];
	current.pan = - ballPan;
#ifdef ERS210
	current.tilt = ballTilt;
#endif
#ifdef ERS7
	current.tilt2 = ballTilt;
#endif
	SetHeading( &current );
	MoveHead( &next );
    } else if ( headingMode == HM_HEADING ) {
	MoveHead( &next );
	if ( last.pan == next.pan && last.tilt == next.tilt
	     && last.tilt2 == next.tilt2 && last.mouth == next.mouth ) {
	    RemoveHeading();
	}
    }
}

void
Vision::ResumeSearching()
{
    if ( lastTrackingObj == TRACK_NONE ) {
	MoveHead( &next );
	return;
    }
    OSYSPRINT(("Vision::ResumeSearhing trackingObj(%d)\n", trackingObj));
    waitingCount = 0;
    trackingObj = lastTrackingObj;
    lastTrackingObj = TRACK_NONE;
    foundThenTracking = true;
    headingMode = HM_HEADING;
    RemoveHeading();
    MoveHead( &next );
}

void
Vision::MoveHead( const HeadingPosition *end )
{
    RCRegion* rgn = FindFreeRegion();
    HeadingPosition start;

    if ( rgn == NULL ) {
	OSYSPRINT(("Vision::MoveHead rgn=NULL\n"));
	return;
    }

    start.tilt  = last.tilt;
    start.pan   = last.pan;
    start.tilt2 = last.tilt2;
    start.mouth = last.mouth;
    last.tilt   = end->tilt;
    last.pan    = end->pan;
    last.tilt2  = end->tilt2;
    last.mouth  = end->mouth;

    double speed = end->speed;
    speed = max(speed, 0.1);
    speed = min(speed, 1.2);

    if ( verboseMode >= 8 ) {
	OSYSPRINT(( "speed=%lf,%lf\n", end->speed, speed ));
    }
    double limit;
    limit = HPositions[HPOS_LIMIT].tilt;
    limit *= speed;
    limit = limit * NUM_FRAMES / 2;
    if ( last.tilt > start.tilt + limit ) {
        last.tilt = start.tilt + limit;
    } else if ( last.tilt < start.tilt - limit ) {
        last.tilt = start.tilt - limit;
    }
    limit = HPositions[HPOS_LIMIT].pan;
    limit *= speed;
    limit = limit * NUM_FRAMES / 2;
    if ( last.pan > start.pan + limit ) {
        last.pan = start.pan + limit;
    } else if ( last.pan < start.pan - limit ) {
        last.pan = start.pan - limit;
    }
    limit = HPositions[HPOS_LIMIT].tilt2;
    limit *= speed;
    limit = limit * NUM_FRAMES / 2;
    if ( last.tilt2 > start.tilt2 + limit ) {
        last.tilt2 = start.tilt2 + limit;
    } else if ( last.tilt2 < start.tilt2 - limit ) {
        last.tilt2 = start.tilt2 - limit;
    }
    limit = HPositions[HPOS_LIMIT].mouth;
    limit *= speed;
    limit = limit * NUM_FRAMES / 2;
    if ( last.mouth > start.mouth + limit ) {
        last.mouth = start.mouth + limit;
    } else if ( last.mouth < start.mouth - limit ) {
        last.mouth = start.mouth - limit;
    }
    SetJointValue(rgn, TILT_INDEX,  start.tilt,  last.tilt);
    SetJointValue(rgn, PAN_INDEX,   start.pan,   last.pan);
    SetJointValue(rgn, TILT2_INDEX, start.tilt2, last.tilt2);
    SetJointValue(rgn, MOUTH_INDEX, start.mouth, last.mouth);
    subject[sbjJoint]->SetData(rgn);
    subject[sbjJoint]->NotifyObservers();
    if ( verboseMode >= 8 ) {
	OSYSPRINT(( "Vision::MoveHead(t%.1f,p%.1f,t%.1f,m%.1f)\n",
		    last.tilt, last.pan, last.tilt2, last.mouth ));
    }
}

bool
Vision::InitHeading( int hcmd )
{
    if ( currentHcmd == hcmd ) {
	// 同じコマンドの実行中は継続させる
	OSYSPRINT(("Vision::InitHeading ignored\n"));
	return false;
    }
    currentHcmd = hcmd;
    nHPos = 0;
    waitingCount = 0;
    foundThenTracking = false;
    if ( verboseMode >= 7 ) {
	OSYSPRINT(( "Vision::InitHeading hcmd=%d\n", currentHcmd ));
    }
    return true;
}

void
Vision::AddHeading( const HeadingPosition *hpos )
{
    if ( nHPos >= MAX_HEADING_POSITION ) {
	return;
    }
    if ( verboseMode >= 7 ) {
	OSYSPRINT(( "Vision::AddHeading(%d) %d\n", currentHcmd, nHPos ));
    }
    hPos[ nHPos ].tilt  = hpos->tilt;
    hPos[ nHPos ].pan   = hpos->pan;
    hPos[ nHPos ].tilt2 = hpos->tilt2;
    hPos[ nHPos ].mouth = hpos->mouth;
    hPos[ nHPos ].speed = hpos->speed;
    nHPos ++;
}

void
Vision::SetSpeedAll( double speed )
{
    for ( int i = 0; i < nHPos; i ++ ) {
	hPos[ i ].speed = speed;
    }
}

void
Vision::SetSpeedLast( double speed )
{
    if ( !nHPos ) {
	return;
    }
    hPos[ nHPos - 1 ].speed = speed;
}

bool
Vision::RemoveHeading()
{
    if ( nHPos <= 0 ) {
	// IOS_HEADING完了
	if ( verboseMode >= 7 ) {
	    OSYSPRINT(( "Adjusting Head Position (%d) completed.\n",
			currentHcmd ));
	}
	currentHcmd = hcmdNone;
	if ( foundThenTracking ) {
	  headingMode = HM_TRACKING;
	} else {
	  headingMode = HM_NONE;
	}
	cdtInfo.seqno = seqno;
	return false;
    }
    if ( waitingCount > 0 ) {
	if ( verboseMode >= 7 ) {
	    OSYSPRINT(("waiting: %d\n", waitingCount));
	}
	waitingCount --;
	return true;
    }
    HeadingPosition *hpos = &hPos[0];
    if ( next.tilt == hpos->tilt
	 && next.pan == hpos->pan
	 && next.tilt2 == hpos->tilt2
	 && next.mouth == hpos->mouth ) {
	waitingCount = (int) hpos->speed;
    }
    SetHeading( &hPos[0] );
    if ( headingMode == HM_NONE ) {
	MoveHead( &next );
	waitingCount = 0;
    }
    headingMode = HM_HEADING;
    for ( int i = 1; i < nHPos; i ++ ) {
	hPos[ i - 1 ].tilt = hPos[ i ].tilt;
	hPos[ i - 1 ].pan = hPos[ i ].pan;
	hPos[ i - 1 ].tilt2 = hPos[ i ].tilt2;
	hPos[ i - 1 ].mouth = hPos[ i ].mouth;
	hPos[ i - 1 ].speed = hPos[ i ].speed;
    }
    nHPos --;
    return true;
}

void
Vision::SetHeading( const HeadingPosition *hpos )
{
    next.tilt  = hpos->tilt;
    next.pan   = hpos->pan;
    next.tilt2 = hpos->tilt2;
    next.mouth = hpos->mouth;
    next.speed = hpos->speed;

    if ( verboseMode >= 7 ) {
	OSYSPRINT(("Vision::SetHeading speed=%lf\n", next.speed));
    }

    const HeadingPosition *min = &HPositions[HPOS_MIN];
    if ( next.tilt < min->tilt ) {
	next.tilt = min->tilt;
    }
    if ( next.pan < min->pan ) {
	next.pan = min->pan;
    }
    if ( next.tilt2 < min->tilt2 ) {
	next.tilt2 = min->tilt2;
    }
    if ( next.mouth < min->mouth ) {
	next.mouth = min->mouth;
    }
    const HeadingPosition *max = &HPositions[HPOS_MAX];
    if ( next.tilt > max->tilt ) {
	next.tilt = max->tilt;
    }
    if ( next.pan > max->pan ) {
	next.pan = max->pan;
    }
    if ( next.tilt2 > max->tilt2 ) {
	next.tilt2 = max->tilt2;
    }
    if ( next.mouth > max->mouth ) {
	next.mouth = max->mouth;
    }
    if ( verboseMode >= 7 ) {
	OSYSPRINT(( "Vision::SetHeading(t%.1f,p%.1f,t%.1f,m%.1f)\n",
		    next.tilt, next.pan, next.tilt2, next.mouth ));
    }
}

void
Vision::ForceHeading( int hcmd, const HeadingPosition *hpos )
{
    if ( verboseMode >= 7 ) {
	OSYSPRINT(( "Vision::ForceHeading hcmd=%d\n", hcmd ));
    }
    if ( InitHeading( hcmd ) ) {
	AddHeading( hpos );
	RemoveHeading();
    }
}

void
Vision::InitTracking( TrackingObject tobj, double _deltaTilt )
{
    bool doUpdate;

    if ( headingMode == HM_TRACKING && trackingObj == tobj
	 && deltaTilt == _deltaTilt ) {
	OSYSPRINT(("Vision::InitTracking ignored\n"));
	return;
    }
    doUpdate = headingMode == HM_IDLE || headingMode == HM_NONE;
    headingMode = HM_TRACKING;
    trackingObj = tobj;
    lastTrackingObj = TRACK_NONE;
    deltaTilt = _deltaTilt;
    currentHcmd = hcmdNone;
    if ( doUpdate ) {
	UpdateHeading();
    }
}

void
Vision::SetJointValue(RCRegion* rgn,
		      int idx, double start, double end)
{
    OCommandVectorData* cmdVecData = (OCommandVectorData*)rgn->Base();

    OCommandInfo* info = cmdVecData->GetInfo(idx);
    info->Set(odataJOINT_COMMAND2, jointID[idx], NUM_FRAMES);

    OCommandData* data = cmdVecData->GetData(idx);
    OJointCommandValue2* jval = (OJointCommandValue2*)data->value;

    double delta = end - start;
    for (int i = 0; i < NUM_FRAMES; i++) {
        double dval = start + (delta * i) / (double)NUM_FRAMES;
        jval[i].value = oradians(dval);
    }
}

RCRegion*
Vision::FindFreeRegion()
{
    for (int i = 0; i < NUM_COMMAND_VECTOR; i++) {
        if (region[i]->NumberOfReference() == 1) return region[i];
    }
    return 0;
}

void
Vision::NotifySensor(const ONotifyEvent& event)
{
    static bool initSensorIndex = false;

    if (visionState == IOS_IDLE) {
        return; // do nothing
    }
    RCRegion* rgn = event.RCData(0);
    if (initSensorIndex == false) {
        OSensorFrameVectorData* sv = (OSensorFrameVectorData*)rgn->Base();
        InitSensorIndex(sv);
        initSensorIndex = true;
    }
    if (sensorRegions.size() == NUM_SENSOR_VECTOR) {
        sensorRegions.front()->RemoveReference();
        sensorRegions.pop_front();
    }
    rgn->AddReference();
    sensorRegions.push_back(rgn);

    observer[event.ObsIndex()]->AssertReady();
}

void
Vision::InitSensorIndex(OSensorFrameVectorData* sensorVec)
{
    // jointID[] is already initialized in Vision::OpenPrimitives().
    for (int i = 0; i < NUM_JOINTS; i++) {
        for (int j = 0; j < sensorVec->vectorInfo.numData; j++) {
            OSensorFrameInfo* info = sensorVec->GetInfo(j);
            if (info->primitiveID == jointID[i]) {
                sensoridx[i] = j;
                OSYSDEBUG(("[%2d] %s\n", sensoridx[i], JOINT_LOCATOR[i]));
                break;
            }
        }
    }
}

void
Vision::GetHeadingPosition( longword frameNum, HeadingPosition *hpos )
{
    list<RCRegion*>::iterator iter = sensorRegions.begin();
    list<RCRegion*>::iterator last = sensorRegions.end();
    OSensorFrameVectorData* sv;
    OSensorFrameInfo* info;
    OSensorFrameData* data;

    while (iter != last) {
        sv = (OSensorFrameVectorData*)(*iter)->Base();
        info = sv->GetInfo(sensoridx[PAN_INDEX]);
        longword fn = info->frameNumber;
        for (int i = 0; i < info->numFrames; i++) {
            if (fn == frameNum) {
                data = sv->GetData(sensoridx[TILT_INDEX]);
                hpos->tilt = degrees((double)data->frame[i].value/1000000.0);
                data = sv->GetData(sensoridx[PAN_INDEX]);
                hpos->pan = degrees((double)data->frame[i].value/1000000.0);
                data = sv->GetData(sensoridx[TILT2_INDEX]);
                hpos->tilt2 = degrees((double)data->frame[i].value/1000000.0);
                data = sv->GetData(sensoridx[MOUTH_INDEX]);
                hpos->mouth = degrees((double)data->frame[i].value/1000000.0);
                return;
            }
            fn++;
            if (fn > oframeMAX_NUMBER) fn = 1;
        }
        ++iter;
    }
    //
    // If sensor data at frameNum is not found, return latest sensor data.
    //
    RCRegion* rgn = sensorRegions.back();
    sv = (OSensorFrameVectorData*)rgn->Base();
    info = sv->GetInfo(sensoridx[PAN_INDEX]);
    data = sv->GetData(sensoridx[TILT_INDEX]);
    hpos->tilt = degrees((double)data->frame[info->numFrames-1].value
			 / 1000000.0);
    data = sv->GetData(sensoridx[PAN_INDEX]);
    hpos->pan = degrees((double)data->frame[info->numFrames-1].value
			/ 1000000.0);
    data = sv->GetData(sensoridx[TILT2_INDEX]);
    hpos->tilt2 = degrees((double)data->frame[info->numFrames-1].value
			  / 1000000.0);
    data = sv->GetData(sensoridx[MOUTH_INDEX]);
    hpos->mouth = degrees((double)data->frame[info->numFrames-1].value
			  / 1000000.0);
}

void
Vision::NotifyPosture(const ONotifyEvent& event)
{
    MoNetPosture newPosture = *(MoNetPosture*) event.Data(0);

    if ( newPosture != currentPosture ) {
	currentPosture = newPosture;
	if ( verboseMode >= 9 ) {
	    OSYSPRINT(( "Vision::NotifyPosture %d\n", currentPosture ));
	}
    }
    observer[event.ObsIndex()]->AssertReady();
}

void
Vision::ExecuteHeading( int hcmd, int cmdID )
{
    if ( verboseMode >= 7 ) {
	OSYSPRINT(( "Vision::ExecuteHeading cmdID=%d\n", cmdID ));
    }
    if ( InitHeading( hcmd ) ) {
	for ( int i = 0; i < MAX_HDPOS; i ++ ) {
	    const HeadingPosition *hp = headings.GetHeadingPosition(cmdID, i);
	    if ( !hp ) {
		break;
	    }
	    AddHeading( hp );
	}
	RemoveHeading();
    }
}

void
Vision::ReadyHeading(const OReadyEvent& event)
{
    OSYSDEBUG(("Vision::ReadyHeading (%d,%d)\n", visionState, headingMode));

    if ( visionState == IOS_IDLE ) {
	return;
    }
    if ( headingMode == HM_IDLE || headingMode == HM_NONE ) {
	OSYSPRINT(("Vision::ReadyHeading ilde or none.\n"));
	currentHcmd = hcmdNone;
    } else if ( headingMode == HM_ADJUSTING ) {
	OSYSPRINT(("Vision::ReadyHeading adjusting done.\n"));
	headingMode = HM_NONE;
	currentHcmd = hcmdNone;
    } else {
	UpdateHeading();
    }
}

void
Vision::AdjustDiffJointValue()
{
    RCRegion* rgn = FindFreeRegion();
    OJointValue current[NUM_JOINTS];

    for (int i = 0; i < NUM_JOINTS; i++) {
        OPENR::GetJointValue(jointID[i], &current[i]);
        SetJointValue( rgn, i,
                       degrees(current[i].value/1000000.0),
                       degrees(current[i].value/1000000.0) );
    }
    subject[sbjJoint]->SetData(rgn);
    subject[sbjJoint]->NotifyObservers();
}
