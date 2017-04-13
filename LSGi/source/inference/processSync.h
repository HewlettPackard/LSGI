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

/**
 * wrapper to manage process communications
 * This methods will used to synchronization process
 * using barriers in nvram as files.
 *
 * The barriers are implemented using spinning method
 * until n process satisfy the counter condition.
 *
 *
 * tere, fei, khrishna
 *  Oct 17, 2015
 *
 * **/

#ifndef PROCESSSYNC_H
#define PROCESSSYNC_H

#include<iostream>
#include<libpmem.h>
#include<fstream>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>

#define MASTERNODE 0

/*Global barrier values*/
extern nvMapFileDescriptor nvmapFdBarrier;
extern VERTEX_VALUE_TYPE * sharedGlobalCouters;

/***********************************************
 * Process Synchronization  headers
 ************************************************/

/**global barrier headers*/
int createGlobalBarrier(std::string fileNameOriginal, int arraySize);
/**init global barriers  */
void initGlobalBarrier(int nProcesses, std::string &originalFileName);

/** add a map for the barrier in FAM*/
void globalBarrieryMap(std::string &fileNameOriginal, nvMapFileDescriptor &nvmapFd, int arraySize);

/**add a barrier or wait for n process*/
void globalBarrier(int nProcess);
/**add a barrier or wait for process i of n process*/
void globalBarrier(int iProcess, int nProcesses);
/**add a barrier or wait for counter value for process i of n process*/
void globalBarrier(int iProcess, int nProcesses, int counter);

/**destroy barrier*/
void destroyGlobalBarrier(int nProcess);
/**clean a barrier*/
void clearBarrier(int nProcesses);

void getFilename(std::string path, std::string &fileName );


#endif /*PROCESSSYNC_H*/
