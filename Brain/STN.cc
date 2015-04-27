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
// Eiichiro ITO, 12 August 2003 (original)
// Eiichiro ITO, 30 August 2005 (modified)
// mailto: GHC02331@nifty.com
#include "STN.h"
#ifndef MAIN
#include <OPENR/OObject.h>
#include "command.h"
#else
#define OSYSPRINT(x) printf x
#endif // MAIN

STNCondition::STNCondition() : conditionId( 0 ), conditionCode( 0 )
{
    for ( int i = 0; i < NumConditionArgs; i ++ ) {
	conditionArgs[ i ] = 0.0;
    }
}

STNCondition::~STNCondition()
{
    conditionId = -1;
}

int
STNCondition::GetConditionId() const
{
    return conditionId;
}

int
STNCondition::GetConditionCode() const
{
    return conditionCode;
}

double
STNCondition::GetConditionArg( int index ) const
{
    if ( index >= NumConditionArgs ) {
	return 0.0;
    }
    return conditionArgs[ index ];
}

bool
STNCondition::IsDefaultCondition() const
{
    return conditionId == condDefault;
}

void
STNCondition::SetCondition( int id, int code, const double *args )
{
    conditionId = id;
    conditionCode = code;
    for ( int i = 0; i < NumConditionArgs; i ++ ) {
	conditionArgs[ i ] = args[ i ];
    }
}

void
STNCondition::Print() const
{
    OSYSPRINT(( "  when %d(%d %3.1lf %3.1lf %3.1lf %3.1lf)\n",
		conditionId, conditionCode,
		conditionArgs[0], conditionArgs[1],
		conditionArgs[2], conditionArgs[3] ));
}

//----------------------------------------------------------------------

STNArc::STNArc() : nextStateId( BottomStateId ),
		   monetCmd(0), headCmd(0), faceCmd(0), internalCmd(0),
		   probability(100), priority(5)
{
#ifdef MAIN
    printf( "+Arc\n" );
#endif // MAIN
}

STNArc::~STNArc()
{
#ifdef MAIN
    printf( "-Arc\n" );
#endif // MAIN
}

int
STNArc::GetNextStateId() const
{
    return nextStateId;
}

int
STNArc::GetMonetCmd() const
{
    return monetCmd;
}

int
STNArc::GetHeadCmd() const
{
    return headCmd;
}

int
STNArc::GetFaceCmd() const
{
    return faceCmd;
}

int
STNArc::GetInternalCmd() const
{
    return internalCmd;
}

const STNCondition&
STNArc::GetCondition1() const
{
    return cond1;
}

const STNCondition&
STNArc::GetCondition2() const
{
    return cond2;
}

bool
STNArc::IsDefaultCondition() const
{
    return cond1.IsDefaultCondition() && cond2.IsDefaultCondition();
}

static int
nextInteger( const char **ptr )
{
    const char *buf = *ptr;
    int ret = atoi( buf );
    buf = strchr( buf, ' ' );
    if ( buf ) {
	buf ++;
    }
    *ptr = buf;
    return ret;
}

static double
nextDouble( const char **ptr )
{
    const char *buf = *ptr;
    double ret = atof( buf );
    buf = strchr( buf, ' ' );
    if ( buf ) {
	buf ++;
    }
    *ptr = buf;
    return ret;
}

bool
STNArc::FromString( const char *line, STN* stn )
{
    const char *org = line;
    int ret, stateId;
    int conditionId1, conditionCode1, conditionId2, conditionCode2;
    double args1[ NumConditionArgs ], args2[ NumConditionArgs ];

    stateId = nextInteger( &line );
    if ( !line ) {
	return false;
    }
    nextStateId = nextInteger( &line );
    if ( !line ) {
	return false;
    }
    conditionId1 = nextInteger( &line );
    if ( !line ) {
	return false;
    }
    conditionCode1 = nextInteger( &line );
    if ( !line ) {
	return false;
    }
    args1[0] = nextDouble( &line );
    if ( !line ) {
	return false;
    }
    args1[1] = nextDouble( &line );
    if ( !line ) {
	return false;
    }
    args1[2] = nextDouble( &line );
    if ( !line ) {
	return false;
    }
    args1[3] = nextDouble( &line );
    if ( !line ) {
	return false;
    }
    conditionId2 = nextInteger( &line );
    if ( !line ) {
	return false;
    }
    conditionCode2 = nextInteger( &line );
    if ( !line ) {
	return false;
    }
    args2[0] = nextDouble( &line );
    if ( !line ) {
	return false;
    }
    args2[1] = nextDouble( &line );
    if ( !line ) {
	return false;
    }
    args2[2] = nextDouble( &line );
    if ( !line ) {
	return false;
    }
    args2[3] = nextDouble( &line );
    if ( !line ) {
	return false;
    }
    monetCmd = nextInteger( &line );
    if ( !line ) {
	return false;
    }
    headCmd = nextInteger( &line );
    if ( !line ) {
	return false;
    }
    faceCmd = nextInteger( &line );
    if ( !line ) {
	return false;
    }
    internalCmd = nextInteger( &line );
    if ( !line ) {
	return false;
    }
    priority = nextInteger( &line );
    if ( !line ) {
	return false;
    }
    probability = nextInteger( &line );
    //
    cond1.SetCondition( conditionId1, conditionCode1, args1 );
    cond2.SetCondition( conditionId2, conditionCode2, args2 );
    STNState* fromState = stn->Add( stateId );
    STNState* toState = stn->Add( nextStateId );
    fromState->Add( this );
    return true;
}

void
STNArc::PrintCondition() const
{
    cond1.Print();
    cond2.Print();
}

void
STNArc::Print() const
{
    OSYSPRINT(( " action N%d M%d H%d F%d I%d PB%d PO%d\n",
		nextStateId,
		monetCmd, headCmd, faceCmd, internalCmd,
		probability, priority ));
    cond1.Print();
    cond2.Print();
}

//----------------------------------------
STNState::STNState( int _stateId ) : stateId( _stateId ),
				     stateType( NormalState ),
				     stnNo( 0 ), bInitial( false )
{
#ifdef MAIN
    printf( "+State(%d,%d,%d)\n", stateId, stateType, stnNo );
#endif // MAIN
}

STNState::STNState( const char *buf )
{
    FromString( buf );
#ifdef MAIN
    printf( "+State(%d,%d,%d)\n", stateId, stateType, stnNo );
#endif // MAIN
}

STNState::~STNState()
{
#ifdef MAIN
    printf( "-State(%d,%d,%d)\n", stateId, stateType, stnNo );
#endif // MAIN
    stateId = -1;
    list<STNArc*>::iterator i ;
    for ( i = arcList.begin(); i != arcList.end(); i ++ ) {
        delete *i;
    }
}

int
STNState::GetStateId() const
{
    return stateId;
}

bool
STNState::isJumpState() const
{
    return stateType == JumpState;
}

bool
STNState::isReturnState() const
{
    return stateType == ReturnState;
}

bool
STNState::isInitial() const
{
    return bInitial;
}

void
STNState::beInitial()
{
    bInitial = true;
}

int
STNState::GetStnNo() const
{
    return stnNo;
}

void
STNState::Print()
{
    OSYSPRINT(( "state %02d(%02d)%c\n",
		stateId, stnNo, bInitial ? '+' : ' ' ));
    list<STNArc*>::iterator i ;
    for ( i = arcList.begin(); i != arcList.end(); i ++ ) {
        (*i)->Print();
    }
}

void
STNState::Add( STNArc *newArc )
{
    arcList.push_back( newArc );
}

list<STNArc*>&
STNState::GetArcList()
{
    return arcList;
}

void
STNState::FromString( const char *buf )
{
    char c;
    int ret, arg1;

    // prefix stateId stateType arg1 # geometry comments
    ret = sscanf( buf, "%c%d %d %d", &c, &stateId, &stateType, &arg1 );
    if ( ret != 4 ) {
	OSYSPRINT(( "STNState::FromString invalid # of Args(%d != %d)\n",
		    ret, 4 ));
    }
    bInitial = c == '+';
    stnNo = arg1;
}

//----------------------------------------

STN::STN()
{
    Initialize();
}

STN::~STN()
{
    DeleteStateList();
}

void
STN::Initialize()
{
    STNState* state = new STNState( BottomStateId );
    state->Add( new STNArc() );
    stateList.push_back( state );
}

void
STN::DeleteStateList()
{
    list<STNState*>::iterator i ;
    for ( i = stateList.begin(); i != stateList.end(); i ++ ) {
        delete *i;
    }
    stateList.clear();
}

void
STN::Clear()
{
    DeleteStateList();
    Initialize();
}

STNState*
STN::Add( STNState *newState )
{
    stateList.push_back( newState );
    return newState;
}

STNState*
STN::Add( int stateId )
{
    list<STNState*>::iterator i;

    for ( i = stateList.begin(); i != stateList.end(); i ++ ) {
	if ( stateId == (*i)->GetStateId() ) {
	    return *i;
	}
    }
    return Add( new STNState( stateId ) );
}

void
STN::FromString( const char *buf )
{
    int line = 1;

    while ( buf && *buf ) {
	if ( !memcmp( buf, StatePrefix, StatePrefixLen ) ) {
	    Add( new STNState( &buf[ StatePrefixLen ] ) );
	} else if ( *buf != '#' && *buf >= ' ' ) {
	    STNArc *newArc = new STNArc();
	    if ( !newArc->FromString( buf, this ) ) {
		delete newArc;
		OSYSPRINT(( "STN::FromString error in (%d)\n", line ));
	    }
	}
	buf = strchr( buf, '\n' );
	if ( buf ) {
	    buf ++;
	}
	line ++;
    }
}

bool
STN::Read( const char *path )
{
    FILE* fp = fopen( path, "r" );
    if ( fp == NULL ) {
        return false;
    }
    OSYSPRINT(( "STN::Read path=%s.\n", path ));

    fseek( fp, 0L, SEEK_END );
    int size = ftell( fp );
    fseek( fp, 0L, SEEK_SET );
    char *filebuf = new char[ size + 2 ];
    fread( filebuf, size, 1, fp );
    filebuf[ size ] = '\n';
    filebuf[ size + 1 ] = 0;
    fclose( fp );

    Clear();
    FromString( filebuf );
    delete[] filebuf;

    if ( !IsExecutable() ) {
	list<STNState*>::iterator i;

	for ( i = stateList.begin(); i != stateList.end(); i ++ ) {
	    if ( 1 == (*i)->GetStateId() ) {
		(*i)->beInitial();
		break;
	    }
	}
    }

    //Print();
    return true;
}

bool
STN::IsEmpty() const
{
    return stateList.size() <= 1;
}

void
STN::Print()
{
    list<STNState*>::iterator i;

    for ( i = stateList.begin(); i != stateList.end(); i ++ ) {
	(*i)->Print();
	OSYSPRINT(( "\n" ));
    }
}

bool
STN::IsExecutable()
{
    list<STNState*>::iterator i;

    for ( i = stateList.begin(); i != stateList.end(); i ++ ) {
	if ( (*i)->isInitial() ) {
	    return true;
	}
    }
    return false;
}

list<STNState*>
STN::GetStateList()
{
    return stateList;
}

STNState*
STN::GetState( int stateId )
{
    list<STNState*>::iterator i;

    for ( i = stateList.begin(); i != stateList.end(); i ++ ) {
	if ( stateId == (*i)->GetStateId() ) {
	    return *i;
	}
    }
    return NULL;
}

#ifdef MAIN
void
test1( STN& stn )
{
    printf( "test1 start\n" );
    stn.FromString( "#*-! 5 1 2" );
    stn.FromString( "1 11 1 1 1 0 2\n" );
    printf( "test1 end\n" );
}

void
test2( STN& stn )
{
    printf( "test2 start\n" );
    stn.FromString( "5 55 5 5 5 0 5" );
    printf( "test2 end\n" );
}

main()
{
    STN stn;
    test1( stn );
    test2( stn );
    stn.FromString( "2 22 2 2 2 0 4\n" );
    stn.FromString( "4 5 4 4 4 0 -1\n" );
    printf( "---\n" );
    stn.Print();
    stn.Clear();
    printf( "---\n" );

    stn.Read( "STN.CFG" );
}
#endif // MAIN
