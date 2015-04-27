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

#include "MoNetCommandInfoManager.h"

MoNetCommandInfoManager::MoNetCommandInfoManager() : infoMap(), infoList()
{
}

MoNetCommandInfoManager::~MoNetCommandInfoManager()
{
}

MoNetCommandInfo* 
MoNetCommandInfoManager::New()
{
    MoNetCommandInfo* info = new MoNetCommandInfo(false);
    infoList.push_back(info);
    return info;
}

MoNetCommandInfo* 
MoNetCommandInfoManager::New(MoNetCommandID commandID, bool useSyncKey)
{
    MoNetCommandInfo* info = new MoNetCommandInfo(useSyncKey);
    infoMap[commandID] = info;
    return info;
}

MoNetCommandInfo*
MoNetCommandInfoManager::Find(MoNetCommandID commandID)
{
    map<MoNetCommandID,
        MoNetCommandInfo*, less<MoNetCommandID> >::iterator iter;

    iter = infoMap.find(commandID);
    if (iter != infoMap.end()) {
        return (*iter).second;
    } else {
        return 0;
    }
}

