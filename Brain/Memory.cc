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
#include <string.h>
#include <OPENR/OSyslog.h>
#include <OPENR/OPENRAPI.h>
#include <OPENR/core_macro.h>
#include <ControlInfo.h>
#include "Brain.h"
#include "command.h"

#define isBetween(x,r1,r2)   ((r1)<=(x)&&(x)<=(r2))

static char *ObjName[MAX_IDENTIFIED_OBJECT] = {
    "?", "Ball", "Blue Goal", "Yellow Goal", "BY Beacon", "YB Beacon",
};
static int TimeoutSec[MAX_IDENTIFIED_OBJECT] = {
    8, 3, 8, 8, 8, 8
};
static int MaxMoNetCount[MAX_IDENTIFIED_OBJECT] = {
    10, 10, 10, 10, 10, 10
};

Memory::Memory()
{
    Clear();
    nDeltas = 0;
    MoNetDelta *delta = delta = &deltas[nDeltas++];
    // 5 "���˿ʤ�(�ᤤ)"
    delta->cmdID = 5; delta->theta = 0; delta->r = 12;
    // 30 "���˿ʤ�(70mm)"
    delta = &deltas[nDeltas++];
    delta->cmdID = 30; delta->theta = 0; delta->r = 8;
    // 112 "����ž����(60��)"
    delta = &deltas[nDeltas++];
    delta->cmdID = 112; delta->theta = -60; delta->r = 0;
    // 75 "����ž����(30��)"
    delta = &deltas[nDeltas++];
    delta->cmdID = 75; delta->theta = -38; delta->r = 0;
    // 111 "����ž����(60��)"
    delta = &deltas[nDeltas++];
    delta->cmdID = 111; delta->theta = 60; delta->r = 0;
    // 76 "����ž����(30��)"
    delta = &deltas[nDeltas++];
    delta->cmdID = 76; delta->theta = 42; delta->r = 0;
    // 77 "����ž����(10��)"
    delta = &deltas[nDeltas++];
    delta->cmdID = 77; delta->theta = -10; delta->r = 0;
    // 78 "����ž����(10��)"
    delta = &deltas[nDeltas++];
    delta->cmdID = 78; delta->theta = 10; delta->r = 0;
    // 16 "�����塼��(90��)"
    delta = &deltas[nDeltas++];
    delta->cmdID = 16; delta->theta = -40; delta->r = 0;
    // 18 "�����塼��(90��)"
    delta = &deltas[nDeltas++];
    delta->cmdID = 18; delta->theta = 30; delta->r = 0;
    // 20 "�ФẸ���塼��(60��)"
    delta = &deltas[nDeltas++];
    delta->cmdID = 20; delta->theta = -30; delta->r = 0;
    // 15 "�Фᱦ���塼��(60��)"
    delta = &deltas[nDeltas++];
    delta->cmdID = 15; delta->theta = 20; delta->r = 0;
}

void
Memory::Clear()
{
    for ( int i = 0; i < MAX_IDENTIFIED_OBJECT; i ++ ) {
	PolarCoord *coord = &coords[i];
	coord->msec = 0;
	coord->moNetCount = 0;
    }
    currentMoNetCmdID = 0;
}

void
Memory::SetCdtInfo( CdtInfo* cdtInfo )
{
    // ���ư��ϵ������ʤ�
    if ( cdtInfo->isMoving ) {
	return;
    }
    int msec = ::GetNowMSec();
    for ( int i = 0; i < MAX_IDENTIFIED_OBJECT; i ++ ) {
	int timeout = TimeoutSec[i];
	PolarCoord *coord = &coords[i];
	FoundInfo *foundInfo = &cdtInfo->foundInfo[i];
	coord->found = foundInfo->found;
	if ( foundInfo->found ) {
	    coord->theta = foundInfo->objPos.pan;
	    coord->r = foundInfo->distance;
	    coord->msec = msec;
	    coord->moNetCount = 0;
	    //OSYSPRINT(("%s found at %4.1f,%d\n",
	    //           ObjName[i], coord->theta, coord->r ));
	} else if ( coord->msec && (msec - coord->msec) > timeout * 1000 ) {
	    // �����ÿ��вᤷ��������̵�뤹�롣
	    coord->msec = 0;
	    OSYSPRINT(("%s lost by time(%d)\n", ObjName[i], timeout));
	}
    }
}

#define deg2rad(d) ((d)*3.1415927/180.0)
#define rad2deg(r) ((r)*180.0/3.1415927)

void
Memory::MoNetExecute( MoNetCommandID cmdID )
{
    currentMoNetCmdID = (int) cmdID;

    MoNetDelta *delta = NULL;
    for ( int i = 0; i < nDeltas; i ++ ) {
	if ( deltas[i].cmdID == cmdID ) {
	    delta = &deltas[i];
	    break;
	}
    }
    if ( delta == NULL ) {
	return;
    }
    OSYSPRINT(("MoNet:%d,%3.0f,%d\n", cmdID, delta->theta, delta->r ));
    for ( int i = 1; i < MAX_IDENTIFIED_OBJECT; i ++ ) {
	PolarCoord *coord = &coords[i];
	if ( !coord->found && coord->msec ) {
	    // �����Ƥ��ʤ��������������롣
	    if ( ++ coord->moNetCount > MaxMoNetCount[i] ) {
		// ��������ư�������������ʤ�
		coord->msec = 0;
		OSYSPRINT(("%s lost by moving\n", ObjName[i]));
		continue;
	    }
	    coord->theta -= delta->theta;
	    if ( delta->r ) {
		double t = deg2rad(coord->theta);
		double a = (double)coord->r * sin(t);
		double b = (double)coord->r * cos(t) - (double)delta->r;
		double r = sqrt(a*a + b*b);
		coord->r = (int) r;
		if ( coord->r != 0 ) {
		    t = acos(b / r);
		    coord->theta = rad2deg(t);
		}
	    }
	    coord->theta = (double) ((((int) coord->theta) + 180) % 360 - 180);
	    OSYSPRINT(("%s new at %4.1f,%d\n",
		       ObjName[i], coord->theta, coord->r ));
	}
    }
}

bool
Memory::CheckRange( EAiboIdentifiedObject id, double angle1, double angle2, double dist1, double dist2 ) const
{
    if ( id < 0 || id >= MAX_IDENTIFIED_OBJECT ) {
	return false;
    }
    const PolarCoord *coord = &coords[id];
    if ( (angle1 == angle2) || (dist1 == dist2) ) {
	// ���٤�Ʊ�졢��Υ��Ʊ��ξ��ϡָ��Ĥ���ʤ��׾��Ȥʤ�
	return !coord->msec;
    }
    if ( !coord->msec ) {
	return false;
    }
    int r = coord->r;
    if ( !isBetween( r, dist1, dist2 ) ) {
	return false;
    }
    double theta = coord->theta;
    // �濴���ϰ�������äƤ���Ф褤
    return isBetween( theta, angle1, angle2 );
}

double
Memory::GetTheta( EAiboIdentifiedObject id ) const
{
    if ( id < 0 || id >= MAX_IDENTIFIED_OBJECT ) {
	return 0.0;
    }
    return coords[id].theta;
}

void
Memory::SetDefaultTimeout( int sec )
{
    OSYSPRINT(("Memory::SetDefaultTimeout %d\n", sec));
    for ( int i = 0; i < MAX_IDENTIFIED_OBJECT; i ++ ) {
	TimeoutSec[i] = sec;
    }
}

void
Memory::SetBallTimeout( int sec )
{
    OSYSPRINT(("Memory::SetBallTimeout %d\n", sec));
    TimeoutSec[IO_BALL] = sec;
}
