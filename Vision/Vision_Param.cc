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
//  26 Feb 2007, Eiichiro ITO, changed goal height.
//
#include "Vision.h"

#define OrangeBallDiameter 85
#define PinkBallDiameter 70
#define GoalWidth 800
#define GoalHeight 350
#define BeaconWidth 110
#define BeaconHeight 200

RecognitionGeometry RGeometry[NumRGeometry] = {
    { OrangeBallDiameter, OrangeBallDiameter, 1, 1 },
    { GoalWidth, GoalHeight, 1, 1 },
    { BeaconWidth, BeaconHeight, 1, 1 },
};
static char *GeometryName[NumRGeometry] = { "Ball", "Goal", "Beacon" };

bool
CheckGeometry( RGeometryIndex index, ObjPosition *obj )
{
    RecognitionGeometry *gp = &RGeometry[ index ];
    if ( obj->w < gp->min_w || obj->h < gp->min_h ) {
	return false;
    }
    return true;
}

void
GeometryFromString( const char *buf )
{
    int index, w, h;

    while ( buf && *buf ) {
	if ( *buf == '#' ) {
	    // skip comment
	} else if ( !memcmp( buf, "GEO ", 4 ) ) {
	    sscanf( &buf[4], "%d %d %d", &index, &w, &h );
	    if ( index >= 0 && index < NumRGeometry ) {
		RecognitionGeometry *gp = &RGeometry[ index ];
		gp->obj_w = w;
		gp->obj_h = h;
	    } else {
		OSYSPRINT(( "Vision::GeometryFromString invalid index(%-10.10s)",
			    buf ));
	    }
	} else if ( !memcmp( buf, "GMIN ", 5 ) ) {
	    sscanf( &buf[5], "%d %d %d", &index, &w, &h );
	    if ( index >= 0 && index < NumRGeometry ) {
		RecognitionGeometry *gp = &RGeometry[ index ];
		gp->min_w = w;
		gp->min_h = h;
	    } else {
		OSYSPRINT(( "Vision::GeometryFromString invalid index(%-10.10s)",
			    buf ));
	    }
	}
	buf = strchr( buf, '\n' );
	if ( buf ) {
	    buf ++;
	}
    }
}

void
PrintGeometry()
{
    OSYSPRINT(( "RecognitionGeometry:\n" ));
    for ( int i = 0; i < NumRGeometry; i ++ ) {
	RecognitionGeometry *gp = &RGeometry[ i ];
	OSYSPRINT(( " %s: W%dxH%d(cm), W>%d H>%d(px)\n",
		    GeometryName[i],
		    gp->obj_w, gp->obj_h,
		    gp->min_w, gp->min_h ));
    }
}

bool
ReadGeometry( const char *path )
{
    OSYSPRINT(( "ReadGeometry from %s.\n", path ));

    FILE* fp = fopen( path, "r" );
    if ( fp == NULL ) {
        OSYSPRINT(( "ReadGeometry: %s %s\n", "can't open", path ));
        return false;
    }
    fseek( fp, 0L, SEEK_END );
    int size = ftell( fp );
    fseek( fp, 0L, SEEK_SET );
    char *filebuf = new char[ size + 1 ];
    fread( filebuf, size, 1, fp );
    filebuf[ size ] = 0;
    fclose( fp );

    GeometryFromString( filebuf );
    PrintGeometry();
    delete[] filebuf;
    return true;
}
