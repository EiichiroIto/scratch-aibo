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

#ifndef Logging_h_DEFINED
#define Logging_h_DEFINED

static const char* const LOGFILE  = "/MS/OPEN-R/MW/CONF/LOG%d.TXT";
const int DEFAULT_LOGSIZE = 32768;
const int DEFAULT_LOGMASK = 0x0301;
const int NUM_LOGCOUNT = 10;
const int LOGMASK_BALL = 0x0001;
const int LOGMASK_GOAL_B = 0x0002;
const int LOGMASK_GOAL_Y = 0x0004;
const int LOGMASK_MONET = 0x0100;
const int LOGMASK_STNINFO = 0x0200;

class Logging {
  private:
    int			maxLogSize;
    int			logMask;
    bool		logging;
    int			startLogMSec;
    int			currentLogCount;
    RCRegion*		rgnLog;
    char*		nextLogPtr;
    int			nextLogIndex;
    int			lastStnNo;
    int			lastStateId;

  public:
    Logging();
    void Allocate();
    void DeAllocate();
    void SetMaxLogSize( int v );
    void SetLogMask( int v );
    void Start();
    void End();
    void Reset();
    void LogCdtDetail( int logID, FoundInfo* fInfo );
    void LogCdtInfo( CdtInfo *cdtInfo );
    void LogMoNetCmd( MoNetCommandID cmdID );
    void LogSTNInfo( STNInfo *stnInfo );

  private:
    void LogString( const char *buf );
};
#endif // Logging_h_DEFINED
