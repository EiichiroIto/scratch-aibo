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
// Eiichiro ITO, 13 August 2003
// mailto: GHC02331@nifty.com

#ifndef Perception_h_DEFINED
#define Perception_h_DEFINED
#include "command.h"
#include "VisionInfo.h"

static const int MAX_COORD = 10;

class Perception {
  private:
    CdtInfo recentCdtInfo;	// 最後に見えた時の情報
    CdtInfo currentCdtInfo;	// 直前の情報
    int lastCheckedMSec;	// 最後に条件チェックを行った時刻
    int nowMSec;		// 現在条件チェックを行っている時刻

  public:
    Perception();
    void SetCdtInfo( CdtInfo* _cdtInfo );
    void UpdateWhenFound( FoundInfo* recent, FoundInfo* current );
    void UpdateNowMSec( int msec );
    bool LostCondition( EAiboIdentifiedObject id ) const;
    bool VisionRangeCondition( EAiboIdentifiedObject id, double angle1, double angle2, double dist1, double dist2 ) const;
    bool VisionNearRangeCondition( EAiboIdentifiedObject id, int x1, int x2, int y1, int y2 ) const;
    int GetCdtInfoSeqno() const;
    char *CdtInfoString();
};
#endif // Perception_h_DEFINED
