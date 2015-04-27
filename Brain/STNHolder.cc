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
#include "Brain.h"
#include <OPENR/OObject.h>
#include <SystemTime.h>
#include <MoNetData.h>
#include "command.h"

#define systemTimeToMilliSeconds(x) ((x).seconds*1000+(x).useconds/1000)
#define CheckValidStnNo(x) ((x)>=0 && (x)<MAX_STN)
#define IsSpecialStnNo(x) ((x)>=0 && (x)<MAX_SPECIAL_STN)

static int verboseMode = 8;

// 条件コードと対応しているので削除や順序変更は不可。
enum EAiboVisionRangeObject {
    VR_BALL = 0,
    VR_DUMMY1, VR_DUMMY2, VR_DUMMY3, VR_DUMMY4,
    VR_GOAL_M, VR_GOAL_T, VR_BEACON_L, VR_BEACON_R,
    MAX_VISION_RANGE_OBJECT
};

// 条件コードと対応しているので項目の削除は不可。
EAiboIdentifiedObject RedTeamIds[] = {
    IO_BALL,
    IO_BALL, IO_BALL, IO_BALL, IO_BALL,
    IO_GOAL_Y, IO_GOAL_B, IO_BEACON_BY, IO_BEACON_YB
};
EAiboIdentifiedObject BlueTeamIds[] = {
    IO_BALL,
    IO_BALL, IO_BALL, IO_BALL, IO_BALL,
    IO_GOAL_B, IO_GOAL_Y, IO_BEACON_YB, IO_BEACON_BY
};

int
GetNowMSec()
{
    SystemTime current;
    GetSystemTime( &current );
    return systemTimeToMilliSeconds(current);
}

STNHolder::STNHolder( Brain* _brain, Perception* _perception, Sensor* _sensor ) :
    brain( _brain ), perception( _perception ), sensor( _sensor ),
    nowMSec( 0 ), bOwnKickoff(false), bRedTeamColor(false), score(SCORE_DRAW),
    returnNo(DefaultReturnNo), messageNo(DefaultMessageNo),
    nStackStnNo(0), timer1MSec(0), timer2MSec(0),
    lastMovedMSec(0), currentStnNo(InvalidStnNo),
    watchDogSec(DEFAULT_WATCHDOG_TIMER_IN_SEC)
{
}

void
STNHolder::ClearAll()
{
    for ( int i = 0; i < MAX_STN; i ++ ) {
	Clear( i );
    }
}

void
STNHolder::Clear( int stnNo )
{
    OSYSDEBUG(( "STNHolder::Clear stnNo=%d", stnNo ));

    if ( !CheckValidStnNo( stnNo ) ) {
	OSYSLOG1(( osyslogERROR,
		   "STNHolder::Clear invalid stn<%02d>\n", stnNo ));
	return;
    }
    stns[ stnNo ].Clear();
}

bool
STNHolder::ReadAll()
{
    int count = 0;
    char path[ 128 ], id[ 3 ];

    for ( int i = 0; i < MAX_STN; i ++ ) {
	strcpy( path, HOLDER_PREFIX );
	id[0] = (i / 10) + '0';
	id[1] = (i % 10) + '0';
	id[2] = 0;
	strcat( path, id );
	strcat( path, ".dst" );
	if ( stns[ i ].Read( path ) ) {
	    processes[ i ].Setup( this, &stns[ i ] );
	    count ++;
	}
    }
    if ( verboseMode >= 7 ) {
	OSYSPRINT(( "STNHolder::Read %d files done.\n", count ));
    }
    // カウンターをリセットする
    counter1 = counter2 = counter3 = 0;
    return count > 0;
}

void
STNHolder::Add( int stnNo, const char *string )
{
    OSYSPRINT(( "STNHolder::Add stnNo=%d,[%10.10s]\n", stnNo, string ));

    if ( !CheckValidStnNo( stnNo ) ) {
	OSYSLOG1(( osyslogERROR,
		   "STNHolder::Add invalid stn<%02d>\n", stnNo ));
	return;
    }
    stns[ stnNo ].FromString( string );
    processes[ stnNo ].Setup( this, &stns[ stnNo ] );
}

void
STNHolder::Print( int stnNo )
{
    if ( !CheckValidStnNo( stnNo ) ) {
	OSYSLOG1(( osyslogERROR,
		   "STNHolder::Print invalid stn<%02d>\n", stnNo ));
	return;
    }
    OSYSPRINT(( "STNHolder::Print stn<%02d>:\n", stnNo ));
    stns[ stnNo ].Print();
}

void
STNHolder::PrintAll()
{
    for ( int i = 0; i < MAX_STN; i ++ ) {
	stns[ i ].Print();
    }
}

void
STNHolder::SetKickoff( bool _bOwnKickoff )
{
    if ( bOwnKickoff != _bOwnKickoff ) {
	bOwnKickoff = _bOwnKickoff;
    }
    if ( verboseMode >= 6 ) {
	OSYSPRINT(( "STNHolder::SetKickoff from %s kickoff\n",
		    bOwnKickoff ? "Own" : "Opponent" ));
    }
}

void
STNHolder::SetTeamColor( bool _bRedTeamColor )
{
    if ( bRedTeamColor != _bRedTeamColor ) {
	bRedTeamColor = _bRedTeamColor;
    }
    if ( verboseMode >= 6 ) {
	OSYSPRINT(( "STNHolder::SetTeamColor My team is %s Color Team\n",
		    bRedTeamColor ? "Red" : "Blue" ));
    }
}

void
STNHolder::SetScore( ScoreInfo _score )
{
    score = _score;
    if ( verboseMode >= 6 ) {
	OSYSPRINT(( "STNHolder::SetScore %d\n", score ));
    }
}

void
STNHolder::SetMessageNo( int msgNo )
{
    messageNo = msgNo;
    if ( verboseMode >= 6 ) {
	OSYSPRINT(( "STNHolder::SetMessageNo Received Message msgNo=%d\n",
		    messageNo ));
    }
    // メッセージ処理中に別のメッセージが来たら無視する
    Interrupt( StnNo_Message );
}

void
STNHolder::SetVerboseMode( int vMode )
{
    verboseMode = vMode;
}

void
STNHolder::Activate( int stnNo, bool bSetInitialState )
{
    nowMSec = ::GetNowMSec();
    UpdateLastMovedMSec();
    if ( bSetInitialState ) {
	processes[ stnNo ].Setup( this, &stns[ stnNo ] );
    }
    if ( verboseMode >= 6 ) {
	OSYSPRINT(( "STNHolder::Activate stn<%02d>\n", stnNo ));
	processes[ stnNo ].Print();
    }
    currentStnNo = stnNo;
}

bool
STNHolder::ColdStart( int newStnNo )
{
    if ( verboseMode >= 7 ) {
	OSYSPRINT(( "STNHolder::ColdStart start stn<%02d>\n", newStnNo ));
    }
    // 実行可能かどうかチェックする
    if ( !IsExecutable( newStnNo ) ) {
	OSYSPRINT(( "STNHolder::ColdStart can't start stn<%02d>\n",
		    newStnNo ));
	return false;
    }
    // 復帰リストを初期化する
    ClearStack();
    // 復帰番号を初期化する
    returnNo = DefaultReturnNo;
    // 開始時刻を設定する
    nowMSec = ::GetNowMSec();
    SetTimer1MSec( nowMSec );
    SetTimer2MSec( nowMSec );
    // 開始させる
    Activate( newStnNo, true );
    return true;
}

void
STNHolder::StopAll()
{
    currentStnNo = InvalidStnNo;
}

bool
STNHolder::JumpStart( int newStnNo )
{
    if ( verboseMode >= 7 ) {
	OSYSPRINT(( "STNHolder::JumpStart start stn<%02d> from stn<%02d>\n",
		    newStnNo, currentStnNo ));
    }
    if ( !IsExecutable( newStnNo ) || IsSpecialStnNo( newStnNo ) ) {
	OSYSPRINT(( "STNHolder::JumpStart can't start stn<%02d>\n",
		    newStnNo ));
	if ( !IsSpecialStnNo( currentStnNo ) ) {
	    // 特殊StnNoからはAllStopped stnをスタートさせない
	    OSYSPRINT(( "STNHolder::JumpStart start emergency stn\n" ));
	    return ColdStart( StnNo_AllStopped );
	}
	return false;
    }
    // stnNoを復帰リストに加える
    PushStnNo( currentStnNo );
    // 復帰番号を初期化する
    returnNo = DefaultReturnNo;
    // 開始させる
    Activate( newStnNo, true );
    return true;
}

bool
STNHolder::Interrupt( int newStnNo )
{
    if ( verboseMode >= 7 ) {
	OSYSPRINT(( "STNHolder::Interrupt start stn<%02d> from stn<%02d>\n",
		    newStnNo, currentStnNo ));
    }
    if ( IsSpecialStnNo( currentStnNo ) ) {
	OSYSPRINT(( "STNHolder::Interrupt can't in special stn<%02d>\n",
		    currentStnNo ));
	return false;
    }
    if ( !IsExecutable( newStnNo ) ) {
	OSYSPRINT(( "STNHolder::Interrupt can't start stn<%02d>\n",
		    newStnNo ));
	return false;
    }
    // アクティブなStn番号を復帰リストに加える
    PushStnNo( currentStnNo );
    // 開始させる
    Activate( newStnNo, true );
    return true;
}

bool
STNHolder::Return( int retNo )
{
    if ( verboseMode >= 7 ) {
	OSYSPRINT(( "STNHolder::Return with returnNo=%d\n", retNo ));
    }
    int newStnNo = PopStnNo();
    // 実行可能で、特殊STNでない場合に復帰できる。
    if ( !IsExecutable( newStnNo ) ||
	 (IsSpecialStnNo( currentStnNo ) && currentStnNo != StnNo_Message) ) {
	OSYSPRINT(( "STNHolder::Return can't return to stn<%02d>\n",
		    newStnNo ));
	if ( !IsSpecialStnNo( currentStnNo ) ) {
	    // 特殊StnNoからはAllStopped stnをスタートさせない
	    OSYSPRINT(( "STNHolder::Return start emergency stn\n" ));
	    return ColdStart( StnNo_AllStopped );
	}
	return false;
    }
    if ( currentStnNo == StnNo_Message ) {
	if ( verboseMode >= 7 ) {
	    OSYSPRINT(( "STNHolder::Return don't set returnNo in Message" ));
	}
    } else {
	// 復帰番号をセットする
	returnNo = retNo;
    }
    // 開始させる
    Activate( newStnNo, false );
    return true;
}

bool
STNHolder::Process( STNCmds* cmds )
{
    nowMSec = ::GetNowMSec();
    int stnNo = currentStnNo;
    int bError = false;
    if ( stnNo == InvalidStnNo ) {
	bError = true;
	OSYSPRINT(( "%s no active stns\n", "STNHolder::Process" ));
    } else if ( !IsSpecialStnNo( stnNo ) &&
		nowMSec - lastMovedMSec >= watchDogSec * 1000 ) {
	bError = true;
	OSYSPRINT(( "%s too long stay in stn<%02d>\n", "STNHolder::Process",
		    stnNo ));
    }
    if ( bError ) {
	if ( stnNo == StnNo_AllStopped ) {
	    OSYSPRINT(( "%s can't start emergency from stn<%02d>\n",
			"STNHolder::Process", stnNo ));
	    return false;
	}
	OSYSPRINT(( "%s start emergency stn\n", "STNHolder::Process" ));
	if ( !ColdStart( StnNo_AllStopped ) ) {
	    return false;
	}
	stnNo = currentStnNo;
	if ( stnNo == InvalidStnNo ) {
	    OSYSPRINT(( "%s can't start emergency stn\n",
			"STNHolder::Process" ));
	    return false;
	}
    }
    // 現在時刻の更新
    perception->UpdateNowMSec( nowMSec );
    STNProcess& process = processes[ stnNo ];
    if ( verboseMode >= 9 ) {
	process.Print();
    }
    STNState *resultState = NULL;
    if ( !process.Execute( cmds, &resultState ) ) {
	if ( resultState->isJumpState() ) {
	    // ジャンプ状態
	    int newStnNo = resultState->GetStnNo();
	    if ( verboseMode >= 8 ) {
		OSYSPRINT(( "STNHolder::Process jump to stn<%02d>\n",
			    newStnNo ));
	    }
	    JumpStart( newStnNo );
	} else if ( resultState->isReturnState() ) {
	    // 復帰状態
	    int retNo = resultState->GetStnNo();
	    if ( verboseMode >= 8 ) {
		OSYSPRINT(( "STNHolder::Process return with %d\n", retNo ));
	    }
	    if ( !Return( retNo ) ) {
		return false;
	    }
	}
    }
    return true;
}

bool
STNHolder::GetCurrentInfo( STNInfo *stnInfo ) const
{
    stnInfo->stnNo = currentStnNo;
    stnInfo->timer1Sec = (nowMSec - timer1MSec) / 1000;
    stnInfo->timer2Sec = (nowMSec - timer2MSec) / 1000;
    stnInfo->counter1 = counter1;
    stnInfo->counter2 = counter2;
    stnInfo->counter3 = counter3;
    if ( currentStnNo == InvalidStnNo ) {
	return false;
    }
    processes[ currentStnNo ].GetCurrentInfo( stnInfo );
    return true;
}

void
STNHolder::InternalExecute( int iCmd )
{
    if ( iCmd == icmdResetTimer1 ) {
	SetTimer1MSec( nowMSec );
    } else if ( iCmd == icmdResetTimer2 ) {
	SetTimer2MSec( nowMSec );
    } else if ( iCmd == icmdClearReturnNo ) {
	returnNo = DefaultReturnNo;
    } else if ( iCmd == icmdClearMessageNo ) {
	messageNo = DefaultMessageNo;
    } else if ( iCmd == icmdCounter1Reset ) {
	OSYSPRINT(("STNHolder::InternalExecute counter1 =0\n"));
	counter1 = 0;
    } else if ( iCmd == icmdCounter1Inc ) {
	if ( counter1 < MAX_COUNTER ) {
	    counter1 ++;
	} else {
	    counter1 = MAX_COUNTER;
	}
	OSYSPRINT(("STNHolder::InternalExecute counter1 ++(%d)\n", counter1));
    } else if ( iCmd == icmdCounter1Dec ) {
	OSYSPRINT(("STNHolder::InternalExecute counter1 --(%d)\n", counter1));
	if ( counter1 > 0 ) {
	    counter1 --;
	} else {
	    counter1 = 0;
	}
    } else if ( iCmd == icmdCounter2Reset ) {
	OSYSPRINT(("STNHolder::InternalExecute counter2 =0\n"));
	counter2 = 0;
    } else if ( iCmd == icmdCounter2Inc ) {
	OSYSPRINT(("STNHolder::InternalExecute counter2 ++(%d)\n", counter1));
	if ( counter2 < MAX_COUNTER ) {
	    counter2 ++;
	} else {
	    counter2 = MAX_COUNTER;
	}
    } else if ( iCmd == icmdCounter2Dec ) {
	OSYSPRINT(("STNHolder::InternalExecute counter2 --(%d)\n", counter1));
	if ( counter2 > 0 ) {
	    counter2 --;
	} else {
	    counter2 = 0;
	}
    } else if ( iCmd == icmdCounter3Reset ) {
	OSYSPRINT(("STNHolder::InternalExecute counter3 =0\n"));
	counter3 = 0;
    } else if ( iCmd == icmdCounter3Inc ) {
	OSYSPRINT(("STNHolder::InternalExecute counter3 ++(%d)\n", counter1));
	if ( counter3 < MAX_COUNTER ) {
	    counter3 ++;
	} else {
	    counter3 = MAX_COUNTER;
	}
    } else if ( iCmd == icmdCounter3Dec ) {
	OSYSPRINT(("STNHolder::InternalExecute counter3 --(%d)\n", counter1));
	if ( counter3 > 0 ) {
	    counter3 --;
	} else {
	    counter3 = 0;
	}
    } else if ( iCmd == icmdStartLog ) {
	brain->logging.Start();
    } else if ( iCmd == icmdEndLog ) {
	brain->logging.End();
    } else if ( iCmd == icmdResetLog ) {
	brain->logging.Reset();
    }
}

bool
STNHolder::IsExecutable( int stnNo )
{
    return CheckValidStnNo( stnNo ) && stns[ stnNo ].IsExecutable();
}

void
STNHolder::PushStnNo( int stnNo )
{
    if ( stnNo == InvalidStnNo ) {
	return;
    } else if ( nStackStnNo >= MAX_STACK_STNNO ) {
      for ( int i = 1; i < nStackStnNo; i ++ ) {
	stackStnNo[ i - 1 ] = stackStnNo[ i ];
      }
      nStackStnNo = MAX_STACK_STNNO - 1;
    }
    stackStnNo[ nStackStnNo ++ ] = stnNo;
}

int
STNHolder::PopStnNo()
{
    if ( nStackStnNo <= 0 ) {
	return InvalidStnNo;
    }
    return stackStnNo[ -- nStackStnNo ];
}

void
STNHolder::ClearStack()
{
    nStackStnNo = 0;
}

void
STNHolder::SetTimer1MSec( int msec )
{
    timer1MSec = msec;
}

int
STNHolder::GetTimer1MSec() const
{
    return timer1MSec;
}

void
STNHolder::SetTimer2MSec( int msec )
{
    timer2MSec = msec;
}

int
STNHolder::GetTimer2MSec() const
{
    return timer2MSec;
}

bool
STNHolder::IsOwnKickoff() const
{
    return bOwnKickoff;
}

bool
STNHolder::IsRedTeamColor() const
{
    return bRedTeamColor;
}

bool
STNHolder::IsWinner() const
{
    return score == SCORE_WIN;
}

bool
STNHolder::IsLoser() const
{
    return score == SCORE_LOSE;
}

bool
STNHolder::IsDrawn() const
{
    return score == SCORE_DRAW;
}

int
STNHolder::GetNowMSec() const
{
    return nowMSec;
}

int
STNHolder::GetReturnNo() const
{
    return returnNo;
}

int
STNHolder::GetMessageNo() const
{
    return messageNo;
}

Brain*
STNHolder::GetBrain() const
{
    return brain;
}

Perception*
STNHolder::GetPerception() const
{
    return perception;
}

Sensor*
STNHolder::GetSensor() const
{
    return sensor;
}

void
STNHolder::UpdateLastMovedMSec()
{
    lastMovedMSec = nowMSec;
}

int
STNHolder::GetCounterValue( int counterNo ) const
{
    if ( counterNo == 1 ) {
	return counter1;
    } else if ( counterNo == 2 ) {
	return counter2;
    } else if ( counterNo == 3 ) {
	return counter3;
    }
    return 0;
}

int
STNHolder::SetWatchDogSec( int sec )
{
    watchDogSec = sec;
}

// --------------------------------------------------------------------------


STNProcess::STNProcess()
{
    nTasks = 0;
    currentTask = 0;
}

void
STNProcess::Setup( STNHolder *_holder, STN *_stn )
{
    holder = _holder;
    stn = _stn;
    nTasks = 0;
    currentTask = 0;
    list<STNState*> stateList = stn->GetStateList();
    list<STNState*>::iterator i;
    for ( i = stateList.begin(); i != stateList.end(); i ++ ) {
	if ( (*i)->isInitial() && nTasks < MAX_TASKS ) {
	    currentStateIds[ nTasks ++ ] = (*i)->GetStateId();
	}
    }
}

void
STNProcess::Print() const
{
    OSYSPRINT(( "%d tasks, current=%d, stateIds=[ ",
		nTasks, currentTask ));
    for ( int i = 0; i < nTasks; i ++ ) {
	OSYSPRINT(( "%d ", currentStateIds[ i ] ));
    }
    OSYSPRINT(( "]\n" ));
}

// nTasks個のタスクを処理する
// ジャンプ状態、復帰状態で中断
bool
STNProcess::Execute( STNCmds *cmds, STNState **result )
{
    cmds->monetCmd = monetcommandID_UNDEF;
    cmds->headCmd = hcmdNone;
    cmds->faceCmd = fcmdNone;
    cmds->internalCmd = icmdNone;
    for ( int i = 0; i < nTasks; i ++ ) {
	currentTask = (currentTask + 1) % nTasks;
	int currentStateId = currentStateIds[ currentTask ];
	if ( currentStateId == BottomStateId ) {
	    continue;
	}
	// 実行すべき状態を取得
	STNState* state = stn->GetState( currentStateId );
	if ( state == NULL ) {
	    currentStateIds[ currentTask ] = BottomStateId;
	    continue;
	}
	if ( verboseMode >= 9 ) {
	    OSYSPRINT(("[ State=%d ", state->GetStateId() ));
	}
	// アクションの中から条件に適合するものを見付ける
	list<STNArc*>& arcList = state->GetArcList();
	STNArc* firedArc = GetFiredArc( arcList );
	if ( verboseMode >= 9 ) {
	    OSYSPRINT(( " ]\n" ));
	}
	if ( !firedArc ) {
	    // 適合するものがない場合
	    continue;
	}
	holder->UpdateLastMovedMSec();
	// 適合したアクションを実行する
	int monetCmd = firedArc->GetMonetCmd();
	int headCmd = firedArc->GetHeadCmd();
	int faceCmd = firedArc->GetFaceCmd();
	int internalCmd = firedArc->GetInternalCmd();
	int nextStateId = firedArc->GetNextStateId();
	if ( verboseMode >= 9 ) {
	    if ( !firedArc->IsDefaultCondition() ) {
		firedArc->PrintCondition();
	    }
	    if ( currentStateId != nextStateId ) {
		OSYSPRINT(( "[ " ));
		OSYSPRINT(( "NextState=%d ", nextStateId ));
		if ( monetCmd >= 0 ) {
		    OSYSPRINT(( "MoNet=%d ", monetCmd ));
		}
		OSYSPRINT(( "Head=%d ", headCmd ));
		OSYSPRINT(( "Face=%d ", faceCmd ));
		OSYSPRINT(( "Internal=%d ", internalCmd ));
		OSYSPRINT(( "]\n" ));
	    }
	}
	if ( monetCmd != monetcommandID_UNDEF ) {
	    cmds->monetCmd = monetCmd;
	}
	if ( headCmd != hcmdNone ) {
	    cmds->headCmd = ConvertHeadCmd(headCmd);
	}
	if ( faceCmd != fcmdNone ) {
	    cmds->faceCmd = faceCmd;
	}
	if ( internalCmd != icmdNone ) {
	    cmds->internalCmd = internalCmd;
	    holder->InternalExecute( internalCmd );
	}
	currentStateIds[ currentTask ] = nextStateId;
	STNState* newState = stn->GetState( nextStateId );
	if ( newState &&
	     (newState->isJumpState() || newState->isReturnState()) ) {
	    *result = newState;
	    return false;
	}
    }
    return true;
}

STNArc*
STNProcess::GetFiredArc( list<STNArc*>& arcList )
{
    list<STNArc*>::iterator i ;

    for ( i = arcList.begin(); i != arcList.end(); i ++ ) {
	if ( CheckCondition( (*i)->GetCondition1() )
	     && CheckCondition( (*i)->GetCondition2() ) ) {
	    return (*i);
	}
    }
    return NULL;
}

EAiboIdentifiedObject
visionIDtoAiboID( enum EAiboVisionRangeObject _id, bool bRedTeam )
{
    return bRedTeam ? RedTeamIds[_id] : BlueTeamIds[_id];
}

bool
STNProcess::CheckCondition( const STNCondition& cond )
{
    Perception *perception = holder->GetPerception();
    Sensor *sensor = holder->GetSensor();
    int conditionId = cond.GetConditionId();
    int conditionCode = cond.GetConditionCode();
    bool bRedTeamColor = holder->IsRedTeamColor();

    if ( conditionId == condVision ) {
	if ( CheckVision( (EAiboVisionRangeObject) conditionCode, perception, bRedTeamColor ) ) {
	    return true;
	}
    } else if ( conditionId == condVisionRange ) {
	EAiboIdentifiedObject id = visionIDtoAiboID( (EAiboVisionRangeObject) conditionCode, bRedTeamColor );
	if ( perception->VisionRangeCondition(
		 id,
		 cond.GetConditionArg(argIndexOfVisionAngle1),
		 cond.GetConditionArg(argIndexOfVisionAngle2),
		 cond.GetConditionArg(argIndexOfVisionDistance1),
		 cond.GetConditionArg(argIndexOfVisionDistance2) ) ) {
	    return true;
	}
    } else if ( conditionId == condVisionNearRange ) {
	EAiboIdentifiedObject id = visionIDtoAiboID( (EAiboVisionRangeObject) conditionCode, bRedTeamColor );
	if ( perception->VisionNearRangeCondition(
		 id,
		 (int) cond.GetConditionArg(argIndexOfVisionNearX1),
		 (int) cond.GetConditionArg(argIndexOfVisionNearX2),
		 (int) cond.GetConditionArg(argIndexOfVisionNearY1),
		 (int) cond.GetConditionArg(argIndexOfVisionNearY2) ) ) {
	    return true;
	}
    } else if ( conditionId == condMemoryRange ) {
	EAiboIdentifiedObject id = visionIDtoAiboID( (EAiboVisionRangeObject) conditionCode, bRedTeamColor );
	Brain *brain = holder->GetBrain();
	if ( brain->memory.CheckRange(
		 id,
		 (int) cond.GetConditionArg(argIndexOfVisionNearX1),
		 (int) cond.GetConditionArg(argIndexOfVisionNearX2),
		 (int) cond.GetConditionArg(argIndexOfVisionNearY1),
		 (int) cond.GetConditionArg(argIndexOfVisionNearY2) ) ) {
	    return true;
	}
    } else if ( conditionId == condSensor ) {
	if ( sensor->IsPressed( conditionCode ) ) {
	    return true;
	}
    } else if ( conditionId == condTimer1 ) {
	if ( CheckTimer( holder->GetTimer1MSec(), conditionCode ) ) {
	    return true;
	}
    } else if ( conditionId == condTimer2 ) {
	if ( CheckTimer( holder->GetTimer2MSec(), conditionCode ) ) {
	    return true;
	}
    } else if ( conditionId == condReturn ) {
	if ( CheckReturn( conditionCode ) ) {
	    return true;
	}
    } else if ( conditionId == condMisc ) {
	if ( CheckMisc( conditionCode, perception ) ) {
	    return true;
	}
    } else if ( conditionId == condCounter1 ) {
	if ( CheckCounter( 1, conditionCode ) ) {
	    return true;
	}
    } else if ( conditionId == condCounter2 ) {
	if ( CheckCounter( 2, conditionCode ) ) {
	    return true;
	}
    } else if ( conditionId == condCounter3 ) {
	if ( CheckCounter( 3, conditionCode ) ) {
	    return true;
	}
    } else {
	return true;
    }
    return false;
}

bool
STNProcess::CheckVision( int code, const Perception *perception,
		    bool bRedTeam ) const
{
    EAiboIdentifiedObject id = IO_UNIDENTIFIED;
    if ( code == vicodeBallLost ) {
	id = IO_BALL;
    } else if ( code == vicodeMikataGoalLost ) {
	id = bRedTeam ? RedTeamIds[VR_GOAL_M] : BlueTeamIds[VR_GOAL_M];
    } else if ( code == vicodeLeftBeaconLost ) {
	id = bRedTeam ? RedTeamIds[VR_BEACON_L] : BlueTeamIds[VR_BEACON_L];
    } else if ( code == vicodeRightBeaconLost ) {
	id = bRedTeam ? RedTeamIds[VR_BEACON_R] : BlueTeamIds[VR_BEACON_R];
    } else if ( code == vicodeTekiGoalLost ) {
	id = bRedTeam ? RedTeamIds[VR_GOAL_T] : BlueTeamIds[VR_GOAL_T];
    } else {
	return false;
    }
    return perception->LostCondition( id );
}

bool
STNProcess::CheckTimer( int timerMSec, int conditionCode ) const
{
    int elapsedMSec = holder->GetNowMSec() - timerMSec;

    if ( elapsedMSec > conditionCode * 100 ) {
	return true;
    }
    return false;
}

bool
STNProcess::CheckMisc( int conditionCode, Perception *perception )
{
    Brain *brain = holder->GetBrain();

    if ( conditionCode == micodeCompleteHeading ) {
	if ( perception->GetCdtInfoSeqno() >= brain->GetHcmdSeqno() ) {
	    return true;
	}
    } else if ( conditionCode == micodeKickoff ) {
	if ( holder->IsOwnKickoff() ) {
	    return true;
	}
    } else if ( conditionCode == micodeWin ) {
	return holder->IsWinner();
    } else if ( conditionCode == micodeLose ) {
	return holder->IsLoser();
    } else if ( conditionCode == micodeDraw ) {
	return holder->IsDrawn();
    }
    return false;
}

bool
STNProcess::CheckReturn( int conditionCode ) const
{
    int returnNo = holder->GetReturnNo();
    if ( conditionCode == rcodeNone && returnNo == DefaultReturnNo ) {
	return true;
    } else if ( conditionCode == rcodeNumber0 && returnNo == 0 ) {
	return true;
    } else if ( conditionCode == rcodeNumber1 && returnNo == 1 ) {
	return true;
    } else if ( conditionCode == rcodeNumber2 && returnNo == 2 ) {
	return true;
    } else if ( conditionCode == rcodeNumber3 && returnNo == 3 ) {
	return true;
    } else if ( conditionCode == rcodeNumber4 && returnNo == 4 ) {
	return true;
    } else if ( conditionCode == rcodeNumber5 && returnNo == 5 ) {
	return true;
    } else if ( conditionCode == rcodeNumber6 && returnNo == 6 ) {
	return true;
    } else if ( conditionCode == rcodeNumber7 && returnNo == 7 ) {
	return true;
    } else if ( conditionCode == rcodeNumber8 && returnNo == 8 ) {
	return true;
    } else if ( conditionCode == rcodeNumber9 && returnNo == 9 ) {
	return true;
    }
    return false;
}

bool
STNProcess::CheckCounter( int counterNo, int conditionCode ) const
{
    int counterValue = holder->GetCounterValue( counterNo );
    int op = condCounterOP( conditionCode );
    int arg = condCounterARG( conditionCode );

    if ( op == counterOpEqual ) {
	if ( verboseMode >= 9 && counterValue == arg ) {
	    OSYSPRINT(("%d == %d, %d-%d\n", counterValue, arg,
		       counterNo, conditionCode));
	}
	return counterValue == arg;
    } else if ( op == counterOpLeq ) {
	if ( verboseMode >= 9 && counterValue <= arg ) {
	    OSYSPRINT(("%d <= %d, %d-%d\n", counterValue, arg,
		       counterNo, conditionCode));
	}
	return counterValue <= arg;
    } else if ( op == counterOpGeq ) {
	if ( verboseMode >= 9 && counterValue >= arg ) {
	    OSYSPRINT(("%d >= %d, %d-%d\n", counterValue, arg,
		       counterNo, conditionCode));
	}
	return counterValue >= arg;
    }
    return false;
}

//               PinkBlue(味方左)   PinkYellow(敵左)
//BlueGoal(味方)                                    YellowGoal(敵)
//               BluePink(味方右)   YellowPink(敵右)
int
STNProcess::ConvertHeadCmd( int hcmd ) const
{
    bool bRedTeamColor = holder->IsRedTeamColor();
    if ( bRedTeamColor ) {
	if ( hcmd == hcmdTrackMikataGoal ) {
	    hcmd = hcmdTrackGoalY;
	} else if ( hcmd == hcmdTrackTekiGoal ) {
	    hcmd = hcmdTrackGoalB;
	} else if ( hcmd == hcmdSearchMikataGoal ) {
	    hcmd = hcmdSearchGoalY;
	} else if ( hcmd == hcmdSearchTekiGoal ) {
	    hcmd = hcmdSearchGoalB;
	} else if ( hcmd == hcmdTrackRightBeacon ) {
	    hcmd = hcmdTrackBeaconYB;
	} else if ( hcmd == hcmdTrackLeftBeacon ) {
	    hcmd = hcmdTrackBeaconBY;
	} else if ( hcmd == hcmdSearchRightBeacon ) {
	    hcmd = hcmdSearchBeaconYB;
	} else if ( hcmd == hcmdSearchLeftBeacon ) {
	    hcmd = hcmdSearchBeaconBY;
	}
    }
    return hcmd;
}

void
STNProcess::GetCurrentInfo( STNInfo *stnInfo ) const
{
    int i;

    for ( i = 0; i < nTasks; i += 1 ) {
	stnInfo->stateIds[ i ] = currentStateIds[ i ];
    }
    for ( ; i < MAX_TASKS; i += 1 ) {
	stnInfo->stateIds[ i ] = 0;
    }
}
