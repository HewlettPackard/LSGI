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

#include<iostream>
#include <libpmem.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>




/*
 * Main methos to manage inference states within the global array
 *
 * The states will be allocated in NVRam as shared mode
 * The methods allow to
 * 	1. create a file,
 * 	2. mapped in memory,
 * 	3. sync,
 * 	4. push from one local to shared and
 * 	5. pull from the shared to local
 *
 * */
#ifndef GLOBALSTATES_H
#define GLOBALSTATES_H

/*ON to debug sync states*/
//#define DEBUG_GB_ON

#define VERTEX_VALUE_TYPE char
#define ITERATOR_INDEX int64_t
#define FLOAT_AGG_VALUE_TYPE float

#define PMAX 80
/*
 * Descriptor to store file pointer and length
 * */
struct nvMapFileDescriptor{

	int fd;
	int lenght;
};


extern std::string FSPath;

VERTEX_VALUE_TYPE *global_array_shared[PMAX];
FLOAT_AGG_VALUE_TYPE *global_array_distribution_shared[PMAX];
ITERATOR_INDEX *runtimeStats[PMAX];


/*Headers*/
/*Method to map persistence structures in memory*/
 void globalArrayMap(std::string &fileName, nvMapFileDescriptor &nvmapFd, ITERATOR_INDEX arraySize );

/*Metohd to map in memory the global array*/
int globalArrayUnMap(VERTEX_VALUE_TYPE *globalArray, nvMapFileDescriptor &nvmapFd);


/*push updates*/
int globalArrayPushUpdates(ITERATOR_INDEX startIndex, ITERATOR_INDEX endIndex, VERTEX_VALUE_TYPE *localGlobalArray, VERTEX_VALUE_TYPE *sharedGlobalArray);

/*pull updates*/
int globalArrayPullUpdates(ITERATOR_INDEX startIndex, ITERATOR_INDEX endIndex, VERTEX_VALUE_TYPE *localGlobalArray, VERTEX_VALUE_TYPE *sharedGlobalArray);

/*
 * Map globalArray distribution in memory
 * @Params:
 * fileName: physical file
 * globalArray: global array pointer to map the file
 * @Return: 0, sucess, 1 error
 * */
void globalArrayDistributionMap(std::string &fileNameOriginal, nvMapFileDescriptor &nvmapFd, ITERATOR_INDEX arraySize,int index){

	std::string  fileName =FSPath +fileNameOriginal+ ".pdist";

	struct stat sb;

	if ((stat (fileName.c_str(), &sb)) == -1){
	//		 createGlobalArray(fileName,arraySize);
	}

	/*open a file*/
	nvmapFd.fd = open(fileName.c_str(),  O_RDWR);

	if (nvmapFd.fd == -1) {
		std::cout <<"\nError opening file for reading, file may not exist:"<<fileName<<"\n";
		exit(EXIT_FAILURE); /*fatal error, we can not continue.*/
	 }
	nvmapFd.lenght=arraySize;

	std::cout <<"\n\t\t "<<fileName<<"\n";

	/*map the file into the memory*/
	global_array_distribution_shared[index]= (FLOAT_AGG_VALUE_TYPE*) pmem_map_file(fileName.c_str(), 0, 0,  0644, NULL, NULL);

	 //std::cout <<"\n==> pointer: "<<(void*)global_array_shared[index]<<"\n\n";

	 if (global_array_distribution_shared[index] == NULL) {
		close(nvmapFd.fd);
		std::cout <<"\nError mmapping the file\n";
		exit(EXIT_FAILURE);
	}


//	return globalArray;
}

/*
 * Map a char array
 * @Params:
 * fileName: physical file
 * globalArray: global array pointer to map the file
 * @Return: 0, sucess, 1 error
 * */
void anyArrayMap(std::string fileNameOriginal, int fd, ITERATOR_INDEX arraySize,int index, ITERATOR_INDEX *mapArray){

	std::string  fileName =FSPath +fileNameOriginal;

	/*open a file*/
	fd = open(fileName.c_str(),  O_RDWR);

	if (fd== -1) {
		std::cout <<"\nError opening file for reading, file may not exist:"<<fileName<<"\n";
		exit(EXIT_FAILURE); /*fatal error, we can not continue.*/
	 }

	/*map the file into the memory*/
	runtimeStats[index]= (ITERATOR_INDEX *) pmem_map_file(fileName.c_str(), 0, 0,  0644, NULL, NULL);

	std::cout <<"\n\n@@@ stats[0]:"<< runtimeStats[index][0];
	std::cout <<"\n      stats[1]:"<<  runtimeStats[index][1];
	std::cout <<"\n      stats[2]:"<<  runtimeStats[index][2];

	 if (runtimeStats[index] == NULL) {
		close(fd);
		std::cout <<"\nError mmapping the file:"<<fileName<<"\n";
		exit(EXIT_FAILURE);
	}

}


/*
 * Push local copies to a shared copy
 * assumes that the shared pointer is map file as MAP_SHARED
 *@params:
 *	startIndex: start position in the array
 *	endIndex:   end position in the array
 *	localGlobalArray: source array
 *	sharedGlobalArray: target array
 *@return :
 * 	 0: success
 *	 -l failture/error
 */
int globalArrayPushUpdates(ITERATOR_INDEX startIndex, ITERATOR_INDEX endIndex, VERTEX_VALUE_TYPE *localGlobalArray, VERTEX_VALUE_TYPE *sharedGlobalArray){


  //	std::cout <<"\n==## push from \t" <<startIndex <<" to\t"<< endIndex;
	 std::memcpy(
	          reinterpret_cast<char*>(sharedGlobalArray),
	          reinterpret_cast<char*>(localGlobalArray) + startIndex,
			  (endIndex-startIndex));

	//std::cout <<"\n using MSYNC Ok ..";
	 pmem_persist(reinterpret_cast<char*>(sharedGlobalArray), (endIndex-startIndex));
	return 0;
}

/*
 * Pull shared copies to a local copy
 * assumes that the shared pointer is map file as MAP_SHARED
 *@params:
 *	startIndex: start position in the array
 *	endIndex:   end position in the array
 *	localGlobalArray: source array
 *	sharedGlobalArray: target array
 *@return :
 * 	 0: success
 *	 -l failture/error
 */
int globalArrayPullUpdates(ITERATOR_INDEX startIndex, ITERATOR_INDEX endIndex, VERTEX_VALUE_TYPE *localGlobalArray, VERTEX_VALUE_TYPE *sharedGlobalArray){

	memcpy(reinterpret_cast<char*>(localGlobalArray)+startIndex,
			reinterpret_cast<char*>(sharedGlobalArray),(endIndex-startIndex));

	//std::cout <<"\n pulling data from shared array ..";
	if (pmem_msync(reinterpret_cast<char*>(sharedGlobalArray), (endIndex-startIndex))==-1){
		std::cout<< "\nError sync file\n";
		return -1;
	}

	return 0;
}




/**
 * Unmap global array
 * @PARAMS:
 * NVMapIndex descriptor
 * @RETURNS:
 * 0, sucess or 1 error
 */
int globalArrayUnMap(VERTEX_VALUE_TYPE *globalArray, nvMapFileDescriptor &nvmapFd){

	#ifdef ONDEBUG
	for (int i =0; i < 5; i ++){
		globalArray[i]=i;
			std::cout <<globalArray[i];
	}
	#endif

	std::cout <<"\n**Unmapping file";
	//pmem_unmap(globalArray,nvmapFd.fd);
	return 0;
}

#endif /*GLOBALSTATES_H*/
