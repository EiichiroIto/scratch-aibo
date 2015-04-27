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
// Eiichiro ITO, 29 August 2005
// mailto: GHC02331@nifty.com
#ifndef MAIN
#include <OPENR/OObject.h>
#include <VisionInfo.h>
#else // MAIN
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
struct HeadingPosition {
    double tilt, pan, tilt2, mouth, speed;
};
#define OSYSPRINT(x) printf x
#endif // MAIN

#include "Headings.h"

static void PrintHeadingPosition( const HeadingPosition *hp );

HeadingCommand::HeadingCommand() : cmdID(0), startIndex(0), endIndex(0)
{
}

int
HeadingCommand::GetCmdID() const
{
    return cmdID;
}

int
HeadingCommand::GetStartIndex() const
{
    return startIndex;
}

int
HeadingCommand::GetEndIndex() const
{
    return endIndex;
}

void
HeadingCommand::Set( int _cmdID, int _startIndex, int _endIndex )
{
    cmdID = _cmdID;
    startIndex = _startIndex;
    endIndex = _endIndex;
}

Headings::Headings() : numHcmd(0), numHpos(0), hcmd(NULL), hpos(NULL)
{
}

Headings::~Headings()
{
    Clear();
}

void
Headings::Clear()
{
    if ( hcmd ) {
	delete[] hcmd;
	hcmd = NULL;
    }
    if ( hpos ) {
	delete[] hpos;
	hpos = NULL;
    }
    numHcmd = numHpos = 0;
}

void
Headings::Print() const
{
    OSYSPRINT(( "Headings:\n" ));
    if ( hcmd == NULL || hpos == NULL ) {
	OSYSPRINT(( " no contents\n" ));
	return;
    }
    for ( int i = 0; i < numHcmd; i ++ ) {
	HeadingCommand *hc = &hcmd[ i ];
	OSYSPRINT(( "CmdID=%d\n", hc->GetCmdID() ));
	for ( int j = hc->GetStartIndex(); j <= hc->GetEndIndex(); j ++ ) {
	    HeadingPosition *hp = &hpos[ j ];
	    PrintHeadingPosition( hp );
	}
    }
}

bool
Headings::ReadConfig( const char *path )
{
    OSYSPRINT(( "Headings::ReadConfig from %s.\n", path ));

    FILE* fp = fopen( path, "r" );
    if ( fp == NULL ) {
        OSYSPRINT(( "Headings::ReadConfig can't open\n" ));
        return false;
    }
    fseek( fp, 0L, SEEK_END );
    int size = ftell( fp );
    fseek( fp, 0L, SEEK_SET );
    char *filebuf = new char[ size + 1 ];
    fread( filebuf, size, 1, fp );
    filebuf[ size ] = 0;
    fclose( fp );
    ReadFromString( filebuf );
    delete[] filebuf;

    //Print();

    return true;
}

void
Headings::ReadFromString( const char *buf )
{
    int _numHcmd, _numHpos;

    if ( !buf || !*buf ) {
	OSYSPRINT(( "(null contents)\n" ));
	return;
    }
    if ( !(buf[0]=='#' && buf[1] == 'H' && buf[2] == 'D' && buf[3] == ' ')) {
	OSYSPRINT(( "(invalid magic)\n" ));
	return;
    }
    if ( sscanf( &buf[4], "%d %d", &_numHcmd, &_numHpos ) != 2 ) {
	OSYSPRINT(( "(invalid parameters)\n" ));
	return;
    }
    if ( _numHcmd <= 0 || _numHcmd >= MAX_HDCMD
	 || _numHpos <= 0 || _numHpos >= MAX_HDPOS ) {
	OSYSPRINT(( "(overflow parameters:%d,%d)\n", _numHcmd, _numHpos ));
	return;
    }
    Clear();
    hcmd = new HeadingCommand[ _numHcmd ];
    hpos = new HeadingPosition[ _numHpos ];
    if ( hcmd == NULL || hpos == NULL ) {
	OSYSPRINT(( "(cannot allocate:%d,%d)\n", _numHcmd, _numHpos ));
	return;
    }
    numHcmd = _numHcmd;
    numHpos = _numHpos;
    int curHcmd = 0, curHpos = 0;

    OSYSPRINT(( " numHcmd=%d, numHpos=%d\n", numHcmd, numHpos ));

    buf = strchr( buf, '\n' );
    if ( buf ) {
	buf ++;
    }
    while ( buf && *buf ) {
	const char *next = strchr( buf, '\n' );
	if ( *buf == '*' ) {
	    int cmdID, num;
	    if ( curHcmd >= numHcmd ) {
		OSYSPRINT(( " over hcmd\n" ));
		return;
	    }
	    buf ++;
	    sscanf( buf, "%d %d", &cmdID, &num );
	    hcmd[curHcmd].Set( cmdID, curHpos, curHpos + num - 1 );
	    curHcmd ++;
	} else if ( *buf != '#' ) {
	    double tilt, pan, tilt2, mouth, speed;
	    if ( curHpos >= numHpos ) {
		OSYSPRINT(( " over hpos\n" ));
		return;
	    }
	    speed = 1.0;
	    sscanf( buf, "%lf %lf %lf %lf %lf",
		    &tilt, &pan, &tilt2, &mouth, &speed );
	    hpos[curHpos].tilt  = tilt;
	    hpos[curHpos].pan   = pan;
	    hpos[curHpos].tilt2 = tilt2;
	    hpos[curHpos].mouth = mouth;
	    hpos[curHpos].speed = speed;
	    curHpos ++;
	}
	if ( next ) {
	    next ++;
	}
	buf = next;
    }
}

const HeadingPosition *
Headings::GetHeadingPosition( int cmdID, int index )
{
    if ( index < 0 ) {
	return NULL;
    }
    for ( int i = 0; i < numHcmd; i ++ ) {
	HeadingCommand *hc = &hcmd[i];
	if ( hc->GetCmdID() == cmdID ) {
	    index += hc->GetStartIndex();
	    if ( index > hc->GetEndIndex() ) {
		return NULL;
	    }
	    return &hpos[ index ];
	}
    }
    return NULL;
}

static void
PrintHeadingPosition( const HeadingPosition *hp )
{
    if ( !hp ) {
	OSYSPRINT(( "(null)\n" ));
	return;
    }
    OSYSPRINT(( " t=%5.1lf p=%5.1lf t=%5.1lf m=%5.1lf (%3.1lf)\n",
		hp->tilt, hp->pan, hp->tilt2, hp->mouth, hp->speed ));
}

#ifdef MAIN
int
main()
{
    Headings h;
    h.ReadConfig( "headings.hed" );
    h.ReadConfig( "headings.hed" );
    OSYSPRINT(( "results:\n" ));
    PrintHeadingPosition( h.GetHeadingPosition( 0, 0 ) );
    PrintHeadingPosition( h.GetHeadingPosition( 0, 1 ) );
    PrintHeadingPosition( h.GetHeadingPosition( 0, 2 ) );
    PrintHeadingPosition( h.GetHeadingPosition( 0, 3 ) );
    PrintHeadingPosition( h.GetHeadingPosition( 0, 4 ) );
    PrintHeadingPosition( h.GetHeadingPosition( 0, 5 ) );
    PrintHeadingPosition( h.GetHeadingPosition( 4, 0 ) );
    PrintHeadingPosition( h.GetHeadingPosition( 4, 1 ) );
    PrintHeadingPosition( h.GetHeadingPosition( 4, 2 ) );
}
#endif
