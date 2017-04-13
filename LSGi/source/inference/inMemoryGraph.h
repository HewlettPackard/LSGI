/**
“© Copyright 2017  Hewlett Packard Enterprise Development LP

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF
USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.”
*/
\
/**
 * \brief inMemoryGraph Data structures that store local graphs in memory for
 * efficient sequential access.
 * Encapsulate graph topology as 3 main elements
 * vertex_info:
 * factor_graph: adjacency list sorted by vertex
 *
 * **/
#ifndef INMEMORYGRAPH_H
#define INMEMORYGRAPH_H

#include <iostream>
#include <fstream>
#include <cstring>
#include<string>
#include<unistd.h>
#include<stdio.h>
#include<stdlib.h>
#include<stdint.h>


/***********************************************
 * CUSTOM TYPES
 * for graph/graph topology and main operations
 ***********************************************/
/*Factor types*/
#define  FACTOR_VALUE_TYPE_INFLATED int64_t
#define  FACTOR_VALUE_TYPE_DEFLATED float
#define  FACTOR_VALUE_INFLATION (1LL << 20)
#define MAX_FACTOR_LENGTH 2
//USE OR NOT FACTORTABLE, 0-not used
#define USEDFACTORTABLE 0

/*Vertex type:  type for the state of the vertex*/
#define VERTEX_VALUE_TYPE char

/*aggregators/couters: type for convergence and counters*/
#define VERTEX_AGG_VALUE_TYPE int
#define FLOAT_AGG_VALUE_TYPE float

/*Topology/Iterators: types for index, offsets, Vertices identifiers*/
#define TOPOLOGY_VALUE_TYPE int64_t
#define ITERATOR_INDEX int64_t

/***********************************************
 * FACTOR CONSTANTS
 * for accessing factor table
 ***********************************************/
/*number of elements for a unary factor*/
#define UNARY_FACTOR_LENGTH 4
/*number of elements for a binary factor*/
#define BINARY_FACTOR_LENGTH 6
/*number of unary factor within a vertex*/
#define NUM_OF_UNARY_FACTORS 1
/*number of binary factor within a vertex*/
#define NUM_OF_BINARY_FACTORS 2
/*number of variables within a domain*/
#define NUM_OF_VALUES_FOR_VARIABLE 2

/****************************************************************
 * GRAPH TOPOLOGY STRUCTURES
 * This is the topology graph that is stored in each socket or node used. This is the main data structure that will be
 * accessed by all the threads for Inference. This is a read-only immutable data structure.
 */

/**\brief vertex pointers
 * Vertex info which point to global states
 * and to the adjance list in the topology array index.
 * */
struct vertex_info {
	/* this is the pointer to the first factor pointer of a vertex.*/
	TOPOLOGY_VALUE_TYPE first_factor_offset;
	/* this is the pointer to the global variable location which holds the actual value for this vertex.*/
	VERTEX_VALUE_TYPE *global_value_ptr;
	/* number of index locations that belong to this vertex in the topology_array*/
	int length;
	/* initial label for the vertex: known state of the vertex*/
	bool label;
};

/** \brief topology_array_element
 * the element of the topology array
 * can be a vertex identifier or factor value or factor pointer*/
union topology_array_element {
	/*pointer to the value of a neighboring vertex.*/
	VERTEX_VALUE_TYPE* vertex_ptr;
	/* offset index to the correct entry in the factor table for the factor considered.*/
	FACTOR_VALUE_TYPE_INFLATED factor_value;
	/* pointer to the correct entry in the factor table for the factor considered.*/
	FACTOR_VALUE_TYPE_INFLATED *factor_ptr;
};

/**topology graph: adjacency list sorted by vertices, edge factors, neighbors*/
struct topology_graph {
	// An entry for each vertex in the graph. Contains links to corresponding index in the topology array.
	struct vertex_info *vertex_array;
	// A big 1-D array that stores the information regarding each factor of each node, along with links to the neighboring
	// vertices of a factor.
	union topology_array_element *topology_array;
	// This is the actual factor table.
	FACTOR_VALUE_TYPE_INFLATED *factor_tables;
};


/***************************************************************************************
 * Global variable for the inMemory-Graph
 * Topology Master pointer
 * Keeps the pointer to the in-memory graph/index structure
 ****************************************************************************************/
extern struct topology_graph *tgMaster;

/*Global values for generic reading of vertex/factors graph*/
extern int numVars;
extern int numEdgeFactors;
extern int topologySizeByVertex;
extern ITERATOR_INDEX num_of_nodes;
extern size_t topologyArraySize;


/**
 * Methods related to the in-memory graph
 *
 * **/

void populate_factors(FACTOR_VALUE_TYPE_INFLATED *factor_table);
void get_factor_pointer(int vertex_id, int neighbor, int num_of_vars_in_factor,
		FACTOR_VALUE_TYPE_INFLATED *factor_table, FACTOR_VALUE_TYPE_INFLATED* return_ptr);

FACTOR_VALUE_TYPE_INFLATED* get_factor_pointer(int vertex_id, int neighbor, int num_of_vars_in_factor,
		FACTOR_VALUE_TYPE_INFLATED *factor_table);
void display_topology_graph(int node, size_t topologySize);
struct topology_graph* get_topology_graph(int node);

/*****************************************************************
 * Factor Methods
 * Enable manage edge factors as factor table
 *****************************************************************/

/*
 * factor deflated
 *convert int64 to float
 */

inline FACTOR_VALUE_TYPE_DEFLATED deflate_factor_value(FACTOR_VALUE_TYPE_INFLATED inflated) {
  return static_cast<FACTOR_VALUE_TYPE_DEFLATED>(inflated) / FACTOR_VALUE_INFLATION;
}
/*
 * factor in inflated
 * convert float to int64
 */
inline FACTOR_VALUE_TYPE_INFLATED inflate_factor_value(FACTOR_VALUE_TYPE_DEFLATED deflated) {
   //cout<<"inflating value "<<deflated<<"\n";
   //cout<<"inflated value "<<static_cast<FACTOR_VALUE_TYPE_INFLATED>(deflated * FACTOR_VALUE_INFLATION)<<"\n";
   return static_cast<FACTOR_VALUE_TYPE_INFLATED>(deflated * FACTOR_VALUE_INFLATION);
}


/*retrive neighborhood size for a given vertex id*/
inline int get_vertex_neighborhood_size(int index, int *factor_count_arr, int *neighbor_count_arr) {
	// this adjacency list is supposed to contain information regarding;
	// - a factor pointer.
	// - the neighbors involved in each factor.
	return(*(factor_count_arr+index) + *(neighbor_count_arr+index));
}


#endif /*INMEMORYGRAPH_H_*/
