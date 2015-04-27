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
#ifndef Vision_Param_h_DEFINED
#define Vision_Param_h_DEFINED

static const char* const DEFART_CONFIG  = "/MS/OPEN-R/MW/CONF/DEFART.CFG";

struct RecognitionGeometry {
    int obj_w;		// 物体の実際の横幅(mm)
    int obj_h;		// 物体の実際の高さ(mm)
    int min_w;		// 最小の幅(pixel)
    int min_h;		// 最小の高さ(pixel)
};

enum RGeometryIndex {
    RGeometryBall = 0, RGeometryGoal, RGeometryBeacon,
};
#define NumRGeometry  3

extern RecognitionGeometry RGeometry[NumRGeometry];
bool CheckGeometry( RGeometryIndex index, ObjPosition *obj );
bool ReadGeometry( const char *path );

#endif // Vision_Param_h_DEFINED
