/**“© Copyright 2017  Hewlett Packard Enterprise Development LP

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

/*
 * \brief Inference configuration
 * The inference configuration include a set of parameters required to
 * perform the inference and the distributed process accros multiple nodes.
 *
 * We have divided 2 type of parameters:
 *
 * 1. inference configuration:
 * 	  parameters related to number of iterations, convergence metrics, e
 * 2. computing nodes configuration
 *    parameters related to the number of partitions, threads, size of the partitions
 *
 * */

#ifndef INFERENCECONFIG_H
#define INFERENCECONFIG_H

#include <iostream>
#include <cstring>
#include<string>
#include <vector>
#include <sstream>
#include<unistd.h>
#include<stdio.h>
#include<stdlib.h>
#include<stdint.h>
using namespace std;



/**********************************************
 * Socket/node and threads configuration
 **********************************************/
/*default value for number of sockets in which topology graph will be replicated.*/
#define NUMBER_OF_SOCKETS 4
// number of maximum CPU cores per socket
#define MAX_CORE_PER_SOCKET 10
// default value for number of total samples
#define NUMBER_OF_ITERS 200
// default value for number of threads to use for actual Gibbs sampling.
#define NUMBER_OF_THREADS 32
// default number of threads per socket (Based on the above values; NUMBER_OF_THREADS/NUMBER_OF_SOCKETS)
#define NUMBER_OF_THREADS_PER_SOCKET 8
// the way in which threads are supposed to access the topology array:
// 0=Round-Robin, 1=Partition, 2=Uniform_Thread_Workload (Fei's benchmark version)
#define TOPOLOGY_ARRAY_ACCESS_METHOD 1
//MASTER NODE to put the main pointer for initial socket for dimensional arrays like topology master
#define MASTERNODE 0

#define PMAX 80

/***********************************************
 * GLOBAL VARIALBES
 * for parameters that can be changed based on user input configuration
 ***********************************************/

/*initial path for the package*/
extern std::string FSPath;

/*input file name of graph in binary
 *this filename is used as root string for
 *getting other used file names, like priors, labels p*/
extern std::string fileName;

/**default input file names*/
/*file that includes the partition size, the line should be the same that computing nodes used*/
extern const std::string PARTITIONSIZE;
/*file that includes the know state for vertices*/
extern const std::string LABELS;


/*on/off to generate convergence statistics*/
extern bool generate_statistics;
/*number of samples to be done*/
extern int num_of_stat_samples;
/*number of iterations to execute the sampling*/
extern int stat_samples_interval;
/*threshold >= if the vetex has converged*/
extern double threshold;

/* default, shuffle every 100 updates*/
extern uint64_t kRefreshIntervalPercent;

/*Global variables for managing multi-threads processing*/
/*selected number of sockets to use*/
extern int num_sockets_to_use;
/*selected numa nodes to use, eg  {1,2}*/
extern int *numaNodes;
/* keeps track of the CPU ID to be used to stick the next thread on a socket to.*/
extern int *cpuid_counter_array;
/* to be used for partition method. Need to know the max number of threads to use per socket, according to the input params*/
extern int *threads_per_socket;
/*number of threads to use from the user*/
extern int num_of_threads;
/*number of threads that will be used per socket. This depends on the total number of threads and number of sockets used*/
extern int num_threads_per_socket;
/*total iterations to run*/
extern int num_total_iters;
/*Iteration counter*/
extern int ITER;

// The flag that will be used to determine what way to access the topology array in (round robin or partition).
// This is similar to TOPOLOGY_ARRAY_ACCESS_METHOD, which gives the default value.
// // 0=Round-Robin, 1=Partition, 2=Uniform_Thread_Workload (Fei's benchmark version)
/*TB refactored*/
extern int topology_array_access;


/*current process Identifier*/
extern int totalProcess;
/*the total number of processes running*/
extern int processId;
/*file descriptors for the input fies*/
struct nvMapFileDescriptor{
	int fd;
	int lenght;
};
/***********************************************************************
 * Method to read input parameters to execute the inference
 *********************************************************************/
/*
 * Load parameters configuration for the inference:
 * number of threads,number of iterations,number of sockets to use, number of samples, convergence threshold
 *
 *
 * **/
void loadConfigurationParameters(std::string line,std::string &output_file, int line_number);


#endif /*INFERENCECONFIG_H*/
