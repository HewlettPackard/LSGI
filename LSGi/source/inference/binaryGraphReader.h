/**
 * LSGi “© Copyright 2017  Hewlett Packard Enterprise Development LP

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

/** @file binaryGraphReader
 * \brief Binary Graph Reader File
 * Encapsulate graph reader object and its methods to read edge list (from-to edge)
 * of the graph plus factors in binary format
 * The methods will be execute in parallel for multi-socket execution.
 *
 * Binary Graph Reader assumes the following  file format in binary:
 * 		1.Header:
 * 		#number of Vertices (long)
 * 		#number of variables or states per vertice (long)
 * 		#number of unary factors in bytes (long)
 * 		#number of edge factors in bytes (long
 *
 * 2. Content for each edge(Binary Variables format)
 * 		fromEdge_ID 	(long)
 * 		toEdge_ID 		(long)
 * 		unary_factor1 	(float)
 * 		unary_factor2 	(float)
 * 		edge_fractor1 	(float) states: (0, 0)
 * 		edge_fractor2 	(float) states: (0, 1)
 * 		edge_fractor2 	(float) states: (1, 0)
 * 		edge_fractor2	(float) states: (1, 1)
 * **/


#ifndef BINARYGRAPHREADER_H
#define BINARYGRAPHREADER_H


#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include<stdint.h>
#include <sys/time.h>
#include <sstream>
#include <cassert>


/******************************************************************************************
 * \brief Debug Flags
 * it should be commented out to enable the debug code.
 * such debug comments or timers
 *****************************************************************************************/
#define DEBUG_BREAKDOWN_IME_ON /**< Enables breakdown time print*/

/**
 * \brief Constants and macros
 * It includes Graph topology constants sizes and  data types used for
 * the inference computation
 */
const int EDGESIZE          = 2;
const int HEADERELEMENTS    = 4; /**< number of elments of the header*/

extern ITERATOR_INDEX *startThreadIndex;
extern ITERATOR_INDEX *endThreadIndex;
/**File info map  by process*/

/******************************************************************************************
 *Global Variables: Data types to manage the graph in memory
 ******************************************************************************************/

/**
 * @type GraphFileMemoryMap:
 * Structure to  manage the catalog of graph partition
 * for the inference
 * graph partition file that it is read
 *
 */
struct GraphFileMemoryMap{

	char *fileName;
	size_t 		byteSize;

	int *factor_count_arr;

   	/**< graph main statistics*/
	size_t  num_of_edges;
	size_t  num_of_nodes;
	size_t  topologyArraySize;
	size_t  lastVertexLength;

	/**< factors size used to build topology index*/
	/**< map variables,unary factors,edge factors*/
	ITERATOR_INDEX numVars;
	ITERATOR_INDEX uFactorSize;
	ITERATOR_INDEX eFactorSize;
	ITERATOR_INDEX factorBlockSize;
	ITERATOR_INDEX headerSize;

	/*partition id**/
	int partitionNodeId;
};
extern GraphFileMemoryMap *graphFileMap;

/***********************************************************************************************
 * Methods to read file sections & build graph index in memory
 ***********************************************************************************************/
int getNumberOfVertices(GraphFileMemoryMap &graphFilePtr);
int getEdgeNeigbornsCounter(GraphFileMemoryMap &graphFilePtrs, int *factor_count_arr, int num_nodes);
int getNeigborns(GraphFileMemoryMap &graphFilePtrs , TOPOLOGY_VALUE_TYPE *temp_topology,
		vertex_info*, size_t num_nodes, int numNodeId);


void readGraphHeaders(GraphFileMemoryMap &graphFilePtrs);
int readVerticesOrder(std::string  fileName, int numberOfVertex, VERTEX_VALUE_TYPE *globalArrayIndex);
int readPartitionSizes(std::string fileName, int num_sockets_to_use, ITERATOR_INDEX *sizeGlobalArrayDistribution);
void readPriors(std::string fileName, VERTEX_VALUE_TYPE **global_array_partitioned, int numNodes);
ITERATOR_INDEX getVerticesFromFile(GraphFileMemoryMap &graphFilePtrs, std::string &fileName);

/*copy factors from the edge list to the topology array*/
void copyFactors(TOPOLOGY_VALUE_TYPE *temp_toplogy, TOPOLOGY_VALUE_TYPE &vertexId1, TOPOLOGY_VALUE_TYPE &vertexId2,ITERATOR_INDEX offset );
void copyFactors2(TOPOLOGY_VALUE_TYPE *temp_toplogy, TOPOLOGY_VALUE_TYPE &vertexId1,
		TOPOLOGY_VALUE_TYPE &vertexId2,ITERATOR_INDEX offset, float *edgeFactors  );

/***********************************************************************************************
 * Methods to check graph statistics and utility methods
 ***********************************************************************************************/

int checkNE(const TOPOLOGY_VALUE_TYPE  *topologyOffset, size_t num_nodes);
int displayArrays(const TOPOLOGY_VALUE_TYPE  *array, size_t size);


inline bool getline(std::ifstream& fin, std::string& line, size_t line_number) {
        return std::getline(fin, line).good();
}


inline std::string trim(const std::string& str) {
        std::string::size_type pos1 = str.find_first_not_of(" \t\r");
        std::string::size_type pos2 = str.find_last_not_of(" \t\r");
        return str.substr(pos1 == std::string::npos ? 0 : pos1, pos2 == std::string::npos ? str.size()-1 : pos2-pos1+1);
}
#endif/* GRAPHREADER_H_*/
