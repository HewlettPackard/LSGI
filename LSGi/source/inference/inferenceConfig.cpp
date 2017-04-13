
/*“© Copyright 2017  Hewlett Packard Enterprise Development LP

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

/*\brief Inference configuration
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

#include "inferenceConfig.h"
#include "utils.h"

/**Var Initialization*/

/*initial path for the package*/
std::string FSPath;

/*input file name of graph in binary
 *this filename is used as root string for
 *getting other used file names, like priors, labels p*/
std::string fileName;

/**default input file names*/
/*file that includes the partition size, the line should be the same that computing nodes used*/
const std::string PARTITIONSIZE=".par";
/*file that includes the know state for vertices*/
const std::string LABELS=".labels";


/*on/off to generate convergence statistics*/
bool generate_statistics  = true;
/*number of samples to be done*/
int num_of_stat_samples   = 1;
/*number of iterations to execute the sampling*/
int stat_samples_interval = 100;
/*threshold >= if the vetex has converged*/
double threshold          = 0.03;

/* default, shuffle every 100 updates*/
uint64_t kRefreshIntervalPercent = 1;

/*Global variables for managing multi-threads processing*/
/*selected number of sockets to use*/
int num_sockets_to_use;
/*selected numa nodes to use, eg  {1,2}*/
int *numaNodes=NULL;
/* keeps track of the CPU ID to be used to stick the next thread on a socket to.*/
int *cpuid_counter_array = NULL;
/* to be used for partition method. Need to know the max number of threads to use per socket, according to the input params*/
int *threads_per_socket=NULL;
/*number of threads to use from the user*/
int num_of_threads;
/*number of threads that will be used per socket. This depends on the total number of threads and number of sockets used*/
int num_threads_per_socket;
/*total iterations to run*/
int num_total_iters;
/*Iteration counter*/
int ITER = 1;

// The flag that will be used to determine what way to access the topology array in (round robin or partition).
// This is similar to TOPOLOGY_ARRAY_ACCESS_METHOD, which gives the default value.
// // 0=Round-Robin, 1=Partition, 2=Uniform_Thread_Workload (Fei's benchmark version)
/*TB refactored*/
int topology_array_access;


/*current process Identifier*/
int totalProcess=2;
/*the total number of processes running*/
int processId=0;


/*****************************************************************
 * Inference Configuartion Methods
 *
 *****************************************************************/


/*
 * Method to read input parameters to execute the inference
 * Load parameters configuration for the inference:
 * number of threads,number of iterations,number of sockets to use, number of samples, convergence threshold
 *
 *
 * **/
void loadConfigurationParameters(std::string line,std::string &output_file, int line_number){

	bool use_default = false;
	std::vector<std::string> params = split(line, ',');
	std::cout<<"\n User Configurations: "<<line;

	if(params.size() == 4) {
			generate_statistics = false;
	}else if(params.size() == 6 || params.size() == 7) {
		generate_statistics = true;
		stat_samples_interval = atoi(params[4].c_str());
		threshold = atof(params[5].c_str());
	}else {
		cout << "\n Line number " << line_number << " is invalid: " << line << endl;
		cout << "\n Using default values for the various parameters!!! " << endl;
		use_default = true;
	}

	if(use_default) {
				num_of_threads = NUMBER_OF_THREADS;
				num_total_iters = NUMBER_OF_ITERS;
				num_threads_per_socket = NUMBER_OF_THREADS_PER_SOCKET;
				topology_array_access = TOPOLOGY_ARRAY_ACCESS_METHOD;
				output_file = "gibbs-samples";
				generate_statistics = false;
				threshold = 0.03;
				stat_samples_interval = 100;
				/*initializing numa nodes*/
				num_sockets_to_use=1;
				numaNodes = (int*)malloc(sizeof(int));
				numaNodes[0] =0;
				cpuid_counter_array = (int*)malloc(sizeof(int));
				cpuid_counter_array[0] = 0;
				kRefreshIntervalPercent=100;
	}else {
				num_of_threads  = atoi(params[0].c_str());
				num_total_iters = atoi(params[1].c_str());
				output_file     = params[2];

				/*numasockets input*/
				/**
				 * NJ changed code. this in temporary until Tere uses the correct format.
				 */
				//num_sockets_to_use=  atoi(params[3].c_str());
				//TODO: remove
				num_sockets_to_use =totalProcess;
				std::cout <<"\n User selected total processes: "<<totalProcess<<"\n";
				numaNodes =  (int*)malloc(num_sockets_to_use*sizeof(int));
				cpuid_counter_array = (int*)malloc(num_sockets_to_use*sizeof(int));
				for (int i = 0 ; i <num_sockets_to_use; i++) {
					numaNodes[i] = i;
					cpuid_counter_array[i] = 0;
				}

				/**
				 * NJ added code to consider the method in which the topology array is accessed. Round-robin or partition:
				 * Tere must change it in future to be able to take this as a parameter. Hard coding the value from
				 * #define TOPOLOGY_ARRAY_ACCESS_METHOD for now!
				 */
				topology_array_access = TOPOLOGY_ARRAY_ACCESS_METHOD;
				if(topology_array_access == 1) {
					threads_per_socket = (int*)malloc(num_sockets_to_use*sizeof(int));
					int node_id_index;
					for(int thread_index=0; thread_index<num_of_threads; thread_index++) {
						node_id_index = thread_index % num_sockets_to_use;
						threads_per_socket[node_id_index] = 0;
					}
				}


				/*detecting refresh rate:*/
				std::string a3(line);
				if (a3.find(std::string("-refresh=")) != std::string::npos) {
					kRefreshIntervalPercent = atoi(a3.substr(a3.find("=")+1,line.size()).c_str());
				   if (kRefreshIntervalPercent == 0) {
					   kRefreshIntervalPercent=1;
					   std::cout <<"\n Invalid REFRESH RATE, then using default =100.";
				   }
				   std::cout <<"\n\trefresh rate to use="<<kRefreshIntervalPercent<< " per iteration\n";
				}


			}

			if(generate_statistics) {
				num_of_stat_samples   = num_total_iters/stat_samples_interval;
			}
}






