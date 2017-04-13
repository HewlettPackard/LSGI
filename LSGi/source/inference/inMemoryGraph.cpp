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
#include "inMemoryGraph.h"
#include "customMalloc.h"

/**Var Initialization*/
/***************************************************************************************
 * Global variable for the inMemory-Graph
 * Topology Master pointer
 * Keeps the pointer to the in-memory graph/index structure
 ****************************************************************************************/
struct topology_graph *tgMaster;

/*Global values for generic reading of vertex/factors graph*/
int numVars;
int numEdgeFactors;
int topologySizeByVertex;
ITERATOR_INDEX num_of_nodes;
size_t topologyArraySize;


/*****************************************************************
 * Memory Graph Methods
 *
 *****************************************************************/

/**
 * There can be multiple ways to store factors. Either read it from a file or hard code it.
 * We can also have different kinds of factors. How we populate the factor table and how we retrieve the
 * index to a particular factor are co-dependent. So if we want different ways to do the same, change both
 * functions accordingly.
 */
void populate_factors(FACTOR_VALUE_TYPE_INFLATED *factor_table) {
	//int factor_table_length = (NUM_OF_UNARY_FACTORS*UNARY_FACTOR_LENGTH) + (NUM_OF_BINARY_FACTORS*BINARY_FACTOR_LENGTH);
	//FACTOR_VALUE_TYPE *factor_table = (FACTOR_VALUE_TYPE*)malloc(factor_table_length*sizeof(FACTOR_VALUE_TYPE));
	// For now, tentatively, I am hard-coding the following factor table.
	// For each factor, the following entries will be shown:
	// # of variables, # of entries in the factor, # the actual entries.
	// First start with unary factor;
	int i=0;
	factor_table[i++] = 1.0;
	factor_table[i++] = 2.0;
	factor_table[i++] = inflate_factor_value(-0.69314);
	factor_table[i++] = inflate_factor_value(-0.69314);
	// now go for the first binary factor (FIRST vertex id < SECOND vertex id).
	factor_table[i++] = 2.0;
	factor_table[i++] = 4.0;
	factor_table[i++] = inflate_factor_value(-0.67334);// (0, 0)
	factor_table[i++] = inflate_factor_value(-0.71334);// (0, 1)
	factor_table[i++] = inflate_factor_value(-1.38629);// (1, 0)
	factor_table[i++] = inflate_factor_value(-0.28768);// (1, 1)
	// now go for the second binary factor (SECOND vertex id < FIRST vertex id).
	factor_table[i++] = 2.0;
	factor_table[i++] = 4.0;
	factor_table[i++] = inflate_factor_value(-0.67334);// (0, 0)
	factor_table[i++] = inflate_factor_value(-1.38629);// (0, 1)
	factor_table[i++] = inflate_factor_value(-0.71334);// (1, 0)
	factor_table[i++] = inflate_factor_value(-0.28768);// (1, 1)
	//return factor_table;

//	for(int j=0; j<i;j++)
	       //cout<<"factor at "<<j<<" is "<<factor_table[j]<<"\n";
}

/**
 * This method is for the rudimentary factor tables above. This has to be changed when we introduce more factor
 * tables.
 */
void get_factor_pointer(int vertex_id, int neighbor, int num_of_vars_in_factor,
		FACTOR_VALUE_TYPE_INFLATED *factor_table, FACTOR_VALUE_TYPE_INFLATED* return_ptr) {
	FACTOR_VALUE_TYPE_INFLATED* fact_ptr;
	if(num_of_vars_in_factor == 0) {
		// this is a unary factor. At the moment there is only one unary factor.
		fact_ptr = factor_table;
	}
	else {
		// if vertex 1 < vertex 2
		if(vertex_id < neighbor)
			fact_ptr = &factor_table[4];
		else
			fact_ptr = &factor_table[10];
	}
	return_ptr = fact_ptr;
}

/**
 * This method is currently tuned for binary factors and unary factors ONLY.
 * It has to change for more complex factors.
 */
FACTOR_VALUE_TYPE_INFLATED* get_factor_pointer(int vertex_id, int neighbor, int num_of_vars_in_factor,
		FACTOR_VALUE_TYPE_INFLATED *factor_table) {
	FACTOR_VALUE_TYPE_INFLATED* fact_ptr;
	if(num_of_vars_in_factor == 0) {
		// this is a unary factor. At the moment there is only one unary factor.
		fact_ptr = factor_table;
	}
	else {
		// if vertex 1 < vertex 2
		if(vertex_id < neighbor)
			fact_ptr = &factor_table[4];
		else
			fact_ptr = &factor_table[10];
	}
	return fact_ptr;
}


/*
 * return the topology pointer partitioned on specific node
 * @PARAMS: node as numa node id
 * @RETURN: pointer to the topology of the node id
 * */
struct topology_graph* get_topology_graph(int node) {

	return tgMaster;
}





