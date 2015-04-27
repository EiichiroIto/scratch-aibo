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

#ifndef CommandNode_h_DEFINED
#define CommandNode_h_DEFINED

#include <stdio.h>
#include <MoNetData.h>

class CommandNode {
public:
    CommandNode(MoNetPosture pos) : posture(pos) {}
    ~CommandNode() {}
    
    friend bool operator< (const CommandNode& lhs, const CommandNode& rhs) {
        return (lhs.posture < rhs.posture) ? true : false;
    }
    friend bool operator== (const CommandNode& lhs, const CommandNode& rhs) {
        return (lhs.posture == rhs.posture) ? true : false;        
    }

    void Print() const { printf("%d", posture); }

private:
    MoNetPosture posture;
};

#endif // CommandNode_h_DEFINED
