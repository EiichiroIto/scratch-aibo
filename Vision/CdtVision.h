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
#ifndef CdtVision_h_DEFINED
#define CdtVision_h_DEFINED
#include "VisionInfo.h"

static const int cdtMaxCol = LAYER_C_WIDTH;
static const int cdtMaxRow = LAYER_C_HEIGHT;
static const int cdtChannels = NUM_CHANNELS;
static const int cdtMaxPos = 8;
static const int cdtDefaultMinimumSize = 2; // 物体とみなすのに必要な最小幅
static const int cdtDefaultMaximumGap = 1; // 連続しているとみなす最大幅
static const int cdtMaxObjects = 8;

class CdtSections {
  private:
    static const int undefinedIndex = -1;
    int beginPos[ cdtMaxPos ];
    int endPos[ cdtMaxPos ];
    int numSections;
    int minimumSize;
    int maximumGap;

  public:
    CdtSections();

    void Clear();
    void AddPos( int pos );
    int GetNumSections() const;
    bool GetSection( int index, int *pos, int *size ) const;
    void SetMinSizeMaxGap( int size, int gap );
    void Print();
    int GetMinimumSize() const;

  private:
    bool AdjustLastData();
    
};

struct ObjRect {
    int x, y, w, h;
};

class RawCdtObject {
  private:
    ObjRect objRects[ cdtMaxObjects ] ;
    int numObjects;

  public:
    RawCdtObject();

    void Clear();
    void Add( int x, int y, int w, int h );
    int GetNumObjects() const;
    bool GetObjRect( int index, ObjRect *rect );
    void Print();
};

class CdtVision {
  private:
    byte colAxials[ cdtMaxCol ];
    byte rowAxials[ cdtMaxRow ];
    byte lineColAxials[ cdtMaxCol ];
    CdtSections colSections[ cdtChannels ];
    CdtSections rowSections[ cdtChannels ];
    CdtSections lineColSections;
    RawCdtObject objects[ cdtChannels ];

  public:
    CdtVision();

    void Update( byte *base );
    int GetNumObjects( int channel );
    bool GetObjectRect( int channel, int index, ObjRect *rect );
    void Print();

  private:
    void UpdateColAxials( byte *base );
    void UpdateRowAxials( byte *base );
    void UpdateLineColAxials( byte *base );
    void SetCdtSections( CdtSections *objInfo, int channel,
			byte *axials, int axialSize );
    void ListupRawCdtObjects( int channel, byte *image );
    bool CheckDotExists( byte *image, int channel, int x, int y ) const;
    bool CheckObjectExists( byte *image, int channel,
			    int x, int y, int w, int h ) const;
};
#endif // CdtVision_h_DEFINED
