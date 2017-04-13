/* LSGi “© Copyright 2017  Hewlett Packard Enterprise Development LP

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
 * This code implements a iterative graph processing engine that
 * take advantange of The Machine architecture - Fabric Attached Memory-large number of cores
 * to scale inference processing on large  graphs.
 @Overview:
 *
 *
 *Graphical Inference is a remarkable algorithm for graph analytics
 * that abstracts knowledge combining probabilities and graph representations
 * to capture useful insights for solving problems like malware detection,
 * genomics, or online advertisement. These are analytics applications
 * that demands fast response. For example, in the security domain, a fast-time
 * response is required for malware detection (3-4 minutes), but state-of-art
 * solutions take close to an hour. Therefore, we need to exploit parallelism
 * to speed-up the runtime, but the main challenge is the system scalability.
 * As the size of graphs increase, the number of cores on a single commodity
 * server (8-32 cores) is not enough to achieve runtime targets consequently
 * more nodes are required.

 * We have build the graph inference engine which computes on graph partitions
 * mapped to multiple computing nodes. Each node performs iterative local processing
 * that needs to read remote outcomes which derives high communication volume over
 * the network and slows down runtimes on traditional clusters. To address this problem,
 * we have built a scalable large scale inference engine taking advantage of
 * The Machine architecture. Our memory-centric approach makes efficient use of the FAM
 * bandwidth and reduces synchronization overhead among computing nodes while exploiting
 * massive parallelism via the large number of cores on The Machine.
 *
 *@Reference Paper:
 * https://www.labs.hpe.com/publications/HPE-2016-101
 *@ Fei,Tere, Krishna, Nandish, Hideaki
 *@ Last update: April 10, 2017
 *
 *Quick Overview
 *Before compiling, install
 *Library Dependencies: libpmem,pthread
 *
 * To compile:
 *
 * use Makefile
 *
 * ./make clean
 * ./make
 */

#include<iostream>
#include<stdio.h>
#include<stdlib.h>
#include<string>
#include "inference.h"
using namespace std;

/**
 * Inference Engine Main Driver
 * @PARMAS: argv[1]= configuration file name
 * 			argv[2]= graph alchemy file name
 * 			argv[3]= update rafreshment rate (number of updates)
 * 			argv[4]= processid
 * 			argv[5]= total process
 * @RETURN : Exit
 */

int main(int argc, char*argv[]) {
	std::cout << "BEGIN!" << std::endl;

	if(argc < 5) {
		std::cout <<"\nError, incorrect input parameters\n"
		 << "\n Usage: <configurationFileName> <inputGraphFile> <processId> <TotalNumberiofProcesses>"
		 << "\n Please input a parameter comma separated file containing: " << endl
		 << " # of threads, # of samples/iterations, output filename, sample interval, convergence_threshold" << std::endl
		 << " Each line in the parameter file must have either the first 3 parameters, or all 5 parameters " << endl;
		return 1;
	}


    /**using default Shared File System*/
    std::string FSPath="/lfs/";

     if (argc >= 6) {
          std::string a3(argv[5]);
          if (a3.find(std::string("-FS=")) != std::string::npos) {
                 FSPath = (a3.substr(std::string("-FS=").size()));
                 if (FSPath.length()==0) {
                     std::cerr << "Invalid Shared file Systemt" << std::endl;
                     return 1;
                 }
          }
      }

     if (atoi(argv[3])<0 || atoi(argv[4])<0){
    	 std::cout <<"\nIncorrect processes identifiers. They should be positive numbers";
    	 return 1;
     }
     if (atoi(argv[4])<1){
    	 std::cout <<"\nIncorrect total number of processes. They should be at least one process.";
    	  return 1;
     }

     std::cout <<"\n===== Inference Parameters =====================\n"
    		   <<"\n input filename:         " << argv[2]
     	 	   <<"\n configuration filename: " << argv[1]
     	 	   <<"\n FAM Update frequency:   " << "1 per iteration"
	           <<"\n processID:              " << argv[3]
     	 	   <<"\n Total Process:          " << argv[4]
		       <<"\n Shared File System:"   << FSPath;


	/********************Launcher of inference processing**************************/

	launch_gibbs_sampler(argv[1],argv[2], atoi(argv[3]), atoi(argv[4]),FSPath);

	/*******************************************************************************/
	std::cout << "\nDONE!" << std::endl;
	return EXIT_SUCCESS;
}
