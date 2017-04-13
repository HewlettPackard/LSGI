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
#include <stdio.h>
#include <iostream>
#include <algorithm>
#include <string>
#include <ctime>
#include <time.h>
#include <sstream>
#include <fstream>
#include <assert.h>
#include <cstring>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "libpmem.h"
#include "globalStates.h"


#ifndef SEARCH_H
#define SEARCH_H

#define ITERATOR_INDEX    int64_t
#define VERTEX_VALUE_TYPE char
#define FLOAT_AGG_VALUE_TYPE float

#define PMAX 80

/*default file names*/
const std::string PARTITIONSIZE=".par";
extern VERTEX_VALUE_TYPE *global_array_shared[PMAX];
extern FLOAT_AGG_VALUE_TYPE *global_array_distribution_shared[PMAX];
extern ITERATOR_INDEX *runtimeStats[PMAX];
/*shared file pointer*/
nvMapFileDescriptor fp[PMAX];

/*list of partition indices by socket*/
ITERATOR_INDEX *sizeGlobalArrayDistribution = NULL;
ITERATOR_INDEX *startThreadIndex = NULL;
ITERATOR_INDEX *endThreadIndex = NULL;
ITERATOR_INDEX vertices=0;

int totalProcess = 0;

/*struct to manage vertex state indices*/
struct vertexState{
	ITERATOR_INDEX       vid;
	FLOAT_AGG_VALUE_TYPE p_0;
	VERTEX_VALUE_TYPE    state;
	int		     iteration;

};

/*struct to manage vertex statistics for each workinger*/
struct vertexStats{
	ITERATOR_INDEX   nonconverge;
	ITERATOR_INDEX   iteration;
	ITERATOR_INDEX   time;
	ITERATOR_INDEX   totalVertices;
	ITERATOR_INDEX   state0;
};

struct queryRequest {
	int type;
	int value;
};


/*Headers*/
void initGlobalStates(std::string graphfileName, int nProcess);
char findState(int vID);
int readPartitionSizes(std::string fileName, int num_sockets_to_use, ITERATOR_INDEX *sizeGlobalArrayDistribution);
inline bool getline(std::ifstream& fin, std::string& line, size_t line_number);
/*main query execusion*/
void doQueryByVid(int socketFd, int vId);
void doQueryThreshold(int socketFd, int threshold);
void doQueryThresholdStats(int socketFd, int threshold);
/*erro msg*/
void error(const char *msg);

/*
*get file name
*/
void getFilename(std::string path, std::string &fileName )
{
        const char *s = strrchr(path.c_str(), '/');
        if(s==NULL) {
                fileName = path;
        } else {
        fileName = strdup(s + 1);
        }
}
void initGlobalStates(std::string graphfileName, int nProcess){

	//std::cout <<"\n global states for dataset:"<<graphfileName
	//		  <<"\n for Nprocess= "<<nProcess <<"\n";

	totalProcess = nProcess;
	/*array to store global state distributions*/
    sizeGlobalArrayDistribution = (ITERATOR_INDEX*)malloc(nProcess*sizeof(ITERATOR_INDEX));

	/*array to read global states sizes*/
	readPartitionSizes(graphfileName+PARTITIONSIZE,nProcess,sizeGlobalArrayDistribution);

	std::cout <<"\n Partition Node  Size  \n";
	std::cout <<"------------------------------------";
	for(int i=0; i<nProcess; i++){
		vertices =vertices +sizeGlobalArrayDistribution[i];
	    std::cout <<"\n node [ "<<i<<" ]\t "<< sizeGlobalArrayDistribution[i];
	}

	std::cout <<"\n\n Shared Vertex States";	
	std::cout <<"\n----------------------------------";
	char buffer [1000];
	int fd=0;
	memset(buffer,0,1000);
	for (int p=0; p<nProcess; p++){
		       std::string name;
			getFilename(graphfileName,name);

				/*adding shared global array for shared instead between process*/
			sprintf(buffer,"%s.%d_%d",name.c_str(),nProcess,p);
			std::string pFile(buffer);
			std::cout <<"\n node [ "<<p<<" ]\t"<<pFile;	
			globalArrayDistributionMap(pFile, fp[p],sizeGlobalArrayDistribution[p]+1,p);
			anyArrayMap(pFile+".stats", fd,3*sizeof(ITERATOR_INDEX),p,runtimeStats[p]);
		}

	//anyArrayMap(pFile+".stats", fd,3*sizeof(ITERATOR_INDEX),p,reinterpret_cast<char*>(runtimeStats));
	/*initialize worker indices to determine vertex id to start and end*/
	startThreadIndex = (ITERATOR_INDEX*)malloc(nProcess*sizeof(ITERATOR_INDEX));
	endThreadIndex   = (ITERATOR_INDEX*)malloc(nProcess*sizeof(ITERATOR_INDEX));

	startThreadIndex[0]= 0;
	endThreadIndex[0]  = sizeGlobalArrayDistribution[0];
	//std::cout <<"\ns:"<<startThreadIndex[0] <<"\t e: "<<endThreadIndex[0];

	for (int m=1; m<nProcess; m++){
		startThreadIndex[m]    = endThreadIndex[m-1];// + sizeGlobalArrayDistribution[m];
		endThreadIndex[m]      = endThreadIndex[m-1] + sizeGlobalArrayDistribution[m];

		//std::cout <<"\ns:"<<startThreadIndex[m] <<"\t e: "<<endThreadIndex[m];
	}
}
/***/
/*
 * Read Partition Size
 * assumes that  there is an specific partition file by number of sockets
 * like graph1.par.8 which implies to read 8 socket partitions
 * */
int readPartitionSizes(std::string fileName, int num_sockets_to_use, ITERATOR_INDEX *sizeGlobalArrayDistribution){

	std::stringstream parName;
	parName << fileName <<"."<< num_sockets_to_use;

//	std::cout <<"\n Reading partitions file:"<< parName.str()<<"\n";
	std::ifstream fin(parName.str().c_str());
	std::string line;
        int line_number=-1;

	while(fin.good() && getline(fin, line, line_number++)){
//		line = trim(line);
		assert(line_number<num_sockets_to_use);
		sizeGlobalArrayDistribution[line_number]=(ITERATOR_INDEX)atoi(line.c_str());

	}

	fin.close();
        //printf("number of lines %d\n",line_number);
	assert(line_number==num_sockets_to_use);
	return line_number;
}

/**
 * Read statitis for the partition p
 * */
void readStatistics(int nProcess, vertexStats &vertexStatsObj){

	int maxTime=0;
	int minIter=1000000;/*large iter*/
	int nonConverge=0;

	std::cout << "\n\neading statistics";
	for (int i =0; i < nProcess; i++){
		pmem_invalidate(reinterpret_cast<char*>(runtimeStats[i]), (3)*sizeof(ITERATOR_INDEX));
		std::cout << "\n p:"<<i;
		std::cout << "\nnon converg:	" <<runtimeStats[i][0];
		std::cout << "\nITERATION:		" <<runtimeStats[i][1];
		std::cout << "\nelapsed time:	" <<runtimeStats[i][2];

		if (maxTime<runtimeStats[i][2])
			maxTime =runtimeStats[i][2];
		if (minIter>runtimeStats[i][1])
			minIter=runtimeStats[i][1];

		nonConverge+=runtimeStats[i][0];	}

	vertexStatsObj.iteration=minIter;
	vertexStatsObj.time = maxTime;
	vertexStatsObj.nonconverge = nonConverge;	
	vertexStatsObj.totalVertices = vertices;
	vertexStatsObj.state0=200010;
	std::cout <<"\nComputing stats:\n";
	std::cout <<"\nmax time:"<<maxTime<<"\tmaxIter"<< minIter<<"\tconver"<<nonConverge <<" v="<<vertices;
}

/*find the state of a given vertex id
 * @PARAMS:vid to search for, state results to return the value
 * @RETURN: nothing, state results is by reference
 * Comments:*/
void findState(int vId, vertexState &state){

	ITERATOR_INDEX vPos=0;
	int p=0;
	char vState=48;


	if (vId>=vertices){
		std::cout <<"Invalid vIndex:"<<vId<< " , max val"<<vertices;
		state.state 	= '*';
		state.p_0   	= 0;
		state.vid   	= vId;
		state.iteration = 0;
		return ;
	}

	for(p=0; p<totalProcess; p++){
		if (vId<endThreadIndex[p]){
			break;
		}
	}

	if (p ==totalProcess){
		std::cout <<"\ninvalid number of process";
		state.state = '*';
		state.p_0   = 0;
		state.vid   = vId;
		state.iteration = 0;
		return;
	}

	vPos = vId- startThreadIndex[p];
//	vPos =vId;

	pmem_invalidate(reinterpret_cast<char*>(global_array_distribution_shared[p]),
                                (1+(endThreadIndex[p]-startThreadIndex[p]))*sizeof(float));
	std::cout <<"\nVstate ="<<global_array_distribution_shared[p][vPos]<< " at pos=" << vPos <<" in part=" <<p << ".\n";

	state.p_0   = global_array_distribution_shared[p][vPos];
	state.vid   = vId;
	state.iteration =global_array_distribution_shared[p][sizeGlobalArrayDistribution[p]];

	if (global_array_distribution_shared[p][vPos]<0.5) {
		state.state = vState+ 1;
	}
	else{
		state.state = vState;
	}
}

ITERATOR_INDEX countNElements(float threshold){

	size_t counter=0;
	size_t index=0;
	size_t p=0;


	for(p=0; p<totalProcess; p++){

	pmem_invalidate(reinterpret_cast<char*>(global_array_distribution_shared[p]),
				(1+(endThreadIndex[p]-startThreadIndex[p]))*sizeof(float));
		for (index=0; index<(endThreadIndex[p]-startThreadIndex[p]); index++){
			if (global_array_distribution_shared[p][index]>=threshold){
				counter++;
			}
		}
	}

	return counter;
}

void findNElements(float threshold,  vertexState *vidResult, ITERATOR_INDEX n){

	ITERATOR_INDEX counter=0;
	ITERATOR_INDEX index=0;
	ITERATOR_INDEX p=0;
	ITERATOR_INDEX vid =0;
	int 		   iteration =0;


	std::cout <<"\n\t threshdold="<<threshold;
	for(p=0; p<totalProcess; p++){
		iteration = global_array_distribution_shared[p][sizeGlobalArrayDistribution[p]];

		
		for (index=0; index<(endThreadIndex[p]-startThreadIndex[p]); index++){
			if(counter>=n)
				break;
			if (global_array_distribution_shared[p][index]>=threshold){
				vidResult[counter].p_0 = global_array_distribution_shared[p][index];
//				if (v<5)
//					std::cout <<"\n p"<< p << "\tindex:"<<index<<""<<global_array_distribution_shared[p][index];

				vidResult[counter].vid = vid;
				vidResult[counter].iteration = iteration;

				if (global_array_distribution_shared[p][index]<0.5){
					vidResult[counter].state = 49;

				}else{
					vidResult[counter].state = 48;

				}
				counter++;
//				vid++;
			}else{
           //             std::cout <<"not found";
                        }
                       vid++;
		}
	}
}


void findAllElements(vertexState *vidResult){

	ITERATOR_INDEX index  = 0;
	ITERATOR_INDEX counter= 0;

	size_t p      = 0;
	int iteration = 0;

	for(p=0; p<totalProcess; p++){
		iteration = global_array_distribution_shared[p][sizeGlobalArrayDistribution[p]];
		for (index=0; index<(endThreadIndex[p]-startThreadIndex[p]); index++){
			vidResult[counter].p_0 = global_array_distribution_shared[p][index];;
			vidResult[counter].vid = index;
			vidResult[counter].iteration = iteration;
			if (global_array_distribution_shared[p][index]<0.5){
				vidResult[counter].state = 49;
			}else{
				vidResult[counter].state = 48;
			}
			counter++;
		}
	}
}


inline bool getline(std::ifstream& fin, std::string& line, size_t line_number) {
        return std::getline(fin, line).good();
}


void doQueryByVid(int socketFd, int vId){

	vertexState stateResponse;
	vertexStats statsResponse;	

	findState(vId, stateResponse);	
	write(socketFd,&stateResponse,sizeof(stateResponse));

	readStatistics(totalProcess, statsResponse);
	write(socketFd,&statsResponse,sizeof(statsResponse));
	
	std::cout <<"\n -> return state for vId:"<< vId <<" -state:"<<stateResponse.state<<"\t p_0"<<stateResponse.p_0<<"\n";
}
/*Retrieve elements above the threshold*/
void doQueryThreshold(int socketFd, int threshold){


	vertexStats statsResponse;
	vertexState *vidResult  = NULL;
	float thresholdPct         = threshold/100.0;
	ITERATOR_INDEX maxVal = 5;


	std::cout <<"\n Threshold Val="<<thresholdPct;
	/*count values*/
	
	readStatistics(totalProcess, statsResponse);

//        write(socketFd,&statsResponse,sizeof(statsResponse));
	
	ITERATOR_INDEX counter = countNElements( thresholdPct);

	statsResponse.state0=counter;
        write(socketFd,&statsResponse,sizeof(statsResponse));
	std::cout <<"\n counter Val="<<counter;
	write(socketFd,&counter,sizeof(counter));

	if (counter ==0 ){
		std::cout <<"\ncounter=0.";
		return;

	}

	vidResult = (vertexState*)malloc(counter*sizeof(vertexState));
		/*retrieve verticesids*/
	findNElements(thresholdPct,  vidResult,counter);

	for (int i =0; i<std::min(maxVal,counter); i++){
				std::cout <<"\nstate ="<<(vidResult+i)->state;
				std::cout <<"\nvId ="<< (vidResult+i)->vid;
				std::cout <<"\np_0 ="<< (vidResult+i)->p_0;
				std::cout <<"\niteration = "<<(vidResult+i)->iteration;

	}

	//write(socketFd,vidResult,counter*sizeof(vertexState));

	write(socketFd,vidResult,counter*sizeof(vertexState));

	if (vidResult != NULL ){
		free(vidResult);
	}
}

/*Retrieve overall statistics by threadhold above the threshold*/
void doQueryThresholdStats(int socketFd, int threshold){

        vertexStats statsResponse;
        float thresholdPct         = threshold/100.0;

        std::cout <<"\n Threshold Val="<<thresholdPct;
        /*count values*/

        readStatistics(totalProcess, statsResponse);
        ITERATOR_INDEX counter = countNElements( thresholdPct);

        statsResponse.state0=counter;
        write(socketFd,&statsResponse,sizeof(statsResponse));
	return;
}



/*Print error msg*/
void error(const char *msg)
{
    perror(msg);
    exit(1);
}

#endif /*SEARCH_H*/
