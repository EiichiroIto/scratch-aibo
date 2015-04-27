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
#ifndef ControlInfo_h_DEFINED
#define ControlInfo_h_DEFINED

// STN番号
static const int StnNo_Initial = 0;
static const int StnNo_Ready = 1;
static const int StnNo_Set = 2;
static const int StnNo_PlayingKickoff = 3;
static const int StnNo_PlayingNoKickoff = 4;
static const int StnNo_PlayingAfterPenalized = 5;
static const int StnNo_Finished = 6;
static const int StnNo_Penalized = 7;
static const int StnNo_Message = 8;
static const int StnNo_AllStopped = 9;
static const int InvalidStnNo = -1;
#define MAX_SPECIAL_STN 10    // ゲーム状態など特殊な状態の個数(0..9)
#define MIN_USER_STN 10       // ユーザー用の最初のSTN番号
#define MAX_STN 100
#define MAX_TASKS 10

enum EAiboControlID {
    CtID_Monet = 0, CtID_MonetSync,
    CtID_StartSTN, CtID_StopSTN, CtID_ReadSTN,
    CtID_ClearSTN, CtID_AddSTN,
    CtID_GetCurrentStateSTN,
    CtID_GetCdtInfo, CtID_SendCdt, CtID_WriteCdt, CtID_ReadCdt,
    CtID_Head, CtID_Face, CtID_GetCdt, CtID_SetPose,
    CtID_ReadHead, CtID_Internal, CtID_SetCamParam
};

struct ControlInfo {
    EAiboControlID id;
    int iValue;
    int i2Value;
    double dValue;
    union {
	char sValue[4096];
	double dValue2[32];
    };
};

struct STNCmds {
    int monetCmd;
    int headCmd;
    int faceCmd;
    int internalCmd;
};

struct STNInfo {
    int stnNo;
    int stateIds[ MAX_TASKS ];
    int timer1Sec, timer2Sec; // タイマー(s)
    int counter1, counter2, counter3; // カウンター
    STNCmds cmds;
};

#define isValidKT(ci) ((ci)->i2Value & 0x80)
#define isKickoff(ci) ((ci)->i2Value & 0x01)
#define isRedTeam(ci) ((ci)->i2Value & 0x02)
#define isWin(ci) ((ci)->i2Value & 0x04)
#define isLose(ci) ((ci)->i2Value & 0x08)

#endif // ControlInfo_h_DEFINED
