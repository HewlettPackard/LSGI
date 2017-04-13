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

#include<iostream>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <fstream>
#include <libpmem.h>
/*
 * wrapper to manage process communications
 * This methods will used to syncroniza process
 * using barries in nvram as files.
 *
 *
 * tere, fei, khrishna
 *  Oct 17, 2015
 *
 * **/
#ifndef PROCESSSYNC_H
#define PROCESSSYNC_H

/*global types*/
#define VERTEX_VALUE_TYPE char
#define ITERATOR_INDEX int64_t

#define MASTERNODE 0
extern int processId;
extern int totalProcess;
/*global barrier headers*/
void initGlobalBarrier(int nProcesses, std::string &originalFileName);
void mapGlobalBarrier(int nProcesses, std::string &originalFileName);
void globalBarrier(int nProcess);
void getGlobalBarrier(int iProcess, int nProcesses, int counter);
void destroyGlobalBarrier(int nProcess);
void clearBarrier(int nProcesses);

extern std::string FSPath;

struct nvMapFileDescriptor{

	int fd;
	int lenght;
};

/*Global barrier values*/
nvMapFileDescriptor nvmapFdBarrier;
VERTEX_VALUE_TYPE * sharedGlobalCouters=NULL;




/**
 * Create global array file using a local arrray init with zeros.
 * @params:
 * 	filename:
 * 	global Array size
 * @return: 0. ok, 1 error
 */
int createGlobalArray(std::string fileNameOriginal, int arraySize){

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

//	std::cout <<"writing after the truncate";
//        write (fdout, localArray, arraySize);


	std::cout<<"\n\nEnd Writing..";
	if (localArray != NULL){
		free(localArray);
		localArray = NULL;
	}
	close(fdout);
	return 0;
}


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


/*Map barrier counters for process syncronizations*/
void globalBarrieryMap(std::string &fileNameOriginal, nvMapFileDescriptor &nvmapFd, int arraySize){

	struct stat sb;
	long pageSize = sysconf(_SC_PAGESIZE);

	//if ((stat (fileName.c_str(), &sb)) == -1)
	//	 createGlobalArray(fileNameOriginal,arraySize);

	std::string name;
        getFilename(fileNameOriginal,name);

	std::string  fileName = FSPath+name+ ".states";
	arraySize = arraySize*pageSize;

	if ((stat (fileName.c_str(), &sb)) == -1){

		if (processId == MASTERNODE){
			createGlobalArray(name,arraySize);
		}
		else{
			std::cout <<"\n barrier file does not exists...";
			while ((stat (fileName.c_str(), &sb)) == -1 && (sb.st_size>0));
		}
	}
	/*open a file*/
	nvmapFd.fd = open(fileName.c_str(),  O_RDWR);

	if (nvmapFd.fd == -1) {
		std::cout <<"\nError opening file for reading, file may not exist:"<<fileName;
		exit(EXIT_FAILURE); /*fatal error, we can not continue.*/
	 }
	nvmapFd.lenght=arraySize;

	std::cout <<"\n==> mapping global states: "<<fileName<<"\n\n";

	/*map the file into the memory*/
	sharedGlobalCouters= (VERTEX_VALUE_TYPE*) pmem_map(nvmapFd.fd);

	 if (sharedGlobalCouters == MAP_FAILED) {
		close(nvmapFd.fd);
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

/*global barriers*/
void mapGlobalBarrier(int nProcesses, std::string &originalFileName){


	char barrierFile[1000];
	sprintf(barrierFile,"%s.%d.barrier",originalFileName.c_str(),totalProcess);
	std::string fileName(barrierFile);
	std::cout << "\ninit barrier "<< "0"<<"/"<<nProcesses <<"\t FILENAME:"<<fileName;


	globalBarrieryMap(fileName,nvmapFdBarrier, nProcesses);

	std::cout <<"\ninit barrier done";
}

/*global barriers*/
void initGlobalBarrier(int nProcesses, std::string &originalFileName){

	long sz = sysconf(_SC_PAGESIZE);

	char barrierFile[1000];
	sprintf(barrierFile,"%s.%d.barrier",originalFileName.c_str(),totalProcess);
	std::string fileName(barrierFile);
	std::cout << "\ninit barrier "<< "0"<<"/"<<nProcesses <<"\t FILENAME:"<<fileName;


	globalBarrieryMap(fileName,nvmapFdBarrier, nProcesses);	
	sharedGlobalCouters[processId*sz]=0;

	/*persist only step flow*/
    pmem_persist(reinterpret_cast<char*>((sharedGlobalCouters+(processId*sz))),(1));

	std::cout <<"\ninit barrier done";
}

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
	}
	std::cout <<"\n total COMPLETED: "<<sum;
	std::cout <<"\n load barrier for process "<<iProcess <<"\t ... END waiting.. ";
}

/**/
void globalBarrier(int iProcess, int nProcesses, int counter){

	int sum = 0;
	char v  = counter;
	long sz = sysconf(_SC_PAGESIZE);

	/*set to flag=1 , arrive..*/
	sharedGlobalCouters[iProcess*sz] = v;
	
	pmem_persist(reinterpret_cast<char*>((sharedGlobalCouters+(processId*sz))), (sz));

	//sync to file
	std::cout <<"\n load barrier for process "<<iProcess <<"\t ... START waiting... ";
	//wait for all to arrive
	while (sum<nProcesses*counter){
		sum =0;

	pmem_invalidate(reinterpret_cast<char*>(sharedGlobalCouters),(nProcesses*sz));
		for (int i =0; i <nProcesses; i++){
			sum += (int)sharedGlobalCouters[i*sz];
		}
	std::cout << "\nsum="<<sum;
	}
	std::cout <<"\n total COMPLETED: "<<sum;
	std::cout <<"\n load barrier for process "<<iProcess <<"\t ... END waiting.. ";
}

/**/
void getGlobalBarrier(int iProcess, int nProcesses, int counter){

	int sum = 0;
	long sz = sysconf(_SC_PAGESIZE);

	/*set to flag=1 , arrive..*/

	//sync to file
	std::cout <<"\n load barrier for process "<<iProcess <<"\t ... START waiting... ";
	//wait for all to arrive
	while (sum<nProcesses*counter){
		sum =0;

	pmem_invalidate(reinterpret_cast<char*>(sharedGlobalCouters),(nProcesses*sz));
		for (int i =0; i <nProcesses; i++){
			sum += (int)sharedGlobalCouters[i*sz];
		}

	std::cout << "\nprocess sum="<<sum;
	usleep(120*1000);
	}
	std::cout <<"\n total COMPLETED: "<<sum;
	std::cout <<"\n load barrier for process "<<iProcess <<"\t ... END waiting.. ";
}
/**/
void destroyGlobalBarrier(int nProcess){
	 long sz = sysconf(_SC_PAGESIZE);
	memset(sharedGlobalCouters,0,nProcess*sz);


//	globalArrayUnMap(sharedGlobalCouters,nvmapFdBarrier);
}



#endif /*PROCESSSYNC_H*/
