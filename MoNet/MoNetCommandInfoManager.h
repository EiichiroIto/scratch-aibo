//
// Copyright 2002 Sony Corporation 
//
// Permission to use, copy, modify, and redistribute this software for
// non-commercial use is hereby granted.
//
// This software is provided "as is" without warranty of any kind,
// either expressed or implied, including but not limited to the
// implied warranties of fitness for a particular purpose.
//

#ifndef MoNetCommandInfoManager_h
#define MoNetCommandInfoManager_h

#include <map>
#include <list>
using namespace std;
#include "MoNetCommandInfo.h"

class MoNetCommandInfoManager {
public:
    MoNetCommandInfoManager();
    ~MoNetCommandInfoManager();

    MoNetCommandInfo* New();
    MoNetCommandInfo* New(MoNetCommandID commandID, bool useSyncKey);
    MoNetCommandInfo* Find(MoNetCommandID commandID);

private:
    map<MoNetCommandID, MoNetCommandInfo*, less<MoNetCommandID> > infoMap;
    list<MoNetCommandInfo*> infoList; // without MoNetCommandID
};

#endif // MoNetCommandInfoManager_h
