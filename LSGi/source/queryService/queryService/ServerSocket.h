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
#include "search.h"

/*
 * using example of sockets from:
 * http://www.linuxhowtos.org/data/6/server2.c
 *
 * */
#ifndef SERVERSOCKET_H_
#define SERVERSOCKET_H_

#define ITERATOR_INDEX int64_t

/*Global vars*/
bool  runningFlag= true;

/*
 * Headers
 * */
/*start communication broker*/
int StartServer(int portNo);
int  doQuery( int socketFd);
void * doQuery(void* fd);
/*start server*/
int StartServer(int portNo){

	
	std::cout <<"\n Query Server starting...";
	std::cout <<"\n----------------------------------";
	std::cout <<"\n Connected on port [ " <<portNo<<" ]";
	 int sockfd;
	 int newsockfd;
	 int portno=portNo;
	
	 socklen_t clilen;
	pthread_t childThread;

	 struct sockaddr_in serv_addr;
	 struct sockaddr_in cli_addr;


	    sockfd = socket(AF_INET, SOCK_STREAM, 0);
	     if (sockfd < 0)
	        error("ERROR opening socket");

	int enable=1;
	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0)
		 error("setsockopt(SO_REUSEADDR) failed");

	     bzero((char *) &serv_addr, sizeof(serv_addr));

	     serv_addr.sin_family = AF_INET;
	     serv_addr.sin_addr.s_addr = INADDR_ANY;
	     serv_addr.sin_port = htons(portno);

	     std::cout<<"\n Query Server Ready!...";
	    
	     if (bind(sockfd, (struct sockaddr *) &serv_addr,
	              sizeof(serv_addr)) < 0)
	              error("ERROR on binding");

	     std::cout<<"\n Waiting New query request: [  ?  ] \n";
	     listen(sockfd,10);
	     clilen = sizeof(cli_addr);
	     while (runningFlag) {
	         newsockfd = accept(sockfd,
	         (struct sockaddr *) &cli_addr, &clilen);


	         if (newsockfd < 0)
	             error("ERROR on accept");
		int *socClient=(int*)malloc(1*sizeof(int));
		*socClient=newsockfd;
		pthread_create(&childThread, 0, doQuery, socClient);
		pthread_detach(childThread);

/*
		//This code fails when runs over /lfs
	         int pid = fork();

	         if (pid < 0)
	             error("ERROR on fork");
	         if (pid == 0)  {
	             close(sockfd);
	             //dostuff(newsockfd);
			int *socClient=(int*)malloc(1*sizeof(int));
                	*socClient=newsockfd;
		        doQuery((void*)socClient);
	              exit(0);
	         }
	         //else close(newsockfd);
		*/
	     } /* end of while */

	     close(sockfd);
	     return 0; /* we never get here */

}
/*
 * Execute the main logic for the client query
 * based on the type of request
 *
 * */
void*  doQuery( void* arg ){

	int socketFd=*(int*)arg;
	/*read query*/
	queryRequest request;
		bzero(&request,sizeof(request));
		int n;

		n = read(socketFd,&request,sizeof(request));
		if (n < 0)
			error("ERROR reading from socket");

	std::cout<<"\n Query Request Type:"<<request.type;
	switch(request.type){
		case 1:
			doQueryByVid(socketFd, request.value);
			break;

		case 2:
			std::cout <<"\n -> return state for vId";
			break;

		case 3:
			doQueryThreshold(socketFd, request.value);
			std::cout <<"\n -> return state for vId";
			break;

		case 4: //stop the queryService
			runningFlag =false;
			std::cout <<"\n -> to exit the query service.";
			break;

		 case 5: //only stasts
                        doQueryThresholdStats(socketFd, request.value);
                        std::cout <<"\n -> return statistics for vId";
                        break;


		default:
			std::cout <<"\n invalid query type";
			//return 1; /**/


		}
//	return 1;

	close(socketFd);
	pthread_exit(0);
}




#endif /*SERVERSOCKET_H_*/
