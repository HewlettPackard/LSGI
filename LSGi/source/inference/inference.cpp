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
#include "customMalloc.h"
#include "utils.h"
#include "inMemoryGraph.h"
#include "inferenceConfig.h"
#include "binaryGraphReader.h"
#include "globalStates.h"
#include "processSync.h"
#include "inference.h"
using namespace std;

/** global timer and time variables*/
struct timeval timestamp;
/** keep the synchronization time */
double syntime;
/*total runtime for the exectuion*/
long runtime;


/********************************************************************
 * General global variables used for inference computing
 * and distributed processing
 *********************************************************************/

/**
 *distributed processing by partitions:
 **/

/*distributed list of partition indices by node, start and end pointers*/
ITERATOR_INDEX *sizeGlobalArrayDistribution;
ITERATOR_INDEX *startThreadIndex;
ITERATOR_INDEX *endThreadIndex;
VERTEX_VALUE_TYPE *globalArrayIndex;


/*current process Identifier*/
//extern int processId;
/*the total number of processes running*/
//extern int totalProcess;

/**phtread barries for the iterations*/
/*first barrier: sampling*/
pthread_barrier_t barrier1;
/*second barrier: states synchronization*/
pthread_barrier_t barrier2;
/*mutex to update cpu counters*/
pthread_mutex_t cpumutex;



/*****************************************************************
 * Inference Methods
 *
 *****************************************************************/

/*
 * return the topology partition based on the vertex id
 * the partition id represents the numa socket id where is placed
 * the vertex info
 * */
int get_partition(TOPOLOGY_VALUE_TYPE vertex_id){

 ITERATOR_INDEX firstInSocket =0;
        ITERATOR_INDEX lastInSocket =-1;
        int i;
       for(i=0;i<num_sockets_to_use; i++){
                firstInSocket = (ITERATOR_INDEX) lastInSocket+1;
                lastInSocket  = (ITERATOR_INDEX)(firstInSocket + sizeGlobalArrayDistribution[i]-1);
            if(vertex_id <= lastInSocket && vertex_id >= firstInSocket)
                   break;
        }

 return i;
}


/*****************************************************************
 * Inference methods
 * multiply factors, get samples, update states
 *****************************************************************/

/**
 * IMPROTANT ASSUMPTION:
 * this method is tailor made for the assumption that the first factor is a unary factor and all other factors
 * are only edge factors. This also assumes that the variable takes only binary values. This function is called
 * when the variable being sampled takes 0 as the value.
 */
inline FACTOR_VALUE_TYPE_INFLATED multiply_factors_zero(struct topology_graph *tg, TOPOLOGY_VALUE_TYPE start_index,
		TOPOLOGY_VALUE_TYPE end_index) {
	// first get the unary factor.
	FACTOR_VALUE_TYPE_INFLATED factor_product = 0.0;
	factor_product += *((tg->topology_array[start_index].factor_ptr)+2);
	start_index++;
	// now fetch for the binary factor.
	while(start_index < end_index) {
		FACTOR_VALUE_TYPE_INFLATED* fact_ind = tg->topology_array[start_index].factor_ptr;
		int jump_index = *(fact_ind);
		int neighbor_val = *(tg->topology_array[start_index+1].vertex_ptr);
		//int neighbor_val = 0;

		factor_product += *(fact_ind + neighbor_val + 2);
		start_index += jump_index;

	}
	return factor_product;
}
/**
 * IMPROTANT ASSUMPTION:
 * this method is tailor made for the assumption that the first factor is a unary factor and all other factors
 * are only edge factors. This also assumes that the variable takes only binary values. This function is called
 * when the variable being sampled takes 1 as the value.
 */
inline FACTOR_VALUE_TYPE_INFLATED multiply_factors_one(struct topology_graph *tg, TOPOLOGY_VALUE_TYPE start_index, TOPOLOGY_VALUE_TYPE end_index) {
	// first get the unary factor.
	FACTOR_VALUE_TYPE_INFLATED factor_product = 0.0;
	factor_product += *((tg->topology_array[start_index].factor_ptr)+3);
	start_index++;
	// now fetch for the binary factor.
	while(start_index < end_index) {
		FACTOR_VALUE_TYPE_INFLATED* fact_ind = tg->topology_array[start_index].factor_ptr;
		int jump_index = *(fact_ind);
		int neighbor_val = *(tg->topology_array[start_index+1].vertex_ptr);
		//int neighbor_val = 1;
		factor_product += *(fact_ind + neighbor_val + 4);
		start_index += jump_index;
	}
	return factor_product;
}



/**
 * Compute the product of the factors, get the conditional probability
 * and then get the sample of the new vertex state.
 * IMPROTANT ASSUMPTION:
 * This method assumes the first factor is always a unary factor.
 * This also assumes we are only looking at binary values for each variable.
 * For example: vertex 0 have 2 unary factors (0,1) then 4 edge factors, then the vertex id of the neighborn
 * The function has some unrolled the loops to help on performance, but limited usability
 */
inline VERTEX_VALUE_TYPE get_sample(ITERATOR_INDEX var, struct topology_graph *tg,
		double random_num, VERTEX_VALUE_TYPE*ga) {

	TOPOLOGY_VALUE_TYPE topology_offset = tg->vertex_array[var].first_factor_offset;
	int length = tg->vertex_array[var].length;
	TOPOLOGY_VALUE_TYPE        end_index = topology_offset+length;
	FACTOR_VALUE_TYPE_INFLATED prob_values[NUM_OF_VALUES_FOR_VARIABLE];
	FACTOR_VALUE_TYPE_INFLATED maxVal = 0;
	VERTEX_VALUE_TYPE neighbor_val    = 0;
	float normalizing_factor          = 0.0;
	float probability_values[NUM_OF_VALUES_FOR_VARIABLE];



	prob_values[0] = ((tg->topology_array[topology_offset].factor_value)); //+2-f0
	topology_offset++;


 	prob_values[1] = ((tg->topology_array[topology_offset].factor_value)); //+3-f1
 	topology_offset++;

 	ITERATOR_INDEX jump;
 	//computing factor product
	while(topology_offset < end_index) {

		//+4
		neighbor_val = ga[tg->topology_array[topology_offset+4].factor_value];
		jump = topology_offset;

	    if (neighbor_val != 0){
	     	   jump++;
	    }

	    prob_values[0]  += tg->topology_array[jump].factor_value;//*( fact_ind+ neighbor_val+2);
	    prob_values[1]  += tg->topology_array[jump+2].factor_value;//*( fact_ind+ neighbor_val+4);
	    topology_offset += topologySizeByVertex;
	 }

    maxVal =max(prob_values[0], prob_values[1]);

    // computing sampling
	// NOTE: This change works only for binary values now!
	probability_values[0] = exp(deflate_factor_value(prob_values[0] - maxVal));
	probability_values[1] = exp(deflate_factor_value(prob_values[1] - maxVal));

	normalizing_factor = probability_values[0] + probability_values[1];


	int new_value = 0;
	float p = 0.0;
	for(; new_value<NUM_OF_VALUES_FOR_VARIABLE-1; new_value++) {
		p += probability_values[new_value]/normalizing_factor;
		if(random_num <= p)
			break;
	}

	/*std::cout <<"\nvar="<<var;
	std::cout <<"\ndeflate_factor_value(prob_values[0] - maxVal):"<<deflate_factor_value(prob_values[0] - maxVal);
	std::cout <<"\ndeflate_factor_value(prob_values[1] - maxVal):"<<deflate_factor_value(prob_values[1] - maxVal);
	std::cout <<"\t unary factors: length:"<<tg->vertex_array[var].length;
	std::cout << "\t max:"<<maxVal
			  << "\t prob_values[0]"<< prob_values[0]
			  << "\t prob_values[1]"<<prob_values[1]
			  << "\t random number:"<<random_num
			  << "\t new value:" <<new_value;*/

	return new_value;
}



/*
 * compute Convergence statistics
 * every number of sample specific from the configuration file
 * */
inline void computeConvergenceAtIteration(int numaNodeId,int &local_samples_seen,
		int local_samples_collected, int &samplesCounter,
		ITERATOR_INDEX start_index,ITERATOR_INDEX endIndex, int threadIndex) {

	int   differing_nodes =0;
	float prob;
	float diff;

	if(local_samples_seen >= stat_samples_interval && samplesCounter < num_of_stat_samples) {

			for(int j=start_index; j<endIndex; j++) {
				prob = global_array_copy_partitioned[j]/(local_samples_collected*(1.0));
				diff = global_array_conv_partitioned[j] - prob;
				if(diff < 0){
					diff = diff*(-1);
				}
				global_array_conv_partitioned[j] = prob;
				if(diff > threshold) {
					differing_nodes++;
				}
			}
			int pos=  (threadIndex*num_of_stat_samples)+ samplesCounter;
			global_array_nonConv_partitioned[pos] =differing_nodes;

			local_samples_seen = 0;
			samplesCounter++;
	}
}



/*****************************************************************
 * Worker methods
 * Find worker tasks, assign workers to core, main inference method
 *****************************************************************/

/*
 * return cpu assigned given the node id
 * enable manage balance workers by node id
 * to keep equal number of assigned threads over the socket
 * */
inline int get_modular_threadID(int node_id_index) {
	int cpu_id = -1;
	// The CPU id to use is:
	// NodeID*MAX_CORE_PER_SOCKET + cpuID (offset)

	// numaNodes[node_id_index]*MAX_CORE_PER_SOCKET -> the first core ID in socket numaNodes[node_id_index]
	// cpuid_counter_array[node_id_index] -> The offset of the CPU id in the socket.
	int first_core_of_socket = numaNodes[node_id_index]*MAX_CORE_PER_SOCKET;
	pthread_mutex_lock(&cpumutex);
	cpu_id = first_core_of_socket + cpuid_counter_array[node_id_index];
	if(cpuid_counter_array[node_id_index] == MAX_CORE_PER_SOCKET-1)
		cpuid_counter_array[node_id_index] = 0;
	else
		cpuid_counter_array[node_id_index] = cpuid_counter_array[node_id_index]+1;
	pthread_mutex_unlock(&cpumutex);
	return cpu_id;
}

/**
 * IMPROTANT: Note that the thread_num_in_socket is set to the a number between 0 and MAX_CORE_PER_SOCKET-1.
 * If we run with number of threads that would lead to more than MAX_CORE_PER_SOCKET threads per socket, then
 * this method will not return the correct value for thread_num_in_socket (because of the if-else). This wrong value
 * might also lead to wrong Gibbs sampling since some variables might never get sampled, in case of PARTITION.
 * ASSUMPTION: We will not be running more than the MAX_CORE_PER_SOCKET-1 threads per socket.
 */
inline int get_modular_threadID(int* thread_num_in_socket, int node_id_index) {
	int cpu_id = -1;
	// The CPU id to use is:
	// NodeID*MAX_CORE_PER_SOCKET + cpuID (offset)

	// numaNodes[node_id_index]*MAX_CORE_PER_SOCKET -> the first core ID in socket numaNodes[node_id_index]
	// cpuid_counter_array[node_id_index] -> The offset of the CPU id in the socket.
	int first_core_of_socket = numaNodes[node_id_index]*MAX_CORE_PER_SOCKET;
	pthread_mutex_lock(&cpumutex);
	cpu_id = first_core_of_socket + cpuid_counter_array[node_id_index];
	*thread_num_in_socket = cpuid_counter_array[node_id_index];
	if(cpuid_counter_array[node_id_index] == MAX_CORE_PER_SOCKET-1)
		cpuid_counter_array[node_id_index] = 0;
	else
		cpuid_counter_array[node_id_index] = cpuid_counter_array[node_id_index]+1;
	pthread_mutex_unlock(&cpumutex);
	return cpu_id;
}

/*
 * Set the thread to specific core in order top
 * avoid context swithed between cores
 * */
inline int stick_this_thread_to_core(int core_id) {
  int num_cores = sysconf(_SC_NPROCESSORS_ONLN);
  if (core_id < 0 || core_id >= num_cores)
    return EINVAL;

  cpu_set_t cpuset;
  CPU_ZERO(&cpuset);
  CPU_SET(core_id, &cpuset);

  pthread_t current_thread = pthread_self();
  return pthread_setaffinity_np(current_thread, sizeof(cpu_set_t), &cpuset);
}

/*
 * retrieves the socket id
 * given a node id,
 * enable manages different ids for socket/nodes
 * */
int get_socketid_index(int node_id_index) {
	if(node_id_index < num_sockets_to_use)
		return numaNodes[node_id_index];
	else {
		std::cout << " ERROR!!!!! invalid node id= "<<node_id_index << std::endl;
		exit(2);
	}
}

/*
 * Determine number of vertex to be assinged to specific thread/worker.
 * Each socket has a contigorous partition, whose size may vary.
 * All threads in the same socket uniformly partition the work
 */
void find_work_index_for_thread(int node_id_index, int thread_id_in_socket, int* socketPartitionSizes,
		ITERATOR_INDEX* thread_iter_start_index, ITERATOR_INDEX* thread_iter_end_index) {

	ITERATOR_INDEX segment_size = sizeGlobalArrayDistribution[node_id_index];

	ITERATOR_INDEX segment_start_index=0;

	for(int i=0; i<node_id_index; i++)
		segment_start_index += socketPartitionSizes[i];

	ITERATOR_INDEX segment_end_index = segment_start_index + segment_size;
	int max_threads_per_socket_to_use = threads_per_socket[node_id_index];

	ITERATOR_INDEX thread_work_size = segment_size/max_threads_per_socket_to_use;

	*thread_iter_start_index = segment_start_index + thread_work_size*thread_id_in_socket;
	*thread_iter_end_index = *thread_iter_start_index + thread_work_size;

	if(thread_id_in_socket == max_threads_per_socket_to_use-1) {
		// the last thread in socket may have to do some extra work.
		*thread_iter_end_index = segment_end_index;
	}
	if(thread_id_in_socket == max_threads_per_socket_to_use-1 && node_id_index == num_sockets_to_use-1) {
		// the last thread in the last socket might have to work on more samples. so the ending value is the num_nodes.
		*thread_iter_end_index = num_of_nodes;
	}
}

/*
 * Determine number of vertex to be assinged to specific thread/worker.
 * With partition method, the assumption is that the dataset is split into contiguous equal segments.
 * For instance, all threads in socket 0, will work in the first segment and all threads in the
 * last socket will be working on the last segment.
 * The other behavior of this code is that, in some cases, the number of threads in some socket might be lesser
 * (normally less by one thread) than other sockets. The threads in such sockets will end up doing more work here
 * since the "segment" size does not change per socket and there will be lesser number of threads working on the
 * same amount of load, compared to another socket which might have more threads to work on the segment load.
 */
void find_work_index_for_thread(int node_id_index, int thread_id_in_socket,
		ITERATOR_INDEX* thread_iter_start_index, ITERATOR_INDEX* thread_iter_end_index) {

	ITERATOR_INDEX segment_size = num_of_nodes/num_sockets_to_use;
	ITERATOR_INDEX segment_start_index = node_id_index*segment_size;
	ITERATOR_INDEX segment_end_index = segment_start_index + segment_size;
	int max_threads_per_socket_to_use = threads_per_socket[node_id_index];

	ITERATOR_INDEX thread_work_size = segment_size/max_threads_per_socket_to_use;
	*thread_iter_start_index = segment_start_index + thread_work_size*thread_id_in_socket;
	*thread_iter_end_index = *thread_iter_start_index + thread_work_size;

	if(thread_id_in_socket == max_threads_per_socket_to_use-1) {
		// the last thread in socket may have to do some extra work.
		*thread_iter_end_index = segment_end_index;
	}
	if(thread_id_in_socket == max_threads_per_socket_to_use-1 && node_id_index == num_sockets_to_use-1) {
		// the last thread in the last socket might have to work on more samples. so the ending value is the num_nodes.
		*thread_iter_end_index = num_of_nodes;
	}
}

/*
 * Determine number of vertex to be assinged to specific thread/worker.
 * This function distributes work among all the threads uniformly. This does NOT consider the number of sockets used
 * like the other overloaded method. So, irrespective of the number of threads in each socket, the amount of work
 * done by each thread will remain the same. This is based on Fei's benchmarking code.
 */
void find_work_index_for_thread(int thread_index,
		ITERATOR_INDEX* thread_iter_start_index, ITERATOR_INDEX* thread_iter_end_index) {

	ITERATOR_INDEX thread_work_size = num_of_nodes/num_of_threads;
	*thread_iter_start_index = thread_work_size*thread_index;
	*thread_iter_end_index = *thread_iter_start_index + thread_work_size;

	if(thread_index == num_of_threads-1) {
		// the last thread in the last socket might have to work on more samples. so the ending value is the num_nodes.
		*thread_iter_end_index = num_of_nodes;
	}
}

/*
 * Determine number of vertex to be assinged to specific thread/worker.
 * */
void find_work_index_for_threadByPartition(int threadIndex,
		ITERATOR_INDEX* threadIterStartIndex,
		ITERATOR_INDEX* threadIterEndIndex,
		int nodePartitionId, int numberOfThreadsOnSocket ) {


	ITERATOR_INDEX threadWorkSize =  sizeGlobalArrayDistribution[nodePartitionId]/numberOfThreadsOnSocket;
	*threadIterStartIndex = startThreadIndex[nodePartitionId]+ threadWorkSize*(threadIndex%numberOfThreadsOnSocket);
	*threadIterEndIndex   = *threadIterStartIndex + threadWorkSize;
	if((threadIndex+1)%numberOfThreadsOnSocket==0){
		*threadIterEndIndex= endThreadIndex[nodePartitionId];
	}

	std::cout <<"\nThread: "<<threadIndex<<"\tstart index: "<<*threadIterStartIndex <<"\t end index:"<<*threadIterEndIndex;
}


/**
 * Main thread function: each worker will execute this method
 * to run the inference model
 * This function partitions the total worker tasks equally among all the threads.
 * This method executes updates to remote partitions in batched
 */
void *launch_tparallel_partition_uniform_thread(void* arg) {
	int thread_index = *(int*)arg;

	int trheadBySocket = num_of_threads;///num_sockets_to_use;
	int node_id_index = thread_index /trheadBySocket;
	node_id_index =processId;
	int node_id = get_socketid_index(node_id_index);



#ifdef NUMALLOC
	struct bitmask* mask = numa_allocate_nodemask();
	numa_bitmask_setbit(mask, node_id);
	numa_bind(mask);
	numa_set_membind(mask);
#endif

	int thread_number_in_socket = 0;

	int cpu_id = get_modular_threadID(&thread_number_in_socket, node_id_index);
	stick_this_thread_to_core(cpu_id);

	ITERATOR_INDEX thread_iter_start_index;
	ITERATOR_INDEX thread_iter_end_index;
	ITERATOR_INDEX partition_offset =0;
	VERTEX_VALUE_TYPE sample_value=0;
	VERTEX_VALUE_TYPE *ga= global_array_partitioned;

	int local_samples_collected = 0;
	int local_samples_seen = 0;
	int samplerCounter     = 0;
	double x;


	find_work_index_for_threadByPartition(thread_index, &thread_iter_start_index, &thread_iter_end_index,
			node_id_index,num_of_threads);

	partition_offset =  thread_iter_start_index -startThreadIndex[processId];

	struct topology_graph *tg = get_topology_graph(node_id);
	struct drand48_data randBuffer;
	srand48_r((ITER+1)*thread_index,&randBuffer);


	for(int i=0; i<num_total_iters; i++) {
		for(ITERATOR_INDEX j=thread_iter_start_index; j<thread_iter_end_index; j++) {

           drand48_r(&randBuffer,&x);

           if (!tg->vertex_array[j].label){
        	   sample_value = get_sample(j, tg, x,ga);
        	   update_local_array(node_id,j,sample_value);
           }else{
        	   //sample_value= to the original/old value
        	   sample_value = *(tg->vertex_array[j].global_value_ptr);
           }

		if (sample_value== 0){
			global_array_copy_partitioned[j] = global_array_copy_partitioned[j] + 1;
		   }

			global_array_distribution_partitioned[j] =(float)(global_array_copy_partitioned[j])/(i+1);

			/*debug code*/
			/*if(thread_index==0 && (j==0 || j ==1)){
				std::cout <<"\ni="<<i+1;
				std::cout <<"\nsample value:" <<    sample_value +48;
				std::cout <<"\nglobal val:"<< global_array_partitioned[j] +48;
				std::cout <<"\n\tcount value:"   <<(global_array_copy_partitioned[j]);
				std::cout <<"\n\tprob dis="      <<global_array_distribution_partitioned[j];
				std::cout <<"\n\tSHARE prob dis="<<global_array_distribution_shared[processId][j];
				int a;
				std::cin >>a;
			}*/


		}


		//update local states on DRAM to shared memory before persisting
		globalArrayPushUpdatesToPersistence(thread_iter_start_index,thread_iter_end_index,
				reinterpret_cast<char*>(global_array_partitioned),
				reinterpret_cast<char*>(global_array_shared[processId]+partition_offset));

		//update local distribution on DRAM to shared memory before persisting
		globalArrayPushUpdatesToPersistence(
				thread_iter_start_index*sizeof(float),thread_iter_end_index*sizeof(float),
						reinterpret_cast<char*>(global_array_distribution_partitioned),
						reinterpret_cast<char*>(global_array_distribution_shared[processId]+partition_offset));

		if(generate_statistics) {
			local_samples_collected++;
			local_samples_seen++;
			computeConvergenceAtIteration(node_id,local_samples_seen,local_samples_collected,samplerCounter,
					thread_iter_start_index,thread_iter_end_index,thread_index);
		    }


			pthread_barrier_wait(&barrier1);
			pthread_barrier_wait(&barrier2);
	}
	return(NULL);
}


/*
 * Main thread launcher based on user configuration
 * */
void launch_sequential_parallel() {

	struct timeval end_tv;
	struct timeval start_tv;
	struct timeval globalTime;
	ITERATOR_INDEX totalConvergeStat=sizeGlobalArrayDistribution[processId];

	pthread_t tid[num_of_threads];
	std::cout <<"\nRunning inference on process:"<<processId <<"\n processing nodes: "
			  <<sizeGlobalArrayDistribution[processId] <<"-startIdex: "<<startThreadIndex[processId];

	/*start time shared counters*/
	gettimeofday(&globalTime, NULL);

	if(pthread_barrier_init(&barrier1, NULL, num_of_threads+1)) {
		cout <<"Could not create a barrier 1\n";
	}

	if(pthread_barrier_init(&barrier2, NULL, num_of_threads+1)) {
			cout <<"Could not create a barrier 1\n";
		}

	for (int i = 0; i < num_of_threads; i++) {
		if (pthread_create(&tid[i], NULL, launch_tparallel_partition_uniform_thread, new int(i))) {
			cout << "Cannot create thread " << i << endl;
		}
	}

	/*update nvram copy*/
	for(int i=0; i<num_total_iters; i++) {
		ITER = i;
		pthread_barrier_wait(&barrier1);
		gettimeofday(&start_tv, NULL);

		
		pmem_persist(reinterpret_cast<char*>(global_array_shared[processId]),
				(endThreadIndex[processId]-startThreadIndex[processId]));

		/*Updating ITER*/
		global_array_distribution_partitioned[sizeGlobalArrayDistribution[processId]] =(float)(ITER+1);
		global_array_distribution_shared[processId][sizeGlobalArrayDistribution[processId]] =(float)(ITER+1);

		pmem_persist(reinterpret_cast<char*>(global_array_distribution_shared[processId]),
			(1 + endThreadIndex[processId]-startThreadIndex[processId])*sizeof(float));

		for (int p=0; p<totalProcess;p++){
			if (p!=processId){

				globalArrayPullUpdates(startThreadIndex[p],endThreadIndex[p],
						global_array_partitioned,
						global_array_shared[p]);
			}
		}


		gettimeofday(&end_tv,NULL);
		syntime += (end_tv.tv_sec - start_tv.tv_sec) +
					(end_tv.tv_usec - start_tv.tv_usec) / 1000000.0;
		runtime =  (end_tv.tv_sec - globalTime.tv_sec) +
					(end_tv.tv_usec - globalTime.tv_usec) / 1000000.0;

		/*persis stats to file*/
		/*runtimeStats[0]= when the convergence is computed*/
		int currentSample =ITER/(stat_samples_interval);
//		totalConvergeStat=0;

		if((ITER%(stat_samples_interval-1)) ==0 && currentSample>0){
			 totalConvergeStat=0;
			for (int m=0; m <num_of_threads; m++){
					int pos=m*num_of_stat_samples+(currentSample-1);		
					totalConvergeStat +=global_array_nonConv_partitioned[pos];
			}
		}
		runtimeStats[0] = totalConvergeStat;		
		runtimeStats[1] = ITER+1;
		runtimeStats[2] = runtime;
				
		//std::cout <<"\n iter "<< runtimeStats[1]<< "\t nonconver="<<runtimeStats[0] <<" \t time:"<< runtimeStats[2] ;
		pmem_persist(reinterpret_cast<char*>(runtimeStats),(3)*sizeof(ITERATOR_INDEX));
		

		pthread_barrier_wait(&barrier2);
	}


	for (int j = 0; j < num_of_threads; j++) {
		pthread_join(tid[j], NULL);
	}
	pthread_barrier_destroy(&barrier1);
	pthread_barrier_destroy(&barrier2);

	std::cout << "\n\n===>SYNC TIME IS:"<<syntime<<"\n";
}
/*
 * Launch inference for a singl thread
 * */
inline void launch_single_thread() {
	struct topology_graph *tg = get_topology_graph(0);
	struct drand48_data randBuffer;
	srand48_r(1,&randBuffer);
	double x;
	for(int i=0; i<num_total_iters; i++) {
		for(int j=0; j<num_of_nodes; j++) {
			drand48_r(&randBuffer,&x);
			VERTEX_VALUE_TYPE sample_value = get_sample(j, tg, x, global_array_partitioned);
			update_global_array(j, sample_value, tg);
		}
		for (int j = 0; j < num_of_nodes; j++) {
			if (global_array_partitioned[j] == 0) {
				global_array_copy_partitioned[j] = global_array_copy_partitioned[j] + 1;
			}
		}
	}
}


/*
 * Create and init global array by partitions using balance worksize by thread.;
 * split the vertices over the  partitions and allocat 2-d array:
 * nxm , n -> number of sockets, m, partition size.
 * partition size assumes that the last socket will keep the rest
 * of vertices when the number of vertices can not be totally balanced
 * @PARAM: graph file pointer
 * @RETURN 0-> error, 1 -> success
 * @Tere
 */
ITERATOR_INDEX initialize_global_arrayByPartition(std::string &graphfileName,GraphFileMemoryMap & graphFileMapPtr) {

	ITERATOR_INDEX vertices  = 0;
	//vertices = getNumberOfVertices(graphFileMapPtr);

	vertices = getVerticesFromFile(graphFileMapPtr,graphfileName);

	num_of_nodes= vertices;

	//TODO
	num_sockets_to_use= totalProcess;

	//TODO: to read from alchemy
	numVars=2;
	numEdgeFactors=pow(numVars,2);
	topologySizeByVertex=numEdgeFactors+1;

	std::cout <<"\n Number of Vertices: "<<vertices;
	std::cout <<"\n Number of vars: "<<numVars;
	std::cout <<"\n Number of vars: "<<numEdgeFactors;
	if (vertices == 0 ){
		std::cout <<"\nError,Invalid  number of vertices, verify input file";		
		return 0; /*invalid number of vertices*/
	}

	if (vertices<num_sockets_to_use){
		std::cout <<"Global array partition can not be done. The number of sockets is more than the data elements.";
		return 0;
	}

	/*local states*/
	global_array_partitioned  = (VERTEX_VALUE_TYPE*)customMalloc(num_of_nodes * sizeof(VERTEX_VALUE_TYPE), 0);
	memset(global_array_partitioned, 0, num_of_nodes * sizeof(VERTEX_VALUE_TYPE));

	/*local counts*/
	global_array_copy_partitioned  = (VERTEX_AGG_VALUE_TYPE*)customMalloc(num_of_nodes * sizeof(VERTEX_AGG_VALUE_TYPE), 0);
	memset(global_array_copy_partitioned, 0, num_of_nodes * sizeof(VERTEX_AGG_VALUE_TYPE));

	/*local distribution*/
	global_array_distribution_partitioned  = (FLOAT_AGG_VALUE_TYPE*)customMalloc((num_of_nodes+1) * sizeof(FLOAT_AGG_VALUE_TYPE), 0);
	memset(global_array_distribution_partitioned, 0, num_of_nodes * sizeof(FLOAT_AGG_VALUE_TYPE));


	/*local convergence statistics*/
	global_array_conv_partitioned  = (FLOAT_AGG_VALUE_TYPE*) customMalloc(num_of_nodes * sizeof(FLOAT_AGG_VALUE_TYPE), 0);
	for (int k=0; k <num_of_nodes; k++){
		global_array_conv_partitioned[k]=3.0;
	}

	global_array_nonConv_partitioned = (VERTEX_AGG_VALUE_TYPE*)customMalloc(num_of_threads*num_of_stat_samples*sizeof(VERTEX_AGG_VALUE_TYPE), 0);
	memset(global_array_nonConv_partitioned, 0, num_of_stat_samples*num_of_threads* sizeof(VERTEX_AGG_VALUE_TYPE));


	/*array to store global state distributions*/
    sizeGlobalArrayDistribution = (ITERATOR_INDEX*)malloc(num_sockets_to_use*sizeof(ITERATOR_INDEX));

    /*array to read global states sizes*/
	readPartitionSizes(graphfileName+PARTITIONSIZE,num_sockets_to_use,sizeGlobalArrayDistribution);

	std::cout <<"\n Graph partitions:";

	for(int i=0; i<num_sockets_to_use; i++){
			//sizeGlobalArrayDistribution[i]=5000000;
	     std::cout <<"\npartition of socket "<<i<<" is "<< sizeGlobalArrayDistribution[i];
	}

	#ifdef DEBUG_GRAPHSTATS
	std::cout <<"\n**Partition Stats                ";
	std::cout <<"\n \tSocket  to use:                "<<	num_sockets_to_use;
	std::cout <<"\n \tThreads to use:                "<<	num_of_threads;
	std::cout <<"\n \tThreads by Socket:             "<<numberOfThreadsBySocket;
	std::cout <<"\n \tThreads by last Socket:        "<<lastSocketThreads;
	std::cout <<"\n \tuniform array size by thread:  "<<vertices/num_of_threads;
	std::cout <<"\n \tuniform array size by socket:  "<<partitionSizeBySocket;
	#endif

	char buffer [1000];
	memset(buffer,0,1000);

	/*MASTER creates the share global arrays*/
	if (processId==MASTERNODE){
		for (int p=0; p<num_sockets_to_use; p++){

                        string name;
			getFilename(fileName,name);
			sprintf(buffer,"%s.%d_%d",name.c_str(),num_sockets_to_use,p);
			string pFile(buffer);

			createGlobalArray(pFile+".states",sizeGlobalArrayDistribution[p]*sizeof(VERTEX_VALUE_TYPE),
					reinterpret_cast<char*>(global_array_partitioned));
			//+1 to add iteration number add at the end
			createGlobalArray(pFile+".pdist",(sizeGlobalArrayDistribution[p]+1)*sizeof(FLOAT_AGG_VALUE_TYPE),
					reinterpret_cast<char*>(global_array_distribution_partitioned));
			/**
			 * add array to save statistics*/
			createGlobalArray(pFile+".stats",3*sizeof(ITERATOR_INDEX),reinterpret_cast<char*>(runtimeStats));
		}
	}

	int fd=0;
	/*barrier to wait for the master to create files*/
	globalBarrier(processId, totalProcess, 1);
	for (int p=0; p<num_sockets_to_use; p++){
			string name;
                        getFilename(fileName,name);
                        sprintf(buffer,"%s.%d_%d",name.c_str(),num_sockets_to_use,p);
			string pFile(buffer);			
			globalArrayMap(pFile, fp[p],sizeGlobalArrayDistribution[p],p);
			if (p==processId) /*maps only local statsfile*/
				anyArrayMap(pFile+".stats", fd,3*sizeof(ITERATOR_INDEX));	
			globalArrayDistributionMap(pFile, fp[p],sizeGlobalArrayDistribution[p],p);
	}


	/*initialize worker indices to determine vertex id to start and end*/
	startThreadIndex = (ITERATOR_INDEX*)malloc(num_sockets_to_use*sizeof(ITERATOR_INDEX));
	endThreadIndex   = (ITERATOR_INDEX*)malloc(num_sockets_to_use*sizeof(ITERATOR_INDEX));

	startThreadIndex[0]= 0;
	endThreadIndex[0]  = sizeGlobalArrayDistribution[0];
	std::cout<<"\n Node work size:";
	std::cout <<"\ns:"<<startThreadIndex[0] <<"\t e:"<<endThreadIndex[0];

	for (int m=1; m<num_sockets_to_use; m++){
		startThreadIndex[m]    = endThreadIndex[m-1];// + sizeGlobalArrayDistribution[m];
		endThreadIndex[m]      = endThreadIndex[m-1] + sizeGlobalArrayDistribution[m];

		std::cout <<"\ns:"<<startThreadIndex[m] <<"\t e: "<<endThreadIndex[m];
	}
	return vertices;
}


void buildTopology(const std::string input_filename, int thread_index,struct topology_graph *tg,
		int num_nodes, GraphFileMemoryMap &localGraphFile) {

		int  numaNode            = numaNodes[thread_index];
		int *factor_count_arr    = localGraphFile.factor_count_arr;
		ITERATOR_INDEX  vertexNESize;
		ITERATOR_INDEX localTopologySize=0;
		ITERATOR_INDEX topologyArraySize = 0;


		tg->vertex_array = (struct vertex_info *)customMalloc(num_nodes*sizeof(struct vertex_info), numaNode);
		for(ITERATOR_INDEX i=0; i<num_nodes; i++) {

			vertexNESize = factor_count_arr[i]+localGraphFile.numVars; //-1 to remove unary factor as neigborn
			topologyArraySize += vertexNESize;
			tg->vertex_array[i].length = vertexNESize;
			tg->vertex_array[i].global_value_ptr = get_vertex_global_pointerByVariedSizePartition(i, numaNode);
		}

		localGraphFile.lastVertexLength = vertexNESize;
		localGraphFile.topologyArraySize = topologyArraySize;
		std::cout<< "\n Total topology_array size:"<< topologyArraySize << " for node...."<< numaNode;

		//updating local pointers of the local topology only
		for (ITERATOR_INDEX i = startThreadIndex[numaNode]; i<endThreadIndex[numaNode]; i++){
			tg->vertex_array[i].first_factor_offset = localTopologySize;
			localTopologySize += tg->vertex_array[i].length;
		}

		std::cout<< "\n Calculated LOCAL topology size:"<< localTopologySize << " for node...."<< numaNode;
		ITERATOR_INDEX totalNodes  =		endThreadIndex[numaNode]-startThreadIndex[numaNode];
		std::cout<< "\n Calculated LOCAL VERTICES size:"<< totalNodes<< " for node...."<< numaNode;
		tg->topology_array = (union topology_array_element*)customMalloc(localTopologySize*sizeof(union topology_array_element),numaNode);
		getNeigborns(localGraphFile,(int64_t*)tg->topology_array, tg->vertex_array,num_nodes, numaNode);

		//print topology for debugging purpose
	/*	std::cout <<"\nPrint topology: "<<
		for (int i =0;  i< localTopologySize;i++){
			std::cout <<" "<< tg->topology_array[i];
		}
	std::cout <<"\nEnd topology: \n";*/


}
/**
 * Parse input file in order to load nodes, neighbors and factors.
 * some limiting assumptions made here!
 * assuming the node IDs start with a 0 (so the index itself can work out as node IDs too.
 * @PARAMS: input filename, topology for the sokcet, total of vertex
 * @RETURN: nothing
 */
void initializeAndreadCounters(const std::string input_filename, int thread_index,struct topology_graph *tg,
		ITERATOR_INDEX num_nodes, GraphFileMemoryMap &localGraphFile) {

	FACTOR_VALUE_TYPE_INFLATED *unaryfactors=NULL;
	int  numaNode         = numaNodes[thread_index];

	tg->factor_tables  = NULL;
	tg->topology_array = NULL;
	tg->vertex_array   = NULL;

	std::cout <<"\n Allocating counters arrays..." <<numaNode<<"\n";

	// create an array to count the number of neighbors of each node.
	localGraphFile.factor_count_arr = (int *)malloc(num_nodes*sizeof(int));
	memset(localGraphFile.factor_count_arr, 0,num_nodes*sizeof(int));

	/*allocate unary factors*/
	unaryfactors =(FACTOR_VALUE_TYPE_INFLATED*)malloc(num_nodes*numVars*sizeof(FACTOR_VALUE_TYPE_INFLATED));
	memset(unaryfactors,0,num_nodes*numVars*sizeof(FACTOR_VALUE_TYPE_INFLATED));

	std::cout <<"\n Reading edge factors..";
	getEdgeNeigbornsCounter(localGraphFile,localGraphFile.factor_count_arr, num_nodes);

	// populate info about the factors,
	//by default this choice has been desactivated.
	if (USEDFACTORTABLE){
		std::cout <<"\n Initializing factor table";
		int factor_table_length = (NUM_OF_UNARY_FACTORS*UNARY_FACTOR_LENGTH) + (NUM_OF_BINARY_FACTORS*BINARY_FACTOR_LENGTH);
		tg->factor_tables       = (FACTOR_VALUE_TYPE_INFLATED*)customMalloc(factor_table_length*sizeof(FACTOR_VALUE_TYPE_INFLATED), numaNode);
		populate_factors(tg->factor_tables );
	}

	if (unaryfactors!=NULL){
		free(unaryfactors);
		unaryfactors=NULL;
	}
}

/*Read labels for the vertices
 * The input file assumes to be the same name +.labels
 * */
void readPriors(std::string fileName, VERTEX_VALUE_TYPE *global_array_partitioned, int numNodes){

	std::ifstream fPriors((fileName+LABELS).c_str());
	std::string line;
	ITERATOR_INDEX vertexId =0;
	VERTEX_VALUE_TYPE priorVal;
	char *p=NULL;
	ITERATOR_INDEX vPos=0;

	std::cout <<"\nreading labels:"<<(fileName+LABELS);
	if (fPriors.good()){
			//read variables
		while(getline(fPriors, line)) {
			p =strtok (&line[0], " ");
			vertexId =atol (p);
			p= strtok (NULL, "/n");
			priorVal = atol(p);

			global_array_partitioned[vertexId]=priorVal;
			//todo
			tgMaster->vertex_array[vertexId].label=true;

			//find the correct file
	
//			for(p_i=0; p_i<totalProcess; p_i++){
//				if (vertexId<endThreadIndex[p_i]){
//				break;
//				}
//			}
	
			if (vertexId>=startThreadIndex[processId] && vertexId<endThreadIndex[processId]){	
				vPos = vertexId- startThreadIndex[processId];				
//				std::cout <<"\n\nupdating  prior for v ="<<vertexId<<" in p ="<<processId;
				global_array_shared[processId][vPos]=priorVal;
			}
			

		}
	}else{
		std::cout <<"\nLabels SKIPPED...";
	}

	fPriors.close();
}


/**
 * Read alchemy file in order to
 * build the graph data structures in-memory:
 * Graph Topology fore a given thread.
 * The thread is the creating a copy for
 * specific NumaNode.
 * @PARAMS: Thread Index
 * @RETURN: Null
 */
void *populate_socket_topology_info(void* arg) {
	int threadIndex = *(int*)arg;

#ifdef NUMALLOC
	struct bitmask* mask = numa_allocate_nodemask();
	numa_bitmask_setbit(mask, numaNodes[threadIndex]);
	numa_bind(mask);
	numa_set_membind(mask);
	numa_set_strict(numaNodes[threadIndex]);
	numa_run_on_node(numaNodes[threadIndex]);*/
#endif

	std::cout <<"\n.............START loading for thread/socket.............."<<threadIndex<<"/"<<numaNodes[threadIndex];
	//numa allocation of the topology for local socket
	tgMaster = (struct topology_graph*)customMalloc(sizeof(struct topology_graph), numaNodes[threadIndex]);


	/*read vertices degree and compute topology sizes*/
	initializeAndreadCounters(fileName, threadIndex, tgMaster, num_of_nodes,graphFileMap[threadIndex]);

	/*build topology index in memory*/
	buildTopology(fileName, threadIndex, tgMaster, num_of_nodes,graphFileMap[threadIndex]);

	std::cout <<"\n.............END loading for socket.............."<<threadIndex<<"/"<<numaNodes[threadIndex];
	return(NULL);
}


/*
 * Loads the graph from an alchemy file
 * and create in-memory data numa aware data structures
 *
 * @PARAMS:inpt file name
 * @1= SUCESS, 0=ERROR
 * */


bool graphLoader(std::string fileName){

	struct timeval tv;
	struct timeval start_tv;
	double graph_loadtime = 0;

	pthread_t socket_threads[num_sockets_to_use];

	//Init Global Array of Data copies
	//tgMaster = (struct topology_graph**) customMalloc(num_sockets_to_use * sizeof(struct topology_graph*), MASTERNODE);
	graphFileMap= (struct GraphFileMemoryMap*) malloc(num_sockets_to_use*sizeof(GraphFileMemoryMap));

	std::cout <<"\n	Initializing the array of graph file readers";

	for (int s=0; s<num_sockets_to_use; s++){

		graphFileMap[s].byteSize     = 0;
		graphFileMap[s].num_of_edges = 0;

		std::stringstream parName;
		parName << fileName;// <<"."<< s;
		graphFileMap[s].fileName=(char *)malloc(parName.str().size()+1);
		strcpy(graphFileMap[s].fileName,parName.str().c_str());

		//graphFileMap[s].fileName[parName.str().size()]="\0";
		readGraphHeaders(graphFileMap[s]);
	}

	//loadGraphFile((char*)fileName.c_str(), graphFileMap[MASTERNODE]);
	std::cout <<"\n file to load:"<<fileName;
	std::cout <<"\n file size:"<<graphFileMap[MASTERNODE].byteSize;

	std::cout <<"\n\n===== Starting graph Loading ===================\n";

	gettimeofday(&start_tv, NULL);


	num_of_nodes =  initialize_global_arrayByPartition(fileName,graphFileMap[MASTERNODE]);


	if(	num_of_nodes == 0 ){
		std::cout <<"\nError, number of VERTEX = 0 or file does not exists, we can not continue.";
		return false;
	}


	/*load graph topology from file in parallel, usign m threads: num_sockets_to_use */
	std::cout << "\nSTART loading, sockets to use: " <<num_sockets_to_use<< std::endl;
	//TODO
//	for (int i = 0; i < num_sockets_to_use; i++) {
	int i=processId;
	//only for the processid
		if (pthread_create(&socket_threads[i], NULL, populate_socket_topology_info, new int(i))) {
				std::cout << "Cannot create thread " << i << std::endl;
		}
//	}
//	for (int i = 0; i < num_sockets_to_use; i++) {
		pthread_join(socket_threads[i], NULL);
//	}

	gettimeofday(&tv, NULL);
			//loadGraphFile((char*)fileName.c_str(), graphFileMap);
	graph_loadtime = (tv.tv_sec - start_tv.tv_sec) +
	(tv.tv_usec - start_tv.tv_usec) / 1000000.0;


	//pthread_barrier_destroy(&barrier1);
	//read labels for the graph

	//TODO
	readPriors(fileName,global_array_partitioned,  num_sockets_to_use);

	std::cout <<"\n **TIME parse and building topologies in parallel: " <<graph_loadtime;
	std::cout << "\nEND loading on sockets: " <<num_sockets_to_use<< std::endl;


#ifdef N
	/*std::cout <<"\n --------------Topology array:--------------------";
	ITERATOR_INDEX checksum =0;
	ITERATOR_INDEX topologyIndex=0;
	for (int s=0; s<num_sockets_to_use; s++){
		topologyIndex=0;

		for (ITERATOR_INDEX i=startThreadIndex[s]; i<endThreadIndex[s]; i++){

			for (ITERATOR_INDEX j=0; j<tgMaster[s]->vertex_array[i].length; j++ ){
				if (i<(startThreadIndex[s]+2 )|| i>(endThreadIndex[s]-2)){
					std::cout <<"\n "<<i <<" -"<<tgMaster[s]->topology_array[topologyIndex].factor_value;
				}
				checksum += tgMaster[s]->topology_array[topologyIndex].factor_value;
				topologyIndex++;
			}
		}
		std::cout <<"\nCHECKSUM="<<checksum<<"............socket ="<<s;
		std::cout <<"\npartitionsize"<<sizeGlobalArrayDistribution[s];
	}*/
	std::cout <<"\n Enter";
	int a;
	std::cin>>a;
#endif
	/*unload file and release file memory*/
    //numa_free(graphFileMap[MASTERNODE].temp_topology_global,  graphFileMap[MASTERNODE].topologyArraySize*sizeof(TOPOLOGY_VALUE_TYPE));
	//numa_free(graphFileMap[MASTERNODE].topology_offset_global, graphFileMap[MASTERNODE].num_of_nodes*sizeof(TOPOLOGY_VALUE_TYPE));

	return true;
}

/**
 * Release the topology in cascade
 * @PARAMS: Nothing
 * @RETURN: Nothing
 */

void releaseTopology(){

	std::cout <<"\nReleasing topology";

	if (tgMaster!=NULL){
				if (tgMaster->factor_tables !=NULL){
					customFree(tgMaster->factor_tables, (NUM_OF_UNARY_FACTORS*UNARY_FACTOR_LENGTH) + (NUM_OF_BINARY_FACTORS*BINARY_FACTOR_LENGTH)*sizeof(FACTOR_VALUE_TYPE_INFLATED));
				}

				if (tgMaster->vertex_array != NULL) {
					customFree(tgMaster->vertex_array, num_of_nodes*sizeof(vertex_info));
				}

				if (tgMaster->topology_array != NULL) {
					customFree(tgMaster->topology_array, topologyArraySize*sizeof(topology_array_element));
				}
				customFree(tgMaster, sizeof(topology_graph));
			}
}
/**
 * Release global vertex arrays
 * @PARAM:Nothing
 * @RETURN:Noting
 */
void releaseGlobalVertexArray(){

	if (global_array_partitioned != NULL){
		customFree(global_array_partitioned,sizeGlobalArrayDistribution[processId]*sizeof(VERTEX_VALUE_TYPE));
		global_array_partitioned = NULL;
	}

	if (global_array_copy_partitioned != NULL){
		customFree(global_array_copy_partitioned,sizeGlobalArrayDistribution[processId]*sizeof(VERTEX_AGG_VALUE_TYPE));
		global_array_copy_partitioned = NULL;
	}


	if (global_array_conv_partitioned != NULL){
		customFree(global_array_conv_partitioned,num_of_nodes * sizeof(FLOAT_AGG_VALUE_TYPE));
		global_array_conv_partitioned =NULL;
	}


	if (sizeGlobalArrayDistribution!=NULL){
		std::cout <<"\nReleasing distribution";
	  	customFree(sizeGlobalArrayDistribution,num_sockets_to_use*sizeof(ITERATOR_INDEX));
		//sizeGlobalArrayDistribution=NULL;
	}

	if (startThreadIndex!=NULL){
		//numa_free(startThreadIndex, num_sockets_to_use*sizeof(ITERATOR_INDEX));
		free(startThreadIndex);
		startThreadIndex = NULL;
	}

	if (endThreadIndex!=NULL){
		//numa_free(endThreadIndex,num_sockets_to_use*sizeof(ITERATOR_INDEX));
	  free(endThreadIndex);
	  endThreadIndex = NULL;
	}
       
	if (numaNodes!=NULL){
	     free(numaNodes);
	     numaNodes = NULL;
	}
	if (cpuid_counter_array!=NULL){
	   free(cpuid_counter_array);
	    cpuid_counter_array = NULL;
	}

	///Todo:
	//upmap local nvram partition.
}



/*Method to get the memory usage for the current execution
 * @Params: by reference
 * 1. vm_usage: virtual memory usage
 * 2. RSS memory (resident memory)
 * */
void process_mem_usage(float& vm_usage, float& resident_set)
{
    vm_usage     = 0.0;
    resident_set = 0.0;

    // the two fields we want
    unsigned long vsize;
    long rss;
    {
        std::string ignore;
        std::ifstream ifs("/proc/self/stat", std::ios_base::in);
        ifs >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore
                >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore
                >> ignore >> ignore >> vsize >> rss;
    }

    long page_size_kb = sysconf(_SC_PAGE_SIZE) / 1024; // in case x86-64 is configured to use 2MB pages

    /*convertig to gigabytes*/
    vm_usage = (vsize / 1024.0)/1024/1024;
    resident_set = (rss * page_size_kb)/1024.0/1024;
}


/*******************************************************************************
 * Output Methods:
 * Write vertex states, timers, convergence statistcs
 * ******************************************************************************/
/**
 * Write the inference result to file
 *
 */
void  writeGlobalArray(std::string outFileName){

	std::ofstream outResults;
	outResults.open(outFileName.c_str());

	if (outResults.good()){
		for (int k =0; k <sizeGlobalArrayDistribution[processId]; k++){
			outResults <<	global_array_partitioned[k];
			outResults <<std::endl;
		}
	}
	outResults.close();
}
/*
 * write convergence results to a fiel
 * */
void writeConvergenceStats(std::string fileName){

	size_t checkSum=0;
	ofstream outStats;
	outStats.open((fileName+".conv").c_str());
	ITERATOR_INDEX totalConvergeStat = 0;

	if (outStats.good()){

		for (int i =0; i <num_of_stat_samples; i++){
			totalConvergeStat=0;
			for (int m=0; m <num_of_threads; m++){
				int pos=m*num_of_stat_samples+i;
				totalConvergeStat +=global_array_nonConv_partitioned[pos];
			}
			//outStats<<snapshot_sample_size[i]<<","<<snapshot_timestamp[i]<<","<<convergenceStat[i] <<"\n";
			outStats<<totalConvergeStat <<"\n";
			checkSum += totalConvergeStat;
		}
		//outStats  <<"\n nonConv checksum="<<checkSum<<"\n";
		//std::cout <<"\n nonConv checksum="<<checkSum<<"\n";
	}
	outStats.close();
}
/**
 * Write the state counter distribution, numbe of zeros for each vertex
 * */
void writeInferencesStates(std::string fileName){

	ofstream out_file;
	out_file.open((fileName + ".tsv").c_str());


	if (out_file.good()){

		size_t checkSum=0;
		VERTEX_AGG_VALUE_TYPE * g_copy=global_array_copy_partitioned;

		for (int j=0; j<num_of_nodes; j++){
				out_file <<g_copy[j] << "\n";
				checkSum += g_copy[j];
		}
		//std::cout <<"\nZero Counts checksum:"<<checkSum;
		out_file.close();
	}
}
/*
 * Create file statistics: time and memory
 * */
void writeTimerAndMemoryStatistics(std::string &fileName, timeval &start_tv,timeval &tv, double  graph_loadtime, std::string &confLine,std::string &inputFile){
	float vm_usage=0;
	float resident_set=0;
	process_mem_usage(vm_usage, resident_set);

	ofstream timer_file;

	timer_file.open((fileName + ".timer").c_str());

	if (timer_file.good()){
	double elapsed = (tv.tv_sec - start_tv.tv_sec) +
			  (tv.tv_usec - start_tv.tv_usec) / 1000000.0;
			timer_file << "Graph Load Time(sec) = " << graph_loadtime << std::endl;
			timer_file << "Gibbs Run Time(sec)  = " << elapsed << std::endl;
			timer_file << "VMEM usage (GB)      = " << vm_usage <<std::endl;
			timer_file << "RSS MEM usage(GB)    = " << resident_set <<std::endl;
			timer_file  << "Input Configuration = " << confLine <<std::endl;
			timer_file  << "Output file         = " << fileName <<std::endl;
			timer_file  << "Graph input file    = " << inputFile  <<std::endl;
			timer_file   <<"sync time(sec)      = "	<< syntime	<< std::endl;

			std::cout <<"\n\n===== Summary of Execution =====================\n";
			std::cout  << "Graph Load Time(sec) = " << graph_loadtime << std::endl;
			std::cout  << "Gibbs Run Time(sec)  = " << elapsed   << std::endl;
			std::cout  << "VMEM usage (GB)      = " << vm_usage  << std::endl;
			std::cout  << "RSS MEM usage(GB)    = " << resident_set << std::endl;
			std::cout  << "Input Configuration  = " << confLine  <<std::endl;
			std::cout  << "Output file          = " << fileName   <<std::endl;
			std::cout  << "Graph input file     = " << inputFile  <<std::endl;
			std::cout   <<"sync time (sec)      = " << syntime	<< std::endl;
	}
	timer_file.close();
}

/*****************************************************************
 * Engine Methods
 * Methods to read configurations, run the model, generate output
 * Main launch_gibbs_sampler
 *****************************************************************/


/*
 * Main to read configuration file and all the flow of the inference
 * Fist read configuration,  execute the graph loader to create
 * graph in-memory, execute the inference and then generate output statistics
 * This method support multiple configurations for the same data load.
 * @PARAMS: configuration file name, graph file input
 * @RETURN: nothing
 * */
void launch_gibbs_sampler(std::string parameters_filename, std::string graphFile, int curProcessId, int allProcess, std::string fileSystemPath) {

		struct timeval tv;
		struct timeval start_tv;
		std::ifstream fin(parameters_filename.c_str());
		std::string line;
		std::string output_file;
		size_t line_number = 1;
		double graph_loadtime = 0;
		bool loadedGraph =false;
		int  runExperimentId = 0;


		/*command line configuration*/
		FSPath    = fileSystemPath;
		fileName  = graphFile;
		processId = curProcessId;
		totalProcess= allProcess;



		/*read header and ignore: header is only use for user info*/
		if (fin.good()){
			getline(fin, line, line_number++);
		}

		while(fin.good() && getline(fin, line, line_number++)) {

			// BUG: Multiple lines in the configuration file causes
			// an unhandled fault, bailout if there is more than one line
			if(runExperimentId > 0) {
				std::cout <<" \nWARNING: multiple lines in config file are not supported, bailing !!!";
				return;
			}

			std::cout <<"\n=================== Execution configuration:\n";
			if(trim(line).length() == 0) {
				continue; /*ignore empty line*/
			}

			/*to load inference configuration*/
			loadConfigurationParameters(line,output_file, line_number);

  		    std::cout <<"\n=================== Adding process synchronization:\n";
			initGlobalBarrier(totalProcess,fileName);

			/*to load the graph input*/
			if (!loadedGraph){
				gettimeofday(&start_tv, NULL);
				loadedGraph = graphLoader(graphFile);
				gettimeofday(&tv, NULL);

				if (loadedGraph== false){
					std::cout <<" \nERROR: Graph loader and builder has failed. Check your input settings";
					return;
				}

				graph_loadtime = (tv.tv_sec - start_tv.tv_sec) +
						(tv.tv_usec - start_tv.tv_usec) / 1000000.0;
				std::cout <<"\n=>Total Loading time:" <<graph_loadtime;
			}


			globalBarrier(processId, totalProcess,2);
			/* now start Gibbs Sampling.*/
			std::cout <<  "\n\n===== Begin Inference!!! =====================\n"
					  << " # of threads = " << num_of_threads << std::endl
					  << " Threshold = " << threshold << std::endl;

			gettimeofday(&start_tv, NULL);
			//generate_statistics=false;
			launch_sequential_parallel();

			gettimeofday(&tv, NULL);
			//*clean loading barrier*/
			clearBarrier(processId);


			std::cout <<"\nEND COMPUTATION\n";
			std::cout <<  "\n\n===== Generating output =====================\n";

			/* execution statistics*/
			/*adding number of experiment*/
			//experimentName << output_file <<"_"<< runExperimentId <<"."<<processId;
			//string oFile(experimentName.str());

			char buffer[100];
			memset (buffer,0,100);

			sprintf(buffer,"%s_p%d_p%d",output_file.c_str(),totalProcess,processId);
			string oFile(buffer);

			/*write stats*/			
			writeTimerAndMemoryStatistics(oFile, start_tv,tv, graph_loadtime, line,fileName);
			/*local states*/
			writeInferencesStates(oFile);
			/*write out the convergence file*/
			if(generate_statistics) {
				writeConvergenceStats(oFile);
			}

			//reinitialize_global_arrayByPartition(num_of_nodes);
			runExperimentId++;
			std::cout <<"\n\nGoing to the next EXP..";
		}
	    fin.close();
		releaseTopology();
		releaseGlobalVertexArray();
}
