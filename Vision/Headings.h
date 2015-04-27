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
#ifndef Heading_h_DEFINED
#define Heading_h_DEFINED

const char* const HEAD_MOTION_CONFIG  = "/MS/OPEN-R/MW/CONF/HEADINGS.CFG";
const int MAX_HDCMD = 100;
const int MAX_HDPOS = 100;

class HeadingCommand {
  public:
    HeadingCommand();
    void Set( int _cmdID, int _startIndex, int _endIndex );
    int GetCmdID() const;
    int GetStartIndex() const;
    int GetEndIndex() const;

  private:
    int cmdID;
    int startIndex;
    int endIndex;
};

class Headings {
  public:
    Headings();
    ~Headings();

    bool ReadConfig( const char *path );
    void Print() const;
    const HeadingPosition *GetHeadingPosition( int cmdID, int index );

  private:
    int numHcmd;
    int numHpos;
    HeadingCommand *hcmd;
    HeadingPosition *hpos;

    void Clear();
    void ReadFromString( const char *buf );
};


#endif // Heading_h_DEFINED
