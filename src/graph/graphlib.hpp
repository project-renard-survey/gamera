/*
 * Copyright (C) 2001 Ichiro Fujinaga, Michael Droettboom, and Karl MacMillan
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#ifndef mgd010103_graphlib_hpp
#define mgd010103_graphlib_hpp

#include <Python.h>
#include <iostream>
#include <set>
#include <map>
#include <queue>
#include <stack>
#include <list>
#include <vector>
#include <exception>
#include <limits>

//////////////////////////////////////////////////////////////////////////////
// Forward references
struct IteratorObject;
struct NodeObject;
struct EdgeObject;
struct GraphObject;

typedef double CostType; // Matches a Python 'float'

//////////////////////////////////////////////////////////////////////////////
// Various container types used throughout the algorithms
typedef std::set<NodeObject*> NodeSet;
typedef std::set<EdgeObject*> EdgeSet;
typedef std::list<NodeObject*> NodeList;
typedef std::list<EdgeObject*> EdgeList;
typedef std::vector<NodeObject*> NodeVector;
typedef std::vector<EdgeObject*> EdgeVector;
typedef std::stack<NodeObject*> NodeStack;
typedef std::stack<EdgeObject*> EdgeStack;
typedef std::queue<NodeObject*> NodeQueue;
typedef std::queue<EdgeObject*> EdgeQueue;
typedef std::map<PyObject*, NodeObject*> DataToNodeMap;
typedef std::map<NodeObject*, EdgeObject*> NodeToEdgeMap;

union Any {
  int Int;
  size_t SizeT;
  long Long;
  bool Bool;
  float Float;
  double Double;
  CostType Cost;
  NodeObject* NodeObjectPtr;
  void* VoidPtr;
  NodeSet* NodeSet_;
};

struct GraphObject {
  PyObject_HEAD
  size_t m_flags;
  NodeVector* m_nodes;
  size_t m_nedges;
  // NodeSet* m_subgraph_roots;
  DataToNodeMap* m_data_to_node;
};

#define FLAG_DIRECTED 1
#define FLAG_CYCLIC 2
#define FLAG_BLOB 4
#define FLAG_MULTI_CONNECTED 8
#define FLAG_SELF_CONNECTED 16

#define FLAG_DEFAULT 0xffff
#define FLAG_FREE 31
#define FLAG_TREE 0
#define FLAG_DAG 5
#define FLAG_UNDIRECTED 14

#define HAS_FLAG(a, b) (a & b)
#define SET_FLAG(a, b) (a |= b)
#define UNSET_FLAG(a, b) (a &= ~b)

#endif