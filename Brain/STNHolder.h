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
// Eiichiro ITO, 12 August 2003
// mailto: GHC02331@nifty.com

#ifndef STNHolder_h_DEFINED
#define STNHolder_h_DEFINED
#include "STN.h"
#include "Perception.h"
#include "Sensor.h"
#include "ControlInfo.h"
#ifndef MAIN
#include <SystemTime.h>
#endif // MAIN

static const char* const HOLDER_PREFIX  = "/MS/OPEN-R/MW/CONF/HOLDER/";
static const int DefaultReturnNo = -1;
static const int DefaultMessageNo = -1;

#define MAX_STACK_STNNO 10
#define DEFAULT_WATCHDOG_TIMER_IN_SEC 60
#define MAX_COUNTER 100

const int argIndexOfVisionAngle1 = 0;
const int argIndexOfVisionAngle2 = 1;
const int argIndexOfVisionDistance1 = 2;
const int argIndexOfVisionDistance2 = 3;
const int argIndexOfVisionNearX1 = 0;
const int argIndexOfVisionNearX2 = 1;
const int argIndexOfVisionNearY1 = 2;
const int argIndexOfVisionNearY2 = 3;

#define condCounterOP(x) ((x) % 10)
#define condCounterARG(x) ((x) / 10)
const int counterOpEqual = 0;
const int counterOpLeq = 1;
const int counterOpGeq = 2;

enum ScoreInfo {
    SCORE_DRAW = 0, SCORE_WIN, SCORE_LOSE
};

int ::GetNowMSec();

class STNHolder;
class Brain;

class STNProcess {
  private:
    STNHolder *holder;
    STN *stn;
    int nTasks;
    int currentStateIds[ MAX_TASKS ];
    int currentTask;

  public:
    STNProcess();
    void Setup( STNHolder *holder, STN *stn );
    void Print() const;
    bool Execute( STNCmds *cmds, STNState **result );
    void GetCurrentInfo( STNInfo *stnInfo ) const;

  private:
    STNArc* GetFiredArc( list<STNArc*>& arcList );
    bool CheckCondition( const STNCondition& cond );
    bool CheckVision( int conditionCode, const Perception *perception, bool bRedTeam ) const;
    bool CheckTimer( int timerMSec, int conditionCode ) const;
    bool CheckMisc( int conditionCode, Perception *perception );
    bool CheckReturn( int conditionCode ) const;
    bool CheckMessage( int conditionCode );
    bool CheckCounter( int conuterNo, int conditionCode ) const;
    int ConvertHeadCmd( int hcmd ) const;
};

class STNHolder {
  private:
    Brain* brain;	     // Brainオブジェクトへのポインタ
    Perception* perception;  // Perceptionオブジェクトへのポインタ
    Sensor* sensor;          // Sensorオブジェクトへのポインタ
    STN stns[ MAX_STN ];     // 状態遷移図
    STNProcess processes[ MAX_STN ];
    int currentStnNo;
    int nowMSec;   	     // 処理中の現在時刻
    bool bOwnKickoff;        // キックオフ: true=自チーム, false=敵チーム
    bool bRedTeamColor;      // チームカラー: true=赤, false=青
    ScoreInfo score;	     // 勝ち負け
    int returnNo;	     // 復帰番号
    int messageNo;	     // メッセージ番号
    int stackStnNo[ MAX_STACK_STNNO ]; // stnNoスタック
    int nStackStnNo;	     // stnNoスタックのデータ数
    int timer1MSec;	     // タイマー1
    int timer2MSec;	     // タイマー2
    int lastMovedMSec;	     // 最後に遷移した時刻
    int counter1;	     // カウンター1
    int counter2;	     // カウンター2
    int counter3;	     // カウンター3
    int watchDogSec;	     // 全停止までの秒数

  public:
    STNHolder( Brain* _brain, Perception* _perception, Sensor* _sensor );

    void ClearAll();
    void Clear( int stnNo );
    bool ReadAll();
    void Add( int stnNo, const char *string );
    void Print( int stnNo );
    void PrintAll();
    void SetKickoff( bool _bOwnKickoff );
    void SetTeamColor( bool _bRedTeamColor );
    void SetScore( ScoreInfo _score );
    bool GetCurrentInfo( STNInfo *stnInfo ) const;
    void SetMessageNo( int msgNo );
    void Activate( int stnNo, bool bSetInitialState );
    bool ColdStart( int stnNo );
    bool Process( STNCmds* cmds );
    void StopAll();
    bool IsOwnKickoff() const;
    bool IsRedTeamColor() const;
    bool IsWinner() const;
    bool IsLoser() const;
    bool IsDrawn() const;
    int GetTimer1MSec() const;
    int GetTimer2MSec() const;
    int GetNowMSec() const;
    int GetReturnNo() const;
    int GetMessageNo() const;
    Brain *GetBrain() const;
    Perception *GetPerception() const;
    Sensor *GetSensor() const;
    void InternalExecute( int iCmd );
    void UpdateLastMovedMSec();
    int GetCounterValue( int counterNo ) const;
    int SetWatchDogSec( int sec );

  private:
    void Initialize();
    bool JumpStart( int newStnNo );
    bool Interrupt( int newStnNo );
    bool Return( int retNo );
    void SetVerboseMode( int vMode );
    bool Start( int stnNo, bool bReturn );
    bool IsExecutable( int stnNo );
    void PushStnNo( int stnNo );
    int PopStnNo();
    void ClearStack();
    void SetTimer1MSec( int msec );
    void SetTimer2MSec( int msec );
};

#endif // STNHolder_h_DEFINED
