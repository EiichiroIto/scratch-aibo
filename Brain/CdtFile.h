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

#ifndef CdtFile_h_DEFINED
#define CdtFile_h_DEFINED
#ifndef MAIN
#include <OPENR/OPENRAPI.h>
#include <OPENR/OObject.h>
#endif // MAIN

static const char* const CDT_CONFIG  = "/MS/OPEN-R/MW/CONF/CDT.CFG";
static const int cdtMaxTables = 32*8;

class CdtTable {
  private:
    int channel;
    int yFrom, yTo;
    int cbFrom, cbTo;
    int crFrom, crTo;

  public:
    CdtTable();

    void Set( int ch, int yF, int yT, int cbF, int cbT, int crF, int crT );
    int GetChannel() const;
    void GetValues( int *yF, int *yT, int *cbF, int *cbT, int *crF, int *crT );
#ifndef MAIN
    void SetCdt( OCdtInfo *cdt );
#endif // MAIN
    void FromString( const char *buf );
    int ToString( char *buf );

    void Print() const;
};

class CdtFile {
  private:
    CdtTable table[ cdtMaxTables ];
    int numTables;

  public:
    CdtFile();
    void Clear();
    void Print() const;
    void FromString( const char *buf );
    int ToString( char *buf );
    void Read( const char *path );
    void Write( const char *path );
    void SetCdtVectorData( OPrimitiveID fbkID );

  private:
    void Add( const char *buf );
    int MaxChannel() const;
    int IsEmpty() const;
};
#endif // CdtFile_h_DEFINED
