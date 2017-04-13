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

#include "inferenceConfig.h"
#include "inMemoryGraph.h"
#include "globalStates.h"

/**var initialization*/

nvMapFileDescriptor fp[PMAX];

/****************************************************************
 * globalStates  Methods
 *
 *****************************************************************/
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
VERTEX_VALUE_TYPE *global_array_shared[PMAX];

/**Shared Global Probability Distribution for vertex states*/
FLOAT_AGG_VALUE_TYPE *global_array_distribution_shared[PMAX];

/**Shared Runtime Statistics: overall time, iteration and **/
ITERATOR_INDEX *runtimeStats;

/**
 * Global-LOCAL arrays for LOCAL vertex states: */
/* this variables are used to sync global-local and shared copies amongh
 * to other computing nodes via FAM*/

/*vertex state*/
VERTEX_VALUE_TYPE 		*global_array_partitioned = NULL;
/*vertex counters*/
VERTEX_AGG_VALUE_TYPE   *global_array_copy_partitioned = NULL;

FLOAT_AGG_VALUE_TYPE    *global_array_distribution_partitioned =NULL;
/**/
FLOAT_AGG_VALUE_TYPE	*global_array_conv_partitioned = NULL;
/*vertex convergence*/
VERTEX_AGG_VALUE_TYPE 	*global_array_nonConv_partitioned = NULL;


/**
 * Create global array file using a local array initialized with zeros.
 * @params:
 * 	filename:
 * 	global Array size
 * @return: 0. ok, 1 error
 */
int createGlobalArray(std::string fileNameOriginal, ITERATOR_INDEX arraySize){

	std::string  fileName =FSPath +fileNameOriginal+ ".states";

	std::ofstream outFile;
	VERTEX_VALUE_TYPE *localArray = (VERTEX_VALUE_TYPE*) malloc (arraySize* sizeof (VERTEX_VALUE_TYPE));

	std::cout <<"\n creating global Array file:"<<fileName<<" \t with size:"<<arraySize<<"\n";

	memset(localArray, 0,arraySize* sizeof (VERTEX_VALUE_TYPE));

	int fdout;
        if ((fdout = open (fileName.c_str(), O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IRWXO)) < 0)
            printf ("can't create %s for writing\n",fileName.c_str());

        if (ftruncate(fdout, arraySize)) {
                printf ("ftruncate error\n");
        }

	std::cout <<"writing after the truncate";
//        write (fdout, localArray, arraySize);


	std::cout<<"\n\nEnd Writing..";
	if (localArray != NULL){
		free(localArray);
		localArray = NULL;
	}
	close(fdout);
	return 0;
}


/**
 * Create global array for probability distributions
 * @params:
 * 	filename:
 * 	global Array size
 * @return: 0. ok, 1 error
 */
int createGlobalArray(std::string fileNameOriginal, ITERATOR_INDEX arraySize, char *localArray){

	std::string  fileName = FSPath+fileNameOriginal;
	std::ofstream outFile;

	std::cout <<"\n creating global Array file:"<<fileName<<" \t with size:"<<arraySize<<"\n";
/*	outFile.open(fileName.c_str(),  std::ios::binary| std::ios::out);

	if (outFile.good()){
		outFile.write(localArray,arraySize);
		outFile.close();
	}
*/

	int fdout;
	if ((fdout = open (fileName.c_str(), O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IRWXO)) < 0)
	    printf ("can't create %s for writing\n", fileName.c_str());

	if (ftruncate(fdout, arraySize)) {
	    	printf ("ftruncate error\n");
  	}

	write (fdout, localArray, arraySize);
	close(fdout);
	return 0;
}


/*
 * Map globalArray in memory
 * @Params:
 * fileName: physical file
 * globalArray: global array pointer to map the file
 * @Return: 0, sucess, 1 error
 * */
void globalArrayMap(std::string &fileNameOriginal, nvMapFileDescriptor &nvmapFd, ITERATOR_INDEX arraySize,int index){

	std::string  fileName = FSPath+fileNameOriginal+ ".states";

	struct stat sb;

	if ((stat (fileName.c_str(), &sb)) == -1)
			 createGlobalArray(fileName,arraySize);

	/*open a file*/
	nvmapFd.fd = open(fileName.c_str(),  O_RDWR);

	if (nvmapFd.fd == -1) {
		std::cout <<"\nError opening file for reading, file may not exist:"<<fileName<<"\n";
		exit(EXIT_FAILURE); /*fatal error, we can not continue.*/
	 }
	nvmapFd.lenght=arraySize;

	std::cout <<"\n==> mapping global states: "<<fileName<<"\n\n";

	/*map the file into the memory*/
	global_array_shared[index]= (VERTEX_VALUE_TYPE*) pmem_map_file(fileName.c_str(), 0, 0,  0644, NULL, NULL);

	 //std::cout <<"\n==> pointer: "<<(void*)global_array_shared[index]<<"\n\n";
	 if (global_array_shared[index] == NULL) {
		close(nvmapFd.fd);
		std::cout <<"\nError mmapping the file\n";
		exit(EXIT_FAILURE);
	}

	#ifdef ONDEBUG
	std::cout <<"\nArray content is:\n";
	for (int i =0; i < arraySize; i ++)
		std::cout <<global_array_shared[index];
	#endif
}



/*
 * Map globalArray probability distribution in memory
 * @Params:
 * fileName: physical file
 * globalArray: global array pointer to map the file
 * @Return: 0, sucess, 1 error
 * */
void globalArrayDistributionMap(std::string &fileNameOriginal, nvMapFileDescriptor &nvmapFd, ITERATOR_INDEX arraySize,int index){

	std::string  fileName = FSPath+fileNameOriginal+ ".pdist";

	struct stat sb;

	if ((stat (fileName.c_str(), &sb)) == -1)
			 createGlobalArray(fileName,arraySize);

	/*open a file*/
	nvmapFd.fd = open(fileName.c_str(),  O_RDWR);

	if (nvmapFd.fd == -1) {
		std::cout <<"\nError opening file for reading, file may not exist:"<<fileName<<"\n";
		exit(EXIT_FAILURE); /*fatal error, we can not continue.*/
	 }
	nvmapFd.lenght=arraySize;

	std::cout <<"\n==> mapping global states: "<<fileName<<"\n\n";

	/*map the file into the memory*/
	global_array_distribution_shared[index]= (FLOAT_AGG_VALUE_TYPE*) pmem_map_file(fileName.c_str(), 0, 0,  0644, NULL, NULL);


//	 if (global_array_distribution_shared[index] == MAP_FAILED) {
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
void anyArrayMap(std::string fileNameOriginal, int fd, ITERATOR_INDEX arraySize){

	std::string  fileName =FSPath +fileNameOriginal;
	struct stat sb;

	if ((stat (fileName.c_str(), &sb)) == -1){
	//		 createGlobalArray(fileName,arraySize);
	}

	/*open a file*/
	fd = open(fileName.c_str(),  O_RDWR);

	if (fd== -1) {
		std::cout <<"\nError opening file for reading, file may not exist<<"<<fileName<<"\n";
		exit(EXIT_FAILURE); /*fatal error, we can not continue.*/
	 }
	std::cout <<"\n\t\t "<<fileName<<"\n";

	/*map the file into the memory*/
	runtimeStats= (ITERATOR_INDEX *) pmem_map_file(fileName.c_str(), 0, 0, 0644, NULL, NULL);

	//pmem_persist(runtimeStats,3*sizeof(ITERATOR_INDEX));
	 if (runtimeStats == NULL) {
		close(fd);
		std::cout <<"\nError mmapping the file\n";
		exit(EXIT_FAILURE);
	}
}

/*
 * Push local copies to a shared copy
 * assumes that the shared pointer is map file as MAP_SHARED
 * the shared array will be persistent later
 *@params:
 *	startIndex: start position in the array
 *	endIndex:   end position in the array
 *	localGlobalArray: source array
 *	sharedGlobalArray: target array
 *@return :
 * 	 0: success
 *	 -l failture/error
 */
int globalArrayPushUpdatesToPersistence(ITERATOR_INDEX startIndex, ITERATOR_INDEX endIndex, char* localGlobalArray, char* sharedGlobalArray){


	 std::memcpy(
	          sharedGlobalArray,
	          localGlobalArray + startIndex,
			  (endIndex-startIndex));

	return 0;
}

/**\brief write to FAM
 * Push shared copies to FAM
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
int globalArrayPushUpdatesToNVM(ITERATOR_INDEX startIndex, ITERATOR_INDEX endIndex, VERTEX_VALUE_TYPE *localGlobalArray, VERTEX_VALUE_TYPE *sharedGlobalArray){


	 std::memcpy(
	          reinterpret_cast<char*>(sharedGlobalArray),
	          reinterpret_cast<char*>(localGlobalArray) + startIndex,
			  (endIndex-startIndex));

	//std::cout <<"\n using MSYNC Ok ..";
	 	if (pmem_msync(reinterpret_cast<char*>(sharedGlobalArray), (endIndex-startIndex))==-1){
	 	std::cout<< "\nError sync file";
	 	return -1;
	 }

	return 0;
}

/**\brief read from FAM
 * Pull shared copies to a local copy:
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


	pmem_invalidate(reinterpret_cast<char*>(sharedGlobalArray),(endIndex-startIndex));
	memcpy(reinterpret_cast<char*>(localGlobalArray)+startIndex,
			reinterpret_cast<char*>(sharedGlobalArray),(endIndex-startIndex));

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
//	pmem_unmap(globalArray,nvmapFd.fd);
	return 0;
}


/**********************************************************************************************
 ** Methods to access and initialize arrays
*********************************************************************************************/

/**
 * retrieve vertex pointers based on the vertex id and numa node id
 * */
VERTEX_VALUE_TYPE* get_vertex_global_pointerByVariedSizePartition(TOPOLOGY_VALUE_TYPE vertex_id, int numaNode) {
    	return global_array_partitioned+vertex_id;
}

/*
 * update global array partitioned on the socketNode
 * @PARAMS socketNode as id, var as vertex id, value as new value
 * @RETURN: noted
 */
//inline void update_local_array(int socketNode, ITERATOR_INDEX var, VERTEX_VALUE_TYPE value){

  //     *(global_array_partitioned+ var) = value;
       //  std::cout <<"\nindex:" <<var<<" - value:"<<value << " next value:"<<global_array_partitioned[i][var];

//}

/*
 * update shared global array
 * */
inline void update_global_array(ITERATOR_INDEX var, VERTEX_VALUE_TYPE value, struct topology_graph *tg) {
	*(tg->vertex_array[var].global_value_ptr) = value;
}


/*
 * update shared global array
 * */
inline void update_global_array(ITERATOR_INDEX var, VERTEX_VALUE_TYPE value){

      for(int i=0; i<num_sockets_to_use; i++){
           *(global_array_partitioned + var) = value;
         //  std::cout <<"\nindex:" <<var<<" - value:"<<value << " next value:"<<global_array_partitioned[i][var];
      }
}
/**
 * This method assumes that the global array was already initialized. Just re-initializing to 0 now.
 */
inline void reinitialize_global_arrayByPartition(int num_nodes) {


	memset(global_array_partitioned, 0, num_of_nodes * sizeof(VERTEX_VALUE_TYPE));
	memset(global_array_copy_partitioned, 0, num_of_nodes * sizeof(VERTEX_AGG_VALUE_TYPE));
	for (int k=0; k <num_of_nodes; k++){
		global_array_conv_partitioned[k]=3.0;
	}

}
