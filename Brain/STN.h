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

#ifndef STN_h_DEFINED
#define STN_h_DEFINED
#include <list>
using std::list;
#include <unistd.h>

const int BottomStateId = 0;
const int condDefault = 0;
#define StatePrefix "*"
const int StatePrefixLen = 1;
const int NumConditionArgs = 4;

class STNArc;
class STNState;
class STN;

class STNCondition {
  private:
    int conditionId;
    int conditionCode;
    double conditionArgs[ NumConditionArgs ];

  public:
    STNCondition();
    ~STNCondition();
    int GetConditionId() const;
    int GetConditionCode() const;
    double GetConditionArg( int index ) const;
    void SetCondition( int id, int code, const double *args );
    bool IsDefaultCondition() const;
    void Print() const;
};

class STNArc {
  private:
    int nextStateId;
    STNCondition cond1, cond2;
    int monetCmd;
    int headCmd;
    int faceCmd;
    int internalCmd;
    int probability;
    int priority;

  public:
    STNArc();
    ~STNArc();
    int GetNextStateId() const;
    int GetMonetCmd() const;
    int GetHeadCmd() const;
    int GetFaceCmd() const;
    int GetInternalCmd() const;
    const STNCondition& GetCondition1() const;
    const STNCondition& GetCondition2() const;
    bool IsDefaultCondition() const;
    bool FromString( const char *string, STN *stn );
    void PrintCondition() const;
    void Print() const;
};

enum StateType {
    NormalState = 0, JumpState, ReturnState
};

class STNState {
  private:
    int stateId;
    StateType stateType;
    int stnNo;
    list<STNArc*> arcList;
    bool bInitial;

  public:
    STNState( int _stateId );
    STNState( const char *buf );
    ~STNState();
    int GetStateId() const;
    bool isJumpState() const;
    bool isReturnState() const;
    bool isInitial() const;
    void beInitial();
    int GetStnNo() const;
    void Print();
    void Add( STNArc* newArc );
    list<STNArc*>& GetArcList();
    void FromString( const char *string );
};

class STN {
  private:
    list<STNState*> stateList;

  private:
    void DeleteStateList();
    void Initialize();
    STNState* Add( STNState* newState );

  public:
    STN();
    ~STN();
    void Print();
    void Clear();
    STNState* Add( int stateId );
    void FromString( const char* path );
    bool Read( const char *path );
    bool IsEmpty() const;
    bool IsExecutable();
    list<STNState*> GetStateList();
    STNState* GetState( int stateId );
};

#endif // STN_h_DEFINED
