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
// Eiichiro ITO, 15 October 2005
// mailto: GHC02331@nifty.com
//
#ifndef VisionInfo_h_DEFINED
#define VisionInfo_h_DEFINED

#define VERBOSE_NONE 0
#define VERBOSE_DEFAULT 5

#ifdef ERS210
#define LAYER_C_WIDTH      (88)
#define LAYER_C_HEIGHT     (72)
#define LAYER_M_WIDTH      (88)
#define LAYER_M_HEIGHT     (72)
#define LAYER_L_WIDTH      (44)
#define LAYER_L_HEIGHT     (36)
#define FIELD_VIEW_H       (57.6)
#define FIELD_VIEW_V       (47.8)
#define NUM_CHANNELS       (8)
#endif // ERS210
#ifdef ERS7
#define LAYER_C_WIDTH      (104)
#define LAYER_C_HEIGHT     (80)
#define LAYER_M_WIDTH      (104)
#define LAYER_M_HEIGHT     (80)
#define LAYER_L_WIDTH      (52)
#define LAYER_L_HEIGHT     (40)
#define FIELD_VIEW_H       (56.9)
#define FIELD_VIEW_V       (45.2)
#define NUM_CHANNELS       (7)
#endif // ERS7

#define LAYER_C_IMAGESIZE  (LAYER_C_WIDTH*LAYER_C_HEIGHT)
#define LAYER_M_IMAGESIZE  (LAYER_M_WIDTH*LAYER_M_HEIGHT*3)
#define LAYER_L_IMAGESIZE  (LAYER_L_WIDTH*LAYER_L_HEIGHT*3)

#define ORANGE_CHANNEL     (0)
#define YELLOW_CHANNEL     (1)
#define AQUABLUE_CHANNEL   (2)
#define DARKBLUE_CHANNEL   (3)
#define RED_CHANNEL        (4)
#define PINK_CHANNEL       (5)
#define GREEN_CHANNEL      (6)
#define BALL_CHANNEL       ORANGE_CHANNEL
#define NUM_CDTOBJECTS     (8)

enum EAiboImageType {
    IMAGE_LAYER_C = 0,
    IMAGE_LAYER_L,
    IMAGE_LAYER_M,
};

enum EAiboIdentifiedObject {
    IO_UNIDENTIFIED = 0, IO_BALL, IO_GOAL_B, IO_GOAL_Y,
    IO_BEACON_BY, IO_BEACON_YB, MAX_IDENTIFIED_OBJECT
};

struct ObjPosition {
    double pan, tilt;
    int x, y, w, h;
};

struct FoundInfo {
    bool found;
    ObjPosition objPos;
    int distance;
    int msec;
};

struct CdtObject {
    EAiboIdentifiedObject label;
    ObjPosition objPos;
};

struct CdtInfo {
    FoundInfo foundInfo[ MAX_IDENTIFIED_OBJECT ];
    int seqno;
    int nCdtObjects[ NUM_CHANNELS ];
    CdtObject cdtObjects[ NUM_CHANNELS ][ NUM_CDTOBJECTS ];
    bool isMoving;
};

struct HeadingPosition {
    double tilt, pan, tilt2, mouth, speed;
};

struct VCmdInfo {
    int hcmd;
    int iValue;
    HeadingPosition hpos;
    int seqno;
};

#endif // VisionInfo_h_DEFINED
