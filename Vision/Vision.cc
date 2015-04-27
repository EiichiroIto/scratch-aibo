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

#define systemTimeToMilliSeconds(x) ((x).seconds*1000+(x).useconds/1000)

Vision::Vision() : visionState(IOS_IDLE), headingMode(HM_IDLE),
		   verboseMode(VERBOSE_NONE),
		   currentPosture(monetpostureUNDEF),
		   currentLayerM(0), currentLayerC(0),
		   trackingObj(TRACK_NONE),
		   currentHcmd(hcmdNone), nHPos(0), waitingCount(0), seqno(0),
		   layerMImage(NULL), layerCImage(NULL),
		   deltaTilt(0), foundThenTracking(false)
{
    last.pan = 0;
    last.tilt = 0;
    last.tilt2 = 0;
    last.mouth = 0;
    next.pan = 0;
    next.tilt = 0;
    next.tilt2 = 0;
    next.mouth = 0;
    ClearVisionInfo();
    InitializeJointInfo();
    ballIdentity.foundThreshold = 2;
    ballIdentity.lostThreshold = 5;
    goalBIdentity.foundThreshold = 2;
    goalBIdentity.lostThreshold = 5;
    goalYIdentity.foundThreshold = 2;
    goalYIdentity.lostThreshold = 5;
    beaconBYIdentity.foundThreshold = 2;
    beaconBYIdentity.lostThreshold = 5;
    beaconYBIdentity.foundThreshold = 2;
    beaconYBIdentity.lostThreshold = 5;
    verboseMode = 6;
    lastTrackingObj = TRACK_NONE;
}

OStatus
Vision::DoInit(const OSystemEvent& event)
{
    NEW_ALL_SUBJECT_AND_OBSERVER;
    REGISTER_ALL_ENTRY;
    SET_ALL_READY_AND_NOTIFY_ENTRY;

    OpenPrimitive();
    NewCommandVectorData();
    layerMImage = new RCRegion( LAYER_M_IMAGESIZE );
    layerCImage = new RCRegion( LAYER_C_IMAGESIZE );
    ReadGeometry( DEFART_CONFIG );
    headings.ReadConfig( HEAD_MOTION_CONFIG );

    return oSUCCESS;
}

OStatus
Vision::DoStart(const OSystemEvent& event)
{
    visionState = IOS_START;

    ENABLE_ALL_SUBJECT;
    ASSERT_READY_TO_ALL_OBSERVER;

    return oSUCCESS;
}    

OStatus
Vision::DoStop(const OSystemEvent& event)
{
    visionState = IOS_IDLE;

    DISABLE_ALL_SUBJECT;
    DEASSERT_READY_TO_ALL_OBSERVER;

    return oSUCCESS;
}

OStatus
Vision::DoDestroy(const OSystemEvent& event)
{
    DELETE_ALL_SUBJECT_AND_OBSERVER;

    layerMImage->RemoveReference();
    layerCImage->RemoveReference();

    return oSUCCESS;
}

void
Vision::NotifyImage(const ONotifyEvent& event)
{
    if (visionState == IOS_IDLE) {
        return; // do nothing
    }

    OFbkImageVectorData* fbkImageVectorData 
        = (OFbkImageVectorData*)event.Data(0);
    OFbkImageInfo* info;
    byte* data;
    byte* dst;

    info = fbkImageVectorData->GetInfo(ofbkimageLAYER_M);
    data = fbkImageVectorData->GetData(ofbkimageLAYER_M);

    dst = (byte*) layerMImage->Base();
    for ( int y = 0; y < LAYER_M_HEIGHT; y += 1 ) {
	memcpy( dst, data, LAYER_M_WIDTH * 3 );
	dst += LAYER_M_WIDTH * 3;
	data += LAYER_M_WIDTH * 3;
	data += LAYER_M_WIDTH * 3;
    }

    info = fbkImageVectorData->GetInfo(ofbkimageLAYER_C);
    data = fbkImageVectorData->GetData(ofbkimageLAYER_C);
    dst = (byte*) layerCImage->Base();
    FilterCImage( dst, data, LAYER_C_WIDTH, LAYER_C_HEIGHT );
    //memcpy( dst, data, LAYER_C_IMAGESIZE );

    SystemTime currentTime;
    GetSystemTime( &currentTime );
    currentMsec = systemTimeToMilliSeconds(currentTime);

    HeadingPosition current;
    OFbkImage cdtImage( info, dst, ofbkimageBAND_CDT );
    GetHeadingPosition( info->frameNumber, &current );
    UpdateVisionInfo( cdtImage, &current );
    UpdateFoundInfo();

    if ( headingMode == HM_HEADING && foundThenTracking ) {
	bool found = false;
	if ( cdtInfo.foundInfo[ IO_BALL ].found
	     && (trackingObj == TRACK_BALL
		 || trackingObj == SEARCH_ANY) ) {
	    found = true;
	    lastTrackingObj = trackingObj;
	    trackingObj = TRACK_BALL;
	} else if ( cdtInfo.foundInfo[ IO_GOAL_B ].found
		    && (trackingObj == TRACK_GOAL_B
			|| trackingObj == SEARCH_ANY
			|| trackingObj == SEARCH_GOAL) ) {
	    found = true;
	    lastTrackingObj = trackingObj;
	    trackingObj = TRACK_GOAL_B;
	} else if ( cdtInfo.foundInfo[ IO_GOAL_Y ].found
		    && (trackingObj == TRACK_GOAL_Y
			|| trackingObj == SEARCH_ANY
			|| trackingObj == SEARCH_GOAL) ) {
	    found = true;
	    lastTrackingObj = trackingObj;
	    trackingObj = TRACK_GOAL_Y;
	} else if ( cdtInfo.foundInfo[ IO_BEACON_BY ].found
		    && (trackingObj == TRACK_BEACON_BY
			|| trackingObj == SEARCH_ANY
			|| trackingObj == SEARCH_BEACON) ) {
	    found = true;
	    lastTrackingObj = trackingObj;
	    trackingObj = TRACK_BEACON_BY;
	} else if ( cdtInfo.foundInfo[ IO_BEACON_YB ].found
		    && (trackingObj == TRACK_BEACON_YB
			|| trackingObj == SEARCH_ANY
			|| trackingObj == SEARCH_BEACON) ) {
	    found = true;
	    lastTrackingObj = trackingObj;
	    trackingObj = TRACK_BEACON_YB;
	}
	if ( found ) {
	    foundThenTracking = false;
	    headingMode = HM_TRACKING;
	    cdtInfo.seqno = seqno;
	    currentHcmd = hcmdNone;
	}
    }
    cdtInfo.isMoving = (headingMode == HM_HEADING) && (waitingCount == 0);
    subject[sbjObjInfo]->SetData((char*) &cdtInfo, sizeof cdtInfo);
    subject[sbjObjInfo]->NotifyObservers();

    observer[event.ObsIndex()]->AssertReady(event.SenderID());
}
