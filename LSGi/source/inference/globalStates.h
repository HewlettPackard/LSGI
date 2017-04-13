/*
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


/**\bried GlobalStates file
 * Main methods to manage memory mappings for inference states and counters
 *
 * The states will be allocated in FAM as shared mode
 * The methods allow to
 * 	1. create a file,
 * 	2. mapped in memory,
 * 	3. sync,
 * 	4. push from one local to shared DRAM copies
 * 	5. pull from the shared to local
 * 	6. access or get vertex state elements
 * 	7. init values
 *
 * We keep local copies that will be
 * */

#ifndef GLOBALSTATES_H
#define GLOBALSTATES_H

#include<iostream>
#include<libpmem.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include<stdint.h>
#include <cassert>

/*ON to debug sync states*/
//#define DEBUG_GB_ON


/**global parameters for the number of the processes
 * this parameter needs to be changed for larger number of processes*/
#define PMAX  80


/**globalArrayPushUpdatesToPersistence
 * Descriptor to store file pointer and length
 * */

extern nvMapFileDescriptor fp[PMAX];

/***********************************************************************************************************
 * Memory Maps Arrays that keep shared information across
 * multiple nodes using FAM as a communication mechanism.
 * Arrays are matrices to keep separated array per node
 * array [N][M]
 * N =number vertices for the partition
 * M =number of process in the processing
 *
 * These are arrays mapped memory file that can be persisted to FAM
 *************************************************************************************************************/

/**Shared Global State array for vertex states*/
extern VERTEX_VALUE_TYPE *global_array_shared[PMAX];

/**Shared Global Probability Distribution for vertex states*/
extern FLOAT_AGG_VALUE_TYPE *global_array_distribution_shared[PMAX];

/**Shared Runtime Statistics: overall time, iteration and **/
extern ITERATOR_INDEX *runtimeStats;

/**
 * Global-LOCAL arrays for LOCAL vertex states: */
/* this variables are used to sync global-local and shared copies amongh
 * to other computing nodes via FAM*/

/*vertex state*/
extern VERTEX_VALUE_TYPE 	*global_array_partitioned;
/*vertex counters*/
extern VERTEX_AGG_VALUE_TYPE   *global_array_copy_partitioned;

extern FLOAT_AGG_VALUE_TYPE    *global_array_distribution_partitioned;
/**/
extern FLOAT_AGG_VALUE_TYPE	*global_array_conv_partitioned;
/*vertex convergence*/
extern VERTEX_AGG_VALUE_TYPE 	*global_array_nonConv_partitioned;

/*************************************************************************************************
 * Methods to manipulate memory mappings
 *
 ***************************************************************************************************/

/*Method to map persistence structures in memory*/
 void globalArrayMap(std::string &fileName, nvMapFileDescriptor &nvmapFd, ITERATOR_INDEX arraySize );

 /*Metohd to map in memory the global array*/
int globalArrayUnMap(VERTEX_VALUE_TYPE *globalArray, nvMapFileDescriptor &nvmapFd);
void globalArrayMap(std::string &fileNameOriginal, nvMapFileDescriptor &nvmapFd, ITERATOR_INDEX arraySize,int index);
void globalArrayDistributionMap(std::string &fileNameOriginal, nvMapFileDescriptor &nvmapFd, ITERATOR_INDEX arraySize,int index);
void anyArrayMap(std::string fileNameOriginal, int fd, ITERATOR_INDEX arraySize);
/* create global array file*/
int createGlobalArray(std::string fileName,ITERATOR_INDEX arraySize,char *sharedGlobalArray);
int createGlobalArray(std::string fileNameOriginal, ITERATOR_INDEX arraySize);

/**push updates between local and shared copies. Both in DRAM.*/
int globalArrayPushUpdates(ITERATOR_INDEX startIndex, ITERATOR_INDEX endIndex, VERTEX_VALUE_TYPE *localGlobalArray, VERTEX_VALUE_TYPE *sharedGlobalArray);

/**push updates from DRAM to FAM*/
int globalArrayPushUpdatesToPersistence(ITERATOR_INDEX startIndex, ITERATOR_INDEX endIndex, char *localGlobalArray, char *sharedGlobalArray);

/**pull updates from FAM to DRAM*/
int globalArrayPullUpdates(ITERATOR_INDEX startIndex, ITERATOR_INDEX endIndex, VERTEX_VALUE_TYPE *localGlobalArray, VERTEX_VALUE_TYPE *sharedGlobalArray);
int globalArrayPushUpdatesToNVM(ITERATOR_INDEX startIndex, ITERATOR_INDEX endIndex, VERTEX_VALUE_TYPE *localGlobalArray, VERTEX_VALUE_TYPE *sharedGlobalArray);

/**Methods to access and initialize arrays*/

VERTEX_VALUE_TYPE* get_vertex_global_pointerByVariedSizePartition(TOPOLOGY_VALUE_TYPE vertex_id, int numaNode);

inline void update_local_array(int socketNode, ITERATOR_INDEX var, VERTEX_VALUE_TYPE value){
  *(global_array_partitioned+ var) = value;
}
inline void update_global_array(ITERATOR_INDEX var, VERTEX_VALUE_TYPE value, struct topology_graph *tg);
inline void update_global_array(ITERATOR_INDEX var, VERTEX_VALUE_TYPE value);
inline void update_local_array(int socketNode, ITERATOR_INDEX var, VERTEX_VALUE_TYPE value);
inline void reinitialize_global_arrayByPartition(int num_nodes);

#endif /*GLOBALSTATES_H*/
