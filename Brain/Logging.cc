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
// Eiichiro ITO, 13 April 2007
// mailto: GHC02331@nifty.com
//
#include <string.h>
#include <OPENR/OSyslog.h>
#include <OPENR/OPENRAPI.h>
#include <OPENR/core_macro.h>
#include <ControlInfo.h>
#include "Brain.h"

Logging::Logging() : maxLogSize(DEFAULT_LOGSIZE), logMask(DEFAULT_LOGMASK),
		     logging(false), startLogMSec(0), currentLogCount(0),
		     rgnLog(NULL), nextLogPtr(NULL), nextLogIndex(0),
		     lastStnNo(0), lastStateId(0)
{
}

void
Logging::Allocate()
{
    OSYSPRINT(("Logging::Allocate %d bytes, mask=%x.\n",
	       maxLogSize, logMask ));
    rgnLog = new RCRegion( maxLogSize );
}

void
Logging::DeAllocate()
{
    if ( rgnLog ) {
	rgnLog->RemoveReference();
    }
    rgnLog = NULL;
}

void
Logging::SetMaxLogSize( int v )
{
    maxLogSize = v;
}

void
Logging::SetLogMask( int v )
{
    logMask = v;
}

void
Logging::Start()
{
    if ( logging ) {
	OSYSPRINT(("Logging::Start() logging in progress\n"));
	return;
    }
    if ( nextLogIndex ) {
	OSYSPRINT(("Logging::Start() log not saved\n"));
	return ;
    }
    if ( !rgnLog ) {
	OSYSPRINT(("Logging::Start() rgnLog not initialized\n"));
	return;
    }
    OSYSPRINT(("Logging::Start() : no=%d\n", currentLogCount));
    if ( !startLogMSec ) {
	startLogMSec = ::GetNowMSec();
    }
    logging = true;
    nextLogIndex = 0;
    nextLogPtr = (char*) rgnLog->Base();
    lastStnNo = 0;
    lastStateId = 0;
}

void
Logging::End()
{
    OSYSPRINT(("Logging::End()\n"));
    if ( nextLogIndex ) {
	logging = false;
	char path[ 256 ];
	sprintf( path, LOGFILE, currentLogCount );
	OSYSPRINT(( "Logging::End wrote log file=%s.\n", path ));
	FILE* fp = fopen( path, "w" );
	if ( fp == NULL ) {
	    OSYSLOG1((osyslogERROR, "%s : %s",
		      "Logging::End()", "fopen FAILED"));
	    return;
	}
	fwrite( (char*) rgnLog->Base(), nextLogIndex, 1, fp );
	fclose( fp );
	currentLogCount += 1;
	currentLogCount %= NUM_LOGCOUNT;
    }
    nextLogIndex = 0;
}

void
Logging::Reset()
{
    OSYSPRINT(("Logging::Reset()\n"));
    logging = false;
    startLogMSec = 0;
    currentLogCount = 0;
    nextLogIndex = 0;
    nextLogPtr = (char*) rgnLog->Base();
}

void
Logging::LogString( const char *buf )
{
    int len = strlen( buf );
    if ( nextLogIndex + len >= maxLogSize - 10 ) {
	logging = false;
        OSYSLOG1((osyslogERROR, "%s : %s",
                  "Logging::String()", "log area overflow"));
	strcpy( nextLogPtr, "...\r\n" );
	nextLogIndex += 5;
    } else {
	strcpy( nextLogPtr, buf );
	nextLogPtr += len;
	nextLogIndex += len;
    }
}

void
Logging::LogCdtDetail( int logID, FoundInfo* fInfo )
{
    int msec = ::GetNowMSec() - startLogMSec;
    char buf[ 128 ];

    if ( fInfo->found ) {
	sprintf( buf, "%d\t%d\t%d\t%d\t%d\r\n",
		 msec, logID,
		 (int) fInfo->objPos.pan, (int) fInfo->objPos.tilt,
		 (int) fInfo->distance );
    } else {
	sprintf( buf, "%d\t%d\r\n", msec, logID );
    }
    LogString( buf );
}

void
Logging::LogCdtInfo( CdtInfo *cdtInfo )
{
    if ( !logging ) {
	return;
    }
    if ( logMask & LOGMASK_BALL ) {
	LogCdtDetail( 1, &cdtInfo->foundInfo[IO_BALL] );
    }
    if ( logMask & LOGMASK_GOAL_B ) {
	LogCdtDetail( 2, &cdtInfo->foundInfo[IO_GOAL_B] );
    }
    if ( logMask & LOGMASK_GOAL_Y ) {
	LogCdtDetail( 3, &cdtInfo->foundInfo[IO_GOAL_Y] );
    }
}

void
Logging::LogMoNetCmd( MoNetCommandID cmdID )
{
    if ( !logging ) {
	return;
    }
    if ( cmdID == monetcommandID_UNDEF ) {
	return;
    }
    if ( !(logMask & LOGMASK_MONET) ) {
	return;
    }
    int msec = ::GetNowMSec() - startLogMSec;
    char buf[ 128 ];
    sprintf( buf, "%d\t9\t%d\r\n", msec, cmdID );
    LogString( buf );
}

void
Logging::LogSTNInfo( STNInfo *stnInfo )
{
    if ( !logging ) {
	return;
    }
    if ( !(logMask & LOGMASK_STNINFO) ) {
	return;
    }
    if ( lastStnNo == stnInfo->stnNo && lastStateId == stnInfo->stateIds[0] ) {
	return;
    }
    lastStnNo = stnInfo->stnNo;
    lastStateId = stnInfo->stateIds[0];
    int msec = ::GetNowMSec() - startLogMSec;
    char buf[ 128 ];
    sprintf( buf, "%d\t0\t%d\t%d\r\n", msec, lastStnNo, lastStateId );
    LogString( buf );
}
