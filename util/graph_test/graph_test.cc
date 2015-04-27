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

#include <DirectedGraph.h>
#include "TestNode.h"
#include "TestArc.h"

const int NT    = 0;
const int STAND = 1;
const int SIT   = 2;
const int SLEEP = 3;
const int WALK  = 4;

main()
{
    DirectedGraph<TestNode, TestArc> graph;
    list<TestArc> path;

    graph.Add(TestArc(NT,    SLEEP));
    graph.Add(TestArc(SLEEP, STAND));
    graph.Add(TestArc(STAND, SLEEP));
    graph.Add(TestArc(SLEEP, SIT));
    graph.Add(TestArc(SIT  , SLEEP));
    graph.Add(TestArc(SIT  , STAND));
    graph.Add(TestArc(STAND, SIT));
    graph.Add(TestArc(STAND, WALK));
    graph.Add(TestArc(WALK , STAND));

    graph.Print();

    int n = graph.Search(TestNode(NT), TestNode(WALK), path);
    printf("n = %d\n", n);

    list<TestArc>::iterator iter = path.begin();
    list<TestArc>::iterator last = path.end();
    while (iter != last) {
        (*iter).Print();
        ++iter;
    }

    printf("\n");
}
