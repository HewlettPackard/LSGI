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
// Name        : QueryClient.cpp
// Author      : gomariat
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C, Ansi-style
//============================================================================

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include "clientSocket.h"


#define LOCALHOST

int SERVICEPORT=58000;
/*server to connect*/

void runQueryClient(queryRequest & request){

	std::cout <<"\n -- Getting inference state: connecting to port=" << SERVICEPORT<<"--";
	connectToServer(SERVICEPORT, "localhost",request);
}
/*
 * Main Driver for the query service client
 * This client will connect a client to request
 * the type of query over the inference.
 *
 * Willl have 3 types of request:
 *
 * 1.search state of a given vertex id
 * 2.retrieve the state of all the vertices
 * 3.retrive the state of a vertices that P(v)>=threshold.
 *
 * */
int main(int argc, char* argv[]) {


	if(argc<3){
		std::cout <<"\nIncorrect Usage: <type of query=1 (byVId), =3 (byThreshold3> <value> <port =default 58000)\n";
		std::cout <<"\nExample: ./clientService 1 0 -> for query type 1 ";
		return 0;
	}

	queryRequest request;
	request.type=atoi(argv[1]);
	request.value=atoi(argv[2]);
	if (argc ==4 ){
		SERVICEPORT=atoi(argv[3]);
	}

	/*send query to inference service*/
	runQueryClient(request);

	std::cout <<std::endl;
	return EXIT_SUCCESS;
}
