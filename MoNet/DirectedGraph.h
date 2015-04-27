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

#ifndef DirectedGraph_h_DEFINED
#define DirectedGraph_h_DEFINED

#include <stdio.h>

#include <list>
#include <map>
#include <set>
#include <multiset.h>
using namespace std;

template <class Node, class Arc>
class DirectedGraph {
private:
    map<Node, list<Arc>, less<Node> > graph_;

private:
    class Path {
    public:
        Path(const Arc& arc) {
            distance_ = arc.Distance();
            path_.push_back(arc);
        }
        ~Path() {}

        void PushBack(const Arc& arc) {
            distance_ += arc.Distance();
            path_.push_back(arc);
        }
        void PopBack() {
            distance_ -= path_.back().Distance();
            path_.pop_back();
        }

        const list<Arc>& ArcList() const { return path_;   }
        int         Distance()   const { return distance_; }
        const Node& LatestNode() const { return (path_.back()).Destination(); }

        friend bool operator< (const Path& a, const Path& b) {
            return (a.distance_ < b.distance_) ? true : false;
        }
    
    private:
        int        distance_;
        list<Arc>  path_;
    };

public:
    DirectedGraph() {}
    ~DirectedGraph() {}

    inline void Add(const Arc& arc);
    inline void Remove(const Arc& arc);
    inline int  Search(const Node& src, const Node& dest, list<Arc>& path);
    inline void Print();
};

template <class Node, class Arc>
void
DirectedGraph<Node, Arc>::Add(const Arc& arc)
{
    ( graph_[arc.Source()] ).push_back(arc);
}

template <class Node, class Arc>
void
DirectedGraph<Node, Arc>::Remove(const Arc& arc)
{
    ( graph_[arc.Source()] ).remove(arc);
}

template <class Node, class Arc>
int
DirectedGraph<Node, Arc>::Search(const Node& src,
                                 const Node& dest, list<Arc>& path)
{
    set<Node, less<Node> > T;
    multiset<Path, less<Path> > F;

    T.insert(src);
    list<Arc>::iterator iter = (graph_[src]).begin();
    list<Arc>::iterator last = (graph_[src]).end();
    while (iter != last) {
        F.insert(Path(*iter)); iter++;
    }
    
    while (1) {

        multiset<Path, less<Path> >::iterator path_iter = F.begin();
        if (path_iter == F.end()) {

            // Path is not found.
            return -1;

        } else if ( (*path_iter).LatestNode() == dest) {

            // Path is found.
            path = (*path_iter).ArcList();
            return (*path_iter).Distance();
        }

        Path nearestPath = *path_iter; 	F.erase(path_iter);
        Node latestNode  = nearestPath.LatestNode();

        if (T.find(latestNode) == T.end()) {

            T.insert(latestNode);

            list<Arc>::iterator iter2 = graph_[latestNode].begin();
            list<Arc>::iterator last2 = graph_[latestNode].end();
            while (iter2 != last2) {
                if ( T.find((*iter2).Destination()) == T.end() ) {
                    nearestPath.PushBack(*iter2);
                    F.insert(nearestPath);
                    nearestPath.PopBack();
                }
                ++iter2;
            }
        }
    }
}

template <class Node, class Arc>
void
DirectedGraph<Node, Arc>::Print()
{
    map<Node, list<Arc>, less<Node> >::iterator iter = graph_.begin();
    map<Node, list<Arc>, less<Node> >::iterator last = graph_.end();
    while (iter != last) {
        printf("Node ");
        (*iter).first.Print();
        printf(" : ");
        list<Arc>::iterator iter2 = (*iter).second.begin();
        list<Arc>::iterator last2 = (*iter).second.end();
        while (iter2 != last2) {
            (*iter2).Print();
            ++iter2;
        }
        printf("\n");
        ++iter;
    }
}

#endif // DirectedGraph_h_DEFINED
