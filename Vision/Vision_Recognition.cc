//
// Copyright 2003 (C) Eiichiro ITO, GHC02331@nifty.com
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
// Eiichiro ITO, 27 August 2003
// mailto: GHC02331@nifty.com
//
// History
//  26 Feb 2007, Eiichiro ITO, modified for new beacon/goal.
//
#include <OPENR/ODataFormats.h>
#include <OPENR/OFbkImage.h>
#include <OPENR/OPENRAPI.h>
#include <OPENR/OSyslog.h>
#include <OPENR/core_macro.h>
#include "Vision.h"

#define xwToPan(pan,x,w)	(-((pan)-FIELD_VIEW_H*((x)-(w)/2.0)/(w)))
#define yhToTilt(tilt,y,h)	((tilt)-FIELD_VIEW_V*((y)-(h)/2.0)/(h))
#define Left(obj)               ((obj)->x - (obj)->w / 2)
#define Right(obj)              ((obj)->x + (obj)->w / 2)
#define Top(obj)                ((obj)->y - (obj)->h / 2)
#define Bottom(obj)             ((obj)->y + (obj)->h / 2)

void
Vision::ClearVisionInfo()
{
    for ( int i = 0; i < MAX_IDENTIFIED_OBJECT; i ++ ) {
	cdtInfo.foundInfo[ i ].found = false;
    }
    for ( int ch = 0; ch < NUM_CHANNELS; ch ++ ) {
	cdtInfo.nCdtObjects[ ch ] = 0;
    }
    memset( &cdtInfo.cdtObjects, 0, sizeof cdtInfo.cdtObjects );
    ballIdentity.seems = false;
    ballIdentity.count = 0;
    goalBIdentity.seems = false;
    goalBIdentity.count = 0;
    goalYIdentity.seems = false;
    goalYIdentity.count = 0;
    beaconBYIdentity.seems = false;
    beaconBYIdentity.count = 0;
    beaconYBIdentity.seems = false;
    beaconYBIdentity.count = 0;
}

void
Vision::UpdateVisionInfo( const OFbkImage& cdtImage, const HeadingPosition *hpos )
{
    int w = cdtImage.Width();
    int h = cdtImage.Height();
    byte* base = cdtImage.Pointer();
    double pan = hpos->pan;
#ifdef ERS210
    double tilt = hpos->tilt;
#endif
#ifdef ERS7
    double tilt = hpos->tilt2;
#endif

    cdtVision.Update( base );
    for ( int ch = 0; ch < cdtChannels; ch ++ ) {
	cdtInfo.nCdtObjects[ ch ] = min( cdtVision.GetNumObjects( ch ),
				    NUM_CDTOBJECTS );
	CdtObject *cdtObject = cdtInfo.cdtObjects[ ch ];
	for ( int idx = 0; idx < cdtInfo.nCdtObjects[ ch ]; idx ++ ) {
	    ObjRect rect;
	    if ( cdtVision.GetObjectRect( ch, idx, &rect ) ) {
		cdtObject[ idx ].objPos.pan  = xwToPan( pan, rect.x, w );
		cdtObject[ idx ].objPos.tilt = yhToTilt( tilt, rect.y, h );
		cdtObject[ idx ].objPos.x = rect.x;
		cdtObject[ idx ].objPos.y = rect.y;
		cdtObject[ idx ].objPos.w = rect.w;
		cdtObject[ idx ].objPos.h = rect.h;
		cdtObject[ idx ].label = IO_UNIDENTIFIED;
	    }
	}
    }
}

void
Vision::IdentifyObject( ObjectIdentity *identity, FoundInfo *info, bool hasSeen )
{
    if ( hasSeen ) {
	if ( identity->seems ) {
	    identity->count ++;
	} else {
	    identity->seems = true;
	    identity->count = 0;
	}
	if ( identity->count > identity->foundThreshold ) {
	    info->found = true;
	    info->msec = currentMsec;
	}
    } else {
	if ( identity->seems ) {
	    identity->seems = false;
	    identity->count = 0;
	} else {
	    identity->count ++;
	}
	if ( identity->count > identity->lostThreshold ) {
	    info->found = false;
	}
    }
}

static bool
CheckAdjacency( ObjPosition *one, ObjPosition *another )
{
    if ( one->x - one->w * 2 > another->x ) {
	return false;
    } else if ( one->x + one->w * 2 < another->x ) {
	return false;
    } else if ( one->y - one->h * 2 > another->y ) {
	return false;
    } else if ( one->y + one->h * 2 < another->y ) {
	return false;
    }
    return true;
}

static bool
CheckUpper( ObjPosition *one, ObjPosition *another )
{
    return one->y < another->y;
}

static bool
CheckIncludes( ObjPosition *one, ObjPosition *another )
{
    if ( Left(one) > Left(another) ) {
	return false;
    } else if ( Right(one) < Right(another) ) {
	return false;
    } else if ( Top(one) > Top(another) ) {
	return false;
    } else if ( Bottom(one) > Bottom(another) ) {
	return false;
    }
    return true;
}

static void
MergeObject( ObjPosition *dst, ObjPosition *upper, ObjPosition *lower )
{
    int left, right, top, bottom;
    double pan, tilt;

    left = min( upper->x - upper->w / 2, lower->x - lower->w / 2 );
    right = max( upper->x + upper->w / 2, lower->x + lower->w / 2 );
    dst->x = (left + right) / 2;
    dst->w = right - left;
    top = min( upper->y - upper->h / 2, lower->y - lower->h / 2 );
    bottom = max( upper->y + upper->h / 2, lower->y + lower->h / 2 );
    dst->y = (top + bottom) / 2;
    dst->h = bottom - top;
    dst->pan = (upper->pan + lower->pan) / 2;
    dst->tilt = (upper->tilt + lower->tilt) / 2;
}

static int
DistanceInCMFromDots( int realObjectSizeInMM, int dotsOnLayerC )
{
    double degree = (double) dotsOnLayerC * FIELD_VIEW_H / LAYER_C_WIDTH;
    double radian = degree / 180 * M_PI;
    return (int) ((double) realObjectSizeInMM / 20.0 / tan( radian / 2.0 ));
}

static char* colorNames[] = {
    "Orange", "Yellow", "Aqua", "Blue", "Red", "Pink", "Undef", "Undef"
};

void
Vision::DetectBeacon( EAiboIdentifiedObject labelBO,
		      EAiboIdentifiedObject labelOB,
		      int channel, CdtObject* blue,
		      FoundInfo *fInfoBO, FoundInfo *fInfoOB,
		      bool *foundBeaconBO, bool *foundBeaconOB )
{
    ObjPosition* posBO = &fInfoBO->objPos;
    ObjPosition* posOB = &fInfoOB->objPos;
    int nOther = cdtInfo.nCdtObjects[ channel ];
    CdtObject* others = cdtInfo.cdtObjects[ channel ];

    for ( int j = 0; j < nOther && (!*foundBeaconBO||!*foundBeaconOB); j ++ ) {
	CdtObject* other = &others[ j ];
	if ( other->label != IO_UNIDENTIFIED ||
	     !CheckGeometry( RGeometryBeacon, &other->objPos ) ) {
	    // ラベルが付いていたら無視
	    continue;
	}
	// 隣接しているかチェック
	if ( !CheckAdjacency( &blue->objPos, &other->objPos ) ) {
	    continue;
	}
	if ( !*foundBeaconBO && CheckUpper( &blue->objPos, &other->objPos ) ) {
	    *foundBeaconBO = true;
	    MergeObject( posBO, &blue->objPos, &other->objPos );
	    // ラベルを付ける
	    blue->label = labelBO;
	    other->label = labelBO;
	    // 距離を計算する
	    int beaconWidth = RGeometry[ RGeometryBeacon ].obj_w;
	    int beaconHeight = RGeometry[ RGeometryBeacon ].obj_h;
	    fInfoBO->distance = 
		min(DistanceInCMFromDots( beaconWidth, posBO->w ),
		    DistanceInCMFromDots( beaconHeight, posBO->h ));
	    if ( verboseMode >= 8 ) {
		OSYSPRINT(( "Beacon[Blue/%s] pan=%05.1f,tilt=05.1f,%dcm\n",
			    colorNames[channel],
			    posBO->pan, posBO->tilt, fInfoBO->distance ));
	    }
	    break;
	} else if ( !*foundBeaconOB ) {
	    *foundBeaconOB = true;
	    MergeObject( posOB, &other->objPos, &blue->objPos );
	    // ラベルを付ける
	    blue->label = labelOB;
	    other->label = labelOB;
	    // 距離を計算する
	    int beaconWidth = RGeometry[ RGeometryBeacon ].obj_w;
	    int beaconHeight = RGeometry[ RGeometryBeacon ].obj_h;
	    fInfoOB->distance = 
		min(DistanceInCMFromDots( beaconWidth, posOB->w ),
		    DistanceInCMFromDots( beaconHeight, posOB->h ));
	    if ( verboseMode >= 8 ) {
		OSYSPRINT(( "Beacon[%s/Blue] pan=%05.1f,tilt=%05.1f,%dcm\n",
			    colorNames[channel],
			    posOB->pan, posOB->tilt, fInfoOB->distance ));
	    }
	    break;
	}
    }
}

void
Vision::DetectBeacon()
{
    bool foundBeaconBY = false;
    bool foundBeaconYB = false;

    // アクアブルーをもとにビーコン検出する
    int nBlues = cdtInfo.nCdtObjects[ AQUABLUE_CHANNEL ];
    CdtObject* blues = cdtInfo.cdtObjects[ AQUABLUE_CHANNEL ];

    for ( int i = 0; i < nBlues; i ++ ) {
	CdtObject* blue = &blues[ i ];
	if ( blue->label != IO_UNIDENTIFIED ||
	     !CheckGeometry( RGeometryBeacon, &blue->objPos ) ) {
	    // ラベルが付いていたら無視
	    continue;
	}
	// 黄色との隣接関係をチェック
	DetectBeacon( IO_BEACON_BY, IO_BEACON_YB,
		      YELLOW_CHANNEL, blue,
		      &cdtInfo.foundInfo[ IO_BEACON_BY ],
		      &cdtInfo.foundInfo[ IO_BEACON_YB ],
		      &foundBeaconBY, &foundBeaconYB );
    }
    IdentifyObject( &beaconBYIdentity, &cdtInfo.foundInfo[ IO_BEACON_BY ],
		    foundBeaconBY );
    IdentifyObject( &beaconYBIdentity, &cdtInfo.foundInfo[ IO_BEACON_YB ],
		    foundBeaconYB );
}

void
Vision::DetectBall()
{
    bool foundBall = false;
    int nBalls = cdtInfo.nCdtObjects[ BALL_CHANNEL ];
    CdtObject* balls = cdtInfo.cdtObjects[ BALL_CHANNEL ];

    for ( int i = 0; i < nBalls; i ++ ) {
	CdtObject* ball = &balls[ i ];
	if ( ball->label != IO_UNIDENTIFIED ) {
	    continue;
	} else if ( !CheckGeometry( RGeometryBall, &ball->objPos ) ) {
	    continue;
	}
	foundBall = true;
	cdtInfo.foundInfo[ IO_BALL ].objPos = ball->objPos;
	ball->label = IO_BALL;
	cdtInfo.foundInfo[ IO_BALL ].distance =
	    DistanceInCMFromDots( RGeometry[ RGeometryBall ].obj_w,
			      max(ball->objPos.w, 
				  ball->objPos.h) );
	if ( verboseMode >= 9 ) {
	    OSYSPRINT(( "Ball[%d] (%5.1f,%5.1f)=%d\n",
			BALL_CHANNEL, ball->objPos.pan, ball->objPos.tilt,
			cdtInfo.foundInfo[ IO_BALL ].distance ));
	}
	break;
    }
    IdentifyObject( &ballIdentity, &cdtInfo.foundInfo[ IO_BALL ], foundBall );
}

void
Vision::DetectGoal( EAiboIdentifiedObject label, int channel,
		    ObjectIdentity *identity, FoundInfo *info )
{
    bool foundGoal = false;
    int nObjects = cdtInfo.nCdtObjects[ channel ];
    CdtObject* objects = cdtInfo.cdtObjects[ channel ];

    for ( int i = 0; i < nObjects ; i ++ ) {
	CdtObject* object = &objects[ i ];
	if ( object->label != IO_UNIDENTIFIED ||
	     !CheckGeometry( RGeometryGoal, &object->objPos ) ) {
	    continue;
	}
	foundGoal = true;
	info->objPos = object->objPos;
	object->label = label;
	int goalHeight = RGeometry[ RGeometryGoal ].obj_h;
	info->distance = DistanceInCMFromDots( goalHeight, info->objPos.h );
	if ( verboseMode >= 9 ) {
	    OSYSPRINT(( "Goal[%d] (%5.1f,%5.1f)=%d\n",
			channel, info->objPos.pan, info->objPos.tilt,
			info->distance ));
	}
	break;
    }
    IdentifyObject( identity, info, foundGoal );
}

void
Vision::IgnoreInclusions( ObjPosition *detected, int channel, EAiboIdentifiedObject label )
{
    int nOther = cdtInfo.nCdtObjects[ channel ];
    CdtObject* others = cdtInfo.cdtObjects[ channel ];

    for ( int j = 0; j < nOther; j ++ ) {
	CdtObject* other = &others[ j ];
	if ( other->label != IO_UNIDENTIFIED ) {
	    continue;
	}
	if ( CheckIncludes( detected, &other->objPos ) ) {
	    if ( verboseMode >= 9 ) {
		OSYSPRINT(( "%s hide channel (%d) by label=%d\n",
			    "Vision::IgnoreInclusions", channel, label ));
	    }
	    other->label = label;
	}
    }
}

void
Vision::IgnoreInclusions( int valid_channel, int invalid_channel, EAiboIdentifiedObject label )
{
    int nValid = cdtInfo.nCdtObjects[ valid_channel ];
    CdtObject* valids = cdtInfo.cdtObjects[ valid_channel ];

    for ( int j = 0; j < nValid; j ++ ) {
	CdtObject* valid = &valids[ j ];
	IgnoreInclusions( &valid->objPos, invalid_channel, label );
    }
}

void
Vision::UpdateFoundInfo()
{
    // ボールの中にゴール/プレイヤーがあったらおかしい
    IgnoreInclusions( BALL_CHANNEL, AQUABLUE_CHANNEL, IO_BALL );
    IgnoreInclusions( BALL_CHANNEL, YELLOW_CHANNEL,   IO_BALL );
    IgnoreInclusions( BALL_CHANNEL, DARKBLUE_CHANNEL, IO_BALL );
    IgnoreInclusions( BALL_CHANNEL, RED_CHANNEL,      IO_BALL );
    DetectBeacon();
    DetectBall();
    DetectGoal( IO_GOAL_B, AQUABLUE_CHANNEL, &goalBIdentity,
		&cdtInfo.foundInfo[ IO_GOAL_B ] );
    DetectGoal( IO_GOAL_Y, YELLOW_CHANNEL,   &goalYIdentity,
		&cdtInfo.foundInfo[ IO_GOAL_Y ] );
}

void
Vision::FilterCImage( byte *dst, const byte *src, int w, int h )
{
    int size = w * h;

    memcpy( dst, src, size );
    memcpy( dst, src, w );    dst += w;    src += w;
    for ( int y = 1; y < h - 1; y += 1 ) {
	*dst++ = *src++;
	for ( int x = 1; x < w - 1; x += 1 ) {
	    byte a[9];
	    a[0] = src[-w-1]; a[1] = src[-w]; a[2] = src[-w+1];
	    a[3] = src[-1];   a[4] = src[0];  a[5] = src[1];
	    a[6] = src[w-1];  a[7] = src[w];  a[8] = src[w+1];
	    byte out = 0, mask = 1;
	    for ( int i = 0; i < 8; i += 1, mask <<= 1 ) {
		int count = 0;
		for ( int j = 0; j < 9; j += 1 ) {
		    count += (a[j] & mask) ? 1 : 0;
		}
		out |= (count > 4) ? mask : 0;
	    }
	    *dst++ = out;
	    src ++;
	}
	*dst++ = *src++;
    }
    memcpy( dst, src, w );    dst += w;    src += w;
}
