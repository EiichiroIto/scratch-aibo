//
// Copyright 2003-2005 (C) Eiichiro ITO, GHC02331@nifty.com
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

#ifndef Memory_h_DEFINED
#define Memory_h_DEFINED
#include "VisionInfo.h"

#define MAX_MONET_DELTA 30

struct PolarCoord {
    bool found;
    int msec;
    double theta;
    int r;
    int moNetCount;
};

struct MoNetDelta {
    MoNetCommandID cmdID;
    double theta;
    int r;
};

class Memory {
  private:
    PolarCoord coords[ MAX_IDENTIFIED_OBJECT ];
    int currentMoNetCmdID;
    MoNetDelta deltas[ MAX_MONET_DELTA ];
    int nDeltas;

  public:
    Memory();
    void Clear();
    void SetCdtInfo( CdtInfo* _cdtInfo );
    void MoNetExecute( MoNetCommandID cmdID );
    bool CheckRange( EAiboIdentifiedObject id, double angle1, double angle2, double dist1, double dist2 ) const;
    int ReadAdjustment( const char *path );
    double GetTheta( EAiboIdentifiedObject id ) const;
    void SetDefaultTimeout( int sec );
    void SetBallTimeout( int sec );

};
#endif // Memory_h_DEFINED
