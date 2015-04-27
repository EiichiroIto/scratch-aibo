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

#include <stdio.h>

class TestNode {
public:
    TestNode(int i) : node(i) {}
    ~TestNode() {}

    friend bool operator< (const TestNode& lhs, const TestNode& rhs) {
        return (lhs.node < rhs.node) ? true : false;
    }
    friend bool operator== (const TestNode& lhs, const TestNode& rhs) {
        return (lhs.node == rhs.node) ? true : false;        
    }

    void Print() const { printf("%d", node); }

private:
    int node;
};
