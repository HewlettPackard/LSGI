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
#include "inMemoryGraph.h"
#include "inferenceConfig.h"
#include "processSync.h"

/**var initialization*/
nvMapFileDescriptor nvmapFdBarrier;
VERTEX_VALUE_TYPE * sharedGlobalCouters=NULL;

/*****************************************************************
 * Process Synchronizatioon Methods
 *
 *****************************************************************/


/*
*util function to parse the path and get only file name
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

/**
 * Create global barrier  file using a local arrray init with zeros.
 * the file is created with pagesize alignment
 * @params:
 * 	filename:
 * 	global Array size
 * @return: 0. ok, 1 error
 */
int createGlobalBarrier(std::string fileNameOriginal, int arraySize){

	std::string  fileName =FSPath +fileNameOriginal+ ".states";
	std::ofstream outFile;

	std::cout <<"\n creating global barrier:"<<fileName<<" \t with size:"<<arraySize<<"\n";

	int fdout;
        if ((fdout = open (fileName.c_str(), O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IRWXO )) < 0)
            printf ("can't create %s for writing\n",fileName.c_str());

        if (ftruncate(fdout, arraySize)) {
                printf ("ftruncate error\n");
        }
	std::cout <<"End creating global barrier";
	close(fdout);
	return 0;
}

/*Map barrier counters for process syncronizations
* if the file does not exist, it will create the file to map
*/
void globalBarrieryMap(std::string &fileNameOriginal, nvMapFileDescriptor &nvmapFd, int arraySize){

	struct stat sb;
	long pageSize = sysconf(_SC_PAGESIZE);

	std::string name;
        getFilename(fileNameOriginal,name);

	std::string  fileName = FSPath+name+ ".states";
	arraySize = arraySize*pageSize;

	if ((stat (fileName.c_str(), &sb)) == -1){

		if (processId == MASTERNODE){
			createGlobalBarrier(name,arraySize);
		}
		else{
			while ((stat (fileName.c_str(), &sb)) == -1  || (sb.st_size<arraySize)){
				usleep(2000000);
			}
		}
	}
	/*open a file*/
	nvmapFd.fd = open(fileName.c_str(),  O_RDWR);

	if (nvmapFd.fd == -1) {
		std::cout <<"\nError opening file for reading, file may not exist:"<<fileName;
		exit(EXIT_FAILURE); /*fatal error, we can not continue.*/
	 }
  close(nvmapFd.fd);
	nvmapFd.lenght=arraySize;

	std::cout <<"\n==> mapping global states: "<<fileName<<"\n\n";

	/*map the file into the memory*/
	sharedGlobalCouters= (VERTEX_VALUE_TYPE*) pmem_map_file(fileName.c_str(), 0, 0,  0644, NULL, NULL);

	 if (sharedGlobalCouters == MAP_FAILED) {
		std::cout <<"\nError mmapping the file\n";
		exit(EXIT_FAILURE);
	}

	std::cout <<"end";
	#ifdef ONDEBUG
	std::cout <<"\nArray content is:\n";
	for (int i =0; i < arraySize; i ++)
		std::cout <<sharedGlobalCouters[i];
	#endif
}

/*global barriers
* init the barrier for the current process,
  if the file exists , it only maps and updates the  counter
  if not, it will create the barrier, then map and update
*/
void initGlobalBarrier(int nProcesses, std::string &originalFileName){

	long sz = sysconf(_SC_PAGESIZE);

	char barrierFile[1000];
	sprintf(barrierFile,"%s.%d.barrier",originalFileName.c_str(),totalProcess);
	std::string fileName(barrierFile);
	std::cout << "\ninit barrier "<< "0"<<"/"<<nProcesses <<"\t FILENAME:"<<fileName;

	/*map file for the barrier*/
	globalBarrieryMap(fileName,nvmapFdBarrier, nProcesses);
	sharedGlobalCouters[processId*sz]=0;
	pmem_persist(reinterpret_cast<char*>((sharedGlobalCouters+(processId*sz))),(1));

	std::cout <<"\ninit barrier done";
}
/*clean the counter to 0 only for the process that has finised*/
void clearBarrier(int processId){
	long sz = sysconf(_SC_PAGESIZE);

	sharedGlobalCouters[processId*sz]=0;
	pmem_persist(reinterpret_cast<char*>((sharedGlobalCouters+(processId*sz))),(1));
}

/**/
void globalBarrier(int iProcess, int nProcesses){

	int sum = 0;
	char v  = 1;

	/*set to flag=1 , arrive..*/
	sharedGlobalCouters[iProcess] = v;
	//sync to file
	std::cout <<"\n load barrier for process "<<iProcess <<"\t ... START waiting... ";
	while (sum<nProcesses){
		sum =0;
		for (int i =0; i <nProcesses; i++){
			sum += (int)sharedGlobalCouters[i];
		}
		usleep(2000000);
	}
	std::cout <<"\n total COMPLETED: "<<sum;
	std::cout <<"\n load barrier for process "<<iProcess <<"\t ... END waiting.. ";
}

/* execute the barrier to specific counter
* the code will stop until everybody reaches the couter number
*TD: more efficient method of pooling.
*/
void globalBarrier(int iProcess, int nProcesses, int counter){

	int sum = 0;
	char v  = counter;
	long sz = sysconf(_SC_PAGESIZE);

	/*set to flag=1 , arrive..*/
	sharedGlobalCouters[iProcess*sz] = v;

	pmem_persist(reinterpret_cast<char*>((sharedGlobalCouters+(processId*sz))), (1));

	//sync to file
	std::cout <<"\n load barrier for process "<<iProcess <<"\t ... START waiting... ";
	//wait for all to arrive
	while (sum<nProcesses*counter){
		sum =0;

	pmem_invalidate(reinterpret_cast<char*>(sharedGlobalCouters),(nProcesses*sz));
		for (int i =0; i <nProcesses; i++){
			sum += (int)sharedGlobalCouters[i*sz];
		}
	usleep(3000000);
	std::cout << "\nsum="<<sum;
	}
	std::cout <<"\n total COMPLETED: "<<sum;
	std::cout <<"\n load barrier for process "<<iProcess <<"\t ... END waiting.. ";
}

/*
*put the barrier in zeros
*/
void destroyGlobalBarrier(int nProcess){
	 long sz = sysconf(_SC_PAGESIZE);
	memset(sharedGlobalCouters,0,nProcess*sz);

}

