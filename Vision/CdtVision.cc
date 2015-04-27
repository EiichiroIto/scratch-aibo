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
//  26 Feb 2007, Eiichiro ITO, modified beacon codes.
//
#ifndef MAIN
#include <OPENR/OObject.h>
#else // MAIN
#include <stdio.h>
#include <string.h>
typedef unsigned char byte;
#define OSYSPRINT(x) printf x
#endif // MAIN
#include "CdtVision.h"

CdtSections::CdtSections() : numSections(0),
			     minimumSize(cdtDefaultMinimumSize),
			     maximumGap(cdtDefaultMaximumGap)
{
}

void
CdtSections::Clear()
{
    numSections = 0;
}

void
CdtSections::AddPos( int pos )
{
    if ( numSections ) {
	int allowablePos = endPos[numSections-1] + maximumGap + 1;
	if ( pos <= allowablePos ) {
	    // 直前の範囲と連続しているとみなされる場合
	    endPos[ numSections - 1 ] = pos;
	    return;
	}
	// 直前の範囲の大きさを確認する
	if ( !AdjustLastData() ) {
	    // 新しい範囲を加える
	    if ( numSections >= cdtMaxPos ) {
		return; // 範囲数が超過しているときは何もしない
	    }
	}
    }
    beginPos[ numSections ] = endPos[ numSections ] = pos;
    numSections ++;
}

int
CdtSections::GetNumSections() const
{
    return numSections;
}

bool
CdtSections::GetSection( int index, int *pos, int *size ) const
{
    if ( index < 0 || index >= numSections ) {
	return false;
    }
    *pos = (beginPos[ index ] + endPos[ index ]) / 2;
    *size = endPos[ index ] - beginPos[ index ] + 1;
    return true;
}

#if 0
void
CdtSections::SetMinSizeMaxGap( int size, int gap )
{
    minimumSize = size;
    maximumGap = gap;
}
#endif

int
CdtSections::GetMinimumSize() const
{
    return minimumSize;
}

void
CdtSections::Print()
{
    for ( int i = 0; i < numSections; i ++ ) {
	OSYSPRINT(( "%d-%d ", beginPos[ i ], endPos[ i ] ));
    }
    OSYSPRINT(( "\n" ));
}

bool
CdtSections::AdjustLastData()
{
    if ( !numSections ) {
	return false;
    }
    int previousSize = endPos[numSections-1] - beginPos[numSections-1] + 1;
    if ( previousSize < minimumSize ) {
	// 直前の範囲が小さすぎる場合
	numSections --;
	return true;
    }
    return false;
}

// ----------------------------------------------------------------------

RawCdtObject::RawCdtObject() : numObjects(0)
{
}

void
RawCdtObject::Clear()
{
    numObjects = 0;
}

void
RawCdtObject::Add( int x, int y, int w, int h )
{
    if ( numObjects >= cdtMaxObjects ) {
	return;
    }
    int area = w * h;
    int i;
    for ( i = 0; i < numObjects; i ++ ) {
	ObjRect *rect = &objRects[ i ];
	if ( area >= rect->w * rect->h ) {
	    for ( int j = numObjects - 1; j >= i; j -- ) {
		objRects[ j + 1 ] = objRects[ j ];
	    }
	    break;
	}
    }
    ObjRect *rect = &objRects[ i ];
    rect->x = x;
    rect->y = y;
    rect->w = w;
    rect->h = h;
    numObjects ++;
}

int
RawCdtObject::GetNumObjects() const
{
    return numObjects;
}

bool
RawCdtObject::GetObjRect( int index, ObjRect *rect )
{
    if ( index < 0 || index >= numObjects ) {
	return false;
    }
    rect->x = objRects[ index ].x;
    rect->y = objRects[ index ].y;
    rect->w = objRects[ index ].w;
    rect->h = objRects[ index ].h;
    return true;
}

void
RawCdtObject::Print()
{
    for ( int i = 0; i < numObjects; i ++ ) {
	ObjRect *rect = &objRects[ i ];
	OSYSPRINT(( "(%d,%d)-[%d,%d] ", rect->x, rect->y, rect->w, rect->h ));
    }
    OSYSPRINT(( "\n" ));
}

// ----------------------------------------------------------------------

CdtVision::CdtVision()
{
    for ( int x = 0; x < cdtMaxCol; x ++ ) {
	colAxials[ x ] = 0;
    }
    for ( int y = 0; y < cdtMaxRow; y ++ ) {
	rowAxials[ y ] = 0;
    }
}

void
CdtVision::Update( byte *base )
{
    UpdateColAxials( base );
    UpdateRowAxials( base );
    UpdateLineColAxials( base );
    for ( int i = 0; i < cdtChannels; i ++ ) {
	SetCdtSections( &colSections[ i ], i, colAxials, cdtMaxCol );
	SetCdtSections( &rowSections[ i ], i, rowAxials, cdtMaxRow );
	ListupRawCdtObjects( i, base );
    }
#if 0 // WhiteLine
    SetCdtSections( &lineColSections, LINE_CHANNEL, lineColAxials, cdtMaxCol );
#endif
}

int
CdtVision::GetNumObjects( int channel )
{
    if ( channel < 0 || channel >= cdtChannels ) {
	return false;
    }
    return objects[ channel ].GetNumObjects();
}

bool
CdtVision::GetObjectRect( int channel, int index, ObjRect *rect )
{
    if ( channel < 0 || channel >= cdtChannels ) {
	return false;
    }
    return objects[ channel ].GetObjRect( index, rect );
}

void
CdtVision::UpdateColAxials( byte *base )
{
    for ( int x = 0; x < cdtMaxCol; x ++ ) {
	byte axial = 0;
	byte *ptr = &base[ x ];
	for ( int y = 0; y < cdtMaxRow - 1; y ++ ) {
	    axial |= *ptr;
	    ptr += cdtMaxCol;
	}
	colAxials[ x ] = axial;
    }
}

void
CdtVision::UpdateRowAxials( byte *base )
{
    for ( int y = 0; y < cdtMaxRow - 1; y ++ ) {
	byte axial = 0;
	for ( int x = 0; x < cdtMaxCol; x ++ ) {
	    axial |= *base++;
	}
	rowAxials[ y ] = axial;
    }
}

void
CdtVision::UpdateLineColAxials( byte *base )
{
    for ( int x = 0; x < cdtMaxCol; x ++ ) {
	byte axial = 0;
	byte *ptr = &base[ x + cdtMaxCol * (cdtMaxRow / 2 - 2) ];
	for ( int y = 0; y < 4; y ++ ) {
	    axial |= *ptr;
	    ptr += cdtMaxCol;
	}
	lineColAxials[ x ] = axial;
    }
}

void
CdtVision::SetCdtSections( CdtSections *objInfo, int channel,
			  byte *axials, int axialSize )
{
    byte bitmask = 1 << channel;

    objInfo->Clear();
    for ( int i = 0; i < axialSize; i ++ ) {
	if ( *axials++ & bitmask ) {
	    objInfo->AddPos( i );
	}
    }
}

void
CdtVision::ListupRawCdtObjects( int channel, byte *image )
{
    int x, y, w, h;
    int ncolSections = colSections[ channel ].GetNumSections();
    int nrowSections = rowSections[ channel ].GetNumSections();
    int minColSize = colSections[ channel ].GetMinimumSize();

    objects[ channel ].Clear();
    for ( int colIndex = 0; colIndex < ncolSections; colIndex ++ ) {
	colSections[ channel ].GetSection( colIndex, &x, &w );
	for ( int rowIndex = 0; rowIndex < nrowSections; rowIndex ++ ) {
	    rowSections[ channel ].GetSection( rowIndex, &y, &h );
#if 1
	    objects[ channel ].Add( x, y, w, h );
#else
	    if ( CheckObjectExists( image, channel, x, y, w, h ) ) {
		objects[ channel ].Add( x, y, w, h );
	    } else if ( w > 0 && w >= minColSize * 2 && w * 3 < h ) {
		int h2 = h / w;
		for ( int i = 0; i < w; i ++ ) {
		    int y2 = (y - h / 2) + i * h2 + h2 / 2;
		    if ( CheckObjectExists( image, channel, x, y2, w, h2 ) ) {
			objects[ channel ].Add( x, y2, w, h2 );
		    }
		}
	    }
#endif
	}
    }
}

bool
CdtVision::CheckDotExists( byte *image, int channel, int x, int y ) const
{
    byte bitmask = 1 << channel;
    int index = x + y * cdtMaxCol;
    if ( !(x >= 0 && y >= 0 && x < cdtMaxCol && y < cdtMaxRow) ) {
	OSYSPRINT(( "!x(%d)y(%d)\n", x, y ));
	return false;
    }
    if ( image[ index ] & bitmask ) {
	return true;
    } else if ( x > 0 && (image[ index - 1 ] & bitmask) ) {
	return true;
    } else if ( y > 0 && (image[ index - cdtMaxCol ] & bitmask) ) {
	return true;
    } else if ( x < cdtMaxCol - 1 && (image[ index + 1 ] & bitmask) ) {
	return true;
    } else if ( y < cdtMaxRow - 1 && (image[ index + cdtMaxCol ] & bitmask) ) {
	return true;
    }
    return false;
}

bool
CdtVision::CheckObjectExists( byte *image, int channel,
			      int x, int y, int w, int h ) const
{
    int cx = w > 10 ? 3 : 1;
    int cy = h > 10 ? 3 : 1;
    int ww = w > 10 ? w / 4 : w / 2;
    int hh = h > 10 ? h / 4 : h / 2;
    int count = 0;
    for ( int xx = 1; xx <= cx; xx ++ ) {
	for ( int yy = 1; yy <= cy; yy ++ ) {
	    int px = x - w / 2 + ww * xx;
	    int py = y - h / 2 + hh * yy;
	    if ( CheckDotExists( image, channel, px, py ) ) {
		count ++;
	    }
	}
    }
    return count * 2 >= cx * cy;
}

void
CdtVision::Print()
{
    OSYSPRINT(( "colAxials:" ));
    for ( int x = 0; x < cdtMaxCol; x ++ ) {
	OSYSPRINT(( "%02x ", colAxials[ x ] ));
    }
    OSYSPRINT(( "\n" ));
    OSYSPRINT(( "rowAxials:" ));
    for ( int y = 0; y < cdtMaxRow; y ++ ) {
	OSYSPRINT(( "%02x ", rowAxials[ y ] ));
    }
    OSYSPRINT(( "\n" ));
    int channel = 0;
    OSYSPRINT(( "Channel = %d:\n", channel ));
    OSYSPRINT(( " colSections: " ));
    colSections[ channel ].Print();
    OSYSPRINT(( " rowSections: " ));
    rowSections[ channel ].Print();
    OSYSPRINT(( "RawCdtObject:\n" ));
    objects[ channel ].Print();
    OSYSPRINT(( "\n" ));
}

#ifdef MAIN
int
main()
{
    CdtVision cdt;
    byte image[ cdtMaxCol * cdtMaxRow ];

    memset( image, 0, cdtMaxCol * cdtMaxRow );
    image[ 10 * cdtMaxCol + 14 ] = 1;
    image[ 10 * cdtMaxCol + 15 ] = 1;
    image[ 10 * cdtMaxCol + 17 ] = 1;
    image[ 10 * cdtMaxCol + 20 ] = 1;
    image[ 10 * cdtMaxCol + 21 ] = 1;
    image[ 11 * cdtMaxCol + 15 ] = 1;
    image[ 12 * cdtMaxCol + 15 ] = 1;
    image[ 16 * cdtMaxCol + 21 ] = 1;
    image[ 16 * cdtMaxCol + 22 ] = 1;
    cdt.UpdateAxials( image );
    cdt.Print();
    // Channel = 0:
    //  colSections: 14-17 20-21 
    //  rowSections: 10-12 
}
#endif // MAIN
