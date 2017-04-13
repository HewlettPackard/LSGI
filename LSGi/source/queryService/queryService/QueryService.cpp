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

//============================================================================
// Name        : QueryService.cpp
// Author      : gomariat
// Version     :
// Copyright   : Your copyright notice
// Description : Query service to read last state/distribution of graph
//============================================================================


#include <iostream>
#include <libpmem.h>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "ServerSocket.h"



int SERVICEPORT=58000;

std::string FSPath;

int startService(std::string datasetfile, int nProcess){

	
	std::cout <<"\n\n----------------------------------------------------\n";
	std::cout <<"              Graph Query Service for LSGi           \n";
         std::cout<<"-----------------------------------------------------\n";

	std::cout <<" Graph:  [ " << datasetfile << " ]\n Shared on [" <<nProcess<< " nodes ]\n";
	/*Load pointers to the dataset*/
	initGlobalStates(datasetfile, nProcess);

	/*start the socket:port enable queries*/
	StartServer(SERVICEPORT);

	/**/
	//finalize();

	return 0;

}

/*
*Parse user parameters to override
*default values for:
 Shared File System  & port  & service host
*/
void  parseUserParams(int argc,char * argv[]){

     int i = 3;

	while(i<argc){
	 	std::string a(argv[i]);
        	 if (a.find(std::string("-FS=")) != std::string::npos) {
                         FSPath = (a.substr(std::string("-FS=").size()));
                         if (FSPath.length() == 0) {
                             std::cerr << "Invalid Shared file Systemt, it will use default:/lfs/" << std::endl;
                             FSPath="/lfs/";
			}
                 }
                else  if (a.find(std::string("-port=")) != std::string::npos) {
                         SERVICEPORT = atoi(a.substr(std::string("-port=").size()).c_str());
                         if (SERVICEPORT == 0) {
                             std::cerr << "Invalid Port number, it will default 58000" << std::endl;
			     SERVICEPORT = 58000;                             
			}
                 }
                i++;
        }
}
/*
 * Main Driver for the query services
 * We will have one query services for each dataset.
 * The name of dataset will be send by command line
 * We will be using the same configuration file than the engine
 * to know number of process running on the dataset which determine
 * the states to map
 *
 * */
int main(int argc, char* argv[]) {

	if(argc<3){
		std::cout <<"\nIncorrect Usage: <dataset to query ><nProcess> <-port=58000> <-FS=SharedFileSystem>)\n";
		std::cout <<"\nExample: ./QueryService gv20M 2";
		return 0;
	}

	std::string datasetFile(argv[1]);
	int nProcess = atoi(argv[2]);

	if (argc >=4) {
		/*parse default params*/
		parseUserParams(argc,argv);
	}else{
		/*use default params*/
	        FSPath ="/lfs/";
        	SERVICEPORT=58000;
	}
      
	startService(datasetFile,nProcess);

	return EXIT_SUCCESS;
}
