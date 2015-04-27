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
// Eiichiro ITO, 15 August 2003
// mailto: GHC02331@nifty.com
#include <string.h>
#include "Posture.h"
#include "MoNetData.h"

#ifdef MAIN
#include <stdio.h>
#include <stdlib.h>
#define OSYSPRINT(x) printf x
#else // MAIN
#include <OPENR/OPENRAPI.h>
#include <OPENR/OObject.h>
#endif // MAIN

Posture::Posture()
{
    size = 0;
    nextPostureCode = 3;
}

void
Posture::Add( const char *_name, int _code )
{
    if ( size >= MaxPostures ) {
	return;
    }
    strcpy( name[ size ], _name );
    code[ size ] = _code;
    size ++;
}

int
Posture::Find( const char *_name ) const
{
    for ( int i = 0; i < size; i ++ ) {
	if ( !stricmp( name[ i ], _name ) ) {
	    return code[ i ];
	}
    }
    return -1;
}

void
Posture::Print() const
{
    OSYSPRINT(( "Postures:\n" ));
    for ( int i = 0; i < size; i ++ ) {
	OSYSPRINT(( " [%s] %d\n", name[ i ], code[ i ] ));
    }
}

#define LINEBUFSIZE 128

void
Posture::Read( const char* path )
{
    char buf[LINEBUFSIZE], *p;
    char posture[128];
    int code;

    OSYSPRINT(( "MoNet::ReadPosture(%s)\n", path ));

    FILE* fp = fopen( path, "r" );
    if ( fp == NULL ) {
        OSYSPRINT(( "%s : %s %s",
		    "Posture::Read()", "can't open", path ));
        return;
    }
    while ( fgets( buf, LINEBUFSIZE, fp ) != 0 ) {
	p = strchr( buf, '\r' );
	if ( p ) {
	    *p = '\0';
	}
	p = strchr( buf, '\n' );
	if ( p ) {
	    *p = '\0';
	}
        if ( *buf == '#' || !*buf ) {
	    continue;
	}
        strcpy( posture, strtok( buf, " \t" ) );
	if ( !stricmp( posture, "undef" ) ) {
	    code = monetpostureUNDEF;
	} else if ( !stricmp( posture, "any" ) ) {
	    code = monetpostureANY;
	} else if ( !stricmp( posture, "nt" ) ) {
	    code = monetpostureNT;
	} else if ( !stricmp( posture, "sleep" ) ) {
	    code = monetpostureSLEEP;
	} else {
	    code = nextPostureCode ++;
	}
	Add( posture, code );
    }
    fclose( fp );
    //Print();
}

#ifdef MAIN
int
main()
{
    Posture posture;
#if 1
    posture.Read( "POSTURE.CFG" );
#else
    posture.Add( "UNDEF", -1 );
    posture.Add( "ANY", 0 );
    posture.Add( "NT", 1 );
    posture.Add( "STAND", 2 );
    posture.Add( "SIT", 3 );
    posture.Add( "SLEEP", 4 );
    posture.Add( "WALK", 5 );
    posture.Add( "TURNL", 6 );
    posture.Add( "TURNR", 7 );
    posture.Print();
#endif

    printf( "stand=%d\n", posture.Find("stand") );
    printf( "turnr=%d\n", posture.Find("turnr") );
    printf( "undef=%d\n", posture.Find("undef") );
    return 0;
}
#endif // MAIN
