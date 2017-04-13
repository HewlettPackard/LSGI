/* LSGi
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


/** VERSION:
 *.
 *\brief Graph Inference.
 *graphical inference is performed on graph partitions mapped to multiple
 *computing nodes. Each node performs iterative local processing that needs
 *to read remote outcomes which derives high communication volume over the network
 *and slows down runtimes on traditional clusters. To address this problem,
 *we have built a scalable large scale inference engine taking advantage of The Machine architecture.
 *Our memory-centric approach makes efficient use of the
 *FAM bandwidth and reduces synchronization overhead among computing nodes
 *while exploiting massive parallelism via the large number of cores on The Machine.
 *This implementation uses Gibbs sampling as inference method.
 *@SEE REFERENCE: https://en.wikipedia.org/wiki/Gibbs_sampling
 *@autor: Tere, Fei, Nadish, Krishna
 *
 */

#ifndef INFERENCE_H
#define INFERENCE_H

#include <vector>
#include <iostream>
#include <sys/time.h>
#include <algorithm>
#include <fstream>
#include <string>
#include <sstream>
#include <assert.h>
#include <math.h>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <time.h>
#include <sys/types.h>
#include <sys/sysinfo.h>
#include <errno.h>
#include <stdint.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <fstream>
#include <string>
#include <cstring>
#include <stdio.h>
#include "inMemoryGraph.h"
using namespace std;



/**********************************************
 * DEBUG FLAGS
 * it should be commented out to enable the debug code.
 **********************************************/
#define DEBUG_TIME_ON /*Enables breakdown time*/

/***********************************************
 * Main  headers
 ************************************************/

/*main method driver*/
void launch_gibbs_sampler(std::string parameters_filename, std::string graphFile,
		int curProcessId, int allProcess, std::string fileSystemPath);

/**graph loader to store in dram and initialization of global states*/
bool graphLoader(std::string fileName);

/**compute inference in parallel*/
void *launch_tparallel_partition_uniform_thread(void* arg);

/**get a sample from a vertex*/
inline VERTEX_VALUE_TYPE get_sample(ITERATOR_INDEX var, struct topology_graph *tg,
		double random_num, VERTEX_VALUE_TYPE*ga);

/**workload assingment by thread*/
void find_work_index_for_threadByPartition(int threadIndex,
		ITERATOR_INDEX* threadIterStartIndex,
		ITERATOR_INDEX* threadIterEndIndex,
		int nodePartitionId, int numberOfThreadsOnSocket );

/**convergence computation*/
inline void computeConvergenceAtIteration(int numaNodeId,int &local_samples_seen,
		int local_samples_collected, int &samplesCounter,
		ITERATOR_INDEX start_index,ITERATOR_INDEX endIndex);
ITERATOR_INDEX getTopologySizeAtNodeId(int numaNodeId, ITERATOR_INDEX &startIndex,
		ITERATOR_INDEX &endInddex);




#endif/* INFERENCE_H_end*/
