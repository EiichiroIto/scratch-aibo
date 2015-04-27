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
// Eiichiro ITO, 12 August 2003
// mailto: GHC02331@nifty.com
#ifndef MAIN
#include <OPENR/OObject.h>
#else // MAIN
#include <stdio.h>
#endif // MAIN
#include "Perception.h"

#define max(x,y) ((x)>(y)?(x):(y))
#define min(x,y) ((x)<(y)?(x):(y))

#define isLeft(w,pan,tilt) ((pan)<-(w))
#define isRight(w,pan,tilt) ((pan)>(w))
#define isForward(w,pan,tilt) (((pan)>=-(w)&&(pan)<=(w)))
#define isDown(pan,tilt)  (((pan)>=-30&&(pan)<=30)&&(tilt)<-33)

#define degreeForWidth(w)	((w)*FIELD_VIEW_H/LAYER_C_WIDTH)
#define widthForDegree(d)	((d)*LAYER_C_WIDTH/FIELD_VIEW_H)

#define isFound(obj)  (currentCdtInfo.foundInfo[obj].found || (recentCdtInfo.foundInfo[obj].found && (recentCdtInfo.foundInfo[obj].msec >= lastCheckedMSec)))
#define getPan(obj)   (recentCdtInfo.foundInfo[obj].objPos.pan)
#define getTilt(obj)  (recentCdtInfo.foundInfo[obj].objPos.tilt)
#define getW(obj)     (recentCdtInfo.foundInfo[obj].objPos.w)
#define getH(obj)     (recentCdtInfo.foundInfo[obj].objPos.h)
#define getDistance(obj)     (recentCdtInfo.foundInfo[obj].distance)

#define isBetween(x,r1,r2)   ((r1)<=(x)&&(x)<=(r2))
#define leftPan(pan,w)	((pan)-degreeForWidth(w)/2)
#define rightPan(pan,w)	((pan)+degreeForWidth(w)/2)

#define PI 3.141592653f

Perception::Perception(): lastCheckedMSec(0), nowMSec(0)
{
}

void
Perception::UpdateWhenFound( FoundInfo* recent, FoundInfo* current )
{
    if ( current->found ) {
	*recent = *current;
    }
}

void
Perception::UpdateNowMSec( int msec )
{
    lastCheckedMSec = nowMSec;
    nowMSec = msec;
}

void
Perception::SetCdtInfo( CdtInfo* _cdtInfo )
{
    if ( _cdtInfo->isMoving ) {
	return;
    }
    currentCdtInfo = *_cdtInfo;
    for ( int i = 0; i < MAX_IDENTIFIED_OBJECT; i ++ ) {
	UpdateWhenFound( &recentCdtInfo.foundInfo[ i ],
			 &_cdtInfo->foundInfo[ i ] );
    }
}

bool
Perception::LostCondition( EAiboIdentifiedObject id ) const
{
    if ( id < 0 || id >= MAX_IDENTIFIED_OBJECT ) {
	return false;
    }
    return !isFound(id);
}

bool
Perception::VisionRangeCondition( enum EAiboIdentifiedObject id, double angle1, double angle2, double dist1, double dist2 ) const
{
    if ( id < 0 || id >= MAX_IDENTIFIED_OBJECT ) {
	return false;
    }
    double pan = getPan(id);
    double tilt = getTilt(id);
    int distance = getDistance(id);
    int w = getW(id);

    if ( (angle1 == angle2) || (dist1 == dist2) ) {
	// 角度が同一、距離が同一の場合は「見つからない」条件となる
	return !currentCdtInfo.foundInfo[id].found;
    }
    if ( !isFound(id) ) {
	return false;
    }
    if ( !isBetween( distance, dist1, dist2 ) ) {
	return false;
    }
    if ( distance >= 25 ) {
	// 遠いとき：完全に範囲内に入っている
	return angle1 <= leftPan(pan,w) && rightPan(pan,w) <= angle2;
    }
    if ( leftPan(pan,w) <= angle1 && angle2 <= rightPan(pan,w) ) {
	// 近いとき：指定範囲より広く見える
	return true;
    }
    // 中心が範囲内に入っていればよい
    return isBetween( pan, angle1, angle2 );
}

bool
Perception::VisionNearRangeCondition( enum EAiboIdentifiedObject id, int x1, int x2, int y1, int y2 ) const
{
    if ( id < 0 || id >= MAX_IDENTIFIED_OBJECT ) {
	return false;
    }
    if ( (x1 == x2) || (y1 == y2) ) {
	// x座標が同一、y座標が同一の場合は「見つからない」条件となる	
	return !currentCdtInfo.foundInfo[id].found;
    }
    if ( !isFound(id) ) {
	return false;
    }
    double pan = getPan(id);
    double distance = getDistance(id);
    int x = (int) (sin(pan * PI/180) * distance);
    int y = (int) (cos(pan * PI/180) * distance);
    if ( !isBetween( x, x1, x2 ) ) {
	return false;
    }
    if ( !isBetween( y, y1, y2 ) ) {
	return false;
    }
    return true;
}

int
Perception::GetCdtInfoSeqno() const
{
    return currentCdtInfo.seqno;
}

int
ObjPositionToString( char *ptr, const ObjPosition *objPos )
{
    sprintf( ptr, "%03d%03d%03d%03d%06.1f%06.1f",
	     objPos->x, objPos->y, objPos->w, objPos->h,
	     objPos->pan, objPos->tilt );
    return 3*4 + 6*2;
}

int
FoundInfoToString( char *ptr, const char *tag, const FoundInfo *foundInfo )
{
    int size = 0;
    sprintf( &ptr[size], "%s", tag );
    size += 2;
    sprintf( &ptr[size], "%01d", foundInfo->found ? 1 : 0 );
    size += 1;
    size += ObjPositionToString( &ptr[size], &foundInfo->objPos );
    sprintf( &ptr[size], "%03d:", foundInfo->distance );
    size += 4;
    return size;
}

char*
Perception::CdtInfoString()
{
    static char ballString[ 4096 ];
    char *ptr = ballString ;

    for ( int ch = 0; ch < NUM_CHANNELS; ch ++ ) {
	sprintf( ptr, "%02d", currentCdtInfo.nCdtObjects[ ch ] );
	ptr += 2;
	CdtObject *cdtObjects = currentCdtInfo.cdtObjects[ch];
	for ( int idx = 0; idx < currentCdtInfo.nCdtObjects[ ch ]; idx ++ ) {
	    ptr += ObjPositionToString( ptr, &cdtObjects[idx].objPos );
	    sprintf( ptr, "%02d", (int) cdtObjects[idx].label );
	    ptr += 2;
	}
	*ptr++ = ':';
    }
    ptr += FoundInfoToString( ptr, "BL",
			      &currentCdtInfo.foundInfo[IO_BALL] );
    ptr += FoundInfoToString( ptr, "GB",
			      &currentCdtInfo.foundInfo[IO_GOAL_B] );
    ptr += FoundInfoToString( ptr, "GY",
			      &currentCdtInfo.foundInfo[IO_GOAL_Y] );
    ptr += FoundInfoToString( ptr, "BY",
			      &currentCdtInfo.foundInfo[IO_BEACON_BY] );
    ptr += FoundInfoToString( ptr, "YB",
			      &currentCdtInfo.foundInfo[IO_BEACON_YB] );
    *ptr = '\0';
    return ballString;
}
