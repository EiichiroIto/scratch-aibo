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
// Eiichiro ITO, 29 August 2003
// mailto: GHC02331@nifty.com
#ifdef MAIN
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define OSYSPRINT(x) printf x
typedef int OPrimitiveID;
#endif // MAIN
#include "CdtFile.h"

CdtTable::CdtTable() : channel(0), yFrom(0), yTo(0),
		       cbFrom(0), cbTo(0), crFrom(0), crTo(0)
{
}

void
CdtTable::Set( int ch, int yF, int yT, int cbF, int cbT, int crF, int crT )
{
    channel = ch;
    yFrom = yF; yTo = yT; cbFrom = cbF; cbTo = cbT; crFrom = crF; crTo = crT;
}

int
CdtTable::GetChannel() const
{
    return channel;
}

void
CdtTable::GetValues( int *yF, int *yT, int *cbF, int *cbT, int *crF, int *crT )
{
    *yF = yFrom; *yT = yTo;
    *cbF = cbFrom; *cbT = cbTo;
    *crF = crFrom; *crT = crTo;
}

#ifndef MAIN
void
CdtTable::SetCdt( OCdtInfo *cdt )
{
    for ( int y = yFrom; y <= yTo; y ++ ) {
	cdt->Set( y, crTo, crFrom, cbTo, cbFrom );
    }
}
#endif // MAIN

void
CdtTable::Print() const
{
    OSYSPRINT(( "ch=%d:Y(%d-%d),Cb(%d-%d),Cr(%d-%d)\n", channel,
		yFrom, yTo, cbFrom, cbTo, crFrom, crTo ));
}

int
CdtTable::ToString( char *buf )
{
    static char localbuf[128];
    sprintf( localbuf, "%1d %03d %03d %03d %03d %03d %03d\r\n",
	     channel, yFrom, yTo, cbFrom, cbTo, crFrom, crTo );
    if ( buf ) {
	strcpy( buf, localbuf );
    }
    return strlen( localbuf );
}

void
CdtTable::FromString( const char *buf )
{
    sscanf( buf, "%d %d %d %d %d %d %d",
	    &channel, &yFrom, &yTo, &cbFrom, &cbTo, &crFrom, &crTo );
}

// ----------------------------------------------------------------------

CdtFile::CdtFile() : numTables(0)
{
}

void
CdtFile::Clear()
{
    numTables = 0;
}

void
CdtFile::Add( const char *buf )
{
    int ch = -1;

    if ( numTables >= cdtMaxTables ) {
	return;
    }
    sscanf( buf, "%d", &ch );
    if ( ch < 0 || ch >= 8 ) {
	return;
    }
    int i;
    for ( i = 0; i < numTables ; i ++ ) {
	if ( ch < table[ i ].GetChannel() ) {
	    for ( int j = numTables - 1; j >= i; j -- ) {
		table[ j + 1 ] = table[ j ];
	    }
	    break;
	}
    }
    table[ i ].FromString( buf );
    numTables ++;
}

int
CdtFile::MaxChannel() const
{
    int max = -1;
    for ( int i = 0; i < numTables; i ++ ) {
	if ( max < table[ i ].GetChannel() ) {
	    max = table[ i ].GetChannel();
	}
    }
    return max;
}

int
CdtFile::IsEmpty() const
{
    return numTables == 0;
}

void
CdtFile::Print() const
{
    OSYSPRINT(( "CdtFile:\n" ));
    for ( int i = 0; i < numTables; i ++ ) {
	OSYSPRINT(( " " ));
	table[ i ].Print();
    }
}

int
CdtFile::ToString( char *buf )
{
    int total = 0;
    for ( int i = 0; i < numTables; i ++ ) {
	int size = table[ i ].ToString( buf );
	if ( buf ) {
	    buf += size;
	}
	total += size;
    }
    if ( buf ) {
	*buf = 0;
    }
    return total + 1;
}

void
CdtFile::FromString( const char *buf )
{
    while ( buf && *buf ) {
	if ( *buf != '#' ) {
	    Add( buf );
	}
	buf = strchr( buf, '\n' );
	if ( buf ) {
	    buf ++;
	}
    }
}

void
CdtFile::Read( const char *path )
{
    OSYSPRINT(( "CdtFile::Read path=%s.\n", path ));

    FILE* fp = fopen( path, "r" );
    if ( fp == NULL ) {
        OSYSLOG1(( osyslogWARNING, "CdtFile::Read can't open (%s)\n", path ));
        return;
    }
    fseek( fp, 0L, SEEK_END );
    int size = ftell( fp );
    fseek( fp, 0L, SEEK_SET );
    char *filebuf = new char[ size + 1 ];
    fread( filebuf, size, 1, fp );
    filebuf[ size ] = 0;
    fclose( fp );

    Clear();
    FromString( filebuf );
    delete[] filebuf;
    return;
}

void
CdtFile::Write( const char *path )
{
    OSYSPRINT(( "CdtFile::Write path=%s.\n", path ));

    if ( IsEmpty() ) {
        OSYSLOG1(( osyslogWARNING,
		   "CdtFile::Write don't write when empty\n" ));
	return;
    }
    FILE* fp = fopen( path, "w" );
    if ( fp == NULL ) {
        OSYSLOG1(( osyslogWARNING, "CdtFile::Write can't open (%s)\n", path ));
        return;
    }
    int size = ToString( NULL );
    char *filebuf = new char[ size ];
    ToString( filebuf );
    fwrite( filebuf, size - 1, 1, fp );
    delete[] filebuf;
    fclose( fp );
}

#ifndef MAIN
void
CdtFile::SetCdtVectorData( OPrimitiveID fbkID )
{
    OStatus result;
    MemoryRegionID  cdtVecID;
    OCdtVectorData* cdtVec;
    OCdtInfo*       cdt;
    int             ch;

    result = OPENR::NewCdtVectorData(&cdtVecID, &cdtVec);
    if (result != oSUCCESS) {
        OSYSLOG1((osyslogERROR, "%s : %s %d",
                  "CdtFile::SetCdtVectorData()",
                  "OPENR::NewCdtVectorData() FAILED", result));
        return;
    }

    cdtVec->SetNumData( MaxChannel() + 1 );

    int lastChannel = -1;
    for ( int i = 0; i < numTables; i ++ ) {
	ch = table[ i ].GetChannel();
	if ( lastChannel != ch ) {
	    lastChannel = ch;
	    cdt = cdtVec->GetInfo( ch );
	    cdt->Init( fbkID, 1 << ch );
	}
	table[ i ].SetCdt( cdt );
    }

    result = OPENR::SetCdtVectorData(cdtVecID);
    if (result != oSUCCESS) {
        OSYSLOG1((osyslogERROR, "%s : %s %d",
                  "CdtFile::SetCdtVectorData()",
                  "OPENR::SetCdtVectorData() FAILED", result));
    }

    result = OPENR::DeleteCdtVectorData(cdtVecID);
    if (result != oSUCCESS) {
        OSYSLOG1((osyslogERROR, "%s : %s %d",
                  "CdtFile::SetCdtVectorData()",
                  "OPENR::DeleteCdtVectorData() FAILED", result));
    }
}
#endif // MAIN

#ifdef MAIN
int
main()
{
    CdtFile cdtFile;

    cdtFile.Read( "CDT.CFG" );
    cdtFile.Write( "CDT.NEW" );
    return 0;
}
#endif // MAIN
